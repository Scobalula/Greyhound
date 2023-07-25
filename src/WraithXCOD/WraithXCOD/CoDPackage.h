#pragma once
#include "CoDFileSystem.h"
#include "CoDFileHandle.h"

class CoDPackageItem
{
public:
	std::string Name;

	CoDPackageItem(const std::string& name)
	{
		Name = name;
	}
};

class CoDPackage
{
protected:
	// The file system this package originated from.
	CoDFileSystem* FileSystem;
	// The file handle.
	CoDFileHandle FileHandle;
public:
	// The items stored within this package.
	std::map<uint64_t, std::unique_ptr<CoDPackageItem>> Items;
	// Creates a new CoD Package
	CoDPackage(CoDFileSystem* fileSystem, const std::string& path);

	// Loads the package.
	virtual void Load() = 0;
};

