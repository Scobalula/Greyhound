#pragma once

#include <string>
#include <cstdint>
#include "CoDFileSystem.h"

// A class that handles reading and extracting PAK resources
class PAKSupport
{
public:

    // -- Extraction functions
    
    // Extracts a resource from an image package (Ghosts)
    static std::unique_ptr<uint8_t[]> GhostsExtractImagePackage(const std::string& PackageName, uint64_t AssetOffset, uint64_t AssetSize, uint32_t& ResultSize);
    // Extracts a resource from an image package (AW / MWR)
    static std::unique_ptr<uint8_t[]> AWExtractImagePackage(const std::string& PackageName, uint64_t AssetOffset, uint64_t AssetSize, uint32_t& ResultSize);
    // Extracts a resource from an image package (AW / MWR)
    static std::unique_ptr<uint8_t[]> AWExtractImagePackageEx(CoDFileSystem* FileSystem, const std::string& PackageName, uint64_t AssetOffset, uint64_t AssetSize, uint32_t& ResultSize);
    // Extracts a resource from an image package (IW)
    static std::unique_ptr<uint8_t[]> IWExtractImagePackage(const std::string& PackageName, uint64_t AssetOffset, uint64_t AssetSize, uint32_t& ResultSize);
};