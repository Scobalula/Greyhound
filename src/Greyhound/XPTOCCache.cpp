#include "pch.h"

// The class we are implementing
#include "XPTOCCache.h"

// We need the following classes
#include "Compression.h"
#include "MemoryReader.h"
#include "PAKSupport.h"
#include "Path.h"

// We need the game files structs
#include "DBGameFiles.h"
#include <WraithBinaryReader.h>
#include <XXHash.h>

XPTOCCache::XPTOCCache()
{
    // Defaults
}

XPTOCCache::~XPTOCCache()
{
    // Clean up if need be
}

void XPTOCCache::LoadPackageCache(const std::string& BasePath)
{
    // Call Base function first!
    CoDPackageCache::LoadPackageCache(BasePath);

    // Set packages path
    this->PackageFilesPath = BasePath;

    // For now, we just want to load the 'xpakfile.toc'
    this->LoadPackage(IO::Path::Combine(BasePath.c_str(), "xpakfile.toc").ToCString());

    // We've finished loading, set status
    this->SetLoadedState();
}

bool XPTOCCache::LoadPackage(const std::string& FilePath)
{
    // Call Base function first
    CoDPackageCache::LoadPackage(FilePath);

    // Open the file
    auto Reader = BinaryReader();
    // Open it
    Reader.Open(FilePath);

    // Read the header
    auto Header = Reader.Read<WWIIXPTocHeader>();

    // Verify the magic
    if (Header.Magic == 0x3030336966663253)
    {
        // Loop and read entries
        for (uint32_t i = 0; i < Header.EntriesCount; i++)
        {
            // Read it
            auto Entry = Reader.Read<WWIIXPTocEntry>();

            // A map of chars to use to convert to hex
            const char HexMap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

            // Prepare the key
            std::string HexBuffer(32, ' ');
            for (int h = 0; h < 16; h++)
            {
                HexBuffer[h * 2] = HexMap[(Entry.Hash[h] & 0xF0) >> 4];
                HexBuffer[h * 2 + 1] = HexMap[Entry.Hash[h] & 0x0F];
            }

            // Get the hash value
            auto Key = Hashing::XXHash::HashString(HexBuffer.c_str());

            // Prepare a cache entry
            PackageCacheObject NewObject;
            // Set data
            NewObject.Offset = Entry.Offset;
            NewObject.CompressedSize = Entry.Size;
            NewObject.UncompressedSize = Entry.Size;
            NewObject.PackageFileIndex = Entry.PackageIndex;

            // Append to database
            CacheObjects.insert(std::make_pair(Key, NewObject));
        }
    }

    // Done 
    return true;
}

std::unique_ptr<uint8_t[]> XPTOCCache::ExtractPackageObject(uint64_t CacheID, uint32_t& ResultSize)
{
    // Prepare to extract if found
    if (CacheObjects.find(CacheID) != CacheObjects.end())
    {
        // Take cache data, and extract from the XPAKFile (Uncompressed size = final chunk size!)
        auto& CacheInfo = CacheObjects[CacheID];
        // Get the XPAKFile name
        auto FilePath = IO::Path::Combine(this->PackageFilesPath.c_str(), string::Format("xpakfile%d.pak", CacheInfo.PackageFileIndex));

        // Ask our PAK library to extract the entry
        return PAKSupport::AWExtractImagePackage(FilePath.ToCString(), CacheInfo.Offset, CacheInfo.UncompressedSize, ResultSize);
    }

    // Set
    ResultSize = 0;

    // Failed to find data
    return nullptr;
}