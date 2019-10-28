#pragma once

#include <cstdint>
#include <memory>

// We need the cod assets
#include "CoDAssets.h"

// A class that handles translating generic Call of Duty Rawfiles to a file
class CoDRawfileTranslator
{
public:
    // -- Translation functions

    // Translates a Rawfile asset
    static void TranslateRawfile(const CoDRawFile_t* Rawfile, const std::string& ExportPath, const bool ATRCompressed, const bool GSCCompressed);
};