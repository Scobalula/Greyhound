#include "stdafx.h"

// The class we are implementing
#include "GameBlackOps3.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDXPoolParser.h"
#include "CoDRawImageTranslator.h"

// We need the following WraithX classes
#include "Strings.h"
#include "FileSystems.h"
#include "MemoryReader.h"
#include "TextWriter.h"
#include "SettingsManager.h"
#include "HalfFloats.h"

// -- Initialize built-in game offsets databases

// Black Ops 3 SP
std::array<DBGameInfo, 8> GameBlackOps3::SinglePlayerOffsets =
{{
    { 0x93FA290, 0x0, 0x4D4F100, 0x0 }, // LATEST
    { 0x830E090, 0x0, 0x3C62F00, 0x0 },
    { 0x82A1780, 0x0, 0x3CEFF00, 0x0 },
    { 0x8194410, 0x0, 0x3BE2D80, 0x0 },
    { 0x8130400, 0x0, 0x3B7ED80, 0x0 },
    { 0x8148970, 0x0, 0x3B8B300, 0x0 },
    { 0x7F76EF0, 0x0, 0x39B9880, 0x0 },
    { 0x7F72E60, 0x0, 0x39B5800, 0x0 }
}};

// -- Finished with databases

// -- Begin XModelStream structures

struct GfxStreamVertex
{
    uint8_t Color[4];

    uint16_t UVUPosition;
    uint16_t UVVPosition;

    int32_t VertexNormal;
    int32_t VertexTangent;
};

struct GfxStreamWeight
{
    uint8_t WeightVal1;
    uint8_t WeightVal2;
    uint8_t WeightVal3;
    uint8_t WeightVal4;

    uint16_t WeightID1;
    uint16_t WeightID2;
    uint16_t WeightID3;
    uint16_t WeightID4;
};

struct GfxStreamFace
{
    uint16_t Index1;
    uint16_t Index2;
    uint16_t Index3;
};

// -- End XModelStream structures

// -- Black Ops 3 Pool Data Structure

struct BO3XAssetPoolData
{
    // The beginning of the pool
    uint64_t PoolPtr;
    
    // The size of the asset header
    uint32_t AssetSize;
    // The maximum pool size
    uint32_t PoolSize;

    // Padding
    uint32_t Padding;

    // The amount of assets in the pool
    uint32_t AssetsLoaded;

    // A pointer to the closest free header
    uint64_t PoolFreeHeadPtr;
};

// Verify that our pool data is exactly 0x20
static_assert(sizeof(BO3XAssetPoolData) == 0x20, "Invalid Pool Data Size (Expected 0x20)");

