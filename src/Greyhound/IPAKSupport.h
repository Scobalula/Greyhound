#pragma once

#include <string>
#include <cstdint>
#include <memory>

// We need the following classes
#include "CoDAssetType.h"

// A class that handles parsing IPAK files
class IPAKSupport
{
public:

    // -- Parsing functions

    // Parse and load image assets within a IPAK file
    static bool ParseIPAK(const std::string& FilePath);

    // Reads an image entry from the package file
    static std::unique_ptr<XImageDDS> ReadImageFile(const CoDImage_t* Image);
};