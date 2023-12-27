#include "stdafx.h"

// The class we are implementing
#include "WraithNameIndex.h"

// We require the following classes
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "MemoryReader.h"
#include "Compression.h"
#include "FileSystems.h"

WraithNameIndex::WraithNameIndex()
{
    // Defaults
}

WraithNameIndex::WraithNameIndex(const std::string& FilePath)
{
    // Load it
    this->LoadIndex(FilePath);
}

void WraithNameIndex::SaveIndex(const std::string& FilePath)
{
    // Prepare to save
    auto Writer = BinaryWriter();
    // Create it
    Writer.Create(FilePath);

    // Write magic and version
    Writer.Write<uint32_t>(0x20494E57);
    Writer.Write<uint16_t>(0x1);

    // Write entries count
    Writer.Write<uint32_t>((uint32_t)NameDatabase.size());

    // Prepare to package up the data
    uint32_t PackageSize = 0;

    // Loop and calculate
    for (auto& KeyValue : NameDatabase)
    {
        // Add 8 for key and then string size
        PackageSize += 8 + (uint32_t)(KeyValue.second.size() + 1);
    }

    // Allocate a buffer
    auto UnpackedData = new int8_t[PackageSize];

    // Offset
    uint32_t DataOffset = 0;

    // Loop and copy over
    for (auto& KeyValue : NameDatabase)
    {
        // Copy Key
        std::memcpy(UnpackedData + DataOffset, &KeyValue.first, 8);
        // Increase
        DataOffset += 8;
        // Copy Value
        std::memcpy(UnpackedData + DataOffset, KeyValue.second.c_str(), KeyValue.second.size() + 1);
        // Increase
        DataOffset += (uint32_t)(KeyValue.second.size() + 1);
    }

    // Allocate a buffer for compressed (At least 1 / 2 of size in addition)
    auto PackedData = new int8_t[PackageSize + (PackageSize / 2)];

    // Compress the data
    auto ResultSize = Compression::CompressLZ4Block(UnpackedData, PackedData, PackageSize, PackageSize + (PackageSize / 2));

    // Clean up unpacked
    delete[] UnpackedData;

    // Check if we got it
    if (ResultSize > 0)
    {
        // Write sizes
        Writer.Write<uint32_t>(ResultSize);
        Writer.Write<uint32_t>(PackageSize);

        // Write data
        Writer.Write(PackedData, ResultSize);
    }

    // Clean up
    delete[] PackedData;
}

void WraithNameIndex::LoadIndex(const std::string& LoadIndex)
{
    // Prepare to load
    if (FileSystems::FileExists(LoadIndex))
    {
        // Prepare a reader
        auto Reader = BinaryReader();
        // Open it
        Reader.Open(LoadIndex);

        // Verify magic ('WNI ')
        if (Reader.Read<uint32_t>() == 0x20494E57)
        {
            // Skip over version (Should be 0x1)
            Reader.Advance(2);

            // Read entry count
            auto EntryCount = Reader.Read<uint32_t>();

            // Read packed sizes
            auto Packed = Reader.Read<uint32_t>();
            auto Unpacked = Reader.Read<uint32_t>();

            // Result size
            uint64_t CompressedResult = 0;
            // Read the compressed data
            auto CompressedBuffer = Reader.Read(Packed, CompressedResult);

            // Check it
            if (CompressedBuffer != nullptr)
            {
                // Prepare to decompress, then read
                auto Decompressed = new int8_t[Unpacked];

                // Decompress it
                auto DecompressResult = Compression::DecompressLZ4Block(CompressedBuffer, Decompressed, Packed, Unpacked);

                // Check
                if (DecompressResult > 0)
                {
                    // Allocate a reader and parse (This will auto clean up)
                    auto MemReader = MemoryReader(Decompressed, DecompressResult);

                    // Loop and read entries
                    for (uint32_t i = 0; i < EntryCount; i++)
                    {
                        // Read key and name
                        auto Key = MemReader.Read<uint64_t>() & 0xFFFFFFFFFFFFFFF;
                        auto Value = MemReader.ReadNullTerminatedString();

                        // Add the entry
                        NameDatabase[Key] = Value;
                    }
                }
                else
                {
                    // Failed
                    delete[] Decompressed;
                }

                // Clean up
                delete[] CompressedBuffer;
            }
        }
    }
}