bool GameBlackOps3::LoadOffsets()
{
    // ----------------------------------------------------
    //    Black Ops 3 pools and sizes, XAssetPoolData is an array of pool info for each asset pool in the game
    //    The index of the assets we use are as follows: xanim (3), xmodel (4), ximage (0x9), xrawfile (0x2f), xfx (0x26)
    //    Index * sizeof(BO3XAssetPoolData) = the offset of the asset info in this array of data, we can verify it using the xmodel pool and checking for "void"
    //  Notice: Black Ops 3 doesn't store a freePoolHandle at the beginning, so we just read on.
    //    On Black Ops 3, "void" will be the first xmodel
    //    Black Ops 3 stringtable, check entries, results may vary
    //    Reading is: (StringIndex * 28) + StringTablePtr + 4
    // ----------------------------------------------------

    // Attempt to load the game offsets
    if (CoDAssets::GameInstance != nullptr)
    {
        // We need the base address of the BO3 Module for ASLR + Heuristics
        auto BaseAddress = CoDAssets::GameInstance->GetMainModuleAddress();

        // Check built-in offsets via game exe mode (SP)
        for (auto& GameOffsets : SinglePlayerOffsets)
        {
            // Read required offsets (XANIM, XMODEL, XIMAGE, XRAWFILE)
            auto AnimPoolData = CoDAssets::GameInstance->Read<BO3XAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BO3XAssetPoolData) * 3));
            auto ModelPoolData = CoDAssets::GameInstance->Read<BO3XAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BO3XAssetPoolData) * 4));
            auto ImagePoolData = CoDAssets::GameInstance->Read<BO3XAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BO3XAssetPoolData) * 0x9));
            auto RawfilePoolData = CoDAssets::GameInstance->Read<BO3XAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BO3XAssetPoolData) * 0x2f));
            auto FxtPoolData = CoDAssets::GameInstance->Read<BO3XAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BO3XAssetPoolData) * 0x26));

            // Apply game offset info
            CoDAssets::GameOffsetInfos.emplace_back(AnimPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ModelPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ImagePoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(RawfilePoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(FxtPoolData.PoolPtr);

            // Verify via first xmodel asset
            auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1]));
            // Check
            if (FirstXModelName == "void")
            {
                // Verify string table, otherwise we are all set
                CoDAssets::GameOffsetInfos.emplace_back(BaseAddress + GameOffsets.StringTable);
                // Read the first string
                if (!Strings::IsNullOrWhiteSpace(LoadStringEntry(2)))
                {
                    // Read and apply sizes
                    CoDAssets::GamePoolSizes.emplace_back(AnimPoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(ModelPoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(ImagePoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(RawfilePoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(FxtPoolData.PoolSize);
                    // Return success
                    return true;
                }
            }
            // Reset
            CoDAssets::GameOffsetInfos.clear();
        }

        // Attempt to locate via heuristic searching
        auto DBAssetsScan = CoDAssets::GameInstance->Scan("63 C1 48 8D 05 ?? ?? ?? ?? 49 C1 E0 ?? 4C 03 C0");
        auto StringTableScan = CoDAssets::GameInstance->Scan("4C 03 F6 33 DB 49 ?? ?? 8B D3 8D 7B");

        // Check that we had hits
        if (DBAssetsScan > 0 && StringTableScan > 0)
        {
            // Load info and verify
            auto GameOffsets = DBGameInfo(
                // Resolve pool info from LEA
                CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan + 0x5) + (DBAssetsScan + 0x9),
                // We don't use size offsets
                0,
                // Resolve strings from LEA
                CoDAssets::GameInstance->Read<uint32_t>(StringTableScan + 0x1D) + (StringTableScan + 0x21),
                // We don't use package offsets
                0
                );

            // In debug, print the info for easy additions later!
#if _DEBUG
            // Format the output
            printf("Heuristic: { 0x%llX, 0x0, 0x%llX, 0x0 }\n", (GameOffsets.DBAssetPools - BaseAddress), (GameOffsets.StringTable - BaseAddress));
#endif

            // Read required offsets (XANIM, XMODEL, XIMAGE)
            auto AnimPoolData = CoDAssets::GameInstance->Read<BO3XAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BO3XAssetPoolData) * 3));
            auto ModelPoolData = CoDAssets::GameInstance->Read<BO3XAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BO3XAssetPoolData) * 4));
            auto ImagePoolData = CoDAssets::GameInstance->Read<BO3XAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BO3XAssetPoolData) * 0x9));
            auto RawfilePoolData = CoDAssets::GameInstance->Read<BO3XAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BO3XAssetPoolData) * 0x2f));
            auto FxtPoolData = CoDAssets::GameInstance->Read<BO3XAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BO3XAssetPoolData) * 0x26));

            // Apply game offset info
            CoDAssets::GameOffsetInfos.emplace_back(AnimPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ModelPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ImagePoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(RawfilePoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(FxtPoolData.PoolPtr);

            // Verify via first xmodel asset
            auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1]));
            // Check
            if (FirstXModelName == "void")
            {
                // Verify string table, otherwise we are all set
                CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);
                // Read the first string
                if (!Strings::IsNullOrWhiteSpace(LoadStringEntry(2)))
                {
                    // Read and apply sizes
                    CoDAssets::GamePoolSizes.emplace_back(AnimPoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(ModelPoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(ImagePoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(RawfilePoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(FxtPoolData.PoolSize);
                    // Return success
                    return true;
                }
            }
        }
    }

    // Failed
    return false;
}

