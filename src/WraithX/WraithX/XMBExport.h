#pragma once

#include <string>

// We need to include external libraries for models
#include "WraithModel.h"

// A class that handles writing XMB model files
class CodXMB
{
public:
	// Export a WraithModel to a XMB file
	static void ExportXMB(const WraithModel& Model, const std::string& FileName);
};