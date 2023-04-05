#include "pch.h"

// The class we are implementing
#include "IWDSupport.h"

// We need the following WraithX classes
#include "MemoryReader.h"
#include "BinaryReader.h"

// We need the file classes
#include "CoDAssets.h"
#include "CoDIWITranslator.h"

// We need the MZ zip code
#include "MiniZ_Zip.h"
#include <Path.h>

bool IWDSupport::ParseIWD(const std::string& FilePath)
{
    // Prepare to load the index list from an IWD (Images only)
    mz_zip_archive ZipArchive;
    // Clear the memory
    std::memset(&ZipArchive, 0, sizeof(ZipArchive));

    // Initialize the file
    if (!mz_zip_reader_init_file(&ZipArchive, FilePath.c_str(), 0))
    {
        // Failed, move on
        return false;
    }

    // We have a working archive, get the file count
    auto FileCount = (uint32_t)mz_zip_reader_get_num_files(&ZipArchive);

    // Iterate over all files for processing
    for (uint32_t i = 0; i < FileCount; i++)
    {
        // Buffer for file data
        mz_zip_archive_file_stat FileInfo;
        // Read the data
        if (mz_zip_reader_file_stat(&ZipArchive, i, &FileInfo))
        {
            // Parse the data and set it up
            auto EntryName = std::string(FileInfo.m_filename);

            // Check if it is an IWI
            if (string(EntryName).EndsWith(".iwi"))
            {
                // Lets add this entry
                auto LoadedImage = new CoDImage_t();
                // Set
                LoadedImage->AssetName = IO::Path::GetFileNameWithoutExtension(EntryName.c_str());
                LoadedImage->AssetPointer = 0;
                LoadedImage->Width = 0;
                LoadedImage->Height = 0;
                LoadedImage->AssetStatus = WraithAssetStatus::Loaded;
                // File entry mode
                LoadedImage->IsFileEntry = true;

                // Add
                CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
            }
        }
    }

    // Clean up
    mz_zip_reader_end(&ZipArchive);

    // Success
    return true;
}

std::unique_ptr<XImageDDS> IWDSupport::ReadImageFile(const CoDImage_t* Image)
{
    // Resulting buffer
    uint32_t ResultSize = 0;
    // We have a streamed image, prepare to extract (Hash the name for the id)
    auto ImageData = CoDAssets::GamePackageCache->ExtractPackageObject(CoDAssets::GamePackageCache->HashPackageID(Image->AssetName), ResultSize);

    // Prepare if we have it
    if (ImageData != nullptr)
    {
        // Prepare to create a MemoryDDS file
        auto Result = CoDIWITranslator::TranslateIWI(ImageData, ResultSize);

        // Check for, and apply patch if required, if we got a raw result
        if (Result != nullptr && ExportManager::Config.GetBool("PatchNormals"))
        {
            // We can check the name here for _n and _nml
            if (string(Image->AssetName).EndsWith("_n") || string(Image->AssetName).EndsWith("_nml") || string(Image->AssetName).EndsWith("_n2") || string(Image->AssetName).EndsWith("_n3"))
            {
                // This is a normal map
                Result->ImagePatchType = ImagePatch::Normal_Bumpmap;
            }
        }

        // Return it
        return Result;
    }

    // Failed
    return nullptr;
}