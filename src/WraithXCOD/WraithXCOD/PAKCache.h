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
};