#pragma once

#include <string>

// We need to include external libraries for models
#include "WraithModel.h"

// A class that handles writing XME model files
class CodXME
{
public:
    // Export a WraithModel to a XME file
    static void ExportXME(const WraithModel& Model, const std::string& FileName);
};