#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <array>

// We need the DBGameAssets and CoDAssetType classes
#include "DBGameAssets.h"
#include "CoDAssetType.h"
#include "WraithModel.h"

// Handles reading from World War 2
class GameWorldWar2
{
public:
    // -- Game functions

    // Loads offsets for World War 2
    static bool LoadOffsets();
    // Loads assets for World War 2
    static bool LoadAssets();

    // Reads an XAnim from World War 2
    static std::unique_ptr<XAnim_t> ReadXAnim(const CoDAnim_t* Animation);
    // Reads a XModel from World War 2
    static std::unique_ptr<XModel_t> ReadXModel(const CoDModel_t* Model);
    // Reads a XImage from World War 2
    static std::unique_ptr<XImageDDS> ReadXImage(const CoDImage_t* Image);
    // Reads an XSound from World War 2
    static std::unique_ptr<XSound> ReadXSound(const CoDSound_t* Sound);
    // Reads an XMaterial from it's logical offset in memory
    static const XMaterial_t ReadXMaterial(uint64_t MaterialPointer);

    // Reads an XImageDDS from a image reference from World War 2
    static std::unique_ptr<XImageDDS> LoadXImage(const XImage_t& Image);
    // Loads a streamed XModel lod, streaming from cache if need be
    static void LoadXModel(const std::unique_ptr<XModel_t>& Model, const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel);

    // Reads a string via it's string index for World War 2
    static std::string LoadStringEntry(uint64_t Index);

private:
    // -- Game offsets databases

    // A list of offsets for World War 2 single player
    static std::array<DBGameInfo, 8> SinglePlayerOffsets;
    // A list of offsets for World War 2 multi player
    static std::array<DBGameInfo, 8> MultiPlayerOffsets;
};