bool GameBlackOps3::LoadAssets()
{
    // Prepare to load game assets, into the AssetPool
    bool NeedsAnims = (SettingsManager::GetSetting("showxanim", "true") == "true");
    bool NeedsModels = (SettingsManager::GetSetting("showxmodel", "true") == "true");
    bool NeedsImages = (SettingsManager::GetSetting("showximage", "false") == "true");
    bool NeedsRawFiles = (SettingsManager::GetSetting("showxrawfiles", "false") == "true");

    // Check if we need assets
    if (NeedsAnims)
    {
        // Store the placeholder anim
        BO3XAnim PlaceholderAnim;
        // Clear it out
        std::memset(&PlaceholderAnim, 0, sizeof(PlaceholderAnim));

        // Parse the XAnim pool
        CoDXPoolParser<uint64_t, BO3XAnim>(CoDAssets::GameOffsetInfos[0], CoDAssets::GamePoolSizes[0], [&PlaceholderAnim](BO3XAnim& Asset, uint64_t& AssetOffset)
        {
            // Validate and load if need be
            auto AnimName = CoDAssets::GameInstance->ReadNullTerminatedString(Asset.NamePtr);

            // Make and add
            auto LoadedAnim = new CoDAnim_t();
            // Set
            LoadedAnim->AssetName = AnimName;
            LoadedAnim->AssetPointer = AssetOffset;
            LoadedAnim->Framerate = Asset.Framerate;
            LoadedAnim->FrameCount = Asset.NumFrames;
            LoadedAnim->BoneCount = Asset.TotalBoneCount;

            // Check placeholder configuration, "void" is the base xanim
            if (AnimName == "void")
            {
                // Set as placeholder animation
                PlaceholderAnim = Asset;
                LoadedAnim->AssetStatus = WraithAssetStatus::Placeholder;
            }
            else if (Asset.BoneIDsPtr == PlaceholderAnim.BoneIDsPtr && Asset.DataBytePtr == PlaceholderAnim.DataBytePtr && Asset.DataShortPtr == PlaceholderAnim.DataShortPtr && Asset.DataIntPtr == PlaceholderAnim.DataIntPtr && Asset.RandomDataBytePtr == PlaceholderAnim.RandomDataBytePtr && Asset.RandomDataIntPtr == PlaceholderAnim.RandomDataIntPtr && Asset.RandomDataShortPtr == PlaceholderAnim.RandomDataShortPtr && Asset.NotificationsPtr == PlaceholderAnim.NotificationsPtr && Asset.DeltaPartsPtr == PlaceholderAnim.DeltaPartsPtr)
            {
                // Set as placeholder, data matches void
                LoadedAnim->AssetStatus = WraithAssetStatus::Placeholder;
            }
            else
            {
                // Set
                LoadedAnim->AssetStatus = WraithAssetStatus::Loaded;
            }

            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedAnim);
        });
    }

    if (NeedsModels)
    {
        // Store the placeholder model
        BO3XModel PlaceholderModel;
        // Clear it out
        std::memset(&PlaceholderModel, 0, sizeof(PlaceholderModel));

        // Parse the XModel pool
        CoDXPoolParser<uint64_t, BO3XModel>(CoDAssets::GameOffsetInfos[1], CoDAssets::GamePoolSizes[1], [&PlaceholderModel](BO3XModel& Asset, uint64_t& AssetOffset)
        {
            // Validate and load if need be
            auto ModelName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(Asset.NamePtr));

            // Make and add
            auto LoadedModel = new CoDModel_t();
            // Set
            LoadedModel->AssetName         = ModelName;
            LoadedModel->AssetPointer      = AssetOffset;
            LoadedModel->BoneCount         = Asset.NumBones;
            LoadedModel->LodCount          = Asset.NumLods;
            LoadedModel->CosmeticBoneCount = Asset.NumCosmeticBones;

            // Check placeholder configuration, "void" is the base xmodel
            if (ModelName == "void")
            {
                // Set as placeholder model
                PlaceholderModel = Asset;
                LoadedModel->AssetStatus = WraithAssetStatus::Placeholder;
            }
            else if (Asset.BoneIDsPtr == PlaceholderModel.BoneIDsPtr && Asset.ParentListPtr == PlaceholderModel.ParentListPtr && Asset.RotationsPtr == PlaceholderModel.RotationsPtr && Asset.TranslationsPtr == PlaceholderModel.TranslationsPtr && Asset.PartClassificationPtr == PlaceholderModel.PartClassificationPtr && Asset.BaseMatriciesPtr == PlaceholderModel.BaseMatriciesPtr && Asset.NumLods == PlaceholderModel.NumLods && Asset.MaterialHandlesPtr == PlaceholderModel.MaterialHandlesPtr && Asset.NumBones == PlaceholderModel.NumBones)
            {
                // Set as placeholder, data matches void
                LoadedModel->AssetStatus = WraithAssetStatus::Placeholder;
            }
            else if (Strings::StartsWith(ModelName, "*"))
            {
                // Set as placeholder, this is a junk entry
                LoadedModel->AssetStatus = WraithAssetStatus::Placeholder;
            }
            else
            {
                // Set
                LoadedModel->AssetStatus = WraithAssetStatus::Loaded;
            }

            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedModel);
        });
    }

    if (NeedsImages)
    {
        // Parse the XImage pool
        CoDXPoolParser<uint64_t, BO3GfxImage>(CoDAssets::GameOffsetInfos[2], CoDAssets::GamePoolSizes[2], [](BO3GfxImage& Asset, uint64_t& AssetOffset)
        {
            // Validate and load if need be
            auto ImageName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(Asset.NamePtr));

            // Check if it's streamed
            // if (Asset.LoadedMipPtr != 0 && Asset.MipLevels[0].HashID == 0)
            {
                // Make and add
                auto LoadedImage = new CoDImage_t();
                // Set
                LoadedImage->AssetName = ImageName;
                LoadedImage->AssetPointer = AssetOffset;
                LoadedImage->Width = (uint16_t)Asset.LoadedMipWidth;
                LoadedImage->Height = (uint16_t)Asset.LoadedMipHeight;
                LoadedImage->Format = Asset.ImageFormat;
                LoadedImage->AssetStatus = WraithAssetStatus::Loaded;
                LoadedImage->Streamed = Asset.MipLevels[0].HashID != 0;

                // Add
                CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
            }
        });
    }

    if (NeedsRawFiles)
    {
        // Parse the Rawfile pool
        CoDXPoolParser<uint64_t, BO3XRawFile>(CoDAssets::GameOffsetInfos[3], CoDAssets::GamePoolSizes[3], [](BO3XRawFile& Asset, uint64_t& AssetOffset)
        {
            // Validate and load if need be
            auto RawfileName = CoDAssets::GameInstance->ReadNullTerminatedString(Asset.NamePtr);

            // Make and add
            auto LoadedRawfile = new CoDRawFile_t();
            // Set
            LoadedRawfile->AssetName = FileSystems::GetFileName(RawfileName);
            LoadedRawfile->RawFilePath = FileSystems::GetDirectoryName(RawfileName);
            LoadedRawfile->AssetPointer = AssetOffset;
            LoadedRawfile->AssetSize = Asset.AssetSize;
            LoadedRawfile->RawDataPointer = Asset.RawDataPtr;
            LoadedRawfile->AssetStatus = WraithAssetStatus::Loaded;

            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedRawfile);
        });
    }

    // Success, error only on specific load
    return true;
}

