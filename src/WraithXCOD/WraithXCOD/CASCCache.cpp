#include "stdafx.h"
#include "CASCCache.h"
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

#include "CASCFileReader.h"

std::mutex LockA;
std::mutex LockB;

std::map<std::string, CASC_FIND_DATA> GetFiles(HANDLE StorageHandle)
{
    // Data
    HANDLE FoundHandle;
    CASC_FIND_DATA CASCFindData;
    std::map<std::string, CASC_FIND_DATA> CASCFiles;
    FoundHandle = CascFindFirstFile(StorageHandle, "*", &CASCFindData, NULL);

    if (FoundHandle != NULL)
    {
        do
        {
            // Check if this is a local file
            if (CASCFindData.bFileAvailable == 1)
            {
                CASCFiles[CASCFindData.szFileName] = CASCFindData;
            }
        } while (CascFindNextFile(FoundHandle, &CASCFindData));
    }

    CascFindClose(FoundHandle);
    return CASCFiles;

}

CASCCache::CASCCache() : StorageHandle(nullptr)
{
    // Default, attempt to load the siren lib
    Siren::Initialize(L"oo2core_6_win64.dll");
}

CASCCache::~CASCCache()
{
    // Clean up if need be
    Siren::Shutdown();
    // Close Handle
    CascCloseStorage(StorageHandle);
}

void CASCCache::LoadPackageCache(const std::string& BasePath)
{
    // Call Base function first!
    CoDPackageCache::LoadPackageCache(BasePath);

    if (!CascOpenStorage(Strings::ToUnicodeString(BasePath).c_str(), NULL, &StorageHandle))
    {
#if _DEBUG
        std::cout << "Failed to open CASC Storage\n";
#endif
        return;
    }

    // Find Files in CASC
    auto Files = GetFiles(StorageHandle);

    // Iterate over file paths
    for (auto& File : Files)
    {
        // We only want XPAKs
        if (FileSystems::GetExtension(File.first) == ".xpak")
        {
            this->LoadPackage(File.first);
        }
    }

    // We've finished loading, set status
    this->SetLoadedState();
}

