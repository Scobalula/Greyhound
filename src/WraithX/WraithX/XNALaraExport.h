#pragma once

#include <string>

// We need to include external libraries for models
#include "WraithModel.h"

// A class that handles writing XNALara model files
class XNALara
{
public:
    // Export a WraithModel to a XNALara mesh file
    static void ExportXNA(const WraithModel& Model, const std::string& FileName);
};