#pragma once

#include <string>
#include <cstdint>

// We need to include external libraries for models
#include "WraithModel.h"

// A class that handles writing SEModel model files
class SEModel
{
public:
    // Export a WraithModel to a SEModel file
    static void ExportSEModel(const WraithModel& Model, const std::string& FileName, bool SupportsScale = false);
};