std::unique_ptr<XAnim_t> GameBlackOps3::ReadXAnim(const CoDAnim_t* Animation)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Prepare to read the xanim
        auto Anim = std::make_unique<XAnim_t>();

        // Read the XAnim structure
        auto AnimData = CoDAssets::GameInstance->Read<BO3XAnim>(Animation->AssetPointer);

        // Copy over default properties
        Anim->AnimationName = Animation->AssetName;
        // Frames and Rate
        Anim->FrameCount = AnimData.NumFrames;
        Anim->FrameRate = AnimData.Framerate;

        // Check for viewmodel animations
        if ((_strnicmp(Animation->AssetName.c_str(), "viewmodel_", 10) == 0) || (_strnicmp(Animation->AssetName.c_str(), "vm_", 3) == 0))
        {
            // This is a viewmodel animation
            Anim->ViewModelAnimation = true;
        }
        // Check for additive animations
        if (AnimData.AssetType == 0x6)
        {
            // This is a additive animation
            Anim->AdditiveAnimation = true;
        }
        // Check for looping
        Anim->LoopingAnimation = (AnimData.LoopingFlag > 0);

        // Read the delta data
        auto AnimDeltaData = CoDAssets::GameInstance->Read<BO3XAnimDeltaParts>(AnimData.DeltaPartsPtr);

        // Copy over pointers
        Anim->BoneIDsPtr = AnimData.BoneIDsPtr;
        Anim->DataBytesPtr = AnimData.DataBytePtr;
        Anim->DataShortsPtr = AnimData.DataShortPtr;
        Anim->DataIntsPtr = AnimData.DataIntPtr;
        Anim->RandomDataBytesPtr = AnimData.RandomDataBytePtr;
        Anim->RandomDataShortsPtr = AnimData.RandomDataShortPtr;
        Anim->RandomDataIntsPtr = AnimData.RandomDataIntPtr;
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

        // Set types, we use half floats for BO3
        Anim->RotationType = AnimationKeyTypes::HalfFloat;
        Anim->TranslationType = AnimationKeyTypes::MinSizeTable;

        // Black Ops 3 doesn't support inline indicies
        Anim->SupportsInlineIndicies = false;

        // Return it
        return Anim;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XModel_t> GameBlackOps3::ReadXModel(const CoDModel_t* Model)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Read the XModel structure
        auto ModelData = CoDAssets::GameInstance->Read<BO3XModel>(Model->AssetPointer);

        // Prepare to read the xmodel (Reserving space for lods)
        auto ModelAsset = std::make_unique<XModel_t>(ModelData.NumLods);

        // Copy over default properties
        ModelAsset->ModelName = Model->AssetName;
        // Bone counts
        ModelAsset->BoneCount = ModelData.NumBones;
        ModelAsset->RootBoneCount = ModelData.NumRootBones;
        ModelAsset->CosmeticBoneCount = ModelData.NumCosmeticBones;

        // Bone data type
        ModelAsset->BoneRotationData = BoneDataTypes::HalfFloat;

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

        // Bone info ptr
        ModelAsset->BoneInfoPtr = ModelData.BoneInfoPtr;

        // Prepare to parse lods
        for (uint32_t i = 0; i < ModelData.NumLods; i++)
        {
            // Read the lod
            auto LODInfo = CoDAssets::GameInstance->Read<BO3XModelLod>(ModelData.ModelLodPtrs[i]);
            // Create the lod and grab reference
            ModelAsset->ModelLods.emplace_back(LODInfo.NumSurfs);
            // Grab reference
            auto& LodReference = ModelAsset->ModelLods[i];

            // Set distance
            LodReference.LodDistance = LODInfo.LodDistance;
            LodReference.LodMaxDistance = LODInfo.LodMinDistance;

            // Set stream key and info ptr
            LodReference.LODStreamKey = LODInfo.LODStreamKey;
            LodReference.LODStreamInfoPtr = LODInfo.XModelMeshPtr;

            // Grab pointer from the lod itself
            auto XSurfacePtr = LODInfo.XSurfacePtr;

            // Skip 8 bytes in materials
            ModelData.MaterialHandlesPtr += 8;
            // Read material handles ptr
            auto MaterialHandlesPtr = CoDAssets::GameInstance->Read<uint64_t>(ModelData.MaterialHandlesPtr);
            // Advance 8 and skip 8 bytes
            ModelData.MaterialHandlesPtr += 16;

            // Load surfaces
            for (uint32_t s = 0; s < LODInfo.NumSurfs; s++)
            {
                // Create the surface and grab reference
                LodReference.Submeshes.emplace_back();
                // Grab reference
                auto& SubmeshReference = LodReference.Submeshes[s];

                // Read the surface data
                auto SurfaceInfo = CoDAssets::GameInstance->Read<BO3XModelSurface>(XSurfacePtr);

                // Apply surface info
                SubmeshReference.VertexCount = SurfaceInfo.VertexCount;
                SubmeshReference.FaceCount = SurfaceInfo.FacesCount;
                SubmeshReference.VertexPtr = SurfaceInfo.VerticiesIndex;
                SubmeshReference.FacesPtr = SurfaceInfo.FacesIndex;

                // Assign weight info to the count slots, to save memory
                SubmeshReference.WeightCounts[0] = SurfaceInfo.Flag1;
                SubmeshReference.WeightCounts[1] = SurfaceInfo.Flag2;
                SubmeshReference.WeightCounts[2] = SurfaceInfo.Flag3;
                SubmeshReference.WeightCounts[3] = SurfaceInfo.Flag4;

                // Read this submesh's material handle
                auto MaterialHandle = CoDAssets::GameInstance->Read<uint64_t>(MaterialHandlesPtr);
                // Create the material and add it
                LodReference.Materials.emplace_back(ReadXMaterial(MaterialHandle));

                // Advance
                XSurfacePtr += sizeof(BO3XModelSurface);
                MaterialHandlesPtr += sizeof(uint64_t);
            }
        }

        // Return it
        return ModelAsset;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XImageDDS> GameBlackOps3::ReadXImage(const CoDImage_t* Image)
{
    // Proxy the image off, determine type if need be
    auto Usage = ImageUsageType::DiffuseMap;

    // Check based on name
    if (Strings::EndsWith(Image->AssetName, "_n") || Strings::EndsWith(Image->AssetName, "_nml"))
    {
        Usage = ImageUsageType::NormalMap;
    }

    // Proxy off
    return LoadXImage(XImage_t(Usage, 0, 0, 0, Image->AssetPointer, Image->AssetName));
}

