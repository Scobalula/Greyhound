#include "stdafx.h"

#include <unordered_set>
#include <unordered_map>

// The class we are implementing
#include "CoDGDTProcessor.h"

// We need the following Wraith classes
#include "BinaryReader.h"
#include "MemoryReader.h"
#include "Strings.h"
#include "Compression.h"
#include "FileSystems.h"
#include "TextWriter.h"
#include "SettingsManager.h"

// -- Structures for reading

struct XModelLodInfo
{
    // The suffix
    std::string LODSuffix;

    float LodDistance;
    float LodMaxDistance;

    XModelLodInfo(std::string Suffix, float Distance, float MaxDistance)
    {
        // Defaults
        LODSuffix = Suffix;
        LodDistance = Distance;
        LodMaxDistance = MaxDistance;
    }

    bool operator==(const XModelLodInfo &other) const
    {
        // Check for equality
        return (LODSuffix == other.LODSuffix && LodDistance == other.LodDistance && LodMaxDistance == other.LodMaxDistance);
    }
};

struct XModelLodHash
{
    std::size_t operator()(const XModelLodInfo& k) const
    {
        // Hash each entry
        return ((std::hash<std::string>()(k.LODSuffix) ^ (std::hash<float>()(k.LodDistance) << 1)) >> 1) ^ (std::hash<float>()(k.LodMaxDistance) << 1);
    }
};

struct SortByLodDistance
{
    inline bool operator() (const XModelLodInfo& lhs, const XModelLodInfo& rhs)
    {
        // Sort by initial distance
        return (lhs.LodDistance < rhs.LodDistance);
    }
};

struct XModelInfo
{
    // A list of lod entries by their suffix
    std::unordered_set<XModelLodInfo, XModelLodHash> LODEntries;
};

struct XMaterialInfo
{
    std::string DiffuseMap;
    std::string NormalMap;
    std::string SpecularMap;
};

// -- End structures for reading

// -- Functions for exporting

// Exports a BO3 formatted GDT file
void ExportBO3GDTFile(const std::unordered_map<std::string, XModelInfo>& ModelInfo, const std::unordered_map<std::string, XMaterialInfo>& MaterialEntries);
// Exports a WAW formatted GDT file
void ExportWAWGDTFile(const std::unordered_map<std::string, XModelInfo>& ModelInfo, const std::unordered_map<std::string, XMaterialInfo>& MaterialEntries);

// -- End function exporting

CoDGDTProcessor::CoDGDTProcessor()
{
    // Defaults
    CacheFileName = "";
}

CoDGDTProcessor::~CoDGDTProcessor()
{
    // Clean up if need be
    this->CloseProcessor();
}

void CoDGDTProcessor::SetupProcessor(const std::string& GameShorthand)
{
    // Ensure closed
    this->CloseProcessor();

    // Prepare to load the GDT cache
    auto GDTCachePath = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "gdt_cache");
    // Make it if it doesn't exist
    FileSystems::CreateDirectory(GDTCachePath);

    // Build file path
    auto CacheFilePath = FileSystems::CombinePath(GDTCachePath, "GameData_" + GameShorthand + "_Cache.gdtc");

    // Set current path
    this->CacheFileName = GameShorthand;

    // Check if file exists, if so, attach, else, create
    if (FileSystems::FileExists(CacheFilePath))
    {
        // Load
        this->CacheHandle.Open(CacheFilePath);
    }
    else
    {
        // Make new file
        this->CacheHandle.Create(CacheFilePath);
        // Write magic ('GDTC')
        this->CacheHandle.Write<uint32_t>(0x43544447);
        // Write version (Currently 0x1)
        this->CacheHandle.Write<uint16_t>(0x1);
    }

    // Caching size of 2MB, then auto-flush
    this->CacheHandle.SetWriteBuffer(2000000);
}

void CoDGDTProcessor::CloseProcessor()
{
    // Close
    this->CacheHandle.Close();
}

