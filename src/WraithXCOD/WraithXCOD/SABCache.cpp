#include "stdafx.h"

// The class we are implementing
#include "SABCache.h"

// We need the following classes

SABCache::SABCache()
{
    // Defaults
}

SABCache::~SABCache()
{
    // Clean up if need be
}

void SABCache::LoadPackageCache(const std::string& BasePath)
{
    // Call Base function first!
    CoDPackageCache::LoadPackageCache(BasePath);

    // Set packages path
    this->PackageFilesPath = BasePath;

    // We've finished loading, set status
    this->SetLoadedState();
}