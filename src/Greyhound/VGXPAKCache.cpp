#include "pch.h"
#include "VGXPAKCache.h"

// We need the game files structs
#include "DBGameFiles.h"

// We need the following classes
#include "Compression.h"
#include "MemoryReader.h"
#include "Siren.h"
#include <Directory.h>
#include "WraithBinaryReader.h"

VGXPAKCache::VGXPAKCache()
{
    // Default, attempt to load the siren lib
    Siren::Initialize(L"oo2core_8_win64.dll");
}

VGXPAKCache::~VGXPAKCache()
{
    // Clean up if need be
    Siren::Shutdown();
}

void VGXPAKCache::LoadPackageCache(const std::string& BasePath)
{
    // Call Base function first!
    CoDPackageCache::LoadPackageCache(BasePath);

    try
    {
        // We need to enumerate all files in this path, load them, and aquire hashes of the names
        auto PathXPAKFiles = IO::Directory::GetFiles(BasePath.c_str(), "*.xpak");

        // Iterate over file paths
        for (auto& XPAKFile : PathXPAKFiles)
        {
            // Some of the smaller blank XPAKs are causing this to return false, since the bigger XPAKs are fine now from what I can tell, just skip the bad ones
            this->LoadPackage(XPAKFile.ToCString());
        }
    }
    catch (...)
    {
    }

    // We've finished loading, set status
    this->SetLoadedState();
}

bool VGXPAKCache::LoadPackage(const std::string& FilePath)
{
#ifdef _DEBUG
    std::cout << "VGXPAKCache::LoadPackage(): Parsing " << FilePath << "\n";
#endif

    // Call Base function first
    CoDPackageCache::LoadPackage(FilePath);

    // Add to package files
    auto PackageIndex = (uint32_t)PackageFilePaths.size();

    // Open the file
    auto Reader = BinaryReader();
    // Open it
    Reader.Open(FilePath, true);

    // Read the header
    auto Header = Reader.Read<VGXPAKHeader>();

    // Check if we have data (Type 3 is data, others are just metadata and refs)
    if (Header.Type != 1)
        return true;

    // Verify the magic and offset
    if (Header.Magic == 0x4950414b && Header.HashOffset < Reader.GetLength())
    {
        // Jump to hash offset
        Reader.SetPosition(Header.HashOffset);
        // Hash result size
        uint64_t HashResult = 0;
        // Read Buffer
        auto Buffer = Reader.Read(Header.HashCount * sizeof(VGXPAKHashEntry), HashResult);


        // Read the hash data into a buffer
        IO::MemoryReader HashData((int8_t*)Buffer, (size_t)HashResult);

        // Loop and setup entries
        for (int64_t i = 0; i < Header.HashCount; i++)
        {
            // Read it
            auto Entry = HashData.Read<VGXPAKHashEntry>();

            // Prepare a cache entry
            PackageCacheObject NewObject;
            // Set data
            NewObject.Offset = (Entry.PackedInfo >> 32) << 7;
            NewObject.CompressedSize = (Entry.PackedInfo >> 1) & 0x3FFFFFFF;
            NewObject.UncompressedSize = 0;
            NewObject.PackageFileIndex = PackageIndex;

            // Append to database
            CacheObjects.insert(std::make_pair(Entry.Key, NewObject));
        }

        // Append the file path
        PackageFilePaths.push_back(FilePath + "data");

#ifdef _DEBUG
        std::cout << "VGXPAKCache::LoadPackage(): Added " << FilePath << " to cache.\n";
#endif

        // No issues
        return true;
    }

#ifdef _DEBUG
    std::cout << "VGXPAKCache::LoadPackage(): Failed to parse package.\n";
#endif

    // Failed
    return false;
}


