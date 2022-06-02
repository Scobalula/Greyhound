#include "stdafx.h"

// The class we are implementing
#include "CoDRawfileTranslator.h"

// We need the following WraithX classes
#include "ProcessReader.h"
#include "Strings.h"
#include "BinaryWriter.h"
#include "MemoryReader.h"
#include "FileSystems.h"
#include "SettingsManager.h"
#include "Compression.h"

void CoDRawfileTranslator::TranslateRawfile(const CoDRawFile_t* Rawfile, const std::string& ExportPath, const bool ATRCompressed, const bool GSCCompressed)
{
    // If the asset size is > 0, continue...
    if (Rawfile->AssetSize > 0)
    {
        // Build the export path
        std::string ExportFolder = ExportPath;

        // Check to preserve the paths
        if (SettingsManager::GetSetting("keeprawpath", "true") == "true")
        {
            // Apply the base path
            ExportFolder = FileSystems::CombinePath(ExportFolder, Rawfile->RawFilePath);
        }

        // Make the directory
        FileSystems::CreateDirectory(ExportFolder);

        // Prepare the export
        if (Strings::EndsWith(Rawfile->AssetName, ".atr") && ATRCompressed)
        {
            // Compressed write
            uintptr_t ResultSize = 0;
            // Reader
            auto MemReader = MemoryReader(CoDAssets::GameInstance->Read((uintptr_t)Rawfile->RawDataPointer, (uintptr_t)Rawfile->AssetSize, ResultSize), (uint64_t)Rawfile->AssetSize);

            // Decompress on success
            if (MemReader.GetCurrentStream() != nullptr)
            {
                // Decompression size
                auto DecompressedSize = MemReader.Read<uint32_t>();
                
                // Ensure success
                if (DecompressedSize > 0)
                {
                    // Decompression buffer
                    auto DecompressionBuffer = new int8_t[DecompressedSize];

                    // Decompress the block
                    Compression::DecompressDeflateBlock(MemReader.GetCurrentStream() + sizeof(uint32_t), DecompressionBuffer, (int32_t)ResultSize, (int32_t)DecompressedSize);

                    // New writer instance
                    auto Writer = BinaryWriter();
                    // Create the file
                    Writer.Create(FileSystems::CombinePath(ExportFolder, Rawfile->AssetName));
                    // Write the raw data
                    Writer.Write(DecompressionBuffer, (uint32_t)DecompressedSize);

                    // Clean up
                    delete[] DecompressionBuffer;
                }
            }
        }
        else if ((Strings::EndsWith(Rawfile->AssetName, ".gsc") || Strings::EndsWith(Rawfile->AssetName, ".csc")) && GSCCompressed)
        {
            // Compressed write
            uintptr_t ResultSize = 0;
            // Reader
            auto MemReader = MemoryReader(CoDAssets::GameInstance->Read((uintptr_t)Rawfile->RawDataPointer, (uintptr_t)Rawfile->AssetSize, ResultSize), (uint64_t)Rawfile->AssetSize);

            // Decompress on success
            if (MemReader.GetCurrentStream() != nullptr)
            {
                // Read Sizes
                auto DecompressedSize = MemReader.Read<uint32_t>();
                auto CompressedSize = MemReader.Read<uint32_t>();

                // Ensure success
                if (DecompressedSize > 0)
                {
                    // Decompression buffer
                    auto DecompressionBuffer = new int8_t[DecompressedSize];

                    // Decompress the block
                    Compression::DecompressZLibBlock(MemReader.GetCurrentStream() + sizeof(uint64_t), DecompressionBuffer, (int32_t)ResultSize, (int32_t)DecompressedSize);

                    // New writer instance
                    auto Writer = BinaryWriter();
                    // Create the file
                    Writer.Create(FileSystems::CombinePath(ExportFolder, Rawfile->AssetName));
                    // Write the raw data
                    Writer.Write(DecompressionBuffer, (uint32_t)DecompressedSize);

                    // Clean up
                    delete[] DecompressionBuffer;
                }
            }
        }
        else
        {
            // Raw write
            uintptr_t ResultSize = 0;

            // Read the data
            auto AssetData = CoDAssets::GameInstance->Read((uintptr_t)Rawfile->RawDataPointer, (uintptr_t)Rawfile->AssetSize, ResultSize);

            // Write on success
            if (AssetData != nullptr)
            {
                // New writer instance
                auto Writer = BinaryWriter();
                // Create the file
                if (Writer.Create(FileSystems::CombinePath(ExportFolder, Rawfile->AssetName)))
                {
                    // Write the raw data
                    Writer.Write(AssetData, (uint32_t)ResultSize);
                }
                // Clean up
                delete[] AssetData;
            }
        }
    }
}