void CoDGDTProcessor::ProcessModelGDT(const std::unique_ptr<WraithModel>& Model)
{
    // Convert the model to GDT data (Ignore _HITBOX)
    if (Strings::EndsWith(Model->AssetName, "_HITBOX"))
        return;
    
    // Calculate buffer size
    // 8 bytes distance, then name + null char
    auto UnpackedSize = 8 + (Model->AssetName.size() + 1);

    // 2 byte material count
    UnpackedSize += 2;

    // Are we using global images
    bool GlobalImages = (SettingsManager::GetSetting("global_images", "true") == "true");

    // Process lod information
    auto LODNumberOffset = Model->AssetName.find("_LOD");
    // The model name
    std::string ModelFolderName = "";

    // Make sure we got it
    if (LODNumberOffset != std::string::npos)
    {
        // We need to clean it, then attach a number
        ModelFolderName = Model->AssetName.substr(0, LODNumberOffset);
    }

    // Generate material specific names
    for (auto& Material : Model->Materials)
    {
        // Add size + null char
        UnpackedSize += (Material.MaterialName.size() + 1);

        // Check if we are using global images
        if (GlobalImages)
        {
            // Strip off the name
            if (Material.DiffuseMapName != "")
            {
                Material.DiffuseMapName = Strings::Format("model_export\\\\_images\\\\%s", FileSystems::GetFileName(Material.DiffuseMapName).c_str());
            }
            if (Material.NormalMapName != "")
            {
                Material.NormalMapName = Strings::Format("model_export\\\\_images\\\\%s", FileSystems::GetFileName(Material.NormalMapName).c_str());
            }
            if (Material.SpecularMapName != "")
            {
                Material.SpecularMapName = Strings::Format("model_export\\\\_images\\\\%s", FileSystems::GetFileName(Material.SpecularMapName).c_str());
            }
        }
        else
        {
            // Append the model directory
            if (Material.DiffuseMapName != "")
            {
                Material.DiffuseMapName = Strings::Format("model_export\\\\%s\\\\_images\\\\%s", ModelFolderName.c_str(), FileSystems::GetFileName(Material.DiffuseMapName).c_str());
            }
            if (Material.NormalMapName != "")
            {
                Material.NormalMapName = Strings::Format("model_export\\\\%s\\\\_images\\\\%s", ModelFolderName.c_str(), FileSystems::GetFileName(Material.NormalMapName).c_str());
            }
            if (Material.SpecularMapName != "")
            {
                Material.SpecularMapName = Strings::Format("model_export\\\\%s\\\\_images\\\\%s", ModelFolderName.c_str(), FileSystems::GetFileName(Material.SpecularMapName).c_str());
            }
        }

        // Add image sizes
        UnpackedSize += (Material.DiffuseMapName.size() + 1);
        UnpackedSize += (Material.NormalMapName.size() + 1);
        UnpackedSize += (Material.SpecularMapName.size() + 1);
    }

    // Allocate buffer
    auto UnpackedBuffer = new uint8_t[UnpackedSize];
    // Unpacked offset
    auto UnpackedOffset = 0;

    // Copy over lod info
    std::memcpy(UnpackedBuffer + UnpackedOffset, &Model->LodDistance, 4);
    std::memcpy(UnpackedBuffer + UnpackedOffset + 4, &Model->LodMaxDistance, 4);

    // Increase
    UnpackedOffset += 8;

    // Copy model name
    std::memcpy(UnpackedBuffer + UnpackedOffset, Model->AssetName.c_str(), Model->AssetName.size() + 1);
    
    // Increase
    UnpackedOffset += (uint32_t)Model->AssetName.size() + 1;

    // Material count
    auto MaterialsCount = (uint16_t)Model->Materials.size();
    // Copy material names, count first
    std::memcpy(UnpackedBuffer + UnpackedOffset, &MaterialsCount, 2);
    
    // Increase
    UnpackedOffset += 2;

    // Iterate over materials
    for (auto& Material : Model->Materials)
    {
        // Copy name
        std::memcpy(UnpackedBuffer + UnpackedOffset, Material.MaterialName.c_str(), Material.MaterialName.size() + 1);
        // Increase
        UnpackedOffset += (uint32_t)Material.MaterialName.size() + 1;

        // Copy image names

        // Copy diffuse
        std::memcpy(UnpackedBuffer + UnpackedOffset, Material.DiffuseMapName.c_str(), Material.DiffuseMapName.size() + 1);
        // Increase
        UnpackedOffset += (uint32_t)Material.DiffuseMapName.size() + 1;

        // Copy normal
        std::memcpy(UnpackedBuffer + UnpackedOffset, Material.NormalMapName.c_str(), Material.NormalMapName.size() + 1);
        // Increase
        UnpackedOffset += (uint32_t)Material.NormalMapName.size() + 1;

        // Copy specular
        std::memcpy(UnpackedBuffer + UnpackedOffset, Material.SpecularMapName.c_str(), Material.SpecularMapName.size() + 1);
        // Increase
        UnpackedOffset += (uint32_t)Material.SpecularMapName.size() + 1;
    }

    // Compress the model data to a buffer
    auto PackedBuffer = new uint8_t[65535];
    // Result packed size
    auto PackedSize = 0;

    // Compress it
    PackedSize = Compression::CompressLZ4Block((const int8_t*)UnpackedBuffer, (int8_t*)PackedBuffer + 4, (uint32_t)UnpackedSize, 65531);

    // Clean up unpacked buffer
    delete[] UnpackedBuffer;

    // If we have data, write
    if (PackedSize > 0)
    {
        // Write the compressed data buffer:
        //    uint16_t PackedSize
        //    uint16_t UnpackedSize
        //    uint8_t* Buffer

        // Copy over data sizes
        std::memcpy(PackedBuffer, &PackedSize, 2);
        std::memcpy(PackedBuffer + 2, &UnpackedSize, 2);

        // Note: This did not need a lock because fwrite is actually atomic
        // So the order won't be the same, but that doesn't matter

        // Write buffer (+ 4 for size of the initial data)
        CacheHandle.Write((const int8_t*)PackedBuffer, PackedSize + 4);
    }

    // Clean up
    delete[] PackedBuffer;
}

