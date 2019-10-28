#include "stdafx.h"

// The class we are implementing
#include "GameModernWarfare4.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDRawImageTranslator.h"
#include "CoDXPoolParser.h"

// We need the DirectX Mesh Class
#include "..\..\DirectXMesh\DirectXMesh.h"

// We need the following WraithX classes
#include "Strings.h"
#include "FileSystems.h"
#include "MemoryReader.h"
#include "SettingsManager.h"
#include "HalfFloats.h"

// -- Initialize built-in game offsets databases

// Modern Warfare 4 SP
std::array<DBGameInfo, 1> GameModernWarfare4::SinglePlayerOffsets =
{{
    { 0xAE9EAF0, 0x0, 0xC8A0300, 0x0 }
}};

// -- Finished with databases

// -- Begin XModelStream structures

struct MW4GfxRigidVerts
{
    uint16_t BoneIndex;
    uint16_t VertexCount;
    uint16_t FacesCount;
    uint16_t FacesIndex;
};

#pragma pack(push, 1)
struct MW4GfxStreamVertex
{
    uint64_t PackedPosition; // Packed 21bits, scale + offset in Mesh Info
    uint32_t BiNormal; // Not vertex normal, value matches IW
    uint16_t UVUPosition;
    uint16_t UVVPosition;
    // Check Offsets 4 and 5 in the Submesh, not always present but are 4 bytes per 
    // vertex chunks of data, Offset 4 seems to be a UV but 4 is worth checking more
    // This could be the normal, value is the same across vertices that are the same in IW
    int32_t VertexNormal;
};
#pragma pack(pop)

struct MW4GfxStreamFace
{
    uint16_t Index1;
    uint16_t Index2;
    uint16_t Index3;
};

// -- End XModelStream structures

// -- Modern Warfare 4 Pool Data Structure

struct MW4XAssetPoolData
{
    // The beginning of the pool
    uint64_t PoolPtr;

    // A pointer to the closest free header
    uint64_t PoolFreeHeadPtr;

    // The maximum pool size
    uint32_t PoolSize;

    // The size of the asset header
    uint32_t AssetSize;
};

// Verify that our pool data is exactly 0x20
static_assert(sizeof(MW4XAssetPoolData) == 0x18, "Invalid Pool Data Size (Expected 0x18)");

bool GameModernWarfare4::LoadOffsets()
{
    // ----------------------------------------------------
    //    Modern Warfare 4 pools and sizes, XAssetPoolData is an array of pool info for each asset pool in the game
    //    The index of the assets we use are as follows: xanim (3), xmodel (4), ximage (0x9)
    //    Index * sizeof(MWXAssetPoolData) = the offset of the asset info in this array of data, we can verify it using the xmodel pool and checking for the model hash (0x04647533e968c910)
    //  Notice: Modern Warfare 4 doesn't store a freePoolHandle at the beginning, so we just read on.
    //    On Modern Warfare 4, (0x04647533e968c910) will be the first xmodel
    //    Modern Warfare 4 stringtable, check entries, results may vary
    //    Reading is: (StringIndex * 28) + StringTablePtr + 8
    // ----------------------------------------------------

    // Attempt to load the game offsets
    if (CoDAssets::GameInstance != nullptr)
    {
        // We need the base address of the MW4 Module for ASLR + Heuristics
        auto BaseAddress = CoDAssets::GameInstance->GetMainModuleAddress();

        // Check built-in offsets via game exe mode (SP)
        for (auto& GameOffsets : SinglePlayerOffsets)
        {
            // Read required offsets (XANIM, XMODEL, XIMAGE, RAWFILE RELATED...)
            auto AnimPoolData = CoDAssets::GameInstance->Read<MW4XAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(MW4XAssetPoolData) * 7));
            auto ModelPoolData = CoDAssets::GameInstance->Read<MW4XAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(MW4XAssetPoolData) * 9));
            auto ImagePoolData = CoDAssets::GameInstance->Read<MW4XAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(MW4XAssetPoolData) * 19));

            // Apply game offset info
            CoDAssets::GameOffsetInfos.emplace_back(AnimPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ModelPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ImagePoolData.PoolPtr);

            // Verify via first xmodel asset
            auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1]));
            // Check
            if (FirstXModelName == "axis_guide_createfx")
            {
                // Verify string table, otherwise we are all set
                CoDAssets::GameOffsetInfos.emplace_back(BaseAddress + GameOffsets.StringTable);
                // Read and apply sizes
                CoDAssets::GamePoolSizes.emplace_back(AnimPoolData.PoolSize);
                CoDAssets::GamePoolSizes.emplace_back(ModelPoolData.PoolSize);
                CoDAssets::GamePoolSizes.emplace_back(ImagePoolData.PoolSize);
                // Return success
                return true;
            }
            // Reset
            CoDAssets::GameOffsetInfos.clear();
        }

        // Attempt to locate via heuristic searching (Note: As of right now, none of the functions got inlined)
        auto DBAssetsScan      = CoDAssets::GameInstance->Scan("48 8D 04 40 4C 8D 8E ?? ?? ?? ?? 4D 8D 0C C1 8D 42 FF");
        auto StringTableScan   = CoDAssets::GameInstance->Scan("4C 8B C2 48 8D 1D ?? ?? ?? ?? 48 2B CB");

        // Check that we had hits
        if (DBAssetsScan > 0 && StringTableScan > 0)
        {
            // Load info and verify
            auto GameOffsets = DBGameInfo(
                // Resolve pool info from LEA
                CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan + 0x7) + BaseAddress,
                // We don't use size offsets
                0,
                // Resolve strings from LEA
                CoDAssets::GameInstance->Read<uint32_t>(StringTableScan + 0x6) + (StringTableScan + 0xA),
                // We don't use package offsets
                0
            );

            // In debug, print the info for easy additions later!
