#include "stdafx.h"
#include "XPAKPackage.h"
#include "XPAKPackageDefinition.h"

bool XPAKPackageDefinition::IsValid(const std::string& path, const std::string& extension, const uint8_t* header, const size_t headerSize)
{
	// Quick exit for bad file extension.
	if (extension != ".xpak")
		return false;

	return true;
}

std::unique_ptr<CoDPackage> XPAKPackageDefinition::Create(CoDFileSystem* fileSystem, const std::string& path)
{
	return std::make_unique<XPAKPackage>(fileSystem, path);
}
