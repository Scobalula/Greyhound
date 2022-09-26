#include "stdafx.h"
#include "XSUBCacheV2.h"
#include "Strings.h"
#include "FileSystems.h"
// We need the game files structs
#include "DBGameFiles.h"

// We need the file system classes.
#include "FileSystem.h"
#include "WinFileSystem.h"
#include "CascFileSystem.h"
#include "FileHandle.h"

// We need the following classes
#include "FileSystems.h"
#include "Strings.h"
#include "Compression.h"
#include "BinaryReader.h"
#include "MemoryReader.h"
#include "Siren.h"

XSUBCacheV2::XSUBCacheV2()
{
    // Default, attempt to load the siren lib
    Siren::Initialize(L"oo2core_8_win64.dll");
}

XSUBCacheV2::~XSUBCacheV2()
{
    // Clean up if need be
    Siren::Shutdown();
    // Close Handle
    FileSystem = nullptr;
}

void XSUBCacheV2::LoadPackageCache(const std::string& BasePath)
{
    // Call Base function first!
    CoDPackageCache::LoadPackageCache(BasePath);

    // Open the file system, check for build info, if build info exists
    // we'll use Casc, otherwise use raw directory.
    if (FileSystems::FileExists(BasePath + "\\.build.info"))
    {
        FileSystem = std::make_unique<ps::CascFileSystem>(BasePath);
    }
    else
    {
        FileSystem = std::make_unique<ps::WinFileSystem>(BasePath);

    }

    // Verify we've successfully opened it.
    if (!FileSystem->IsValid())
    {
        FileSystem = nullptr;
        return;
    }

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
    // Secon pass, catch xpak.
    FileSystem->EnumerateFiles("*.xpak", [this](const std::string& name, const size_t size)
    {
        try
        {
            this->LoadPackage(name);
        }
        catch (...)
        {

        }
    });

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

    // Open CASC File
    // TODO: Implement read, seek, etc. right in this handle class.
    auto Handle = ps::FileHandle(FileSystem->OpenFile(FilePath, "r"), FileSystem.get());

    // Read the header
    auto Header = FileSystem->Read<XSUBHeaderV2>(Handle.GetHandle());
    // The final file location
    std::string FinalPath = FilePath;
    // Check if we have data (Type 3 is data, others are just metadata and refs)
    if (Header.Type == 1)
    {
        // Append data file, type 1 files contain a seperate file for the actual 
        // data as they are built on demand by the game.
        FinalPath += "data";
    }
    else if (Header.Type != 3)
    {
        return false;
    }

    // Verify the magic and offset
    if (Header.Magic == 0x4950414b && Header.HashOffset < FileSystem->Size(Handle.GetHandle()))
    {
        // Jump to hash offset
        FileSystem->Seek(Handle.GetHandle(), Header.HashOffset, SEEK_SET);
        // Read Buffer
        auto Buffer = FileSystem->Read(Handle.GetHandle(), Header.HashCount * sizeof(XSUBHashEntryV2));

        // Verify result
        if (Buffer == nullptr)
        {
            return false;
        }

        // Read the hash data into a buffer
        auto HashData = MemoryReader((int8_t*)Buffer.release(), Header.HashCount * sizeof(XSUBHashEntryV2));

        // Loop and setup entries
        for (int64_t i = 0; i < Header.HashCount; i++)
        {
            // Read it
            auto Entry = HashData.Read<XSUBHashEntryV2>();

            // Prepare a cache entry
            PackageCacheObject NewObject{};
            // Set data
            NewObject.Offset           = (Entry.PackedInfo >> 32) << 7;
            NewObject.CompressedSize   = (Entry.PackedInfo >> 1) & 0x3FFFFFFF;
            NewObject.UncompressedSize = 0;
            NewObject.PackageFileIndex = PackageIndex;

            // Append to database
            CacheObjects.insert(std::make_pair(Entry.Key, NewObject));
        }

        // Append the file path
        PackageFilePaths.push_back(FinalPath);

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
        // Aquire lock
        std::lock_guard<std::shared_mutex> Gaurd(ReadMutex);

        // Take cache data, and extract from the XPAK (Uncompressed size = offset of data segment!)
        auto& CacheInfo = CacheObjects[CacheID];
        // Get the XPAK name
        auto& XPAKFileName = PackageFilePaths[CacheInfo.PackageFileIndex];
        // Open CASC File
        // TODO: Implement read, seek, etc. right in this handle class.
        auto Handle = ps::FileHandle(FileSystem->OpenFile(XPAKFileName, "r"), FileSystem.get());

        auto ReadUsage = 0.0;
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
        XSUBBlockV2 Blocks[256];

        // Hop to the beginning offset
        FileSystem->Seek(Handle.GetHandle(), BlockPosition + 2, SEEK_SET);

        // Raw block, hacky check, probably will be info
        // within Gfx Mips and other data
        if (FileSystem->Read<uint64_t>(Handle.GetHandle()) != CacheID)
        {
            // Hop to the beginning offset
            FileSystem->Seek(Handle.GetHandle(), BlockPosition, SEEK_SET);
            // Read
            FileSystem->Read(Handle.GetHandle(), (uint8_t*)ResultBuffer.get(), 0, CacheInfo.CompressedSize);
            // Set size
            TotalDataSize = CacheInfo.CompressedSize;
            // Done
            return ResultBuffer;
        }

        while ((uint64_t)FileSystem->Tell(Handle.GetHandle()) < BlockEnd)
        {
            // Hop to the beginning offset, skip header
            FileSystem->Seek(Handle.GetHandle(), BlockPosition + 22, SEEK_SET);

            // Read blocks
            auto BlockCount = (uint32_t)FileSystem->Read<uint8_t>(Handle.GetHandle());
            std::memset(Blocks, 0, sizeof(Blocks));
            FileSystem->Read(Handle.GetHandle(), (uint8_t*)&Blocks, 0, BlockCount * sizeof(XSUBBlockV2));

            // Loop for block count
            for (uint32_t i = 0; i < BlockCount; i++)
            {
                // Hop to the beginning offset, skip header
                FileSystem->Seek(Handle.GetHandle(), BlockPosition + Blocks[i].BlockOffset, SEEK_SET);
                // Read result
                uint64_t ReadSize = 0;
                // Read the block
                auto DataBlock = FileSystem->Read(Handle.GetHandle(), Blocks[i].CompressedSize);

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
                    FileSystem->Seek(Handle.GetHandle(), Blocks[i].CompressedSize, SEEK_CUR);
                    // Done
                    break;
                }
            }

            // Set next block
            BlockPosition = ((FileSystem->Tell(Handle.GetHandle()) + 0x7F) & 0xFFFFFFFFFFFFF80);
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