#if _DEBUG
            // Format the output
            printf("Heuristic: { 0x%llX, 0x0, 0x%llX, 0x0 }\n", (GameOffsets.DBAssetPools - BaseAddress), (GameOffsets.StringTable - BaseAddress));
#endif

            // Read required offsets (XANIM, XMODEL, XIMAGE)
            auto AnimPoolData = CoDAssets::GameInstance->Read<MW4XAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(MW4XAssetPoolData) * 7));
            auto ModelPoolData = CoDAssets::GameInstance->Read<MW4XAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(MW4XAssetPoolData) * 9));
            auto ImagePoolData = CoDAssets::GameInstance->Read<MW4XAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(MW4XAssetPoolData) * 19));

            // Apply game offset info
            CoDAssets::GameOffsetInfos.emplace_back(AnimPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ModelPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ImagePoolData.PoolPtr);

            // Verify via first xmodel asset
            auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1]));
            // Check
            if (FirstXModelName == "axis_guide_createfx")
            {
                // Verify string table, otherwise we are all set
                CoDAssets::GameOffsetInfos.emplace_back(BaseAddress + GameOffsets.StringTable);
                // Read and apply sizes
                CoDAssets::GamePoolSizes.emplace_back(AnimPoolData.PoolSize);
                CoDAssets::GamePoolSizes.emplace_back(ModelPoolData.PoolSize);
                CoDAssets::GamePoolSizes.emplace_back(ImagePoolData.PoolSize);
                // Return success
                return true;
            }
            // Reset
            CoDAssets::GameOffsetInfos.clear();
        }
    }

    // Failed
    return false;
}

