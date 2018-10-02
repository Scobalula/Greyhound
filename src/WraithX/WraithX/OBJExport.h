#pragma once

#include <string>

// We need to include external libraries for models
#include "WraithModel.h"

// A class that handles writing OBJ model files
class WavefrontOBJ
{
public:
	// Export a WraithModel to a Wavefront OBJ file
	static void ExportOBJ(const WraithModel& Model, const std::string& FileName);
};