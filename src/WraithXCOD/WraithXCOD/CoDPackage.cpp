#include "stdafx.h"
#include "CoDPackage.h"

CoDPackage::CoDPackage(CoDFileSystem* fileSystem, const std::string& path) : FileSystem(fileSystem)
{
	if (FileSystem == nullptr)
		return;
	
	FileHandle.Open(fileSystem->OpenFile(path, "r"), fileSystem);
}