bool GameModernWarfare4::LoadAssets()
{
    // Prepare to load game assets, into the AssetPool
    bool NeedsAnims = (SettingsManager::GetSetting("showxanim", "true") == "true");
    bool NeedsModels = (SettingsManager::GetSetting("showxmodel", "true") == "true");
    bool NeedsImages = (SettingsManager::GetSetting("showximage", "false") == "true");

    // Check if we need assets
    if (NeedsAnims)
    {
        // Animations are the first offset and first pool, skip 8 byte pointer to free head
        auto AnimationOffset = CoDAssets::GameOffsetInfos[0];
        auto AnimationCount = CoDAssets::GamePoolSizes[0];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (AnimationCount * sizeof(MW4XAnim)) + AnimationOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[0];

        // Store the placeholder anim
        MW4XAnim PlaceholderAnim;
        // Clear it out
        std::memset(&PlaceholderAnim, 0, sizeof(PlaceholderAnim));

        // Loop and read
        for (uint32_t i = 0; i < AnimationCount; i++)
        {
            // Read
            auto AnimResult = CoDAssets::GameInstance->Read<MW4XAnim>(AnimationOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((AnimResult.NamePtr > MinimumPoolOffset && AnimResult.NamePtr < MaximumPoolOffset) || AnimResult.NamePtr == 0)
            {
                // Advance
                AnimationOffset += sizeof(MW4XAnim);
                // Skip this asset
                continue;
            }

            // Validate and load if need be
            auto AnimName = CoDAssets::GameInstance->ReadNullTerminatedString(AnimResult.NamePtr);

            // Make and add
            auto LoadedAnim = new CoDAnim_t();
            // Set
            LoadedAnim->AssetName = AnimName;
            LoadedAnim->AssetPointer = AnimationOffset;
            LoadedAnim->Framerate = AnimResult.Framerate;
            LoadedAnim->FrameCount = AnimResult.NumFrames;
            LoadedAnim->AssetStatus = WraithAssetStatus::Loaded;

            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedAnim);

            // Advance
            AnimationOffset += sizeof(MW4XAnim);
        }
    }

    if (NeedsModels)
    {
        // Models are the second offset and second pool, skip 8 byte pointer to free head
        auto ModelOffset = CoDAssets::GameOffsetInfos[1];
        auto ModelCount = CoDAssets::GamePoolSizes[1];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (ModelCount * sizeof(MW4XModel)) + ModelOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[1];

        // Store the placeholder model
        MW4XModel PlaceholderModel;
        // Clear it out
        std::memset(&PlaceholderModel, 0, sizeof(PlaceholderModel));

        // Loop and read
        for (uint32_t i = 0; i < ModelCount; i++)
        {
            // Read
            auto ModelResult = CoDAssets::GameInstance->Read<MW4XModel>(ModelOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((ModelResult.NamePtr > MinimumPoolOffset && ModelResult.NamePtr < MaximumPoolOffset) || ModelResult.NamePtr == 0)
            {
                // Advance
                ModelOffset += sizeof(MW4XModel);
                // Skip this asset
                continue;
            }

            // Validate and load if need be
            auto ModelName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(ModelResult.NamePtr));

            // Make and add
            auto LoadedModel = new CoDModel_t();
            // Set
            LoadedModel->AssetName = ModelName;
            LoadedModel->AssetPointer = ModelOffset;
            LoadedModel->BoneCount = ModelResult.NumBones + ModelResult.UnkBoneCount;
            LoadedModel->LodCount = ModelResult.NumLods;

            // Check placeholder configuration, "empty_model" is the base xmodel swap
            if (ModelName == "void")
            {
                // Set as placeholder model
                PlaceholderModel = ModelResult;
                LoadedModel->AssetStatus = WraithAssetStatus::Placeholder;
            }
            else if ((ModelResult.BoneIDsPtr == PlaceholderModel.BoneIDsPtr && ModelResult.ParentListPtr == PlaceholderModel.ParentListPtr && ModelResult.RotationsPtr == PlaceholderModel.RotationsPtr && ModelResult.TranslationsPtr == PlaceholderModel.TranslationsPtr && ModelResult.PartClassificationPtr == PlaceholderModel.PartClassificationPtr && ModelResult.BaseMatriciesPtr == PlaceholderModel.BaseMatriciesPtr && ModelResult.NumLods == PlaceholderModel.NumLods && ModelResult.MaterialHandlesPtr == PlaceholderModel.MaterialHandlesPtr && ModelResult.NumBones == PlaceholderModel.NumBones) || ModelResult.NumLods == 0)
            {
                // Set as placeholder, data matches void, or the model has no lods
                LoadedModel->AssetStatus = WraithAssetStatus::Placeholder;
            }
            else
            {
                // Set
                LoadedModel->AssetStatus = WraithAssetStatus::Loaded;
            }

            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedModel);

            // Advance
            ModelOffset += sizeof(MW4XModel);
        }
    }

    if (NeedsImages)
    {
        // Images are the third offset and third pool, skip 8 byte pointer to free head
        auto ImageOffset = CoDAssets::GameOffsetInfos[2];
        auto ImageCount = CoDAssets::GamePoolSizes[2];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (ImageCount * sizeof(MW4GfxImage)) + ImageOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[2];

        // Loop and read
        for (uint32_t i = 0; i < ImageCount; i++)
        {
            // Read
            auto ImageResult = CoDAssets::GameInstance->Read<MW4GfxImage>(ImageOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((ImageResult.NamePtr > MinimumPoolOffset && ImageResult.NamePtr < MaximumPoolOffset) || ImageResult.NamePtr == 0)
            {
                // Advance
                ImageOffset += sizeof(MW4GfxImage);
                // Skip this asset
                continue;
            }

            // Validate and load if need be
            auto ImageName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(ImageResult.NamePtr));

            // Check if it's streamed
            if(ImageResult.LoadedMipLevels > 0)
            {

                // Make and add
                auto LoadedImage = new CoDImage_t();
                // Set
                LoadedImage->AssetName = ImageName;
                LoadedImage->AssetPointer = ImageOffset;
                LoadedImage->Width = (uint16_t)ImageResult.LoadedMipWidth;
                LoadedImage->Height = (uint16_t)ImageResult.LoadedMipHeight;
                LoadedImage->Format = ImageResult.ImageFormat;
                LoadedImage->AssetStatus = WraithAssetStatus::Loaded;

                // Add
                CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
            }

            // Advance
            ImageOffset += sizeof(MW4GfxImage);
        }
    }

    // Success, error only on specific load
    return true;
}

