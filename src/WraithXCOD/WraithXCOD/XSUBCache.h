#pragma once
// We need the package cache
#include "CoDAssets.h"
#include "CoDPackageCache.h"
#include "Casc.h"
#include <shared_mutex>

// A class that handles reading, caching and extracting CASC Resources
class XSUBCache : public CoDPackageCache
{
private:
    // Cache Mutex
    std::shared_mutex ReadMutex;
public:
    // Constructors
    XSUBCache();
    virtual ~XSUBCache();

    // Implement the load function
    virtual void LoadPackageCache(const std::string& BasePath);
    // Implement the load package
    virtual bool LoadPackage(const std::string& FilePath);
    // Implement the extract raw function
    virtual std::unique_ptr<uint8_t[]> ExtractPackageObjectRaw(uint64_t CacheID, uint32_t& ResultSize);
    // Implement the extract function
    virtual std::unique_ptr<uint8_t[]> ExtractPackageObject(uint64_t CacheID, uint32_t& ResultSize);
};