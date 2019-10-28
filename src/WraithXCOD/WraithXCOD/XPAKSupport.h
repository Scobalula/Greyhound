#pragma once

#include <string>
#include <cstdint>
#include <memory>

// We need the following classes
#include "CoDAssetType.h"

// A class that handles parsing XPAK files
class XPAKSupport
{
public:

    // -- Parsing functions

    // Parse and load image assets within a XPAK file
    static bool ParseXPAK(const std::string& FilePath);

    // Reads an image entry from the package file
    static std::unique_ptr<XImageDDS> ReadImageFile(const CoDImage_t* Image);
};