std::unique_ptr<XAnim_t> GameModernWarfare4::ReadXAnim(const CoDAnim_t* Animation)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Prepare to read the xanim
        auto Anim = std::make_unique<XAnim_t>();

        // Read the XAnim structure
        auto AnimData = CoDAssets::GameInstance->Read<MW4XAnim>(Animation->AssetPointer);

        // Copy over default properties
        Anim->AnimationName = Animation->AssetName;
        // Frames and Rate
        Anim->FrameCount = AnimData.NumFrames;
        Anim->FrameRate = AnimData.Framerate;

        // Check for viewmodel animations
        //if ((_strnicmp(Animation->AssetName.c_str(), "viewmodel_", 10) == 0) || (_strnicmp(Animation->AssetName.c_str(), "vm_", 3) == 0))
        //{
        //    // This is a viewmodel animation
        //    Anim->ViewModelAnimation = true;
        //}

        // Check for looping
        Anim->LoopingAnimation = (AnimData.Flags & 1);

        // Read the delta data
        auto AnimDeltaData = CoDAssets::GameInstance->Read<MW4XAnimDeltaParts>(AnimData.DeltaPartsPtr);

        // Copy over pointers
        Anim->BoneIDsPtr = AnimData.BoneIDsPtr;
        Anim->DataBytesPtr = AnimData.DataBytePtr;
        Anim->DataShortsPtr = AnimData.DataShortPtr;
        Anim->DataIntsPtr = AnimData.DataIntPtr;
        Anim->RandomDataBytesPtr = AnimData.RandomDataBytePtr;
        Anim->RandomDataShortsPtr = AnimData.RandomDataShortPtr;
        Anim->RandomDataIntsPtr = AnimData.RandomDataIntPtr;
        Anim->LongIndiciesPtr = AnimData.LongIndiciesPtr;
        Anim->NotificationsPtr = AnimData.NotificationsPtr;

        // Bone ID index size
        Anim->BoneIndexSize = 4;

        // Copy over counts
        Anim->NoneRotatedBoneCount = AnimData.NoneRotatedBoneCount;
        Anim->TwoDRotatedBoneCount = AnimData.TwoDRotatedBoneCount;
        Anim->NormalRotatedBoneCount = AnimData.NormalRotatedBoneCount;
        Anim->TwoDStaticRotatedBoneCount = AnimData.TwoDStaticRotatedBoneCount;
        Anim->NormalStaticRotatedBoneCount = AnimData.NormalStaticRotatedBoneCount;
        Anim->NormalTranslatedBoneCount = AnimData.NormalTranslatedBoneCount;
        Anim->PreciseTranslatedBoneCount = AnimData.PreciseTranslatedBoneCount;
        Anim->StaticTranslatedBoneCount = AnimData.StaticTranslatedBoneCount;
        Anim->NoneTranslatedBoneCount = AnimData.NoneTranslatedBoneCount;
        Anim->TotalBoneCount = AnimData.TotalBoneCount;
        Anim->NotificationCount = AnimData.NotificationCount;

        // Copy delta
        Anim->DeltaTranslationPtr = AnimDeltaData.DeltaTranslationsPtr;
        Anim->Delta2DRotationsPtr = AnimDeltaData.Delta2DRotationsPtr;
        Anim->Delta3DRotationsPtr = AnimDeltaData.Delta3DRotationsPtr;

        // Set types, we use dividebysize for MW4
        Anim->RotationType = AnimationKeyTypes::DivideBySize;
        Anim->TranslationType = AnimationKeyTypes::MinSizeTable;

        // Modern Warfare 4 supports inline indicies
        Anim->SupportsInlineIndicies = true;

        // Return it
        return Anim;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XModel_t> GameModernWarfare4::ReadXModel(const CoDModel_t* Model)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Read the XModel structure
        auto ModelData = CoDAssets::GameInstance->Read<MW4XModel>(Model->AssetPointer);

        // Prepare to read the xmodel (Reserving space for lods)
        auto ModelAsset = std::make_unique<XModel_t>(ModelData.NumLods);

        // Copy over default properties
        ModelAsset->ModelName = Model->AssetName;
        // Bone counts
        ModelAsset->BoneCount = ModelData.NumBones + ModelData.UnkBoneCount;
        ModelAsset->RootBoneCount = ModelData.NumRootBones;

        // Bone data type
        ModelAsset->BoneRotationData = BoneDataTypes::DivideBySize;

        // We are streamed
        ModelAsset->IsModelStreamed = true;

        // Bone id info
        ModelAsset->BoneIDsPtr = ModelData.BoneIDsPtr;
        ModelAsset->BoneIndexSize = 4;

        // Bone parent info
        ModelAsset->BoneParentsPtr = ModelData.ParentListPtr;
        ModelAsset->BoneParentSize = 1;

        // Local bone pointers
        ModelAsset->RotationsPtr = ModelData.RotationsPtr;
        ModelAsset->TranslationsPtr = ModelData.TranslationsPtr;

        // Global matricies
        ModelAsset->BaseMatriciesPtr = ModelData.BaseMatriciesPtr;

        // Prepare to parse lods
        for (uint32_t i = 0; i < ModelData.NumLods; i++)
        {
            // Create the lod and grab reference
            ModelAsset->ModelLods.emplace_back(ModelData.ModelLods[i].NumSurfs);
            // Grab reference
            auto& LodReference = ModelAsset->ModelLods[i];

            // Set distance
            LodReference.LodDistance = ModelData.ModelLods[i].LodDistance;

            // Set stream key and info ptr
            LodReference.LODStreamInfoPtr = ModelData.ModelLods[i].MeshPtr;

            // Grab pointer from the lod itself
            auto XSurfacePtr = ModelData.ModelLods[i].SurfsPtr;

            // Load surfaces
            for (uint32_t s = 0; s < ModelData.ModelLods[i].NumSurfs; s++)
            {
                // Create the surface and grab reference
                LodReference.Submeshes.emplace_back();
                // Grab reference
                auto& SubmeshReference = LodReference.Submeshes[s];

                // Read the surface data
                auto SurfaceInfo = CoDAssets::GameInstance->Read<MW4XModelSurface>(XSurfacePtr);

                // Apply surface info
                SubmeshReference.RigidWeightsPtr = SurfaceInfo.RigidWeightsPtr;
                SubmeshReference.VertListcount   = SurfaceInfo.VertListCount;
                SubmeshReference.VertexCount     = SurfaceInfo.VertexCount;
                SubmeshReference.FaceCount       = SurfaceInfo.FacesCount;
                SubmeshReference.VertexPtr       = SurfaceInfo.Offsets[0];
                SubmeshReference.FacesPtr        = SurfaceInfo.Offsets[1];
                SubmeshReference.Scale           = fmaxf(fmaxf(SurfaceInfo.Min, SurfaceInfo.Scale), SurfaceInfo.Max);
                SubmeshReference.XOffset         = SurfaceInfo.XOffset;
                SubmeshReference.YOffset         = SurfaceInfo.YOffset;
                SubmeshReference.ZOffset         = SurfaceInfo.ZOffset;

                // Assign weights
                SubmeshReference.WeightCounts[0] = SurfaceInfo.WeightCounts[0];
                SubmeshReference.WeightCounts[1] = SurfaceInfo.WeightCounts[1];
                SubmeshReference.WeightCounts[2] = SurfaceInfo.WeightCounts[2];
                SubmeshReference.WeightCounts[3] = SurfaceInfo.WeightCounts[3];
                SubmeshReference.WeightCounts[4] = SurfaceInfo.WeightCounts[4];
                SubmeshReference.WeightCounts[5] = SurfaceInfo.WeightCounts[5];
                SubmeshReference.WeightCounts[6] = SurfaceInfo.WeightCounts[6];
                SubmeshReference.WeightCounts[7] = SurfaceInfo.WeightCounts[7];
                // Weight pointer
                SubmeshReference.WeightsPtr = SurfaceInfo.WeightsPtr;

                // Read this submesh's material handle
                auto MaterialHandle = CoDAssets::GameInstance->Read<uint64_t>(ModelData.MaterialHandlesPtr);
                // Create the material and add it
                LodReference.Materials.emplace_back(ReadXMaterial(MaterialHandle));

                // Advance
                XSurfacePtr += sizeof(MW4XModelSurface);
                ModelData.MaterialHandlesPtr += sizeof(uint64_t);
            }
        }

        // Return it
        return ModelAsset;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XImageDDS> GameModernWarfare4::ReadXImage(const CoDImage_t* Image)
{
    // Proxy off
    return LoadXImage(XImage_t(ImageUsageType::DiffuseMap, 0, Image->AssetPointer, Image->AssetName));
}

