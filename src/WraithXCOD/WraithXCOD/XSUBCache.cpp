#include "stdafx.h"
#include "XSUBCache.h"
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

#pragma pack(push, 1)
struct BOCWXSubHeader
{
    uint32_t Magic;
    uint16_t Unknown1;
    uint16_t Version;
    uint64_t Unknown;
    uint64_t Type;
    uint64_t Size;
    uint8_t UnknownHashes[1896];
    int64_t FileCount;
    int64_t DataOffset;
    int64_t DataSize;
    int64_t HashCount;
    int64_t HashOffset;
    int64_t HashSize;
    int64_t Unknown3;
    int64_t UnknownOffset;
    int64_t Unknown4;
    int64_t IndexCount;
    int64_t IndexOffset;
    int64_t IndexSize;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOCWXSubHashEntry
{
    uint64_t Key;
    uint64_t PackedInfo;
};
#pragma pack(pop)

XSUBCache::XSUBCache()
{
    // Default, attempt to load the siren lib
    Siren::Initialize(L"oo2core_8_win64.dll");
}

XSUBCache::~XSUBCache()
{
    // Clean up if need be
    Siren::Shutdown();
    // Close Handle
    Container.Close();
}

void XSUBCache::LoadPackageCache(const std::string& BasePath)
{
    // Call Base function first!
    CoDPackageCache::LoadPackageCache(BasePath);

    try
    {
        // Open Storage
        Container.Open(BasePath);

        for (auto& File : Container.GetFileEntries())
        {
            // We only want XPAKs
            if (FileSystems::GetExtension(File.first) == ".xsub" && File.second.Exists)
            {
                try
                {
                    this->LoadPackage(File.first);
                }
                catch (...)
                {

                }
            }
        }
    }
    catch (...)
    {
    }

    // We've finished loading, set status
    this->SetLoadedState();
}

bool XSUBCache::LoadPackage(const std::string& FilePath)
{
#ifdef _DEBUG
    std::cout << "XSUBCache::LoadPackage(): Parsing " << FilePath << "\n";
#endif

    // Call Base function first
    CoDPackageCache::LoadPackage(FilePath);

    // Add to package files
    auto PackageIndex = (uint32_t)PackageFilePaths.size();

    // Open CASC File
    auto Reader = Container.OpenFile(FilePath);

    // Read the header
    auto Header = Reader.Read<BOCWXSubHeader>();

    // Check if we have data (Type 3 is data, others are just metadata and refs)
    if (Header.Type != 3)
        return true;


    // Verify the magic and offset
    if (Header.Magic == 0x4950414b && Header.HashOffset < Reader.GetLength())
    {
        // Jump to hash offset
        Reader.SetPosition(Header.HashOffset);
        // Hash result size
        uint64_t HashResult = 0;
        // Read Buffer
        auto Buffer = Reader.Read(Header.HashCount * sizeof(BOCWXSubHashEntry), HashResult);
        // Read the hash data into a buffer
        auto HashData = MemoryReader((int8_t*)Buffer.release(), HashResult);

        // Loop and setup entries
        for (int64_t i = 0; i < Header.HashCount; i++)
        {
            // Read it
            auto Entry = HashData.Read<BOCWXSubHashEntry>();

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
        PackageFilePaths.push_back(FilePath);

#ifdef _DEBUG
        std::cout << "XSUBCache::LoadPackage(): Added " << FilePath << " to cache.\n";
#endif

        // No issues
        return true;
    }

#ifdef _DEBUG
    std::cout << "XSUBCache::LoadPackage(): Failed to parse package.\n";
#endif

    // Failed
    return false;
}

std::unique_ptr<uint8_t[]> XSUBCache::ExtractPackageObjectRaw(uint64_t CacheID, uint32_t & ResultSize)
{
    // Prepare to extract if found
    if (CacheObjects.find(CacheID) != CacheObjects.end())
    {
        // Aquire lock
        std::lock_guard<std::shared_mutex> Gaurd(ReadMutex);

        // Take cache data, and extract from the XPAK (Uncompressed size = offset of data segment!)
        auto& CacheInfo = CacheObjects[CacheID];
        // Get the XPAK name
        auto& XPAKFileName = PackageFilePaths[CacheInfo.PackageFileIndex];
        // Open CASC File
        auto Reader = Container.OpenFile(XPAKFileName);
        // A buffer for data read
        uint64_t DataRead = 0;
        // Hop to the beginning offset
        Reader.SetPosition(CacheInfo.Offset);

        // Allocate just the raw size of the buffer, that's all we're returning
        auto ResultBuffer = Reader.Read(CacheInfo.CompressedSize, DataRead);

        // Set result size
        ResultSize = (uint32_t)CacheInfo.CompressedSize;
        // Return the safe buffer
        return ResultBuffer;
    }

    // Set
    ResultSize = 0;
    // Failed to find data
    return nullptr;
}

std::unique_ptr<uint8_t[]> XSUBCache::ExtractPackageObject(uint64_t CacheID, uint32_t& ResultSize)
{
    // Prepare to extract if found
    if (CacheObjects.find(CacheID) != CacheObjects.end())
    {
        // Aquire lock
        std::lock_guard<std::shared_mutex> Gaurd(ReadMutex);

        // Take cache data, and extract from the XPAK (Uncompressed size = offset of data segment!)
        auto& CacheInfo = CacheObjects[CacheID];
        // Get the XPAK name
        auto& XPAKFileName = PackageFilePaths[CacheInfo.PackageFileIndex];
        // Open CASC File
        auto Reader = Container.OpenFile(XPAKFileName);
#if _DEBUG
        printf("XSUBCache::ExtractPackageObject(): Streaming Object: 0x%llx from CASC File: %s\n", CacheID, XPAKFileName.c_str());
#endif // _DEBUG


        // Hop to the beginning offset
        Reader.SetPosition(CacheInfo.Offset);

        // A buffer for data read
        uint64_t DataRead = 0;
        // A buffer for total size
        uint64_t TotalDataSize = 0;
        // Decompressed Size
        uint64_t DecompressedSize = CacheInfo.UncompressedSize;

        // A buffer for the data, this will eventually be shipped off, it's 50MB of memory
        auto DataTemporaryBuffer = new int8_t[0x2400000];
        auto DataTemporaryBufferSize = 0x2400000;

        // Loop until we have all our data
        while (DataRead < CacheInfo.CompressedSize)
        {
            // Read the block header
            auto Count = Reader.Read<uint32_t>();
            auto Offset = Reader.Read<uint32_t>();
            // I think there are uncompressed blocks with no info
            // for now skip them, very rare, need to see how to check for them
            if (Count > 256)
                break;
            // Read Variable Length Commands Buffer
            uint32_t Commands[256];
            Reader.Read((uint8_t*)&Commands, 0, Count <= 30 ? 120 : Count * 4);
#if _DEBUG
            std::cout << "XSUBCache::LoadPackage(): Block Begin " << Reader.GetPosition() << ".\n";
            std::cout << "XSUBCache::LoadPackage(): Block Count " << Count * 4 << ".\n";
#endif

            // Loop for block count
            for (uint32_t i = 0; i < Count; i++)
            {
                // Unpack the command information
                uint64_t BlockSize = (Commands[i] & 0xFFFFFF);
                uint32_t CompressedFlag = (Commands[i] >> 24);

                // Get current position
                uint64_t CurrentPosition = Reader.GetPosition();

                // Check the block type (3 = compressed (lz4), 8 = compressed (oodle), 0 = raw data, anything else = skip over!)
                if (CompressedFlag == 0x3)
                {
                    // Read result
                    uint64_t ReadSize = 0;
                    // Read the block
                    auto DataBlock = Reader.Read(BlockSize, ReadSize);

                    // Check if we read data
                    if (DataBlock != nullptr)
                    {
                        // Decompress the LZ4 block
                        auto Result = Compression::DecompressLZ4Block((const int8_t*)DataBlock.get(), DataTemporaryBuffer + TotalDataSize, (uint32_t)BlockSize, 0x2400000);

                        // Append size
                        TotalDataSize += Result;
                    }
                }
                else if (CompressedFlag == 0x8 || CompressedFlag == 0x9)
                {
                    // Read result
                    uint64_t ReadSize = 0;
                    // Read the block
                    auto DataBlock = Reader.Read(BlockSize, ReadSize);

                    // Check if we read data
                    if (DataBlock != nullptr)
                    {
                        // Read oodle decompressed size
                        uint32_t DecompressedSize = *(uint32_t*)(DataBlock.get());
                        // Realloc if needed
                        if (TotalDataSize + DecompressedSize >= DataTemporaryBufferSize)
                        {
                            DataTemporaryBufferSize += 0x2400000 + DecompressedSize;
                            DataTemporaryBuffer = (int8_t*)std::realloc(DataTemporaryBuffer, DataTemporaryBufferSize);
                        }
                        // Decompress the Oodle block
                        auto Result = Siren::Decompress((const uint8_t*)DataBlock.get() + 4, (uint32_t)BlockSize - 4, (uint8_t*)DataTemporaryBuffer + TotalDataSize, DecompressedSize);
                        // Append size
                        TotalDataSize += Result;
                    }
                }
                else if (CompressedFlag == 0x6)
                {
                    // Read result
                    uint64_t ReadSize = 0;
                    // Read the block
                    auto DataBlock = Reader.Read(BlockSize, ReadSize);
                    // Check if we're at the end of the block/less than the max block size, if so, use that
                    uint64_t RawBlockSize = DecompressedSize < 262112 ? DecompressedSize : 262112;
                    // Subtract from our total size
                    DecompressedSize -= RawBlockSize;

                    // Check if we read data
                    if (DataBlock != nullptr)
                    {
                        // Decompress the Oodle block
                        auto Result = Siren::Decompress((const uint8_t*)DataBlock.get(), (uint32_t)BlockSize, (uint8_t*)DataTemporaryBuffer + TotalDataSize, RawBlockSize);
                        // Append size
                        TotalDataSize += RawBlockSize;
                    }
                }
                else if (CompressedFlag == 0x0)
                {
                    // Read result
                    uint64_t ReadSize = 0;
                    // Read the block
                    auto DataBlock = Reader.Read(BlockSize, ReadSize);
                    // Subtract from our total size
                    DecompressedSize -= BlockSize;

                    // Check if we read data
                    if (DataBlock != nullptr)
                    {
                        // Realloc if needed
                        if (TotalDataSize + BlockSize >= DataTemporaryBufferSize)
                        {
                            DataTemporaryBufferSize += 0x2400000 + BlockSize;
                            DataTemporaryBuffer = (int8_t*)std::realloc(DataTemporaryBuffer, DataTemporaryBufferSize);
                        }
                        // We just need to append it
                        std::memcpy(DataTemporaryBuffer + TotalDataSize, DataBlock.get(), BlockSize);
                        // Append size
                        TotalDataSize += BlockSize;
                    }
                }
                else
                {
                    // As far as we care, any other flag value is padding (0xCF is one of them)
                    Reader.Advance(BlockSize);
                }

                // We must append the block size and pad it properly (If it's the last block)
                uint64_t NextSegmentOffset = 0;
                uint64_t TotalBlockSize = 0;

                // Calculate
                if ((i + 1) < Count)
                {
                    // Not the last, standard
                    TotalBlockSize = BlockSize;
                    NextSegmentOffset = (CurrentPosition + BlockSize);
                }
                else
                {
                    // We must pad this
                    NextSegmentOffset = (((CurrentPosition + BlockSize) + 0x7F) & 0xFFFFFFFFFFFFF80);
                    TotalBlockSize += (NextSegmentOffset - CurrentPosition);
                }

                // Append to data read (Cus it includes padding)
                DataRead += TotalBlockSize;

                // Jump to next segment
                Reader.SetPosition(NextSegmentOffset);
            }

#if _DEBUG
            std::cout << "XSUBCache::LoadPackage(): Block End " << Reader.GetPosition() << ".\n";
            std::cout << "XSUBCache::LoadPackage(): Block Count " << Count * 4 << ".\n";
#endif

            // We must append the size of a header
            DataRead += Count <= 30 ? 128 : 8 + (4 * Count);
        }

        // If we got here, the result size is totaldatasize, we need to allocate a safe buffer, copy, then clean up properly
        auto ResultBuffer = std::make_unique<uint8_t[]>((uint32_t)TotalDataSize);
        // Copy over the buffer
        std::memcpy(ResultBuffer.get(), DataTemporaryBuffer, TotalDataSize);

        // Clean up
        delete[] DataTemporaryBuffer;

        // Set result size
        ResultSize = (uint32_t)TotalDataSize;

        // Return the safe buffer
        return ResultBuffer;
    }

    // Set
    ResultSize = 0;

    // Failed to find data
    return nullptr;
}