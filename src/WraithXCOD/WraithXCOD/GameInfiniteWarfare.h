#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <array>

// We need the DBGameAssets and CoDAssetType classes
#include "DBGameAssets.h"
#include "CoDAssetType.h"

// Handles reading from Infinite Warfare
class GameInfiniteWarfare
{
public:
    // -- Game functions

    // Loads offsets for Infinite Warfare
    static bool LoadOffsets();
    // Loads assets for Infinite Warfare
    static bool LoadAssets();

    // Reads an XAnim from Infinite Warfare
    static std::unique_ptr<XAnim_t> ReadXAnim(const CoDAnim_t* Animation);
    // Reads a XModel from Infinite Warfare
    static std::unique_ptr<XModel_t> ReadXModel(const CoDModel_t* Model);
    // Reads a XImage from Infinite Warfare
    static std::unique_ptr<XImageDDS> ReadXImage(const CoDImage_t* Image);

    // Reads an XImageDDS from a image reference from Infinite Warfare
    static std::unique_ptr<XImageDDS> LoadXImage(const XImage_t& Image);

    // Reads a string via it's string index for Infinite Warfare
    static std::string LoadStringEntry(uint64_t Index);

private:
    // -- Game offsets databases

    // A list of offsets for Infinite Warfare single player
    static std::array<DBGameInfo, 6> SinglePlayerOffsets;

    // -- Game utilities

    // Reads an XMaterial from it's logical offset in memory
    static const XMaterial_t ReadXMaterial(uint64_t MaterialPointer);
};