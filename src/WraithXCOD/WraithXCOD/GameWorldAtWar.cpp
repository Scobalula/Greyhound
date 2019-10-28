#include "stdafx.h"

// The class we are implementing
#include "GameWorldAtWar.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDIWITranslator.h"

// We need the following WraithX classes
#include "Strings.h"
#include "FileSystems.h"
#include "SettingsManager.h"

// -- Initialize built-in game offsets databases

// World at War SP
std::array<DBGameInfo, 1> GameWorldAtWar::SinglePlayerOffsets =
{{
    { 0x8DC828, 0x8DC5D0, 0x3702400, 0 }
}};
// World at War MP
std::array<DBGameInfo, 1> GameWorldAtWar::MultiPlayerOffsets =
{{
    { 0x8D0958, 0x8D06E8, 0xF66B400, 0 }
}};

// -- Finished with databases

bool GameWorldAtWar::LoadOffsets()
{
    // ----------------------------------------------------
    //    World at War pools, DBAssetPools is an array of uint32 (ptrs) of each asset pool in the game
    //    The index of the assets we use are as follows: xanim (4), xmodel (5)
    //    Index * 4 = the offset of the pool pointer in this array of pools, we can verify it using the xmodel pool and checking for "void" or "defaultactor"
    //    On World at War, "void" will be the first xmodel for SP and "defaultactor" or "defaultweapon" will be the first xmodel for MP
    //    World at War stringtable, check entries, results may vary
    //    Reading is: (StringIndex * 12) + StringTablePtr + 4
    // ----------------------------------------------------

    // Attempt to load the game offsets
    if (CoDAssets::GameInstance != nullptr)
    {
        // Check built-in offsets via game exe mode (SP/MP)
        for (auto& GameOffsets : (CoDAssets::GameFlags == SupportedGameFlags::SP) ? SinglePlayerOffsets : MultiPlayerOffsets)
        {
            // Read required offsets (XANIM, XMODEL)
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 4)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 5)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 0xA)));
            // Verify via first xmodel asset
            auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(CoDAssets::GameOffsetInfos[1] + 4));
            // Check (Differs on SP/MP)
            if (FirstXModelName == "void" || FirstXModelName == "defaultactor" || FirstXModelName == "defaultweapon")
            {
                // Verify string table, otherwise we are all set
                CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);
                // Read the first string
                if (!Strings::IsNullOrWhiteSpace(LoadStringEntry(2)))
                {
                    // Read and apply sizes
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 4)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 5)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0xA)));

                    // Return success
                    return true;
                }
            }
            // Reset
            CoDAssets::GameOffsetInfos.clear();
        }

        // Attempt to locate via heuristic searching
        auto DBAssetsScan = CoDAssets::GameInstance->Scan("FF D2 8B F0 83 C4 04 85 F6 75");
        auto StringTableScan = CoDAssets::GameInstance->Scan("F7 EE D1 FA 8B C2 C1 E8 1F");

        // Check that we had hits
        if (DBAssetsScan > 0 && StringTableScan > 0)
        {
            // Load info and verify
            auto GameOffsets = DBGameInfo(CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan - 0xD), CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan + 0x25), CoDAssets::GameInstance->Read<uint32_t>(StringTableScan - 0x9), 0);
            // Read required offsets (XANIM, XMODEL)
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 4)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 5)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 0xA)));
            // Verify via first xmodel asset
            auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(CoDAssets::GameOffsetInfos[1] + 4));
            // Check (Differs on SP/MP)
            if (FirstXModelName == "void" || FirstXModelName == "defaultactor" || FirstXModelName == "defaultweapon")
            {
                // Verify string table, otherwise we are all set
                CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);
                // Read the first string
                if (!Strings::IsNullOrWhiteSpace(LoadStringEntry(2)))
                {
                    // Read and apply sizes
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 4)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 5)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0xA)));

                    // Return success
                    return true;
                }
            }
        }
    }

    // Failed
    return false;
}

