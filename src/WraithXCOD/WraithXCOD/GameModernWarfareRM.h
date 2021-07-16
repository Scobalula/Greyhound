#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <array>

// We need the DBGameAssets and CoDAssetType classes
#include "DBGameAssets.h"
#include "CoDAssetType.h"

// Handles reading from Modern Warfare RM
class GameModernWarfareRM
{
public:
    // -- Game functions

    // Loads offsets for Modern Warfare RM
    static bool LoadOffsets();
    // Loads assets for Modern Warfare RM
    static bool LoadAssets();

    // Reads an XAnim from Modern Warfare RM
    static std::unique_ptr<XAnim_t> ReadXAnim(const CoDAnim_t* Animation);
    // Reads a XModel from Modern Warfare RM
    static std::unique_ptr<XModel_t> ReadXModel(const CoDModel_t* Model);
    // Reads a XImage from Modern Warfare RM
    static std::unique_ptr<XImageDDS> ReadXImage(const CoDImage_t* Image);

    // Reads an XImageDDS from a image reference from Modern Warfare RM
    static std::unique_ptr<XImageDDS> LoadXImage(const XImage_t& Image);


    // Reads an XMaterial from it's logical offset in memory
    static XMaterial_t ReadXMaterial(uint64_t MaterialPointer);

    // Reads a string via it's string index for Modern Warfare RM
    static std::string LoadStringEntry(uint64_t Index);

private:
    // -- Game offsets databases

    // A list of offsets for Modern Warfare RM single player
    static std::array<DBGameInfo, 1> SinglePlayerOffsets;
    // A list of offsets for Modern Warfare RM multi player
    static std::array<DBGameInfo, 1> MultiPlayerOffsets;

    // -- Game utilities
};