#include "stdafx.h"

// The class we are implementing
#include "PAKCache.h"

// We need the following classes
#include "CoDFileHandle.h"
#include "Compression.h"

PAKCache::PAKCache()
{
    // Defaults
}

PAKCache::~PAKCache()
{
    // Clean up if need be
}

void PAKCache::LoadPackageCache(const std::string& BasePath)
{
    // Call Base function first!
    CoDPackageCache::LoadPackageCache(BasePath);

    // Set packages path
    this->PackageFilesPath = BasePath;

    // We've finished loading, set status
    this->SetLoadedState();
}

std::unique_ptr<uint8_t[]> PAKCache::ExtractPackageObject(const std::string& PackageName, const uint64_t AssetOffset, const uint64_t AssetSize, size_t& ResultSize)
{
    // Prepare to extract an image asset (AW, MWR)
    ResultSize = 0;

    // Open CASC File
    auto Reader = CoDFileHandle(FileSystem->OpenFile(PackageName, "r"), FileSystem.get());;

    // Jump to the offset
    Reader.Seek(AssetOffset, SEEK_SET);

    // Read total uncompressed size
    auto TotalUncompressedSize = Reader.Read<uint64_t>();
    // Skip 4 bytes
    Reader.Seek(4, SEEK_CUR);

    // Allocate the result buffer
    auto ResultBuffer = std::make_unique<uint8_t[]>((uint32_t)TotalUncompressedSize);

    // Total uncompressed size read
    uint64_t TotalReadUncompressedSize = 0;

    // Loop until we have all data
    while (TotalUncompressedSize != TotalReadUncompressedSize)
    {
        // Read compressed and uncompressed size
        auto CompressedSize = Reader.Read<uint32_t>();
        auto UncompressedSize = Reader.Read<uint32_t>();
        auto CompressedData = Reader.Read(CompressedSize);

        // Make sure we read it
        if (CompressedData != nullptr)
        {
            // Decompress it
            auto DecompressResult = Compression::DecompressLZ4Block((const int8_t*)CompressedData.get(), (int8_t*)ResultBuffer.get() + TotalReadUncompressedSize, CompressedSize, UncompressedSize);
        }

        // Skip over padding
        auto CurrentPosition = Reader.Tell();
        // Skip it
        Reader.Seek(((Reader.Tell() + 0x3) & 0xFFFFFFFFFFFFFFC), SEEK_SET);

        // Advance the stream
        TotalReadUncompressedSize += UncompressedSize;
    }

    // Set result size
    ResultSize = (uint32_t)TotalUncompressedSize;

    // Return result
    return ResultBuffer;
}

