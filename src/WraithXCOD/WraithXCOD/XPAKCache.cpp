#include "stdafx.h"

// The class we are implementing
#include "XPAKCache.h"

// We need the following classes
#include "FileSystems.h"
// #include "Strings.h"
#include "Compression.h"
// #include "BinaryReader.h"
#include "MemoryReader.h"
#include "Siren.h"

// We need the game files structs
#include "DBGameFiles.h"

// We need the file classes
#include "CoDAssets.h"

// We need the file system classes.
#include "CoDFileHandle.h"

XPAKCache::XPAKCache()
{
    // Default, attempt to load the siren lib
    Siren::Initialize(L"oo2core_6_win64.dll");
}

XPAKCache::~XPAKCache()
{
    // Clean up if need be
    Siren::Shutdown();
}

void XPAKCache::LoadPackageCache(const std::string& BasePath)
{
    // Call Base function first!
    CoDPackageCache::LoadPackageCache(BasePath);

    // Grab files
    FileSystem->EnumerateFiles("*.xpak", [this](const std::string& name, const size_t size)
    {
        this->LoadPackage(name);
    });

    // We've finished loading, set status
    this->SetLoadedState();
}

bool XPAKCache::LoadPackage(const std::string& FilePath)
{
    // Call Base function first
    CoDPackageCache::LoadPackage(FilePath);

    // Add to package files
    auto PackageIndex = (uint32_t)PackageFilePaths.size();

    // Open the file
    auto Reader = CoDFileHandle(FileSystem->OpenFile(FilePath, "r"), FileSystem.get());

    // Read the header
    auto Header = Reader.Read<BO3XPakHeader>();

    // If MW4 we need to skip the new bytes
    if (Header.Version == 0xD)
    {
        Reader.Seek(0, SEEK_SET);
        Reader.Read((uint8_t*)&Header, 0, 24);
        Reader.Seek(288, SEEK_CUR);
        Reader.Read((uint8_t*)&Header + 24, 0, 96);
    }

    // Verify the magic and offset
    if (Header.Magic == 0x4950414b && Header.HashOffset < Reader.Size())
    {
        // Jump to hash offset
        Reader.Seek(Header.HashOffset, SEEK_SET);

        // Loop and setup entries
        for (uint64_t i = 0; i < Header.HashCount; i++)
        {
            // Read it
            auto Entry = Reader.Read<BO3XPakHashEntry>();

            // Prepare a cache entry
            PackageCacheObject NewObject{};
            // Set data
            NewObject.Offset = Header.DataOffset + Entry.Offset;
            NewObject.CompressedSize = Entry.Size & 0xFFFFFFFFFFFFFF; // 0x80 in last 8 bits in some entries in new XPAKs
            NewObject.UncompressedSize = 0;
            NewObject.PackageFileIndex = PackageIndex;
            // Append to database
            CacheObjects.insert(std::make_pair(Entry.Key, NewObject));
        }

        // Append the file path
        PackageFilePaths.push_back(FilePath);

        // No issues
        return true;
    }

    // Failed 
    return false;
}

std::unique_ptr<uint8_t[]> XPAKCache::ExtractPackageObject(uint64_t CacheID, int32_t Size, uint32_t& ResultSize)
{
    // Prepare to extract if found
    if (CacheObjects.find(CacheID) != CacheObjects.end())
    {
        // Take cache data, and extract from the XPAK (Uncompressed size = offset of data segment!)
        const auto& CacheInfo = CacheObjects[CacheID];
        // Get the XPAK name
        const auto& XPAKFileName = PackageFilePaths[CacheInfo.PackageFileIndex];
        // Open File
        auto Reader = CoDFileHandle(FileSystem->OpenFile(XPAKFileName, "r"), FileSystem.get());

        if (!Reader.IsValid())
            return nullptr;

#if _DEBUG
        printf("XPAKCache::ExtractPackageObject(): Streaming Object: 0x%llx from File: %s\n", CacheID, XPAKFileName.c_str());
#endif // _DEBUG

        // Hop to the beginning offset
        Reader.Seek(CacheInfo.Offset, SEEK_SET);

        // Decompressed Size
        const uint64_t DecompressedSize = Size == -1 ? CacheInfo.UncompressedSize : Size;
        // Output Size
        size_t ResultSizeSizeT = 0;

        const auto payload = Reader.Read(CacheInfo.CompressedSize);
        auto outputBuffer = DecompressPackageObject(CacheID, payload.get(), CacheInfo.CompressedSize, DecompressedSize, ResultSizeSizeT);

        // TODO: Switch to size_t in package class
        ResultSize = (uint32_t)ResultSizeSizeT;
        // Return the safe buffer
        return outputBuffer;
    }

    // Set
    ResultSize = 0;

    // Failed to find data
    return nullptr;
}

