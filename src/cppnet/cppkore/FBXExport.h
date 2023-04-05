#pragma once

#include <string>

// We need to include external libraries for models
#include "WraithModel.h"

// A class that handles writing Maya model files
class FBX
{
public:

    // Export a WraithModel to an FBX File
    static void ExportFBX(const WraithModel& Model, const std::string& FileName);
};