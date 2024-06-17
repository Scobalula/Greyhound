#pragma once

#include <string>
#include <cstdint>

// We need to include external libraries for models
#include "WraithModel.h"
#include "WraithAnim.h"

// A class that handles writing GLTF model files
class GLTF
{
public:
    // Export a WraithModel to a GLTF file
    static void ExportGLTF(const WraithModel& Model, const std::string& FileName, bool SupportsScale = false, bool writeBinary = false);
    // Export a WraithAnim to a GLTF file
    static void ExportGLTF(const WraithAnim& Model, const std::string& FileName, bool SupportsScale = false);
};