#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <array>

// We need the DBGameAssets and CoDAssetType classes
#include "DBGameAssets.h"
#include "CoDAssetType.h"
#include "WraithModel.h"
#include "WraithNameIndex.h"

// Handles reading from Black Ops CW
class GameBlackOpsCW
{
public:
    // -- Game Name Caches
    static WraithNameIndex AssetNameCache;
    static WraithNameIndex StringCache;

    // -- Game functions

    // Loads offsets for Black Ops CW
    static bool LoadOffsets();
    // Loads assets for Black Ops CW
    static bool LoadAssets();

    // Reads an XAnim from Black Ops CW
    static std::unique_ptr<XAnim_t> ReadXAnim(const CoDAnim_t* Animation);
    // Reads a XModel from Black Ops CW
    static std::unique_ptr<XModel_t> ReadXModel(const CoDModel_t* Model);
    // Reads a XImage from Black Ops CW
    static std::unique_ptr<XImageDDS> ReadXImage(const CoDImage_t* Image);
    // Reads an XSound from World War 2
    static std::unique_ptr<XSound> ReadXSound(const CoDSound_t* Sound);
    // Reads an XMaterial from it's logical offset in memory
    static const XMaterial_t ReadXMaterial(uint64_t MaterialPointer);
    // Reads an XImageDDS from a image reference from Black Ops CW
    static std::unique_ptr<XImageDDS> LoadXImage(const XImage_t& Image);
    // Loads a streamed XModel lod, streaming from cache if need be
    static void LoadXModel(const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel);

    // String Handlers for Black Ops CW
    static std::string LoadStringEntry(uint64_t Index);

    // Perform setup required before ripping
    static void PerformInitialSetup();

private:
    // -- Game offsets databases

    // A list of offsets for Black Ops CW single player
    static std::array<DBGameInfo, 1> SinglePlayerOffsets;

    // -- Game utilities
};