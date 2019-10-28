#pragma once

#include <cstdint>
#include <memory>

// We need the Asset types
#include "CoDAssetType.h"

// A class that handles converting IWI files to DDS files in memory
class CoDIWITranslator
{
public:
    // -- Conversion function

    // Translates an IWI file to a DDS file
    static std::unique_ptr<XImageDDS> TranslateIWI(const std::unique_ptr<uint8_t[]>& IWIBuffer, uint32_t IWIBufferSize);
};