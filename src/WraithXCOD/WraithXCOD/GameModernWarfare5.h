#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <array>

// We need the DBGameAssets and CoDAssetType classes
#include "DBGameAssets.h"
#include "CoDAssetType.h"
#include "WraithModel.h"
#include "WraithAnim.h"
#include "WraithNameIndex.h"

// We need the XModel Translator 
#include "CoDXModelTranslator.h"

// We need the following WraithX classes
#include "MemoryReader.h"

// Handles reading from Modern Warfare 5
class GameModernWarfare5
{
public:
    // -- Game functions

    // Loads offsets for Modern Warfare 5
    static bool LoadOffsets();
    // Loads assets for Modern Warfare 5
    static bool LoadAssets();

    // Reads an XAnim from Modern Warfare 5
    static std::unique_ptr<XAnim_t> ReadXAnim(const CoDAnim_t* Animation);
    // Reads a XModel from Modern Warfare 5
    static std::unique_ptr<XModel_t> ReadXModel(const CoDModel_t* Model);
    // Reads a XImage from Modern Warfare 5
    static std::unique_ptr<XImageDDS> ReadXImage(const CoDImage_t* Image);
    // Reads an XSound from Modern Warfare 5
    static std::unique_ptr<XSound> ReadXSound(const CoDSound_t* Sound);
    // Reads an XMaterial from it's logical offset in memory
    static const XMaterial_t ReadXMaterial(uint64_t MaterialPointer);
    // Reads a XImage from Modern Warfare 5
    static void TranslateRawfile(const CoDRawFile_t* Rawfile, const std::string& ExportPath);

    // Reads an XImageDDS from a image reference from Modern Warfare 5
    static std::unique_ptr<XImageDDS> LoadXImage(const XImage_t& Image);
    // Loads a streamed XModel lod, streaming from cache if need be.
    static void LoadXModel(const std::unique_ptr<XModel_t>& Model, const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel);
    // Loads a streamed XAnim, streaming from cache if need be.
    static void LoadXAnim(const std::unique_ptr<XAnim_t>& Anim, std::unique_ptr<WraithAnim>& ResultAnim);

    // String Handlers for Modern Warfare 5
    static std::string LoadStringEntry(uint64_t Index);

    // Perform setup required before ripping
    static void PerformInitialSetup();
    // Perform setup required after game is closed.
    static void PerformShutDown();
private:

    // Prepares Vertex Weights
    static void PrepareVertexWeights(MemoryReader& ComplexReader, std::vector<WeightsData>& Weights, const XModelSubmesh_t& Submesh);
};