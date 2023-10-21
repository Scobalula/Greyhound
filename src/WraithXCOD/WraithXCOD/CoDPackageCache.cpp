#include "stdafx.h"

// The class we are implementing
#include "CoDPackageCache.h"

#include "FileSystems.h"
#include "WinFileSystem.h"
#include "CascFileSystem.h"

CoDPackageCache::CoDPackageCache()
{
    // Defaults
    HasLoaded = false;
    CacheLoading = false;
}

CoDPackageCache::~CoDPackageCache()
{
    // Clean up if need be
    CacheObjects.clear();
    PackageFilePaths.clear();
    PackageFilePaths.shrink_to_fit();
}

bool CoDPackageCache::HasCacheLoaded()
{
    // Aquire lock
    std::lock_guard<std::mutex> Lock(CacheMutex);

    // Return result
    return HasLoaded;
}

bool CoDPackageCache::IsCacheLoading()
{
    // Aquire lock
    std::lock_guard<std::mutex> Lock(CacheMutex);

    // Return result
    return CacheLoading;
}

void CoDPackageCache::LoadPackageCache(const std::string& BasePath)
{
    // Aquire lock
    std::lock_guard<std::mutex> Lock(CacheMutex);

    // DEBUG
#ifdef _DEBUG
    printf("LoadPackageCache(): Begin loading...\n");
#endif

    // Set that we are loading
    CacheLoading = true;

    // Open the file system, check for build info, if build info exists
    // we'll use Casc, otherwise use raw directory.
    if (FileSystems::FileExists(BasePath + "\\.build.info"))
    {
        FileSystem = std::make_unique<CascFileSystem>(BasePath);
    }
    else
    {
        FileSystem = std::make_unique<WinFileSystem>(BasePath);
    }

    // Verify we've successfully opened it.
    if (!FileSystem->IsValid())
    {
        FileSystem = nullptr;
    }
}

bool CoDPackageCache::LoadPackage(const std::string& FilePath)
{
    // Nothing, default, but ensure status is loading
    CacheLoading = true;
    HasLoaded = false;
    return true;
}

void CoDPackageCache::SetLoadedState()
{
    // Aquire lock
    std::lock_guard<std::mutex> Lock(CacheMutex);

    // DEBUG
#ifdef _DEBUG
    printf("SetLoadedState(): Cache loaded %d objects.\n", (uint32_t)CacheObjects.size());
#endif

    // Set loading complete
    HasLoaded = true;
    CacheLoading = false;
}

CoDFileSystem* CoDPackageCache::GetFileSystem()
{
    return FileSystem.get();
}

void CoDPackageCache::LoadPackageCacheAsync(const std::string& BasePath)
{
    // Start a new thread for loading
    std::thread LoadThread([BasePath, this]
    {
        // Send it off
        this->LoadPackageCache(BasePath);
    });

    // Detatch while we load
    LoadThread.detach();
}

void CoDPackageCache::LoadPackageAsync(const std::string& FilePath)
{
    // Start a new thread for loading
    std::thread LoadThread([FilePath, this]
    {
        // Send it off
        this->LoadPackage(FilePath);
        // Set the loaded state, this is only called on single entries
        this->SetLoadedState();
    });

    // Detatch while we load
    LoadThread.detach();
}

void CoDPackageCache::WaitForPackageCacheLoad()
{
    // Wait for us to load
    while (this->IsCacheLoading())
    {
        // Sleep to make sure the CPU isn't being over worked
        Sleep(100);
    }
}