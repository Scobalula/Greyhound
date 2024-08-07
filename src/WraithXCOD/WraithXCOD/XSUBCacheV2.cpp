#include "stdafx.h"
#include "XSUBCacheV2.h"
#include "Strings.h"
#include "FileSystems.h"
// We need the game files structs
#include "DBGameFiles.h"

// We need the following classes
#include "FileSystems.h"
#include "Strings.h"
#include "Compression.h"
#include "BinaryReader.h"
#include "MemoryReader.h"
#include "Siren.h"

#include "CoDFileHandle.h"

XSUBCacheV2::XSUBCacheV2()
{
    // Default, attempt to load the siren lib
    Siren::Initialize(L"oo2core_8_win64.dll");
}

XSUBCacheV2::~XSUBCacheV2()
{
    // Clean up if need be
    Siren::Shutdown();
}

void XSUBCacheV2::LoadPackageCache(const std::string& BasePath)
{
    // Call Base function first!
    CoDPackageCache::LoadPackageCache(BasePath);

    // Verify we've successfully opened it.
    if (!FileSystem->IsValid())
    {
        FileSystem = nullptr;
    }
    else
    {
        // First pass, catch xsub.
        FileSystem->EnumerateFiles("*.xsub", [this](const std::string& name, const size_t size)
        {
            try
            {
                this->LoadPackage(name);
            }
            catch (...)
            {

            }
        });
    }


    // We've finished loading, set status
    this->SetLoadedState();
}

bool XSUBCacheV2::LoadPackage(const std::string& FilePath)
{
#ifdef _DEBUG
    std::cout << "XSUBCacheV2::LoadPackage(): Parsing " << FilePath << "\n";
#endif

    // Call Base function first
    CoDPackageCache::LoadPackage(FilePath);

    // Add to package files
    auto PackageIndex = (uint32_t)PackageFilePaths.size();

    auto Reader = CoDFileHandle(FileSystem->OpenFile(FilePath, "r"), FileSystem.get());

    // Read the header
    auto Header = Reader.Read<VGXSUBHeader>();

    // Check if we have data (Type 3 is data, others are just metadata and refs)
    if (Header.Type != 3)
        return true;


    // Verify the magic and offset
    if (Header.Magic == 0x4950414b && Header.HashOffset < Reader.Size())
    {
        // Jump to hash offset
        Reader.Seek(Header.HashOffset, SEEK_SET);
        // Read Buffer
        auto Buffer = Reader.Read(Header.HashCount * sizeof(VGXSUBHashEntry));
        // Read the hash data into a buffer
        auto HashData = MemoryReader((int8_t*)Buffer.release(), Header.HashCount * sizeof(VGXSUBHashEntry));

        // Loop and setup entries
        for (int64_t i = 0; i < Header.HashCount; i++)
        {
            // Read it
            auto Entry = HashData.Read<VGXSUBHashEntry>();

            // Prepare a cache entry
            PackageCacheObject NewObject{};
            // Set data
            NewObject.Offset = (Entry.PackedInfo >> 32) << 7;
            NewObject.CompressedSize = (Entry.PackedInfo >> 1) & 0x3FFFFFFF;
            NewObject.UncompressedSize = (Entry.PackedInfo >> 1) & 0xFFFFFFFF;
            NewObject.PackageFileIndex = PackageIndex;

            // Append to database
            CacheObjects.insert(std::make_pair(Entry.Key, NewObject));
        }

        // Append the file path
        PackageFilePaths.push_back(FilePath);

#ifdef _DEBUG
        std::cout << "XSUBCacheV2::LoadPackage(): Added " << FilePath << " to cache.\n";
#endif

        // No issues
        return true;
    }

#ifdef _DEBUG
    std::cout << "XSUBCacheV2::LoadPackage(): Failed to parse package.\n";
#endif

    // Failed
    return false;
}

