#pragma once

#include <string>
#include <cstdint>
#include <memory>

// We need the package cache
#include "CoDPackageCache.h"

// A class that handles reading, caching and extracting XPAK resources
class XPAKCache : public CoDPackageCache
{
public:
    // Constructors
    XPAKCache();
    virtual ~XPAKCache();

    // Implement the load function
    virtual void LoadPackageCache(const std::string& BasePath);
    // Implement the load package
    virtual bool LoadPackage(const std::string& FilePath);
    // Implement the extract function
    virtual std::unique_ptr<uint8_t[]> ExtractPackageObject(uint64_t CacheID, int32_t Size, uint32_t& ResultSize);

    // Decompresses a compressed package object.
    static std::unique_ptr<uint8_t[]> DecompressPackageObject(uint64_t cacheID, uint8_t* buffer, size_t bufferSize, size_t decompressedSize, size_t& resultSize);
};