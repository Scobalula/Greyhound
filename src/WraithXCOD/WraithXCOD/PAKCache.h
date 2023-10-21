#pragma once

#include <string>
#include <cstdint>

// We need the package cache
#include "CoDPackageCache.h"

// A class that handles storing PAK file info
class PAKCache : public CoDPackageCache
{
public:
    // Constructors
    PAKCache();
    virtual ~PAKCache();

    // Implement the load function
    virtual void LoadPackageCache(const std::string& BasePath);
    // Returns a cache object (nullptr if not found)
    virtual std::unique_ptr<uint8_t[]> ExtractPackageObject(const std::string& PackageName, const uint64_t AssetOffset, const uint64_t AssetSize, size_t& ResultSize);
};