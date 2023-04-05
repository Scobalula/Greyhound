#pragma once

#include <string>

// We need to include external libraries for models
#include "WraithModel.h"

// A class that handles writing Maya model files
class Maya
{
public:
    // Export a WraithModel to a Maya ASCII file (Bind included in the same folder)
    static void ExportMaya(const WraithModel& Model, const std::string& FileName);
};