const XMaterial_t GameModernWarfare4::ReadXMaterial(uint64_t MaterialPointer)
{
    // Prepare to parse the material
    auto MaterialData = CoDAssets::GameInstance->Read<MW4XMaterial>(MaterialPointer);

    // Allocate a new material with the given image count
    XMaterial_t Result(MaterialData.ImageCount);
    // Clean the name, then apply it
    Result.MaterialName = FileSystems::GetFileNameWithoutExtension(CoDAssets::GameInstance->ReadNullTerminatedString(MaterialData.NamePtr));

    // Iterate over material images, assign proper references if available
    for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
    {
        // Read the image info
        auto ImageInfo = CoDAssets::GameInstance->Read<MW4XMaterialImage>(MaterialData.ImageTablePtr);
        // Read the image name (End of image - 8)
        auto ImageName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(ImageInfo.ImagePtr));

        // Default type
        auto DefaultUsage = ImageUsageType::Unknown;
        // Check 
        switch (ImageInfo.Type)
        {
        case 0:
            DefaultUsage = ImageUsageType::DiffuseMap;
            break;
        }

        // Assign the new image
        Result.Images.emplace_back(DefaultUsage, ImageInfo.Type, ImageInfo.ImagePtr, ImageName);

        // Advance
        MaterialData.ImageTablePtr += sizeof(IWXMaterialImage);
    }
    // Return it
    return Result;
}

