#pragma once
#include "CoDPackage.h"

class XPAKPackage : public CoDPackage
{
public:
	// Creates a new CoD Package
	XPAKPackage(CoDFileSystem* fileSystem, const std::string& path);

	// Loads the package.
	void Load();
};

