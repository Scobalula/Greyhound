#include "stdafx.h"

// The class we are implementing
#include "CoDPackageCache.h"

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