void GameModernWarfare4::PrepareVertexWeights(std::vector<WeightsData>& Weights, const XModelSubmesh_t & Submesh)
{
    // The index of read weight data
    uint32_t WeightDataIndex = 0;

    // Prepare the simple, rigid weights
    for (uint32_t i = 0; i < Submesh.VertListcount; i++)
    {
        // Simple weights build, rigid, just apply the proper bone id
        auto RigidInfo = CoDAssets::GameInstance->Read<MW4GfxRigidVerts>(Submesh.RigidWeightsPtr + (i * sizeof(MW4GfxRigidVerts)));
        // Apply bone ids properly
        for (uint32_t w = 0; w < RigidInfo.VertexCount; w++)
        {
            // Apply
            Weights[WeightDataIndex].BoneValues[0] = RigidInfo.BoneIndex;
            // Advance
            WeightDataIndex++;
        }
    }

    // Total weight data read
    uintptr_t ReadDataSize = 0;
    // Calculate the size of weights buffer
    auto WeightsDataLength = ((4 * Submesh.WeightCounts[0]) + (8 * Submesh.WeightCounts[1]) + (12 * Submesh.WeightCounts[2]) + (16 * Submesh.WeightCounts[3]) + (20 * Submesh.WeightCounts[4]) + (24 * Submesh.WeightCounts[5]) + (28 * Submesh.WeightCounts[6]) + (32 * Submesh.WeightCounts[7]));
    // Read the weight data
    auto WeightsData = MemoryReader(CoDAssets::GameInstance->Read(Submesh.WeightsPtr, WeightsDataLength, ReadDataSize), ReadDataSize);

    // Loop over the number of counts (8 in total)
    for (int i = 0; i < 8; i++)
    {
        // Loop over the number of bones for this chunk
        for (int k = 0; k < i + 1; k++)
        {
            // Store local weight index
            uint32_t LocalWeightDataIndex = WeightDataIndex;

            for (int w = 0; w < Submesh.WeightCounts[i]; w++)
            {
                // Assign count
                Weights[LocalWeightDataIndex].WeightCount = k + 1;
                // Read Weight and Index
                Weights[LocalWeightDataIndex].BoneValues[k] = WeightsData.Read<uint16_t>();

                // Process weight if applicable
                if (k > 0)
                {
                    // Read value and subtract from first bone
                    Weights[LocalWeightDataIndex].WeightValues[k] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
                    Weights[LocalWeightDataIndex].WeightValues[0] -= Weights[LocalWeightDataIndex].WeightValues[k];
                }
                else
                {
                    // Skip 2 padding
                    WeightsData.Advance(2);
                }

                // Advance
                LocalWeightDataIndex++;
            }
        }

        // Increment
        WeightDataIndex += Submesh.WeightCounts[i];
    }
}

std::unique_ptr<XImageDDS> GameModernWarfare4::LoadXImage(const XImage_t& Image)
{
    // Prepare to load an image, we need to rip loaded and streamed ones
    uint32_t ResultSize = 0;

    // We must read the image data
    auto ImageInfo = CoDAssets::GameInstance->Read<MW4GfxImage>(Image.ImagePtr);

    // Calculate the largest image mip
    uint32_t LargestMip = 0;
    uint32_t LargestWidth = 0;
    uint32_t LargestHeight = 0;
    uint64_t LargestHash = 0;

    // Loop and calculate
    for (uint32_t i = 0; i < 4; i++)
    {
        // Compare widths
        if (ImageInfo.MipLevels[i].Width > LargestWidth)
        {
            LargestMip = i;
            LargestWidth = ImageInfo.MipLevels[i].Width;
            LargestHeight = ImageInfo.MipLevels[i].Height;
            LargestHash = ImageInfo.MipLevels[i].HashID;
        }
    }

    // Calculate proper image format
    switch (ImageInfo.ImageFormat)
    {
    // RGBA
    case 7: ImageInfo.ImageFormat = 28; break;
    // BC1
    case 33:
    case 34:
    case 35: ImageInfo.ImageFormat = 71; break;
    case 36: ImageInfo.ImageFormat = 74; break;
    case 38: ImageInfo.ImageFormat = 77; break;
    case 39: ImageInfo.ImageFormat = 80; break;
    // BC7
    case 44: 
    case 45: ImageInfo.ImageFormat = 98; break;
    // Fall back to BC1
    default: ImageInfo.ImageFormat = 71; break;
    }

    // Buffer
    std::unique_ptr<uint8_t[]> ImageData = nullptr;

    // Check if we're missing a hash / size
    if (LargestWidth != 0 || LargestHash != 0)
    {
        // We have a streamed image, prepare to extract
        ImageData = CoDAssets::GamePackageCache->ExtractPackageObject(LargestHash, ResultSize);
    }

    // Prepare if we have it
    if (ImageData != nullptr)
    {
        // Prepare to create a MemoryDDS file
        auto Result = CoDRawImageTranslator::TranslateBC(ImageData, ResultSize, LargestWidth, LargestHeight, ImageInfo.ImageFormat);

        // Check for, and apply patch if required, if we got a raw result
        if (Result != nullptr && Image.ImageUsage == ImageUsageType::NormalMap && (SettingsManager::GetSetting("patchnormals", "true") == "true"))
        {
            // Set normal map patch
            Result->ImagePatchType = ImagePatch::Normal_Expand;
        }

        // Return it
        return Result;
    }

    // Failed to load the image
    return nullptr;
}

