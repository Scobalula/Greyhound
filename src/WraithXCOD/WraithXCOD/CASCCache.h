#pragma once
// We need the package cache
#include "CoDAssets.h"
#include "CoDPackageCache.h"
#include "CascLib.h"
#include <shared_mutex>

// A class that handles reading, caching and extracting CASC Resources
class CASCCache : public CoDPackageCache
{
private:
	// Storage Handle
	HANDLE StorageHandle;
	// Cache Mutex
	std::shared_mutex ReadMutex;
public:
	// Constructors
	CASCCache();
	virtual ~CASCCache();

	// Implement the load function
	virtual void LoadPackageCache(const std::string& BasePath);
	// Implement the load package
	virtual bool LoadPackage(const std::string& FilePath);
	// Implement the extract function
	virtual std::unique_ptr<uint8_t[]> ExtractPackageObject(uint64_t CacheID, uint32_t& ResultSize);
};

