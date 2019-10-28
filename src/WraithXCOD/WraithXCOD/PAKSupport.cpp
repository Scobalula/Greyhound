#include "stdafx.h"

// The class we are implementing
#include "PAKSupport.h"

// We need the following classes
#include "Hashing.h"
#include "FileSystems.h"
#include "Strings.h"
#include "BinaryReader.h"
#include "Compression.h"

std::unique_ptr<uint8_t[]> PAKSupport::GhostsExtractImagePackage(const std::string& PackageName, uint64_t AssetOffset, uint64_t AssetSize, uint32_t& ResultSize)
{
    // Prepare to extract an image asset (Ghosts)
    ResultSize = 0;

    // Make sure the file exists
    if (FileSystems::FileExists(PackageName))
    {
        // Open the file
        auto Reader = BinaryReader();
        // Open it
        Reader.Open(PackageName);

        // Jump to the offset
        Reader.SetPosition(AssetOffset);
        
        // Read result
        uint64_t ReadSize = 0;
        // Read the data
        auto CompressedData = Reader.Read(AssetSize, ReadSize);

        // Make sure we got it
        if (CompressedData != nullptr)
        {
            // Allocate a decompressed buffer (25MB)
            auto TempWorkingBuffer = new int8_t[30000000];

            // Decompress
            auto Result = Compression::DecompressZLibBlock((const int8_t*)CompressedData, TempWorkingBuffer, (uint32_t)AssetSize, 30000000);            
            
            // Clean up
            delete[] CompressedData;

            // Allocate result
            auto ResultBuffer = std::make_unique<uint8_t[]>(Result);
            // Copy to result
            std::memcpy(ResultBuffer.get(), TempWorkingBuffer, Result);

            // Clean up
            delete[] TempWorkingBuffer;

            // Set result size
            ResultSize = Result;

            // Return result
            return ResultBuffer;
        }
    }

    // Failed
    return nullptr;
}

std::unique_ptr<uint8_t[]> PAKSupport::AWExtractImagePackage(const std::string& PackageName, uint64_t AssetOffset, uint64_t AssetSize, uint32_t& ResultSize)
{
    // Prepare to extract an image asset (AW, MWR)
    ResultSize = 0;

    // Make sure the file exists
    if (FileSystems::FileExists(PackageName))
    {
        // Open the file
        auto Reader = BinaryReader();
        // Open it
        Reader.Open(PackageName);

        // Jump to the offset
        Reader.SetPosition(AssetOffset);

        // Read total uncompressed size
        auto TotalUncompressedSize = Reader.Read<uint64_t>();
        // Skip 4 bytes
        Reader.Advance(4);

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

            // Read size
            uint64_t ReadCompress = 0;
            // Read compressed data
            auto CompressedData = Reader.Read(CompressedSize, ReadCompress);

            // Make sure we read it
            if (CompressedData != nullptr)
            {
                // Decompress it
                auto DecompressResult = Compression::DecompressLZ4Block((const int8_t*)CompressedData, (int8_t*)ResultBuffer.get() + TotalReadUncompressedSize, CompressedSize, UncompressedSize);

                // Clean up
                delete[] CompressedData;
            }

            // Skip over padding
            auto CurrentPosition = Reader.GetPosition();
            // Skip it
            Reader.SetPosition(((CurrentPosition + 0x3) & 0xFFFFFFFFFFFFFFC));

            // Advance the stream
            TotalReadUncompressedSize += UncompressedSize;
        }

        // Set result size
        ResultSize = (uint32_t)TotalUncompressedSize;

        // Return result
        return ResultBuffer;
    }

    // Failed
    return nullptr;
}

std::unique_ptr<uint8_t[]> PAKSupport::IWExtractImagePackage(const std::string& PackageName, uint64_t AssetOffset, uint64_t AssetSize, uint32_t& ResultSize)
{
    // Prepare to extract an image asset (IW)
    ResultSize = 0;

    // Make sure the file exists
    if (FileSystems::FileExists(PackageName))
    {
        // Open the file
        auto Reader = BinaryReader();
        // Open it
        Reader.Open(PackageName);

        // Jump to the offset (And skip 4 bytes)
        Reader.SetPosition(AssetOffset + 4);

        // Read total uncompressed size
        auto TotalUncompressedSize = Reader.Read<uint64_t>();
        // Skip 4 bytes
        Reader.Advance(4);

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

            // Skip 4 bytes
            Reader.Advance(4);

            // Read size
            uint64_t ReadCompress = 0;
            // Read compressed data
            auto CompressedData = Reader.Read(CompressedSize, ReadCompress);

            // Make sure we read it
            if (CompressedData != nullptr)
            {
                // Decompress it
                auto DecompressResult = Compression::DecompressLZ4Block((const int8_t*)CompressedData, (int8_t*)ResultBuffer.get() + TotalReadUncompressedSize, CompressedSize, UncompressedSize);

                // Clean up
                delete[] CompressedData;
            }

            // Skip over padding
            auto CurrentPosition = Reader.GetPosition();
            // Skip it
            Reader.SetPosition(((CurrentPosition + 0x3) & 0xFFFFFFFFFFFFFFC));

            // Advance the stream
            TotalReadUncompressedSize += UncompressedSize;
        }

        // Set result size
        ResultSize = (uint32_t)TotalUncompressedSize;

        // Return result
        return ResultBuffer;
    }

    // Failed
    return nullptr;
}