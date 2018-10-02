#include "stdafx.h"

// The class we are implementing
#include "PAKCache.h"

// We need the following classes

PAKCache::PAKCache()
{
	// Defaults
}

PAKCache::~PAKCache()
{
	// Clean up if need be
}

void PAKCache::LoadPackageCache(const std::string& BasePath)
{
	// Call Base function first!
	CoDPackageCache::LoadPackageCache(BasePath);

	// Set packages path
	this->PackageFilesPath = BasePath;

	// We've finished loading, set status
	this->SetLoadedState();
}