const XMaterial_t GameBlackOps3::ReadXMaterial(uint64_t MaterialPointer)
{
    // Prepare to parse the material
    auto MaterialData = CoDAssets::GameInstance->Read<BO3XMaterial>(MaterialPointer);

    // Allocate a new material with the given image count
    XMaterial_t Result(MaterialData.ImageCount);
    // Clean the name, then apply it
    Result.MaterialName = FileSystems::GetFileNameWithoutExtension(CoDAssets::GameInstance->ReadNullTerminatedString(MaterialData.NamePtr));

    // Iterate over material images, assign proper references if available
    for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
    {
        // Read the image info
        auto ImageInfo = CoDAssets::GameInstance->Read<BO3XMaterialImage>(MaterialData.ImageTablePtr);
        // Read the image name (End of image - 16)
        auto ImageName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(ImageInfo.ImagePtr + (sizeof(BO3GfxImage) - 16)));

        // Default type
        auto DefaultUsage = ImageUsageType::Unknown;
        // Check 
        switch (ImageInfo.SemanticHash)
        {
        case 0xA0AB1041:
            DefaultUsage = ImageUsageType::DiffuseMap;
            break;
        case 0x59D30D0F:
            DefaultUsage = ImageUsageType::NormalMap;
            break;
        case 0xEC443804:
            DefaultUsage = ImageUsageType::SpecularMap;
            break;
        }

        // Assign the new image
        Result.Images.emplace_back(DefaultUsage, ImageInfo.SemanticHash, 0, 0, ImageInfo.ImagePtr, ImageName);

        // Advance
        MaterialData.ImageTablePtr += sizeof(BO3XMaterialImage);
    }

    // Return it
    return Result;
}

