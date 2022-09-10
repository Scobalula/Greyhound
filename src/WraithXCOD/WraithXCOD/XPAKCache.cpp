#include "stdafx.h"

// The class we are implementing
#include "XPAKCache.h"

// We need the following classes
#include "FileSystems.h"
#include "Strings.h"
#include "Compression.h"
#include "BinaryReader.h"
#include "MemoryReader.h"
#include "Siren.h"

// We need the game files structs
#include "DBGameFiles.h"

// We need the file classes
#include "CoDAssets.h"

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

    // We need to enumerate all files in this path, load them, and aquire hashes of the names
    auto PathXPAKFiles = FileSystems::GetFiles(BasePath, "*.xpak");

    // Iterate over file paths
    for (auto& XPAKFile : PathXPAKFiles)
    {
        // Some of the smaller blank XPAKs are causing this to return false, since the bigger XPAKs are fine now from what I can tell, just skip the bad ones
        this->LoadPackage(XPAKFile);
    }

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
    auto Reader = BinaryReader();
    // Open it
    Reader.Open(FilePath, true);

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
        // Read the hash data into a buffer
        auto Buffer = Reader.Read(Header.HashCount * sizeof(BO3XPakHashEntry), HashResult);
        auto HashData = MemoryReader(Buffer, HashResult);

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

        // For MW we must parse the entries as they do not store sizes, the game simply subtracts against the size here
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
                        auto ResultBuffer = Strings::SplitString(std::string((char*)PropertiesBuffer, (char*)PropertiesBuffer + ReadSize), '\n');

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


                        // Clean up
                        delete[] PropertiesBuffer;
                    }
                }
            }
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
        auto& CacheInfo = CacheObjects[CacheID];
        // Get the XPAK name
        auto& XPAKFileName = PackageFilePaths[CacheInfo.PackageFileIndex];

        // Open the file
        auto Reader = BinaryReader();
        // Open it
        Reader.Open(XPAKFileName, true);

        // Hop to the beginning offset
        Reader.SetPosition(CacheInfo.Offset);

        // A buffer for data read
        uint64_t DataRead = 0;
        // A buffer for total size
        uint64_t TotalDataSize = 0;
        // Decompressed Size
        uint64_t DecompressedSize = Size == -1 ? CacheInfo.UncompressedSize : Size;

        // A buffer for the data, this will eventually be shipped off, it's 36MB of memory
        // or where possible, use the size from the game, as this is a hefty allocation
        auto ResultBufferSize = Size == -1 ? 0x2400000 : Size;
        auto ResultBuffer = std::make_unique<uint8_t[]>(ResultBufferSize);
        auto TempBuffer = std::make_unique<uint8_t[]>(CacheInfo.CompressedSize); // TODO: Eventually cache XPAK files and do one big read.

        // Loop until we have all our data
        while (DataRead < CacheInfo.CompressedSize)
        {
            // Read the block header
            auto BlockPosition = Reader.GetPosition();
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
                    Reader.Read(TempBuffer.get(), BlockSize, ReadSize);

                    // Check if we read data
                    if (ReadSize == BlockSize)
                    {
                        // Decompress the LZ4 block
                        auto Result = Compression::DecompressLZ4Block((const int8_t*)TempBuffer.get(), (int8_t*)ResultBuffer.get() + TotalDataSize, (uint32_t)BlockSize, Size - TotalDataSize);

                        // Append size
                        TotalDataSize += Result;
                    }
                    else
                    {
                        return nullptr;
                    }
                }
                else if (CompressedFlag == 0x8)
                {
                    // Read result
                    uint64_t ReadSize = 0;
                    // Read the block
                    Reader.Read(TempBuffer.get(), BlockSize, ReadSize);

                    // Check if we read data
                    if (ReadSize == BlockSize)
                    {
                        // Read oodle decompressed size
                        uint32_t DecompressedSize = *(uint32_t*)(TempBuffer.get());

                        // Decompress the Oodle block
                        auto Result = Siren::Decompress((const uint8_t*)TempBuffer.get() + 4, (uint32_t)BlockSize - 4, ResultBuffer.get() + TotalDataSize, DecompressedSize);

                        // Append size
                        TotalDataSize += Result;
                    }
                }
                else if (CompressedFlag == 0x6)
                {
                    // Read result
                    uint64_t ReadSize = 0;
                    // Read the block
                    Reader.Read(TempBuffer.get(), BlockSize, ReadSize);
                    // Check if we're at the end of the block/less than the max block size, if so, use that
                    uint64_t RawBlockSize = std::min<uint64_t>(DecompressedSize, 262112);
                    // Subtract from our total size
                    DecompressedSize -= RawBlockSize;

                    // Check if we read data
                    if (ReadSize == BlockSize)
                    {
                        // Decompress the Oodle block
                        auto Result = Siren::Decompress((const uint8_t*)TempBuffer.get(), (uint32_t)BlockSize, (uint8_t*)ResultBuffer.get() + TotalDataSize, RawBlockSize);
                        // Append size
                        TotalDataSize += RawBlockSize;
                    }
                }
                else if (CompressedFlag == 0x0)
                {
                    // Read result
                    uint64_t ReadSize = 0;
                    // Read the block
                    Reader.Read(TempBuffer.get(), BlockSize, ReadSize);
                    // Subtract from our total size
                    DecompressedSize -= BlockSize;

                    // Check if we read data
                    if (ReadSize == BlockSize)
                    {
                        // We just need to append it
                        std::memcpy(ResultBuffer.get() + TotalDataSize, TempBuffer.get(), BlockSize);

                        // Append size
                        TotalDataSize += BlockSize;
                    }
                }
                else
                {
                    // As far as we care, any other flag value is padding (0xCF is one of them)
                    Reader.Advance(BlockSize);
                }

                // Pad for MW
                if (CoDAssets::GameID == SupportedGames::ModernWarfare4)
                    BlockSize = (BlockSize + 3) & 0xFFFFFFFC;

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