void CoDGDTProcessor::ExportGameGDTs()
{
    // Make sure we're closed
    this->CloseProcessor();

    // Check if we need a GDT
    auto ExportBO3GDT = (SettingsManager::GetSetting("exportgdt_bo3", "true") == "true");
    auto ExportWAWGDT = (SettingsManager::GetSetting("exportgdt_waw", "false") == "true");
    auto ClearGDTCache = (SettingsManager::GetSetting("cleargdt_exit", "true") == "true");

    // Now, grab all cache files
    auto CacheFiles = FileSystems::GetFiles(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "gdt_cache"), "*.gdtc");

    // Continue for generation if need be
    if (ExportBO3GDT || ExportWAWGDT)
    {
        // A buffer for unique GDT data
        std::unordered_map<std::string, XModelInfo> XModelEntries;
        std::unordered_map<std::string, XMaterialInfo> XMaterialEntries;

        // Iterate and load
        for (auto& CacheFile : CacheFiles)
        {
            // Load GDT data
            auto Reader = BinaryReader();
            // Open file
            Reader.Open(CacheFile);

            // Verify magic
            if (Reader.Read<uint32_t>() == 0x43544447)
            {
                // Valid, skip version for now
                Reader.Advance(2);

                // Read until end
                while (Reader.GetPosition() != Reader.GetLength())
                {
                    // Read block size and unpacked size
                    auto PackedSize = Reader.Read<uint16_t>();
                    auto UnpackedSize = Reader.Read<uint16_t>();

                    // Result
                    uint64_t ReadResult = 0;
                    // Read data
                    auto PackedData = Reader.Read(PackedSize, ReadResult);

                    // Make sure we got it, then decompress
                    if (PackedData != nullptr)
                    {
                        // Prepare to decompress
                        auto UnpackedData = new int8_t[UnpackedSize];

                        // Decompress
                        auto ResultSize = Compression::DecompressLZ4Block((const int8_t*)PackedData, UnpackedData, PackedSize, UnpackedSize);

                        // Read if successful
                        if (ResultSize > 0)
                        {
                            // Parse the decompressed block (This auto-disposes of unpacked data)
                            auto BlockReader = MemoryReader(UnpackedData, ResultSize);

                            // Read Min / Max distance, then lod name
                            auto Min = BlockReader.Read<float>();
                            auto Max = BlockReader.Read<float>();

                            // Read lod name
                            auto LODName = BlockReader.ReadNullTerminatedString();
                            auto ModelName = LODName;

                            // Lod index
                            auto LODIndex = 0;

                            // Process lod information
                            auto LODNumberOffset = LODName.find("_LOD");

                            // Make sure we got it
                            if (LODNumberOffset != std::string::npos)
                            {
                                // We need to clean it, then attach a number
                                ModelName = LODName.substr(0, LODNumberOffset);
                                LODName = LODName.substr(LODNumberOffset);
                            }

                            // Set entry
                            XModelEntries[ModelName].LODEntries.emplace(LODName, Min, Max);

                            // Read material count
                            auto MaterialCount = BlockReader.Read<uint16_t>();

                            // Loop and read materials
                            for (uint32_t i = 0; i < MaterialCount; i++)
                            {
                                // Read material name
                                auto MaterialName = BlockReader.ReadNullTerminatedString();

                                // Read 3 material entry strings, diffuse, normal, specular
                                auto Diffuse = BlockReader.ReadNullTerminatedString();
                                auto Normal = BlockReader.ReadNullTerminatedString();
                                auto Specular = BlockReader.ReadNullTerminatedString();

                                // Apply material entry
                                XMaterialEntries[MaterialName].DiffuseMap = Diffuse;
                                XMaterialEntries[MaterialName].NormalMap = Normal;
                                XMaterialEntries[MaterialName].SpecularMap = Specular;
                            }
                        }
                        else
                        {
                            // Clean up, failed
                            delete[] UnpackedData;
                        }

                        // Clean up
                        delete[] PackedData;
                    }
                }
            }
        }

        // Prepare to output a formatted GDT
        FileSystems::CreateDirectory(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "exported_gdts"));

        // Export based on settings
        if (ExportBO3GDT) { ExportBO3GDTFile(XModelEntries, XMaterialEntries); }
        if (ExportWAWGDT) { ExportWAWGDTFile(XModelEntries, XMaterialEntries); }
    }

    // Check to clear cache
    if (ClearGDTCache)
    {
        // Clear all files
        for (auto& CacheFile : CacheFiles) { FileSystems::DeleteFile(CacheFile); }
    }
}

