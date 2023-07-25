#pragma once
#include "CoDPackage.h"

class CoDPackageDefinition
{
public:
	// Checks if the provided file path is valid for this package.
	virtual bool IsValid(const std::string& path, const std::string& extension, const uint8_t* header, const size_t headerSize) = 0;
	// Creates the package.
	virtual std::unique_ptr<CoDPackage> Create(CoDFileSystem* fileSystem, const std::string& path) = 0;
};