std::unique_ptr<uint8_t[]> VGXPAKCache::ExtractPackageObject(uint64_t CacheID, int32_t Size, uint32_t& ResultSize)
{
    // Prepare to extract if found
    if (CacheObjects.find(CacheID) != CacheObjects.end())
    {
        // Take cache data, and extract from the XPAK (Uncompressed size = offset of data segment!)
        auto& CacheInfo = CacheObjects[CacheID];
        // Get the XPAK name
        auto& XPAKFileName = PackageFilePaths[CacheInfo.PackageFileIndex];
        // Open the file
        auto Reader = BinaryReader();
        // Open it
        Reader.Open(XPAKFileName, true);

#if _DEBUG
        printf("XSUBCache::ExtractPackageObject(): Streaming Object: 0x%llx from CASC File: %s\n", CacheID, XPAKFileName.c_str());
#endif // _DEBUG

        // A buffer for total size
        uint64_t TotalDataSize = 0;

        // A buffer for the data, this will eventually be shipped off, it's 50MB of memory
        auto DataTemporaryBufferSize = Size > 0 ? Size : 0x2400000;
        auto ResultBuffer = std::make_unique<uint8_t[]>(DataTemporaryBufferSize);
        // Block Info
        auto BlockPosition = CacheInfo.Offset;
        auto BlockEnd = CacheInfo.Offset + CacheInfo.CompressedSize;
        VGXPAKBlock Blocks[256];

        // Hop to the beginning offset
        Reader.SetPosition(BlockPosition + 2);

        // Raw block, hacky check, probably will be info
        // within Gfx Mips and other data
        if (Reader.Read<uint64_t>() != CacheID)
        {
            // Hop to the beginning offset
            Reader.SetPosition(BlockPosition);
            // Read
            Reader.Read((uint8_t*)ResultBuffer.get(), 0, CacheInfo.CompressedSize);
            // Set size
            TotalDataSize = CacheInfo.CompressedSize;
            // Done
            return ResultBuffer;
        }

        while ((uint64_t)Reader.GetPosition() < BlockEnd)
        {
            // Hop to the beginning offset, skip header
            Reader.SetPosition(BlockPosition + 22);

            // Read blocks
            auto BlockCount = (uint32_t)Reader.Read<uint8_t>();
            uint64_t Result = 0;
            std::memset(Blocks, 0, sizeof(Blocks));
            Reader.Read((uint8_t*)&Blocks, BlockCount * sizeof(VGXPAKBlock), Result);

            // Loop for block count
            for (uint32_t i = 0; i < BlockCount; i++)
            {
                // Hop to the beginning offset, skip header
                Reader.SetPosition(BlockPosition + Blocks[i].BlockOffset);
                // Read result
                uint64_t ReadSize = 0;
                // Read the block
                auto DataBlock = std::unique_ptr<int8_t[]>(Reader.Read(Blocks[i].CompressedSize, ReadSize));

                switch (Blocks[i].Compression)
                {
                case 0x3:
                    // Decompress the LZ4 block
                    WraithCompression::DecompressLZ4Block((const int8_t*)DataBlock.get(), (int8_t*)ResultBuffer.get() + Blocks[i].DecompressedOffset, Blocks[i].CompressedSize, Blocks[i].DecompressedSize);
                    // Append size
                    TotalDataSize += Blocks[i].DecompressedSize;
                    // Done
                    break;
                case 0x6:
                    // Decompress the Oodle block
                    Siren::Decompress((const uint8_t*)DataBlock.get(), Blocks[i].CompressedSize, ResultBuffer.get() + Blocks[i].DecompressedOffset, Blocks[i].DecompressedSize);
                    // Append size
                    TotalDataSize += Blocks[i].DecompressedSize;
                    // Done
                    break;
                case 0x0:
                    // We just need to append it
                    std::memcpy(ResultBuffer.get() + Blocks[i].DecompressedOffset, DataBlock.get(), Blocks[i].CompressedSize);
                    // Append size
                    TotalDataSize += Blocks[i].DecompressedSize;
                    // Done
                    break;
                default:
                    // As far as we care, any other flag value is padding (0xCF is one of them)
                    Reader.Advance(Blocks[i].CompressedSize);
                    // Done
                    break;
                }
            }

            // Set next block
            BlockPosition = ((Reader.GetPosition() + 0x7F) & 0xFFFFFFFFFFFFF80);
        }

        // Return the safe buffer
        ResultSize = (uint32_t)TotalDataSize;
        return ResultBuffer;
    }

    // Set
    ResultSize = 0;

    // Failed to find data
    return nullptr;
}