void CoDGDTProcessor::ReloadProcessor()
{
    // Reload
    if (this->CacheFileName != "")
    {
        // We can set it up again
        this->SetupProcessor(this->CacheFileName);
    }
}

void ExportWAWGDTFile(const std::unordered_map<std::string, XModelInfo>& ModelInfo, const std::unordered_map<std::string, XMaterialInfo>& MaterialEntries)
{
    // Prepare to format a WAW GDT
    auto Writer = TextWriter();

    // File name
    auto FileName = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "exported_gdts\\WraithWAW.gdt");

    // Check setting for unique file names (Don't overwrite)
    if (SettingsManager::GetSetting("overwrite_gdt", "true") == "false")
    {
        // We must find unique name
        uint32_t FileIndex = 0;
        // Loop
        while (FileSystems::FileExists(FileName))
        {
            // Increase
            FileIndex++;

            // Set
            FileName = FileSystems::CombinePath(FileSystems::GetApplicationPath(), Strings::Format("exported_gdts\\WraithWAW%d", FileIndex) + ".gdt");
        }
    }

    // Make the file
    Writer.Create(FileName);
    // Set buffering
    Writer.SetWriteBuffer(0x100000);

    // Output initial braces
    Writer.WriteLine("{");

    // Format models first
    for (auto& Model : ModelInfo)
    {
        // Start model block
        Writer.WriteLineFmt("\t\"%s\" ( \"xmodel.gdf\" )\n\t{", Model.first.c_str());

        // We need to export the lod entries for this model, but they also must be in proper order
        std::vector<XModelLodInfo> SortedLods;
        // Reserve space for 8 max (No resizing)
        SortedLods.reserve(8);

        // Sort them
        for (auto& LodEntry : Model.second.LODEntries)
        {
            SortedLods.emplace_back(LodEntry);
        }
        // Sort
        std::sort(SortedLods.begin(), SortedLods.end(), SortByLodDistance());

        // Output them (Max 4)
        for (uint32_t i = 0; i < (uint32_t)SortedLods.size() && i < 4; i++)
        {
            // Check i
            switch (i)
            {
            case 0:
                Writer.WriteLineFmt("\t\t\"filename\" \"%s\"\n\t\t\"highLodDist\" \"%f\"", (Model.first + "\\\\" + Model.first + SortedLods[i].LODSuffix + ".xmodel_export").c_str(), SortedLods[i].LodDistance);
                break;
            case 1:
                Writer.WriteLineFmt("\t\t\"mediumLod\" \"%s\"\n\t\t\"mediumLodDist\" \"%f\"", (Model.first + "\\\\" + Model.first + SortedLods[i].LODSuffix + ".xmodel_export").c_str(), SortedLods[i].LodDistance);
                break;
            case 2:
                Writer.WriteLineFmt("\t\t\"lowLod\" \"%s\"\n\t\t\"lowLodDist\" \"%f\"", (Model.first + "\\\\" + Model.first + SortedLods[i].LODSuffix + ".xmodel_export").c_str(), SortedLods[i].LodDistance);
                break;
            case 3:
                Writer.WriteLineFmt("\t\t\"lowestLod\" \"%s\"\n\t\t\"lowestLodDist\" \"%f\"", (Model.first + "\\\\" + Model.first + SortedLods[i].LODSuffix + ".xmodel_export").c_str(), SortedLods[i].LodDistance);
                break;
            }
        }

        // End model block
        Writer.WriteLine("\t\t\"type\" \"rigid\"\n\t}");
    }

    // Format materials next
    for (auto& Material : MaterialEntries)
    {
        // Export material
        Writer.WriteLineFmt("\t\"%s\" ( \"material.gdf\" )\n\t{\n\t\t\"materialType\" \"model phong\"\n\t\t\"blendFunc\" \"Replace*\"", Material.first.c_str());

        // We need to check image entries
        if (Material.second.DiffuseMap != "")
        {
            // Link diffuse
            Writer.WriteLineFmt("\t\t\"colorMap\" \"%s\"", Material.second.DiffuseMap.c_str());
        }
        if (Material.second.NormalMap != "")
        {
            // Link normal
            Writer.WriteLineFmt("\t\t\"normalMap\" \"%s\"", Material.second.NormalMap.c_str());
        }
        if (Material.second.SpecularMap != "")
        {
            // Link spec
            Writer.WriteLineFmt("\t\t\"specColorMap\" \"%s\"", Material.second.SpecularMap.c_str());
        }

        // End material block
        Writer.WriteLine("\t\t\"surfaceType\" \"<none>\"\n\t\t\"template\" \"material.template\"\n\t}");
    }

    // Output end brace
    Writer.WriteLine("}");
}

