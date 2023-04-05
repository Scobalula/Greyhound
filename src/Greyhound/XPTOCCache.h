#pragma once

#include <string>
#include <cstdint>
#include <memory>

// We need the package cache
#include "CoDPackageCache.h"

// A class that handles reading, caching and extracting XPAKFile resources
class XPTOCCache : public CoDPackageCache
{
public:
    // Constructors
    XPTOCCache();
    virtual ~XPTOCCache();

    // Implement the load function
    virtual void LoadPackageCache(const std::string& BasePath);
    // Implement the load package
    virtual bool LoadPackage(const std::string& FilePath);
    // Implement the extract function
    virtual std::unique_ptr<uint8_t[]> ExtractPackageObject(uint64_t CacheID, uint32_t& ResultSize);
};