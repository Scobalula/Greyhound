#pragma once
#include "CoDPackageDefinition.h"

class XPAKPackageDefinition : public CoDPackageDefinition
{
public:
	// Checks if the provided file path is valid for this package.
	bool IsValid(const std::string& path, const std::string& extension, const uint8_t* header, const size_t headerSize);
	// Creates the package.
	std::unique_ptr<CoDPackage> Create(CoDFileSystem* fileSystem, const std::string& path);
};