void ExportBO3GDTFile(const std::unordered_map<std::string, XModelInfo>& ModelInfo, const std::unordered_map<std::string, XMaterialInfo>& MaterialEntries)
{
    // Prepare to format a BO3 GDT
    auto Writer = TextWriter();

    // File name
    auto FileName = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "exported_gdts\\WraithBO3.gdt");

    // Check setting for unique file names (Don't overwrite)
    if (SettingsManager::GetSetting("overwrite_gdt", "true") == "false")
    {
        // We must find unique name
        uint32_t FileIndex = 0;
        // Loop
        while (FileSystems::FileExists(FileName))
        {
            // Increase
            FileIndex++;

            // Set
            FileName = FileSystems::CombinePath(FileSystems::GetApplicationPath(), Strings::Format("exported_gdts\\WraithBO3%d", FileIndex) + ".gdt");
        }
    }

    // Make the file
    Writer.Create(FileName);
    // Set buffering
    Writer.SetWriteBuffer(0x100000);

    // Output initial braces
    Writer.WriteLine("{");

    // Format models first
    for (auto& Model : ModelInfo)
    {
        // Start model block
        Writer.WriteLineFmt("\t\"%s\" ( \"xmodel.gdf\" )\n\t{", Model.first.c_str());

        // We need to export the lod entries for this model, but they also must be in proper order
        std::vector<XModelLodInfo> SortedLods;
        // Reserve space for 8 max (No resizing)
        SortedLods.reserve(8);

        // Sort them
        for (auto& LodEntry : Model.second.LODEntries)
        {
            SortedLods.emplace_back(LodEntry);
        }
        // Sort
        std::sort(SortedLods.begin(), SortedLods.end(), SortByLodDistance());

        // Output them (Max 4)
        for (uint32_t i = 0; i < (uint32_t)SortedLods.size() && i < 4; i++)
        {
            // Check i
            switch (i)
            {
            case 0:
                Writer.WriteLineFmt("\t\t\"filename\" \"%s\"\n\t\t\"highLodDist\" \"%f\"", (Model.first + "\\\\" + Model.first + SortedLods[i].LODSuffix + ".xmodel_bin").c_str(), SortedLods[i].LodDistance);
                break;
            case 1:
                Writer.WriteLineFmt("\t\t\"mediumLod\" \"%s\"\n\t\t\"mediumLodDist\" \"%f\"", (Model.first + "\\\\" + Model.first + SortedLods[i].LODSuffix + ".xmodel_bin").c_str(), SortedLods[i].LodDistance);
                break;
            case 2:
                Writer.WriteLineFmt("\t\t\"lowLod\" \"%s\"\n\t\t\"lowLodDist\" \"%f\"", (Model.first + "\\\\" + Model.first + SortedLods[i].LODSuffix + ".xmodel_bin").c_str(), SortedLods[i].LodDistance);
                break;
            case 3:
                Writer.WriteLineFmt("\t\t\"lowestLod\" \"%s\"\n\t\t\"lowestLodDist\" \"%f\"", (Model.first + "\\\\" + Model.first + SortedLods[i].LODSuffix + ".xmodel_bin").c_str(), SortedLods[i].LodDistance);
                break;
            }
        }

        // End model block
        Writer.WriteLine("\t\t\"type\" \"rigid\"\n\t}");
    }

    // Format materials next
    for (auto& Material : MaterialEntries)
    {
        // Export material
        Writer.WriteLineFmt("\t\"%s\" ( \"material.gdf\" )\n\t{\n\t\t\"materialType\" \"lit\"", Material.first.c_str());

        // We need to check image entries
        if (Material.second.DiffuseMap != "")
        {
            // Link diffuse
            Writer.WriteLineFmt("\t\t\"colorMap\" \"%s_c\"", Material.first.c_str());
        }
        if (Material.second.NormalMap != "")
        {
            // Link normal
            Writer.WriteLineFmt("\t\t\"normalMap\" \"%s_n\"", Material.first.c_str());
        }
        if (Material.second.SpecularMap != "")
        {
            // Link spec
            Writer.WriteLineFmt("\t\t\"specColorMap\" \"%s_s\"", Material.first.c_str());
        }

        // End material block
        Writer.WriteLine("\t\t\"surfaceType\" \"<none>\"\n\t\t\"template\" \"material.template\"\n\t}");

        // Check image entries
        if (Material.second.DiffuseMap != "")
        {
            // Append diffuse block
            Writer.WriteLineFmt("\t\"%s_c\" ( \"image.gdf\" )\n\t{\n\t\t\"baseImage\" \"%s\"", Material.first.c_str(), Material.second.DiffuseMap.c_str());
            // End diffuse block
            Writer.WriteLine("\t\t\"semantic\" \"diffuseMap\"\n\t\t\"imageType\" \"Texture\"\n\t\t\"compressionMethod\" \"compressed high color\"\n\t\t\"type\" \"image\"\n\t}");
        }
        if (Material.second.NormalMap != "")
        {
            // Append normal block
            Writer.WriteLineFmt("\t\"%s_n\" ( \"image.gdf\" )\n\t{\n\t\t\"baseImage\" \"%s\"", Material.first.c_str(), Material.second.NormalMap.c_str());
            // End normal block
            Writer.WriteLine("\t\t\"semantic\" \"normalMap\"\n\t\t\"imageType\" \"Texture\"\n\t\t\"compressionMethod\" \"compressed\"\n\t\t\"type\" \"image\"\n\t}");
        }
        if (Material.second.SpecularMap != "")
        {
            // Append specular block
            Writer.WriteLineFmt("\t\"%s_s\" ( \"image.gdf\" )\n\t{\n\t\t\"baseImage\" \"%s\"", Material.first.c_str(), Material.second.SpecularMap.c_str());
            // End specular block
            Writer.WriteLine("\t\t\"semantic\" \"specularMap\"\n\t\t\"imageType\" \"Texture\"\n\t\t\"compressionMethod\" \"compressed\"\n\t\t\"type\" \"image\"\n\t}");
        }
    }

    // Output end brace
    Writer.WriteLine("}");
}