bool CASCCache::LoadPackage(const std::string& FilePath)
{
#ifdef _DEBUG
    std::cout << "CASCCache::LoadPackage(): Parsing " << FilePath << "\n";
#endif

    // Call Base function first
    CoDPackageCache::LoadPackage(FilePath);

    // Add to package files
    auto PackageIndex = (uint32_t)PackageFilePaths.size();
    
    // Open CASC File
    auto Reader = CASCFileReader(StorageHandle, FilePath);

    // Read the header
    auto Header = Reader.Read<BO3XPakHeader>();

    // If MW4 we need to skip the new bytes
    if (Header.Version == 0xD)
    {
        Reader.SetPosition(0);
        uint64_t Result;
        Reader.Read((uint8_t*)&Header, 24, Result);
        Reader.Advance(288);
        Reader.Read((uint8_t*)&Header + 24, 96, Result);
    }

    // Verify the magic and offset
    if (Header.Magic == 0x4950414b && Header.HashOffset < Reader.GetLength())
    {
        // Jump to hash offset
        Reader.SetPosition(Header.HashOffset);
        // Hash result size
        uint64_t HashResult = 0;
        // Read Buffer
        auto Buffer = Reader.Read(Header.HashCount * sizeof(BO3XPakHashEntry), HashResult);


        // Read the hash data into a buffer
        auto HashData = MemoryReader((int8_t*)Buffer.release(), HashResult);

        // Loop and setup entries
        for (uint64_t i = 0; i < Header.HashCount; i++)
        {
            // Read it
            auto Entry = HashData.Read<BO3XPakHashEntry>();

            // Prepare a cache entry
            PackageCacheObject NewObject;
            // Set data
            NewObject.Offset = Header.DataOffset + Entry.Offset;
            NewObject.CompressedSize = Entry.Size & 0xFFFFFFFFFFFFFF; // 0x80 in last 8 bits in some entries in new XPAKs
            NewObject.UncompressedSize = 0;
            NewObject.PackageFileIndex = PackageIndex;

            // Append to database
            CacheObjects.insert(std::make_pair(Entry.Key, NewObject));
        }

        // For MW we must parse the entries
        if (CoDAssets::GameID == SupportedGames::ModernWarfare4)
        {
            // Jump to index offset
            Reader.SetPosition(Header.IndexOffset);

            // Loop and setup entries
            for (uint64_t i = 0; i < Header.IndexCount; i++)
            {
                // Read hash and properties size
                auto Hash = Reader.Read<uint64_t>();
                auto Properties = Reader.Read<uint64_t>();
                auto Entry = CacheObjects.find(Hash);

                if (Entry == CacheObjects.end())
                {
                    // Skip properties
                    Reader.Advance(Properties);
                }
                else
                {
                    // Result
                    uint64_t ReadSize = 0;
                    // Read buffer and parse
                    auto PropertiesBuffer = Reader.Read(Properties, ReadSize);

                    // Check if valid
                    if (PropertiesBuffer != nullptr)
                    {
                        // Results
                        auto ResultBuffer = Strings::SplitString(std::string((char*)PropertiesBuffer.get(), (char*)PropertiesBuffer.get() + ReadSize), '\n');

                        // Loop and take the ones we need
                        for (auto& KeyValue : ResultBuffer)
                        {
                            auto KeyValuePair = Strings::SplitString(KeyValue, ':');

                            if (KeyValuePair.size() == 2)
                            {
                                if (KeyValuePair[0] == "size0")
                                {
                                    Entry->second.UncompressedSize = std::strtoull(KeyValuePair[1].c_str(), 0, 0);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Append the file path
        PackageFilePaths.push_back(FilePath);

#ifdef _DEBUG
        std::cout << "CASCCache::LoadPackage(): Added " << FilePath << " to cache.\n";
#endif

        // No issues
        return true;
    }

#ifdef _DEBUG
    std::cout << "CASCCache::LoadPackage(): Failed to parse package.\n";
#endif

    // Failed 
    return false;
}

std::unique_ptr<uint8_t[]> CASCCache::ExtractPackageObject(uint64_t CacheID, uint32_t& ResultSize)
{
    // Prepare to extract if found
    if (CacheObjects.find(CacheID) != CacheObjects.end())
    {
        //try
        //{
        //    // Aquire lock
        //    std::lock_guard<std::shared_mutex> Gaurd(ReadMutex);
        //}
        //catch (std::exception & e)
        //{
        //    std::cout << e.what() << "\n";
        //}


        // Take cache data, and extract from the XPAK (Uncompressed size = offset of data segment!)
        auto& CacheInfo = CacheObjects[CacheID];
        // Get the XPAK name
        auto& XPAKFileName = PackageFilePaths[CacheInfo.PackageFileIndex];
        // Open CASC File
        auto Reader = CASCFileReader(StorageHandle, XPAKFileName);

#if _DEBUG
        printf("CASCCache::ExtractPackageObject(): Streaming Object: 0x%llx from CASC File: %s\n", CacheID, XPAKFileName.c_str());
#endif // _DEBUG


        // Hop to the beginning offset
        Reader.SetPosition(CacheInfo.Offset);

        // A buffer for data read
        uint64_t DataRead = 0;
        // A buffer for total size
        uint64_t TotalDataSize = 0;
        // Decompressed Size
        uint64_t DecompressedSize = CacheInfo.UncompressedSize;

        // A buffer for the data, this will eventually be shipped off, it's 36MB of memory
        auto DataTemporaryBuffer = new int8_t[0x2400000];

        // Loop until we have all our data
        while (DataRead < CacheInfo.CompressedSize)
        {
            // Read the block header
            auto BlockHeader = Reader.Read<BO3XPakDataHeader>();

            // Loop for block count
            for (uint32_t i = 0; i < BlockHeader.Count; i++)
            {
                // Unpack the command information
                uint64_t BlockSize = (BlockHeader.Commands[i] & 0xFFFFFF);
                uint32_t CompressedFlag = (BlockHeader.Commands[i] >> 24);

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
                else if (CompressedFlag == 0x8)
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
                    // Pad the Block Size
                    BlockSize = (BlockSize + 3) & 0xFFFFFFFC;

                    // Check if we read data
                    if (DataBlock != nullptr)
                    {
                        // Check if we're at the end of the block/less than the max block size, if so, use that
                        uint64_t RawBlockSize = DecompressedSize < 262112 ? DecompressedSize : 262112;
                        // Subtract from our total size
                        DecompressedSize -= RawBlockSize;

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

                    // Check if we read data
                    if (DataBlock != nullptr)
                    {
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
                if ((i + 1) < BlockHeader.Count)
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

            // We must append the size of a header
            DataRead += sizeof(BO3XPakDataHeader);
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