std::unique_ptr<XImageDDS> GameBlackOps3::LoadXImage(const XImage_t& Image)
{
    // Prepare to load an image, we need to rip loaded and streamed ones
    uint32_t ResultSize = 0;

    // We must read the image data
    auto ImageInfo = CoDAssets::GameInstance->Read<BO3GfxImage>(Image.ImagePtr);

    // Calculate the largest image mip
    uint32_t LargestMip = 0;
    uint32_t LargestWidth = 0;
    uint32_t LargestHeight = 0;
    uint64_t LargestHash = 0;
    uint64_t LargestSize = 0;

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
            LargestSize = (uint64_t)(i == 0 ? ImageInfo.MipLevels[i].Size >> 4 : (ImageInfo.MipLevels[i].Size >> 4) - (ImageInfo.MipLevels[i - 1].Size >> 4));
        }
    }

    // Calculate proper image format (Convert signed to unsigned)
    switch (ImageInfo.ImageFormat)
    {
        // Fix invalid BC1_SRGB images, swap to BC1_UNORM
    case 72: ImageInfo.ImageFormat = 71; break;
        // Fix invalid BC2_SRGB images, swap to BC2_UNORM
    case 75: ImageInfo.ImageFormat = 74; break;
        // Fix invalid BC3_SRGB images, swap to BC3_UNORM
    case 78: ImageInfo.ImageFormat = 77; break;
        // Fix invalid BC7_SRGB images, swap to BC7_UNORM
    case 99: ImageInfo.ImageFormat = 98; break;
    }

    // Buffer
    std::unique_ptr<uint8_t[]> ImageData = nullptr;

    // Check if we're missing a hash / size
    if (LargestWidth == 0 || LargestHash == 0)
    {
        // Set sizes
        LargestWidth = ImageInfo.LoadedMipWidth;
        LargestHeight = ImageInfo.LoadedMipHeight;

        // Temporary size
        uintptr_t ImageMemoryResult = 0;
        // We have a loaded image, prepare to dump from memory
        auto ImageMemoryBuffer = CoDAssets::GameInstance->Read(ImageInfo.LoadedMipPtr, ImageInfo.LoadedMipSize, ImageMemoryResult);

        // Make sure we got it
        if (ImageMemoryBuffer != nullptr)
        {
            // Allocate a safe block
            ImageData = std::make_unique<uint8_t[]>((uint32_t)ImageMemoryResult);
            // Copy data over
            std::memcpy(ImageData.get(), ImageMemoryBuffer, ImageMemoryResult);

            // Set size
            ResultSize = (uint32_t)ImageMemoryResult;

            // Clean up
            delete[] ImageMemoryBuffer;
        }
    }
    else
    {
        // We have a streamed image, prepare to extract
        ImageData = CoDAssets::GamePackageCache->ExtractPackageObject(LargestHash, LargestSize, ResultSize);
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

void GameBlackOps3::LoadXModel(const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel)
{
    // Check if we want Vertex Colors
    bool ExportColors = (SettingsManager::GetSetting("exportvtxcolor", "true") == "true");
    // Read the mesh information
    auto MeshInfo = CoDAssets::GameInstance->Read<BO3XModelMeshInfo>(ModelLOD.LODStreamInfoPtr);

    // A buffer for the mesh data
    std::unique_ptr<uint8_t[]> MeshDataBuffer = nullptr;
    // Resulting size
    uint64_t MeshDataBufferSize = 0;

    // Determine if we need to load the mesh or not (Second bit in flags)
    if (MeshInfo.StatusFlag & (1 << 1))
    {
        // Result size
        uintptr_t ResultSize = 0;
        // The mesh is already loaded, just read it
        auto TemporaryBuffer = CoDAssets::GameInstance->Read(MeshInfo.XModelMeshBufferPtr, MeshInfo.XModelMeshBufferSize, ResultSize);

        // Copy and clean up
        if (TemporaryBuffer != nullptr)
        {
            // Allocate safe
            MeshDataBuffer = std::make_unique<uint8_t[]>(MeshInfo.XModelMeshBufferSize);
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
        MeshDataBuffer = CoDAssets::GamePackageCache->ExtractPackageObject(ModelLOD.LODStreamKey, MeshInfo.XModelMeshBufferSize, ResultSize);
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

            // Jump to vertex position data, advance to this submeshes verticies
            MeshReader.SetPosition(MeshInfo.VertexOffset + (Submesh.VertexPtr * 12));

            // Iterate over verticies
            for (uint32_t i = 0; i < Submesh.VertexCount; i++)
            {
                // Make a new vertex
                auto& Vertex = Mesh.AddVertex();

                // Read and assign position
                Vertex.Position = MeshReader.Read<Vector3>();
            }

            // Jump to vertex info data, advance to this submeshes info
            MeshReader.SetPosition(MeshInfo.UVOffset + (Submesh.VertexPtr * 16));

            // Iterate over verticies
            for (uint32_t i = 0; i < Submesh.VertexCount; i++)
            {
                // Grab the reference
                auto& Vertex = Mesh.Verticies[i];

                // Read vertex data
                auto VertexData = MeshReader.Read<GfxStreamVertex>();

                // Add UV layer
                Vertex.AddUVLayer(HalfFloats::ToFloat(VertexData.UVUPosition), HalfFloats::ToFloat(VertexData.UVVPosition));

                // Add Colors if we want them
                if (ExportColors)
                {
                    Vertex.Color[0] = VertexData.Color[0];
                    Vertex.Color[1] = VertexData.Color[1];
                    Vertex.Color[2] = VertexData.Color[2];
                    Vertex.Color[3] = VertexData.Color[3];
                }
                else
                {
                    Vertex.Color[0] = 0xFF;
                    Vertex.Color[1] = 0xFF;
                    Vertex.Color[2] = 0xFF;
                    Vertex.Color[3] = 0xFF;
                }

                // Unpack normal
                int32_t PackedX = (((VertexData.VertexNormal >> 0) & ((1 << 10) - 1)) - 512);
                int32_t PackedY = (((VertexData.VertexNormal >> 10) & ((1 << 10) - 1)) - 512);
                int32_t PackedZ = (((VertexData.VertexNormal >> 20) & ((1 << 10) - 1)) - 512);
                // Calculate
                Vertex.Normal.X = ((float)PackedX / 511.0f);
                Vertex.Normal.Y = ((float)PackedY / 511.0f);
                Vertex.Normal.Z = ((float)PackedZ / 511.0f);
            }

            // Jump to vertex weight data, advance to this submeshes info
            MeshReader.SetPosition(MeshInfo.WeightsOffset + (Submesh.VertexPtr * 12));

            // Iterate over verticies
            for (uint32_t i = 0; i < Submesh.VertexCount; i++)
            {
                // Grab the reference
                auto& Vertex = Mesh.Verticies[i];

                // Check if we're a complex weight, up to four weights
                if ((MeshInfo.WeightCount > TotalReadWeights) && (((uint8_t)Submesh.WeightCounts[2] & 2) > 0))
                {
                    // Read weight data
                    auto VertexWeight = MeshReader.Read<GfxStreamWeight>();

                    // Add if need be
                    Vertex.AddVertexWeight(VertexWeight.WeightID1, (VertexWeight.WeightVal1 / 255.0f));
                    // Calculate max
                    MaximumWeightIndex = std::max<uint32_t>(VertexWeight.WeightID1, MaximumWeightIndex);

                    // Check for value 2
                    if (VertexWeight.WeightVal2 > 0)
                    {
                        Vertex.AddVertexWeight(VertexWeight.WeightID2, (VertexWeight.WeightVal2 / 255.0f));
                        // Calculate max
                        MaximumWeightIndex = std::max<uint32_t>(VertexWeight.WeightID2, MaximumWeightIndex);
                    }

                    // Check for value 3
                    if (VertexWeight.WeightVal3 > 0)
                    {
                        Vertex.AddVertexWeight(VertexWeight.WeightID3, (VertexWeight.WeightVal3 / 255.0f));
                        // Calculate max
                        MaximumWeightIndex = std::max<uint32_t>(VertexWeight.WeightID3, MaximumWeightIndex);
                    }

                    // Check for value 4
                    if (VertexWeight.WeightVal4 > 0)
                    {
                        Vertex.AddVertexWeight(VertexWeight.WeightID4, (VertexWeight.WeightVal4 / 255.0f));
                        // Calculate max
                        MaximumWeightIndex = std::max<uint32_t>(VertexWeight.WeightID4, MaximumWeightIndex);
                    }

                    // Increase
                    TotalReadWeights++;
                }
                else
                {
                    // Simple weight
                    Vertex.AddVertexWeight(0, 1.0);
                }
            }

            // Jump to face data, advance to this submeshes faces
            MeshReader.SetPosition(MeshInfo.FacesOffset + (Submesh.FacesPtr * 2));

            // Iterate over faces
            for (uint32_t i = 0; i < Submesh.FaceCount; i++)
            {
                // Read data
                auto Face = MeshReader.Read<GfxStreamFace>();

                // Add the face
                Mesh.AddFace(Face.Index1, Face.Index2, Face.Index3);
            }
        }

        // Prepare to generate stream bones if we had a conflict
        if (MaximumWeightIndex > (ResultModel->BoneCount() - 1))
        {
            // Generate stream bones
            auto CurrentBoneCount = ResultModel->BoneCount();
            auto WantedBoneCount = (MaximumWeightIndex + 1);

            // Loop and create
            for (uint32_t i = 0; i < (WantedBoneCount - CurrentBoneCount); i++)
            {
                auto& StreamBone = ResultModel->AddBone();

                // Set name and parent
                StreamBone.TagName = Strings::Format("smod_bone%d", i + 1);
                StreamBone.BoneParent = -1;
            }

            // Ensure root is tag_origin
            ResultModel->Bones[0].TagName = "smod_bone0";
        }
    }
}

std::string GameBlackOps3::LoadStringEntry(uint64_t Index)
{
    // Read and return (Offsets[5] = StringTable)
    return CoDAssets::GameInstance->ReadNullTerminatedString((28 * Index) + CoDAssets::GameOffsetInfos[5] + 4);
}