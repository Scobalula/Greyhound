#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <array>

// We need the DBGameAssets and CoDAssetType classes
#include "DBGameAssets.h"
#include "CoDAssetType.h"

// Handles reading from Ghosts
class GameGhosts
{
public:
    // -- Game functions

    // Loads offsets for Ghosts
    static bool LoadOffsets();
    // Loads assets for Ghosts
    static bool LoadAssets();

    // Reads an XAnim from Ghosts
    static std::unique_ptr<XAnim_t> ReadXAnim(const CoDAnim_t* Animation);
    // Reads a XModel from Ghosts
    static std::unique_ptr<XModel_t> ReadXModel(const CoDModel_t* Model);
    // Reads a XImage from Ghosts
    static std::unique_ptr<XImageDDS> ReadXImage(const CoDImage_t* Image);

    // Reads an XImageDDS from a image reference from Ghosts
    static std::unique_ptr<XImageDDS> LoadXImage(const XImage_t& Image);

    // Reads a string via it's string index for Ghosts
    static std::string LoadStringEntry(uint64_t Index);

private:
    // -- Game offsets databases

    // A list of offsets for Ghosts single player
    static std::array<DBGameInfo, 2> SinglePlayerOffsets;
    // A list of offsets for Ghosts multi player
    static std::array<DBGameInfo, 2> MultiPlayerOffsets;

    // -- Game utilities

    // Reads an XMaterial from it's logical offset in memory
    static const XMaterial_t ReadXMaterial(uint64_t MaterialPointer);
};