#include "pch.h"

// The class we are implementing
#include "GameModernWarfare.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDIWITranslator.h"

// We need the following WraithX classes
#include <Path.h>

// -- Initialize built-in game offsets databases

// Modern Warfare SP
std::array<DBGameInfo, 2> GameModernWarfare::SinglePlayerOffsets =
{{
    { 0x6DF200, 0x6DEFA0, 0xFFD480, 0 },
    { 0x6DF200, 0x6DEFA0, 0xFFD480, 0 }
}};
// Modern Warfare MP
std::array<DBGameInfo, 2> GameModernWarfare::MultiPlayerOffsets =
{{
    { 0x7275C0, 0x727380, 0x1826180, 0 },
    { 0x7265E0, 0x7263A0, 0x150A280, 0 }
}};

// -- Finished with databases

bool GameModernWarfare::LoadOffsets()
{
    // ----------------------------------------------------
    //    Modern Warfare pools, DBAssetPools is an array of uint32 (ptrs) of each asset pool in the game
    //    The index of the assets we use are as follows: xanim (2), xmodel (3)
    //    Index * 4 = the offset of the pool pointer in this array of pools, we can verify it using the xmodel pool and checking for "void" or "defaultactor"
    //    On Modern Warfare, "void" will be the first xmodel for SP and "defaultactor" or "defaultweapon" will be the first xmodel for MP
    //    Modern Warfare stringtable, check entries, results may vary
    //    Reading is: (StringIndex * 12) + StringTablePtr + 4
    // ----------------------------------------------------

    // Attempt to load the game offsets
    if (CoDAssets::GameInstance != nullptr)
    {
        // Check built-in offsets via game exe mode (SP/MP)
        for (auto& GameOffsets : (CoDAssets::GameFlags == SupportedGameFlags::SP) ? SinglePlayerOffsets : MultiPlayerOffsets)
        {
            // Read required offsets (XANIM, XMODEL, LOADED SOUND)
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 2)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 3)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 9)));
            // Verify via first xmodel asset
            auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(CoDAssets::GameOffsetInfos[1] + 4));
            // Check (Differs on SP/MP)
            if (FirstXModelName == "void" || FirstXModelName == "defaultactor" || FirstXModelName == "defaultweapon" || FirstXModelName == "defaultvehicle")
            {
                // Verify string table, otherwise we are all set
                CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);
                // Read the first string
                if (!string::IsNullOrWhiteSpace(LoadStringEntry(2).c_str()))
                {
                    // Read and apply sizes
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 2)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 3)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 9)));
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
            auto GameOffsets = DBGameInfo(CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan - 0xD), CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan + 0x24), CoDAssets::GameInstance->Read<uint32_t>(StringTableScan - 0x9), 0);
            // Read required offsets (XANIM, XMODEL, LOADED SOUND)
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 2)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 3)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBAssetPools + (4 * 9)));
            // Verify via first xmodel asset
            auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(CoDAssets::GameOffsetInfos[1] + 4));
            // Check (Differs on SP/MP)
            if (FirstXModelName == "void" || FirstXModelName == "defaultactor" || FirstXModelName == "defaultweapon" || FirstXModelName == "defaultvehicle")
            {
                // Verify string table, otherwise we are all set
                CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);
                // Read the first string
                if (!string::IsNullOrWhiteSpace(LoadStringEntry(2).c_str()))
                {
                    // Read and apply sizes
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 2)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 3)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 9)));
                    // Return success
                    return true;
                }
            }
        }
    }

    // Failed
    return false;
}