bool GameWorldAtWar::LoadAssets()
{
    // Prepare to load game assets, into the AssetPool
    bool NeedsAnims = (SettingsManager::GetSetting("showxanim", "true") == "true");
    bool NeedsModels = (SettingsManager::GetSetting("showxmodel", "true") == "true");
    bool NeedsSounds = (SettingsManager::GetSetting("showxsounds", "false") == "true");

    // Check if we need assets
    if (NeedsAnims)
    {
        // Animations are the first offset and first pool, skip 4 byte pointer to free head
        auto AnimationOffset = CoDAssets::GameOffsetInfos[0] + 4;
        auto AnimationCount = CoDAssets::GamePoolSizes[0];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (AnimationCount * sizeof(WAWXAnim)) + AnimationOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[0];

        // Store the placeholder anim
        WAWXAnim PlaceholderAnim;
        // Clear it out
        std::memset(&PlaceholderAnim, 0, sizeof(PlaceholderAnim));

        // Loop and read
        for (uint32_t i = 0; i < AnimationCount; i++)
        {
            // Read
            auto AnimResult = CoDAssets::GameInstance->Read<WAWXAnim>(AnimationOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((AnimResult.NamePtr > MinimumPoolOffset && AnimResult.NamePtr < MaximumPoolOffset) || AnimResult.NamePtr == 0)
            {
                // Advance
                AnimationOffset += sizeof(WAWXAnim);
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

            // Check placeholder configuration, "void" is the base xanim
            if (AnimName == "void")
            {
                // Set as placeholder animation
                PlaceholderAnim = AnimResult;
                LoadedAnim->AssetStatus = WraithAssetStatus::Placeholder;
            }
            else if (AnimResult.BoneIDsPtr == PlaceholderAnim.BoneIDsPtr && AnimResult.DataBytePtr == PlaceholderAnim.DataBytePtr && AnimResult.DataShortPtr == PlaceholderAnim.DataShortPtr && AnimResult.DataIntPtr == PlaceholderAnim.DataIntPtr && AnimResult.RandomDataBytePtr == PlaceholderAnim.RandomDataBytePtr && AnimResult.RandomDataIntPtr == PlaceholderAnim.RandomDataIntPtr && AnimResult.RandomDataShortPtr == PlaceholderAnim.RandomDataShortPtr && AnimResult.NotificationsPtr == PlaceholderAnim.NotificationsPtr && AnimResult.DeltaPartsPtr == PlaceholderAnim.DeltaPartsPtr)
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

            // Advance
            AnimationOffset += sizeof(WAWXAnim);
        }
    }

    if (NeedsModels)
    {
        // Models are the second offset and second pool, skip 4 byte pointer to free head
        auto ModelOffset = CoDAssets::GameOffsetInfos[1] + 4;
        auto ModelCount = CoDAssets::GamePoolSizes[1];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (ModelCount * sizeof(WAWXModel)) + ModelOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[1];

        // Store the placeholder model
        WAWXModel PlaceholderModel;
        // Clear it out
        std::memset(&PlaceholderModel, 0, sizeof(PlaceholderModel));

        // Loop and read
        for (uint32_t i = 0; i < ModelCount; i++)
        {
            // Read
            auto ModelResult = CoDAssets::GameInstance->Read<WAWXModel>(ModelOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((ModelResult.NamePtr > MinimumPoolOffset && ModelResult.NamePtr < MaximumPoolOffset) || ModelResult.NamePtr == 0)
            {
                // Advance
                ModelOffset += sizeof(WAWXModel);
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
            LoadedModel->BoneCount = ModelResult.NumBones;
            LoadedModel->LodCount = ModelResult.NumLods;

            // Check placeholder configuration, "void" is the base xmodel
            if (ModelName == "void")
            {
                // Set as placeholder model
                PlaceholderModel = ModelResult;
                LoadedModel->AssetStatus = WraithAssetStatus::Placeholder;
            }
            else if (ModelResult.BoneIDsPtr == PlaceholderModel.BoneIDsPtr && ModelResult.ParentListPtr == PlaceholderModel.ParentListPtr && ModelResult.RotationsPtr == PlaceholderModel.RotationsPtr && ModelResult.TranslationsPtr == PlaceholderModel.TranslationsPtr && ModelResult.PartClassificationPtr == PlaceholderModel.PartClassificationPtr && ModelResult.BaseMatriciesPtr == PlaceholderModel.BaseMatriciesPtr && ModelResult.SurfacesPtr == PlaceholderModel.SurfacesPtr && ModelResult.MaterialHandlesPtr == PlaceholderModel.MaterialHandlesPtr && ModelResult.NumBones == PlaceholderModel.NumBones)
            {
                // Set as placeholder, data matches void
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
            ModelOffset += sizeof(WAWXModel);
        }
    }

    if (NeedsSounds)
    {
        // Sounds are the third offset and second pool, skip 4 byte pointer to free head
        auto SoundOffset = CoDAssets::GameOffsetInfos[2] + 4;
        auto SoundCount = CoDAssets::GamePoolSizes[2];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (SoundCount * sizeof(WAWLoadedSound)) + SoundOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[2];

        // Loop and read
        for (uint32_t i = 0; i < SoundCount; i++)
        {
            // Read
            auto SoundResult = CoDAssets::GameInstance->Read<WAWLoadedSound>(SoundOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((SoundResult.NamePtr > MinimumPoolOffset && SoundResult.NamePtr < MaximumPoolOffset) || SoundResult.NamePtr == 0)
            {
                // Advance
                SoundOffset += sizeof(WAWLoadedSound);
                // Skip this asset
                continue;
            }

            // Validate and load if need be
            auto SoundName = CoDAssets::GameInstance->ReadNullTerminatedString(SoundResult.NamePtr);

            // Make and add
            auto LoadedSound = new CoDSound_t();
            // Set
            LoadedSound->AssetName = FileSystems::GetFileNameWithoutExtension(SoundName);
            LoadedSound->AssetPointer = SoundResult.SoundDataPtr;
            LoadedSound->AssetSize = SoundResult.SoundDataSize;
            LoadedSound->IsFileEntry = false;
            LoadedSound->FullPath = FileSystems::GetDirectoryName(SoundName);
            LoadedSound->DataType = SoundDataTypes::WAV_WithHeader;
            LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
            LoadedSound->Length = 0;

            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);

            // Advance
            SoundOffset += sizeof(WAWLoadedSound);
        }
    }

    // Success, error only on specific load
    return true;
}

std::unique_ptr<XAnim_t> GameWorldAtWar::ReadXAnim(const CoDAnim_t* Animation)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Prepare to read the xanim
        auto Anim = std::make_unique<XAnim_t>();

        // Read the XAnim structure
        auto AnimData = CoDAssets::GameInstance->Read<WAWXAnim>(Animation->AssetPointer);

        // Copy over default properties
        Anim->AnimationName = Animation->AssetName;
        // Frames and Rate
        Anim->FrameCount = AnimData.NumFrames;
        Anim->FrameRate = AnimData.Framerate;

        // Check for viewmodel animations
        if ((_strnicmp(Animation->AssetName.c_str(), "viewmodel_", 10) == 0))
        {
            // This is a viewmodel animation
            Anim->ViewModelAnimation = true;
        }
        // Check for looping
        Anim->LoopingAnimation = (AnimData.Looped > 0);

        // Read the delta data
        auto AnimDeltaData = CoDAssets::GameInstance->Read<WAWXAnimDeltaParts>(AnimData.DeltaPartsPtr);

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
        Anim->BoneIndexSize = 2;

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

        // Set types, we use dividebysize for WAW
        Anim->RotationType = AnimationKeyTypes::DivideBySize;
        Anim->TranslationType = AnimationKeyTypes::MinSizeTable;

        // World at War supports inline indicies
        Anim->SupportsInlineIndicies = true;

        // Return it
        return Anim;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XModel_t> GameWorldAtWar::ReadXModel(const CoDModel_t* Model)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Read the XModel structure
        auto ModelData = CoDAssets::GameInstance->Read<WAWXModel>(Model->AssetPointer);

        // Prepare to read the xmodel (Reserving space for lods)
        auto ModelAsset = std::make_unique<XModel_t>(ModelData.NumLods);

        // Copy over default properties
        ModelAsset->ModelName = Model->AssetName;
        // Bone counts
        ModelAsset->BoneCount = ModelData.NumBones;
        ModelAsset->RootBoneCount = ModelData.NumRootBones;

        // Bone data type
        ModelAsset->BoneRotationData = BoneDataTypes::DivideBySize;

        // Bone id info
        ModelAsset->BoneIDsPtr = ModelData.BoneIDsPtr;
        ModelAsset->BoneIndexSize = 2;

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

            // Grab pointer (Skip to this lods data)
            auto XSurfacePtr = ModelData.SurfacesPtr + (ModelData.ModelLods[i].SurfacesIndex * sizeof(WAWXModelSurface));

            // Load surfaces
            for (uint32_t s = 0; s < ModelData.ModelLods[i].NumSurfs; s++)
            {
                // Create the surface and grab reference
                LodReference.Submeshes.emplace_back();
                // Grab reference
                auto& SubmeshReference = LodReference.Submeshes[s];

                // Read the surface data
                auto SurfaceInfo = CoDAssets::GameInstance->Read<WAWXModelSurface>(XSurfacePtr);

                // Apply surface info
                SubmeshReference.VertListcount = SurfaceInfo.VertListCount;
                SubmeshReference.RigidWeightsPtr = SurfaceInfo.RigidWeightsPtr;
                SubmeshReference.VertexCount = SurfaceInfo.VertexCount;
                SubmeshReference.FaceCount = SurfaceInfo.FacesCount;
                SubmeshReference.VertexPtr = SurfaceInfo.VerticiesPtr;
                SubmeshReference.FacesPtr = SurfaceInfo.FacesPtr;

                // Assign weights
                SubmeshReference.WeightCounts[0] = SurfaceInfo.WeightCounts[0];
                SubmeshReference.WeightCounts[1] = SurfaceInfo.WeightCounts[1];
                SubmeshReference.WeightCounts[2] = SurfaceInfo.WeightCounts[2];
                SubmeshReference.WeightCounts[3] = SurfaceInfo.WeightCounts[3];
                // Weight pointer
                SubmeshReference.WeightsPtr = SurfaceInfo.WeightsPtr;

                // Read this submesh's material handle
                auto MaterialHandle = CoDAssets::GameInstance->Read<uint32_t>(ModelData.MaterialHandlesPtr);
                // Create the material and add it
                LodReference.Materials.emplace_back(ReadXMaterial(MaterialHandle));

                // Advance
                XSurfacePtr += sizeof(WAWXModelSurface);
                ModelData.MaterialHandlesPtr += sizeof(uint32_t);
            }
        }

        // Return it
        return ModelAsset;
    }
    // Not running
    return nullptr;
}

const XMaterial_t GameWorldAtWar::ReadXMaterial(uint64_t MaterialPointer)
{
    // Prepare to parse the material
    auto MaterialData = CoDAssets::GameInstance->Read<WAWXMaterial>(MaterialPointer);

    // Allocate a new material with the given image count
    XMaterial_t Result(MaterialData.ImageCount);
    // Clean the name, then apply it
    Result.MaterialName = FileSystems::GetFileNameWithoutExtension(CoDAssets::GameInstance->ReadNullTerminatedString(MaterialData.NamePtr));

    // Iterate over material images, assign proper references if available
    for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
    {
        // Read the image info
        auto ImageInfo = CoDAssets::GameInstance->Read<WAWXMaterialImage>(MaterialData.ImageTablePtr);
        // Read the image name (End of image - 4)
        auto ImageName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(ImageInfo.ImagePtr + (sizeof(WAWGfxImage) - 4)));

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
        case 0x34ECCCB3:
            DefaultUsage = ImageUsageType::SpecularMap;
            break;
        }

        // Assign the new image
        Result.Images.emplace_back(DefaultUsage, ImageInfo.SemanticHash, ImageInfo.ImagePtr, ImageName);

        // Advance
        MaterialData.ImageTablePtr += sizeof(WAWXMaterialImage);
    }

    // Return it
    return Result;
}