std::unique_ptr<uint8_t[]> XSUBCacheV2::ExtractPackageObject(uint64_t CacheID, int32_t Size, uint32_t& ResultSize)
{
    // Prepare to extract if found
    if (CacheObjects.find(CacheID) != CacheObjects.end())
    {
        // Take cache data, and extract from the XPAK (Uncompressed size = offset of data segment!)
        auto& CacheInfo = CacheObjects[CacheID];
        // Get the XPAK name
        auto& XPAKFileName = PackageFilePaths[CacheInfo.PackageFileIndex];
        auto Reader = CoDFileHandle(FileSystem->OpenFile(XPAKFileName, "r"), FileSystem.get());


        auto ReadUsage = 0.0;
#if _DEBUG
        // printf("XSUBCache::ExtractPackageObject(): Streaming Object: 0x%llx from CASC File: %s\n", CacheID, XPAKFileName.c_str());
#endif // _DEBUG

        // A buffer for total size
        uint64_t TotalDataSize = 0;

        // A buffer for the data, this will eventually be shipped off, it's 50MB of memory
        auto DataTemporaryBufferSize = Size > 0 ? Size : 0x2400000;
        auto ResultBuffer = std::make_unique<uint8_t[]>(DataTemporaryBufferSize);
        // Block Info
        auto BlockPosition = CacheInfo.Offset;
        auto BlockEnd = CacheInfo.Offset + CacheInfo.CompressedSize;
        VGXSUBBlock Blocks[256];

        // Hop to the beginning offset
        Reader.Seek(BlockPosition + 2, SEEK_SET);

        // Raw block, hacky check, probably will be info
        // within Gfx Mips and other data
        if (Reader.Read<uint64_t>() != CacheID)
        {
            // Hop to the beginning offset
            Reader.Seek(BlockPosition, SEEK_SET);
            // Read
            Reader.Read((uint8_t*)ResultBuffer.get(), 0, CacheInfo.CompressedSize);
            // Set size
            TotalDataSize = CacheInfo.CompressedSize;
            // Done
            return ResultBuffer;
        }

        while ((uint64_t)Reader.Tell() < BlockEnd)
        {
            // Hop to the beginning offset, skip header
            Reader.Seek(BlockPosition + 22, SEEK_SET);

            // Read blocks
            auto BlockCount = (uint32_t)Reader.Read<uint8_t>();
            std::memset(Blocks, 0, sizeof(Blocks));
            Reader.Read((uint8_t*)&Blocks, 0, BlockCount * sizeof(VGXSUBBlock));

            // Loop for block count
            for (uint32_t i = 0; i < BlockCount; i++)
            {
                // Hop to the beginning offset, skip header
                Reader.Seek(BlockPosition + Blocks[i].BlockOffset, SEEK_SET);
                // Read the block
                auto DataBlock = Reader.Read(Blocks[i].CompressedSize);

                switch (Blocks[i].Compression)
                {
                case 0x3:
                    // Decompress the LZ4 block
                    Compression::DecompressLZ4Block((const int8_t*)DataBlock.get(), (int8_t*)ResultBuffer.get() + Blocks[i].DecompressedOffset, Blocks[i].CompressedSize, Blocks[i].DecompressedSize);
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
                    Reader.Seek(Blocks[i].CompressedSize, SEEK_CUR);
                    // Done
                    break;
                }
            }

            // Set next block
            BlockPosition = ((Reader.Tell() + 0x7F) & 0xFFFFFFFFFFFFF80);
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

std::unique_ptr<uint8_t[]> XSUBCacheV2::DecompressPackageObject(uint64_t cacheID, uint8_t* buffer, size_t bufferSize, size_t decompressedSize, size_t& resultSize)
{
    resultSize = 0;

    // We don't accept unknown sizes in xsub, caller must know how much memory is needed.
    if (decompressedSize == 0)
    {
        return nullptr;
    }

    // Our final big blob of data to return, this will be the entire decompressed buffer.
    auto result = std::make_unique<uint8_t[]>(decompressedSize);
    auto reader = MemoryReader((int8_t*)buffer, bufferSize, true);

    size_t blockPosition = 0;
    VGXSUBBlock Blocks[256];

    while (reader.GetPosition() < reader.GetLength())
    {
        blockPosition = reader.GetPosition();

        // Verify block magic.
        if (reader.Read<uint16_t>() != 0xF01D)
        {
            resultSize = 0;
            return nullptr;
        }
        // Verify our hash matches.
        if (reader.Read<uint64_t>() != cacheID)
        {
            resultSize = 0;
            return nullptr;
        }

        // Skip unknown values, seem to be offsets/sizes/indices?
        reader.Advance(12);

        auto BlockCount = (size_t)reader.Read<uint8_t>();

        // If we have 0 blocks, we're possibly EOF, either way it shouldn't happen
        // and we shouldn't continue after it has occured.
        if (BlockCount == 0)
        {
            break;
        }

        std::memset(Blocks, 0, sizeof(Blocks));
        reader.Read(BlockCount * sizeof(VGXSUBBlock), (int8_t*)&Blocks);

        // Loop for block count
        for (uint32_t i = 0; i < BlockCount; i++)
        {
            reader.SetPosition(blockPosition + Blocks[i].BlockOffset);
            
            auto dataBlock = reader.GetCurrentStream(Blocks[i].CompressedSize);

            // If we hit EOF on this, we can't verify this stream is valid or something is wrong.
            if (dataBlock == nullptr)
            {
                resultSize = 0;
                return nullptr;
            }

            switch (Blocks[i].Compression)
            {
            case 0x3:
                Compression::DecompressLZ4Block((const int8_t*)dataBlock, (int8_t*)result.get() + Blocks[i].DecompressedOffset, Blocks[i].CompressedSize, Blocks[i].DecompressedSize);
                resultSize += Blocks[i].DecompressedSize;
                break;
            case 0x6:
                Siren::Decompress((const uint8_t*)dataBlock, Blocks[i].CompressedSize, result.get() + Blocks[i].DecompressedOffset, Blocks[i].DecompressedSize);
                resultSize += Blocks[i].DecompressedSize;
                break;
            case 0x0:
                std::memcpy(result.get() + Blocks[i].DecompressedOffset, dataBlock, Blocks[i].CompressedSize);
                resultSize += Blocks[i].DecompressedSize;
                break;
            default:
                reader.Advance(Blocks[i].CompressedSize);
                break;
            }
        }

        reader.SetPosition((reader.GetPosition() + 0x7F) & 0xFFFFFFFFFFFFF80);
    }

    return result;
}