bool GameModernWarfare::LoadAssets()
{
    // Prepare to load game assets, into the AssetPool
    bool NeedsAnims = ExportManager::Config.GetBool("LoadAnimations");
    bool NeedsModels = ExportManager::Config.GetBool("LoadModels");
    bool NeedsSounds = ExportManager::Config.GetBool("LoadSounds");

    // Check if we need assets
    if (NeedsAnims)
    {
        // Animations are the first offset and first pool, skip 4 byte pointer to free head
        auto AnimationOffset = CoDAssets::GameOffsetInfos[0] + 4;
        auto AnimationCount = CoDAssets::GamePoolSizes[0];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (AnimationCount * sizeof(MWXAnim)) + AnimationOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[0];

        // Store the placeholder anim
        MWXAnim PlaceholderAnim;
        // Clear it out
        std::memset(&PlaceholderAnim, 0, sizeof(PlaceholderAnim));

        // Loop and read
        for (uint32_t i = 0; i < AnimationCount; i++)
        {
            // Read
            auto AnimResult = CoDAssets::GameInstance->Read<MWXAnim>(AnimationOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((AnimResult.NamePtr > MinimumPoolOffset && AnimResult.NamePtr < MaximumPoolOffset) || AnimResult.NamePtr == 0)
            {
                // Advance
                AnimationOffset += sizeof(MWXAnim);
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
            LoadedAnim->BoneCount = AnimResult.TotalBoneCount;

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
            AnimationOffset += sizeof(MWXAnim);
        }
    }

    if (NeedsModels)
    {
        // Models are the second offset and second pool, skip 4 byte pointer to free head
        auto ModelOffset = CoDAssets::GameOffsetInfos[1] + 4;
        auto ModelCount = CoDAssets::GamePoolSizes[1];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (ModelCount * sizeof(MWXModel)) + ModelOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[1];

        // Store the placeholder model
        MWXModel PlaceholderModel;
        // Clear it out
        std::memset(&PlaceholderModel, 0, sizeof(PlaceholderModel));

        // Loop and read
        for (uint32_t i = 0; i < ModelCount; i++)
        {
            // Read
            auto ModelResult = CoDAssets::GameInstance->Read<MWXModel>(ModelOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((ModelResult.NamePtr > MinimumPoolOffset && ModelResult.NamePtr < MaximumPoolOffset) || ModelResult.NamePtr == 0)
            {
                // Advance
                ModelOffset += sizeof(MWXModel);
                // Skip this asset
                continue;
            }

            // Validate and load if need be
            auto ModelName = IO::Path::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(ModelResult.NamePtr).c_str());

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
            ModelOffset += sizeof(MWXModel);
        }
    }

    if (NeedsSounds)
    {
        // Sounds are the third offset and second pool, skip 4 byte pointer to free head
        auto SoundOffset = CoDAssets::GameOffsetInfos[2] + 4;
        auto SoundCount = CoDAssets::GamePoolSizes[2];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (SoundCount * sizeof(MWLoadedSound)) + SoundOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[2];

        // Loop and read
        for (uint32_t i = 0; i < SoundCount; i++)
        {
            // Read
            auto SoundResult = CoDAssets::GameInstance->Read<MWLoadedSound>(SoundOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((SoundResult.NamePtr > MinimumPoolOffset && SoundResult.NamePtr < MaximumPoolOffset) || SoundResult.NamePtr == 0)
            {
                // Advance
                SoundOffset += sizeof(MWLoadedSound);
                // Skip this asset
                continue;
            }

            // Validate and load if need be
            auto SoundName = CoDAssets::GameInstance->ReadNullTerminatedString(SoundResult.NamePtr);

            // Make and add
            auto LoadedSound = new CoDSound_t();
            // Set
            LoadedSound->AssetName = IO::Path::GetFileNameWithoutExtension(SoundName.c_str());
            LoadedSound->AssetPointer = SoundResult.SoundDataPtr;
            LoadedSound->FrameRate = SoundResult.FrameRate;
            LoadedSound->FrameCount = SoundResult.FrameCount;
            LoadedSound->AssetSize = SoundResult.SoundDataSize;
            LoadedSound->ChannelsCount = SoundResult.Channels;
            LoadedSound->IsFileEntry = false;
            LoadedSound->FullPath = IO::Path::GetDirectoryName(SoundName.c_str());
            LoadedSound->DataType = SoundDataTypes::WAV_NeedsHeader;
            LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
            LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));

            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);

            // Advance
            SoundOffset += sizeof(MWLoadedSound);
        }
    }

    // Success, error only on specific load
    return true;
}