std::unique_ptr<XImageDDS> GameWorldAtWar::LoadXImage(const XImage_t& Image)
{
    // Prepare to load an image, we only support IWD images from the image cache
    uint32_t ResultSize = 0;
    // Attempt to load it
    auto ImageData = CoDAssets::GamePackageCache->ExtractPackageObject(CoDAssets::GamePackageCache->HashPackageID(Image.ImageName), ResultSize);

    // Check
    if (ImageData != nullptr)
    {
        // Parse from an IWI
        auto Result = CoDIWITranslator::TranslateIWI(ImageData, ResultSize);

        // Check for, and apply patch if required, if we got an IWI result
        if (Result != nullptr && Image.ImageUsage == ImageUsageType::NormalMap && (SettingsManager::GetSetting("patchnormals", "true") == "true"))
        {
            // Set normal map patch
            Result->ImagePatchType = ImagePatch::Normal_Bumpmap;
        }

        // Return it
        return Result;
    }

    // Failed to load the image
    return nullptr;
}

std::string GameWorldAtWar::LoadStringEntry(uint64_t Index)
{
    // Read and return (Offsets[3] = StringTable)
    return CoDAssets::GameInstance->ReadNullTerminatedString((12 * Index) + CoDAssets::GameOffsetInfos[3] + 4);
}