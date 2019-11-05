#pragma once

#include <string>

// We need to include external libraries for models
#include "WraithModel.h"

// A class that handles writing SMD model files
class ValveSMD
{
public:
    // Export a WraithModel to a Valve SMD file
    static void ExportSMD(const WraithModel& Model, const std::string& FileName);
};