std::unique_ptr<XAnim_t> GameModernWarfare::ReadXAnim(const CoDAnim_t* Animation)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Prepare to read the xanim
        auto Anim = std::make_unique<XAnim_t>();

        // Read the XAnim structure
        auto AnimData = CoDAssets::GameInstance->Read<MWXAnim>(Animation->AssetPointer);

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
        auto AnimDeltaData = CoDAssets::GameInstance->Read<MWXAnimDeltaParts>(AnimData.DeltaPartsPtr);

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

        // Set types, we use dividebysize for MW
        Anim->RotationType = AnimationKeyTypes::DivideBySize;
        Anim->TranslationType = AnimationKeyTypes::MinSizeTable;

        // Modern Warfare supports inline indicies
        Anim->SupportsInlineIndicies = true;

        // Return it
        return Anim;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XModel_t> GameModernWarfare::ReadXModel(const CoDModel_t* Model)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Read the XModel structure
        auto ModelData = CoDAssets::GameInstance->Read<MWXModel>(Model->AssetPointer);

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
            auto XSurfacePtr = ModelData.SurfacesPtr + (ModelData.ModelLods[i].SurfacesIndex * sizeof(MWXModelSurface));

            // Load surfaces
            for (uint32_t s = 0; s < ModelData.ModelLods[i].NumSurfs; s++)
            {
                // Create the surface and grab reference
                LodReference.Submeshes.emplace_back();
                // Grab reference
                auto& SubmeshReference = LodReference.Submeshes[s];

                // Read the surface data
                auto SurfaceInfo = CoDAssets::GameInstance->Read<MWXModelSurface>(XSurfacePtr);

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
                XSurfacePtr += sizeof(MWXModelSurface);
                ModelData.MaterialHandlesPtr += sizeof(uint32_t);
            }
        }

        // Return it
        return ModelAsset;
    }
    // Not running
    return nullptr;
}

const XMaterial_t GameModernWarfare::ReadXMaterial(uint64_t MaterialPointer)
{
    // Prepare to parse the material
    auto MaterialData = CoDAssets::GameInstance->Read<MWXMaterial>(MaterialPointer);

    // Allocate a new material with the given image count
    XMaterial_t Result(MaterialData.ImageCount);
    // Clean the name, then apply it
    Result.MaterialName = IO::Path::GetFileNameWithoutExtension(CoDAssets::GameInstance->ReadNullTerminatedString(MaterialData.NamePtr).c_str());

    // Iterate over material images, assign proper references if available
    for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
    {
        // Read the image info
        auto ImageInfo = CoDAssets::GameInstance->Read<MWXMaterialImage>(MaterialData.ImageTablePtr);
        // Read the image name (End of image - 4)
        auto ImageName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(ImageInfo.ImagePtr + (sizeof(MWGfxImage) - 4)));

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
        MaterialData.ImageTablePtr += sizeof(MWXMaterialImage);
    }

    // Return it
    return Result;
}

std::unique_ptr<XImageDDS> GameModernWarfare::LoadXImage(const XImage_t& Image)
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
        if (Result != nullptr && Image.ImageUsage == ImageUsageType::NormalMap && ExportManager::Config.GetBool("PatchNormals"))
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

std::string GameModernWarfare::LoadStringEntry(uint64_t Index)
{
    // Read and return (Offsets[2] = StringTable)
    return CoDAssets::GameInstance->ReadNullTerminatedString((12 * Index) + CoDAssets::GameOffsetInfos[3] + 4);
}