void GameModernWarfare4::LoadXModel(const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel)
{
    // Scale to use
    auto ScaleConstant = (1.0f / 0x1FFFFF) * 2.0f;
    // Check if we want Vertex Colors
    bool ExportColors = (SettingsManager::GetSetting("exportvtxcolor", "true") == "true");
    // Read the mesh information
    auto MeshInfo = CoDAssets::GameInstance->Read<MW4XModelMesh>(ModelLOD.LODStreamInfoPtr);
    // Read Buffer Info
    auto BufferInfo = CoDAssets::GameInstance->Read<MW4XModelMeshBufferInfo>(MeshInfo.MeshBufferPointer);

    // A buffer for the mesh data
    std::unique_ptr<uint8_t[]> MeshDataBuffer = nullptr;
    // Resulting size
    uint64_t MeshDataBufferSize = 0;

    // Determine if we need to load the mesh or not (even a streamed mesh can be in memory if loaded)
    if (BufferInfo.BufferPtr != 0)
    {
        // Result size
        uintptr_t ResultSize = 0;
        // The mesh is already loaded, just read it
        auto TemporaryBuffer = CoDAssets::GameInstance->Read(BufferInfo.BufferPtr, BufferInfo.BufferSize, ResultSize);

        // Copy and clean up
        if (TemporaryBuffer != nullptr)
        {
            // Allocate safe
            MeshDataBuffer = std::make_unique<uint8_t[]>(BufferInfo.BufferSize);
            // Copy over
            std::memcpy(MeshDataBuffer.get(), TemporaryBuffer, (size_t)ResultSize);
            // Set size
            MeshDataBufferSize = ResultSize;

            // Clean up
            delete[] TemporaryBuffer;
        }
    }
    else
    {
        // Result size
        uint32_t ResultSize = 0;
        // We must read from the cache
        MeshDataBuffer = CoDAssets::GamePackageCache->ExtractPackageObject(MeshInfo.LODStreamKey, ResultSize);
        // Set size
        MeshDataBufferSize = ResultSize;
    }

    // Continue on success
    if (MeshDataBuffer != nullptr)
    {
        // Make a reader to begin reading the mesh (Don't close)
        auto MeshReader = MemoryReader((int8_t*)MeshDataBuffer.get(), MeshDataBufferSize, true);

        // The total weighted verticies
        uint32_t TotalReadWeights = 0;
        // The maximum weight index
        uint32_t MaximumWeightIndex = ResultModel->BoneCount() - 1;

        // Prepare it for submeshes
        ResultModel->PrepareSubmeshes((uint32_t)ModelLOD.Submeshes.size());

        // Iterate over submeshes
        for (auto& Submesh : ModelLOD.Submeshes)
        {
            // Create and grab a new submesh
            auto& Mesh = ResultModel->AddSubmesh();

            // Set the material (COD has 1 per submesh)
            Mesh.AddMaterial(Submesh.MaterialIndex);

            // Prepare the mesh for the data
            Mesh.PrepareMesh(Submesh.VertexCount, Submesh.FaceCount);

            // Pre-allocate vertex weights (Data defaults to weight 1.0 on bone 0)
            auto VertexWeights = std::vector<WeightsData>(Submesh.VertexCount);

            // Setup weights
            PrepareVertexWeights(VertexWeights, Submesh);

            // Jump to vertex position data, advance to this submeshes verticies
            MeshReader.SetPosition(Submesh.VertexPtr);

            // Values for DirectX
            auto Positions = std::make_unique<DirectX::XMFLOAT3[]>((size_t)Submesh.VertexCount);
            auto Indices = std::make_unique<uint32_t[]>((size_t)Submesh.FaceCount * 3);
            auto Normals = std::make_unique<DirectX::XMFLOAT3[]>((size_t)Submesh.VertexCount);

            // Iterate over verticies
            for (uint32_t i = 0; i < Submesh.VertexCount; i++)
            {
                // Make a new vertex
                auto& Vertex = Mesh.AddVertex();

                auto StreamVertex = MeshReader.Read<MW4GfxStreamVertex>();

                // Read and assign position
                Vertex.Position = Vector3(
                    (((((StreamVertex.PackedPosition >> 0)  & 0x1FFFFF) * ScaleConstant) - 1.0f) * Submesh.Scale) + Submesh.XOffset,
                    (((((StreamVertex.PackedPosition >> 21) & 0x1FFFFF) * ScaleConstant) - 1.0f) * Submesh.Scale) + Submesh.YOffset,
                    (((((StreamVertex.PackedPosition >> 42) & 0x1FFFFF) * ScaleConstant) - 1.0f) * Submesh.Scale) + Submesh.ZOffset);

                // Set position for DirectX
                Positions[i].x = Vertex.Position.X;
                Positions[i].y = Vertex.Position.Y;
                Positions[i].z = Vertex.Position.Z;

                // Add UV layer
                Vertex.AddUVLayer(HalfFloats::ToFloat(StreamVertex.UVUPosition), HalfFloats::ToFloat(StreamVertex.UVVPosition));

                // Apply Color (some models don't store colors, so we need to check ptr below)
                Vertex.Color[0] = 255;
                Vertex.Color[1] = 255;
                Vertex.Color[2] = 255;
                Vertex.Color[3] = 255;

                // Assign weights
                auto& WeightValue = VertexWeights[i];

                // Iterate
                for (uint32_t w = 0; w < WeightValue.WeightCount; w++)
                {
                    // Add new weight
                    Vertex.AddVertexWeight(WeightValue.BoneValues[w], WeightValue.WeightValues[w]);
                }
            }

            // Jump to color data, if it's stored and we want it
            //if(ExportColors && Submesh)

            // Jump to face data, advance to this submeshes faces
            MeshReader.SetPosition(Submesh.FacesPtr);

            // Iterate over faces
            for (uint32_t i = 0; i < Submesh.FaceCount; i++)
            {
                // Read data
                auto Face = MeshReader.Read<MW4GfxStreamFace>();

                // Add the face
                Mesh.AddFace(Face.Index1, Face.Index2, Face.Index3);

                // Assign Face Indices for DirectX
                Indices[((size_t)i * 3) + 0] = Face.Index1;
                Indices[((size_t)i * 3) + 1] = Face.Index3;
                Indices[((size_t)i * 3) + 2] = Face.Index2;
            }

            // Last Step for Normals, generate them via DirectX, by area gives the best results
            // TODO: Figure out how to unpack normals, this works pretty well for now and my brain is too fucking fried
            // Either the value defined in MW4GfxStreamFace or check some of the ptrs in the submesh
            DirectX::ComputeNormals(Indices.get(), Submesh.FaceCount, Positions.get(), Submesh.VertexCount, DirectX::CNORM_WEIGHT_BY_AREA, Normals.get());

            // Iterate over verticies
            for (uint32_t i = 0; i < Submesh.VertexCount; i++)
            {
                if (std::abs(Normals[i].x + Normals[i].y + Normals[i].z) < 0.00001)
                {
                    // Set to 1.0
                    Mesh.Verticies[i].Normal = Vector3(1.0f, 0.0f, 0.0f);
                }
                else
                {
                    // Set new Normal
                    Mesh.Verticies[i].Normal = Vector3(Normals[i].x, Normals[i].y, Normals[i].z);
                }
            }
        }
    }
}

std::string GameModernWarfare4::LoadStringEntry(uint64_t Index)
{
    // Read and return (Offsets[3] = StringTable)
    return CoDAssets::GameInstance->ReadNullTerminatedString((28 * Index) + CoDAssets::GameOffsetInfos[3] + 8);
}
void GameModernWarfare4::PerformInitialSetup()
{
    // Prepare to copy the oodle dll
    auto OurPath = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "oo2core_6_win64.dll");

    // Copy if not exists
    if (!FileSystems::FileExists(OurPath))
        FileSystems::CopyFile(FileSystems::CombinePath(FileSystems::GetDirectoryName(CoDAssets::GameInstance->GetProcessPath()), "oo2core_7_win64.dll"), OurPath);
}