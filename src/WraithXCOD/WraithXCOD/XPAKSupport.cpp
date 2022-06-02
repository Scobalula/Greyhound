#include "stdafx.h"

// The class we are implementing
#include "XPAKSupport.h"

// We need the following WraithX classes
#include "MemoryReader.h"
#include "BinaryReader.h"
#include "FileSystems.h"
#include "Strings.h"
#include "SettingsManager.h"
#include "WraithNameIndex.h"

// We need the file classes
#include "CoDAssets.h"
#include "CoDRawImageTranslator.h"
#include "DBGameFiles.h"

// -- Start structures for reading

struct XImageMipInfo
{
    uint32_t Width;
    uint32_t Height;
    std::string Format;
    uint64_t Key;

    XImageMipInfo()
    {
        Width = 0;
        Height = 0;
        Format = "";
        Key = 0;
    }

    XImageMipInfo(uint32_t SizeWidth, uint32_t SizeHeight, std::string FormatStr, uint64_t KeyVal)
    {
        Width = SizeWidth;
        Height = SizeHeight;
        Format = FormatStr;
        Key = KeyVal;
    }
};

// -- End read structures

// -- Function for formats

uint16_t GetFormatFromString(const std::string& Format);

// -- End functions

bool XPAKSupport::ParseXPAK(const std::string& FilePath)
{
    // Prepare to load the index list from an XPAK (Images only)
    auto Reader = BinaryReader();
    // Open it
    Reader.Open(FilePath);

    // Read the header
    auto Header = Reader.Read<BO3XPakHeader>();

    // If MW4 we need to skip the new bytes
    if (Header.Version == 0xD)
    {
        Reader.SetPosition(0);
        uint64_t Result;
        Reader.Read((uint8_t*)&Header, 24, Result);
        Reader.Advance(288);
        Reader.Read((uint8_t*)&Header + 24, 96, Result);
    }

    // Load Package Files
    WraithNameIndex XPAKNames;

    // Verify the magic
    if (Header.Magic == 0x4950414b)
    {
        // Check if this is just an index package
        if (Header.HashCount <= 0)
        {
            // Just succeed, don't load
            return true;
        }

        // Load Bo4's Index
        if (Header.Version == 11)
        {
            XPAKNames.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo4_ximage.wni"));
        }

        // Jump to hash offset
        Reader.SetPosition(Header.HashOffset);

        // List of hashes
        std::set<uint64_t> IncludedHashes;

        {
            // Hash result size
            uint64_t HashResult = 0;
            // Read the hash data into a buffer
            auto Buffer = Reader.Read(Header.HashCount * sizeof(BO3XPakHashEntry), HashResult);
            auto HashData = MemoryReader(Buffer, HashResult);

            // Loop and insert
            for (uint64_t i = 0; i < Header.HashCount; i++)
            {
                IncludedHashes.insert(HashData.Read<BO3XPakHashEntry>().Key);
            }
        }

        // Jump to index offset
        Reader.SetPosition(Header.IndexOffset);

        // Store temporary information
        int32_t Width = 0;
        int32_t Height = 0;
        std::string Format = "";
        std::string Name = "";
        std::string Type = "";

        // A list of image mips
        std::unordered_map<std::string, XImageMipInfo> ImageMipMaps;

        // Loop and setup entries
        for (uint64_t i = 0; i < Header.IndexCount; i++)
        {
            // Reset temporary info
            Width = 0;
            Height = 0;
            Format = "";
            Name = "";
            Type = "";

            // Read hash and properties size
            auto Hash = Reader.Read<uint64_t>();
            auto Properties = Reader.Read<uint64_t>();

            // Ensure in map
            if (IncludedHashes.find(Hash) == IncludedHashes.end())
            {
                // Skip properties
                Reader.Advance(Properties);
                continue;
            }

            // Result
            uint64_t ReadSize = 0;
            // Read buffer and parse
            auto PropertiesBuffer = Reader.Read(Properties, ReadSize);

            // Check if valid
            if (PropertiesBuffer != nullptr)
            {
                // Results
                auto ResultBuffer = Strings::SplitString(std::string((char*)PropertiesBuffer, (char*)PropertiesBuffer + ReadSize), '\n');

                // Loop and take the ones we need
                for (auto& KeyValue : ResultBuffer)
                {
                    // Check each value
                    if (_strnicmp(KeyValue.c_str(), "width:", 6) == 0)
                    {
                        // Notice: Width is actually height
                        Strings::ToInteger(KeyValue.substr(7), Height);
                    }
                    else if (_strnicmp(KeyValue.c_str(), "height:", 7) == 0)
                    {
                        // Notice: Height is actually width
                        Strings::ToInteger(KeyValue.substr(8), Width);
                    }
                    else if (_strnicmp(KeyValue.c_str(), "name:", 5) == 0)
                    {
                        Name = KeyValue.substr(6);

                        // Check for Bo4
                        if (Header.Version == 11)
                        {
                            // Convert the hex value and mask to 60bit
                            int64_t Result = std::strtoull(Name.c_str(), 0, 16) & 0xFFFFFFFFFFFFFFF;

                            // Check for success
                            if (Result != 0)
                            {
                                // Set new name
                                Name = Strings::Format("ximage_%llx", Result);

                                // Check for an override in the name DB
                                if (XPAKNames.NameDatabase.find(Result) != XPAKNames.NameDatabase.end())
                                    Name = XPAKNames.NameDatabase[Result];
                            }

                        }
                    }
                    else if (_strnicmp(KeyValue.c_str(), "format:", 7) == 0)
                    {
                        Format = KeyValue.substr(8);
                    }
                    else if (_strnicmp(KeyValue.c_str(), "type:", 5) == 0)
                    {
                        Type = KeyValue.substr(6);
                    }
                }

                // If the type is an image, we can add it
                if (Type == "image")
                {
                    // Check mip map against width
                    if (ImageMipMaps[Name].Width < (uint32_t)Width)
                    {
                        // Set data to the next largest
                        ImageMipMaps[Name] = XImageMipInfo(Width, Height, Format, Hash);
                    }
                }

                // Clean up
                delete[] PropertiesBuffer;
            }
        }

        // Loop and add the highest mips
        for (auto& KeyValue : ImageMipMaps)
        {
            // Make and add
            auto LoadedImage = new CoDImage_t();
            // Set
            LoadedImage->AssetName = KeyValue.first;
            LoadedImage->AssetPointer = KeyValue.second.Key;
            LoadedImage->Width = (uint16_t)KeyValue.second.Width;
            LoadedImage->Height = (uint16_t)KeyValue.second.Height;
            LoadedImage->Format = GetFormatFromString(KeyValue.second.Format);
            LoadedImage->AssetStatus = WraithAssetStatus::Loaded;
            // File entry mode
            LoadedImage->IsFileEntry = true;

            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
        }
    }
    else
    {
        // Failed
        return false;
    }

    // Determine game from here
    switch (Header.Version)
    {
    case 0xA: CoDAssets::GameID = SupportedGames::BlackOps3; break;
    case 0xB: CoDAssets::GameID = SupportedGames::BlackOps4; break;
    case 0xD: CoDAssets::GameID = SupportedGames::ModernWarfare4; break;
    }

    // Success unless otherwise
    return true;
}

