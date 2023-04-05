#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <array>

// We need the DBGameAssets and CoDAssetType classes
#include "DBGameAssets.h"
#include "CoDAssetType.h"
#include "WraithModel.h"

// Handles reading from Black Ops 3
class GameBlackOps3
{
public:
    // -- Game functions

    // Loads offsets for Black Ops 3
    static bool LoadOffsets();
    // Loads assets for Black Ops 3
    static bool LoadAssets();

    // Reads an XAnim from Black Ops 3
    static std::unique_ptr<XAnim_t> ReadXAnim(const CoDAnim_t* Animation);
    // Reads a XModel from Black Ops 3
    static std::unique_ptr<XModel_t> ReadXModel(const CoDModel_t* Model);
    // Reads a XImage from Black Ops 3
    static std::unique_ptr<XImageDDS> ReadXImage(const CoDImage_t* Image);

    // Reads an XImageDDS from a image reference from Black Ops 3
    static std::unique_ptr<XImageDDS> LoadXImage(const XImage_t& Image);
    // Loads a streamed XModel lod, streaming from cache if need be
    static void LoadXModel(const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel);

    // Reads a string via it's string index for Black Ops 3
    static std::string LoadStringEntry(uint64_t Index);

private:
    // -- Game offsets databases

    // A list of offsets for Black Ops 3 single player
    static std::array<DBGameInfo, 8> SinglePlayerOffsets;

    // -- Game utilities

    // Reads an XMaterial from it's logical offset in memory
    static const XMaterial_t ReadXMaterial(uint64_t MaterialPointer);
};