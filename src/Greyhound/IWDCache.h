#pragma once

#include <string>
#include <cstdint>

// We need the package cache
#include "CoDPackageCache.h"

// A class that handles reading, caching and extracting IWD resources
class IWDCache : public CoDPackageCache
{
public:
    // Constructors
    IWDCache();
    virtual ~IWDCache();

    // Implement the load function
    virtual void LoadPackageCache(const std::string& BasePath);
    // Implement the load package
    virtual bool LoadPackage(const std::string& FilePath);
    // Implement the extract function
    virtual std::unique_ptr<uint8_t[]> ExtractPackageObject(uint64_t CacheID, uint32_t& ResultSize);
    // Implement hash function
    virtual uint64_t HashPackageID(const std::string& Value);
};