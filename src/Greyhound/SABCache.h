#pragma once

#include <string>
#include <cstdint>

// We need the package cache
#include "CoDPackageCache.h"

// A class that handles storing SAB file info
class SABCache : public CoDPackageCache
{
public:
    // Constructors
    SABCache();
    virtual ~SABCache();

    // Implement the load function
    virtual void LoadPackageCache(const std::string& BasePath);
};