#include "pch.h"

// The class we are implementing
#include "IWDCache.h"

// We need the following classes

// We need the MZ zip code
#include "MiniZ_Zip.h"
#include <Directory.h>
#include <XXHash.h>

IWDCache::IWDCache()
{
    // Defaults
}

IWDCache::~IWDCache()
{
    // Clean up if need be
}

void IWDCache::LoadPackageCache(const std::string& BasePath)
{
    // Call Base function first!
    CoDPackageCache::LoadPackageCache(BasePath);

    // We need to enumerate all files in this path, load them, and aquire hashes of the names
    auto PathIWDFiles = IO::Directory::GetFiles(BasePath.c_str(), "*.iwd");

    // Iterate over file paths
    for (auto& IWDFile : PathIWDFiles)
    {
        // Load it
        this->LoadPackage(IWDFile.ToCString());
    }

    // We've finished loading, set status
    this->SetLoadedState();
}

bool IWDCache::LoadPackage(const std::string& FilePath)
{
    // Call Base function first
    CoDPackageCache::LoadPackage(FilePath);

    // Add to package files
    auto PackageIndex = (uint32_t)PackageFilePaths.size();

    // Parse the IWD file, append the image references
    mz_zip_archive ZipArchive;
    // Clear the memory
    std::memset(&ZipArchive, 0, sizeof(ZipArchive));

    // Initialize the file
    if (!mz_zip_reader_init_file(&ZipArchive, FilePath.c_str(), 0))
    {
        // Failed, move on
        return false;
    }
    else
    {
        // Append the file path
        PackageFilePaths.push_back(FilePath);
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
                auto AssetEntryHash = Hashing::XXHash::HashString(IO::Path::GetFileNameWithoutExtension(EntryName.c_str()));

                // Make the entry
                PackageCacheObject NewObject;
                // Set data
                NewObject.Offset = i;
                NewObject.CompressedSize = FileInfo.m_comp_size;
                NewObject.UncompressedSize = FileInfo.m_uncomp_size;
                NewObject.PackageFileIndex = PackageIndex;

                // Append to database
                CacheObjects.insert(std::make_pair(AssetEntryHash, NewObject));
            }
        }
    }

    // Clean up
    mz_zip_reader_end(&ZipArchive);

    // Done 
    return true;
}

std::unique_ptr<uint8_t[]> IWDCache::ExtractPackageObject(uint64_t CacheID, uint32_t& ResultSize)
{
    // Prepare to extract if found
    if (CacheObjects.find(CacheID) != CacheObjects.end())
    {
        // Take cache data, and extract from the IWD
        auto& CacheInfo = CacheObjects[CacheID];
        // Get the IWD name
        auto& IWDFileName = PackageFilePaths[CacheInfo.PackageFileIndex];

        // Buffer for zip data
        mz_zip_archive ZipArchive;
        // Clear the memory
        std::memset(&ZipArchive, 0, sizeof(ZipArchive));
        // Initialize it
        if (!mz_zip_reader_init_file(&ZipArchive, IWDFileName.c_str(), 0))
        {
            // Failed, move on
            return nullptr;
        }

        // Allocate a new buffer
        auto ResultBuffer = std::make_unique<uint8_t[]>(CacheInfo.UncompressedSize);

        // Extract to the memory buffer from offset which is index
        if (mz_zip_reader_extract_to_mem(&ZipArchive, (mz_uint)CacheInfo.Offset, ResultBuffer.get(), CacheInfo.UncompressedSize, 0))
        {
            // Clean up
            mz_zip_reader_end(&ZipArchive);
            // Set result size
            ResultSize = (uint32_t)CacheInfo.UncompressedSize;
            // Return result buffer
            return ResultBuffer;
        }

        // Clean up
        mz_zip_reader_end(&ZipArchive);
    }

    // Set
    ResultSize = 0;
    
    // Failed to find data
    return nullptr;
}

uint64_t IWDCache::HashPackageID(const std::string& Value)
{
    // Hash it
    return Hashing::XXHash::HashString(Value.c_str());
}