std::unique_ptr<XImageDDS> XPAKSupport::ReadImageFile(const CoDImage_t* Image)
{
    // Grab the format byte
    auto ImageFormat = Image->Format;

    // Calculate proper image format (Convert signed to unsigned)
    switch (Image->Format)
    {
        // Fix invalid BC1_SRGB images, swap to BC1_UNORM
    case 72: ImageFormat = 71; break;
        // Fix invalid BC2_SRGB images, swap to BC2_UNORM
    case 75: ImageFormat = 74; break;
        // Fix invalid BC3_SRGB images, swap to BC3_UNORM
    case 78: ImageFormat = 77; break;
        // Fix invalid BC7_SRGB images, swap to BC7_UNORM
    case 99: ImageFormat = 98; break;
    }

    // Resulting buffer
    uint32_t ResultSize = 0;
    // We have a streamed image, prepare to extract (AssetPointer = Image Key)
    auto ImageData = CoDAssets::GamePackageCache->ExtractPackageObject(Image->AssetPointer, ResultSize);

    // Prepare if we have it
    if (ImageData != nullptr)
    {
        // Prepare to create a MemoryDDS file
        auto Result = CoDRawImageTranslator::TranslateBC(ImageData, ResultSize, Image->Width, Image->Height, (uint8_t)ImageFormat);

        // Check for, and apply patch if required, if we got a raw result
        if (Result != nullptr && (SettingsManager::GetSetting("patchnormals", "true") == "true"))
        {
            // We can check the name here for _n and _nml
            if (Strings::EndsWith(Image->AssetName, "_n") || Strings::EndsWith(Image->AssetName, "_nml") | Strings::EndsWith(Image->AssetName, "_n2") || Strings::EndsWith(Image->AssetName, "_n3"))
            {
                // This is a normal map
                Result->ImagePatchType = ImagePatch::Normal_Expand;
            }
        }

        // Return it
        return Result;
    }

    // Failed
    return nullptr;
}

uint16_t GetFormatFromString(const std::string& Format)
{
    // Check format
    if (Format == "BC1" || Format == "BC1_SRGB" || Format == "33" || Format == "34" || Format == "35")
    {
        return 71;
    }
    else if (Format == "BC2" || Format == "BC2_SRGB")
    {
        return 74;
    }
    else if (Format == "BC3" || Format == "BC3_SRGB" || Format == "38")
    {
        return 77;
    }
    else if (Format == "BC4" || Format == "BC4_SNORM" || Format == "39")
    {
        return 80;
    }
    else if (Format == "BC5")
    {
        return 83;
    }
    else if (Format == "BC5_SNORM")
    {
        return 84;
    }
    else if (Format == "BC6_UH")
    {
        return 95;
    }
    else if (Format == "BC6_SH")
    {
        return 96;
    }
    else if (Format == "R9G9B9E5")
    {
        return 67;
    }
    else if (Format == "R16G16B16A16F")
    {
        return 10;
    }
    else if (Format == "R8G8B8A8_SRGB" || Format == "A8R8G8B8_SRGB" || Format == "R8G8B8A8" || Format == "7")
    {
        return 28;
    }
    else if (Format == "R8_UN")
    {
        return 61;
    }
    else if (Format == "BC7" || Format == "BC7_SRGB" || Format == "44" || Format == "45")
    {
        return 98;
    }

    // Default
    return 71;
}