std::unique_ptr<uint8_t[]> XPAKCache::DecompressPackageObject(uint64_t cacheID, uint8_t* buffer, size_t bufferSize, size_t decompressedSize, size_t& resultSize)
{
    resultSize = 0;

    // We don't accept unknown sizes in xsub, caller must know how much memory is needed.
    if (decompressedSize == 0)
        return nullptr;

    // Our final big blob of data to return, this will be the entire decompressed buffer.
    auto result = std::make_unique<uint8_t[]>(decompressedSize);
    auto reader = MemoryReader((int8_t*)buffer, bufferSize, true);
    auto remaining = decompressedSize;

    while (reader.GetPosition() < reader.GetLength())
    {
        // Read the block header
        const auto blockHeader = reader.Read<BO3XPakDataHeader>();

        // Loop for block count
        for (uint32_t i = 0; i < blockHeader.Count; i++)
        {
            // Unpack the command information
            const size_t blockSize = (blockHeader.Commands[i] & 0xFFFFFF);
            const size_t flag = (blockHeader.Commands[i] >> 24);

            // The block decompressed size passed to the decompression method
            size_t blockDecompressedSize = 0;

            // Get the current stream, avoids constant allocations as we know we have a memory pointer.
            const auto dataBlock = reader.GetCurrentStream(blockSize);

            // If we hit EOF on this, we can't verify this stream is valid or something is wrong.
            if (dataBlock == nullptr)
            {
                resultSize = 0;
                return nullptr;
            }

            // TODO: Need to be migrate more comments from the old code here
            // Check the block type (anything else = skip over!)
            switch (flag)
            {
                case 0x3: // compressed (lz4)
                {
                    blockDecompressedSize = remaining;
                    const size_t decompressedResult = Compression::DecompressLZ4Block(dataBlock, (int8_t*)result.get() + resultSize, blockSize, (int32_t)(blockDecompressedSize));
                    resultSize += decompressedResult;
                    remaining -= decompressedResult;
                    break;
                }
                case 0x6:
                {
                    blockDecompressedSize = std::min<size_t>(remaining, 262112);
                    Siren::Decompress((const uint8_t*)dataBlock, blockSize, result.get() + resultSize, blockDecompressedSize);
                    resultSize += blockDecompressedSize;
                    remaining -= blockDecompressedSize;
                    break;
                }
                case 0x8: // compressed (oodle)
                {
                    blockDecompressedSize = *(uint32_t*)(dataBlock);
                    Siren::Decompress((const uint8_t*)(dataBlock + 4), blockSize - 4, result.get() + resultSize, blockDecompressedSize);
                    resultSize += blockDecompressedSize;
                    remaining -= blockDecompressedSize;
                    break;
                }
                case 0x0: // raw data
                {
                    std::memcpy(result.get() + resultSize, dataBlock, blockSize);
                    resultSize += blockSize;
                    remaining -= blockSize;
                    break;
                }
                default:
                {
                    break;
                }
            }

            // TODO: Don't like depending on game for this, maybe use XPAK version and pass in flag?
            if (CoDAssets::GameID == SupportedGames::ModernWarfare4)
                reader.Advance(((blockSize + 3) & 0xFFFFFFFC) - blockSize);
        }

        reader.SetPosition((reader.GetPosition() + 0x7F) & 0xFFFFFFFFFFFFF80);
    }

    return result;
}
