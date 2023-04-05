#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <array>

// We need the DBGameAssets and CoDAssetType classes
#include "DBGameAssets.h"
#include "CoDAssetType.h"

// Handles reading from Advanced Warfare
class GameAdvancedWarfare
{
public:
    // -- Game functions

    // Loads offsets for Advanced Warfare
    static bool LoadOffsets();
    // Loads assets for Advanced Warfare
    static bool LoadAssets();
    // Loads assets for Advanced Warfare (Parasyte)
    static bool LoadAssetsPS();

    // Reads an XAnim from Advanced Warfare
    static std::unique_ptr<XAnim_t> ReadXAnim(const CoDAnim_t* Animation);
    // Reads a XModel from Advanced Warfare
    static std::unique_ptr<XModel_t> ReadXModel(const CoDModel_t* Model);
    // Reads a XImage from Advanced Warfare
    static std::unique_ptr<XImageDDS> ReadXImage(const CoDImage_t* Image);

    // Reads an XImageDDS from a image reference from Advanced Warfare
    static std::unique_ptr<XImageDDS> LoadXImage(const XImage_t& Image);
    // Reads an XImageDDS from a image reference from Advanced Warfare (Parasyte)
    static std::unique_ptr<XImageDDS> LoadXImagePS(const XImage_t& Image);

    // Reads a string via it's string index for Advanced Warfare
    static std::string LoadStringEntry(uint64_t Index);

private:
    // -- Game offsets databases

    // A list of offsets for Advanced Warfare single player
    static std::array<DBGameInfo, 1> SinglePlayerOffsets;
    // A list of offsets for Advanced Warfare multi player
    static std::array<DBGameInfo, 1> MultiPlayerOffsets;
    
    // -- Game utilities

    // Reads an XMaterial from it's logical offset in memory
    static const XMaterial_t ReadXMaterial(uint64_t MaterialPointer);
};