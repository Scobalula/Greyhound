#include "stdafx.h"

// The class we are implementing
#include "GameInfiniteWarfare.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDRawImageTranslator.h"
#include "PAKSupport.h"

// We need the following WraithX classes
#include "Strings.h"
#include "FileSystems.h"
#include "SettingsManager.h"

// -- Initialize built-in game offsets databases

// Infinite Warfare SP
std::array<DBGameInfo, 6> GameInfiniteWarfare::SinglePlayerOffsets =
{{
    { 0x1414663D0, 0x141466290, 0x1460B9600, 0x1458D5900 },
    { 0x141466400, 0x1414662C0, 0x1460BA600, 0x1458D6900 },
    { 0x141466400, 0x1414662C0, 0x1460B9600, 0x1458D5900 },
    { 0x141465300, 0x1414651C0, 0x1460B8600, 0x1458D4900 },
    { 0x1414652F0, 0x1414651B0, 0x1460B8600, 0x1458D4900 },
    { 0x1414641F0, 0x1414640B0, 0x1460B7600, 0x1458D3900 }
}};

// -- Finished with databases

// -- Structures for reading

enum class GfxImageMapType : uint8_t
{
    MAPTYPE_CUBE = 0x5
};

// -- End structures for reading

bool GameInfiniteWarfare::LoadOffsets()
{
    // ----------------------------------------------------
    //    Infinite Warfare pools, DBAssetPools is an array of uint64 (ptrs) of each asset pool in the game
    //    The index of the assets we use are as follows: xanim (6), xmodel (8), ximage (0x12)
    //    Index * 8 = the offset of the pool pointer in this array of pools, we can verify it using the xmodel pool and checking for "viewmodel_default"
    //    On Infinite Warfare, "viewmodel_default" will be the first xmodel
    //    Infinite Warfare stringtable, check entries, results may vary
    //    Reading is: (StringIndex * 20) + StringTablePtr + 8
    // ----------------------------------------------------

    // Attempt to load the game offsets
    if (CoDAssets::GameInstance != nullptr)
    {
        // Check built-in offsets via game exe mode (SP)
        for (auto& GameOffsets : SinglePlayerOffsets)
        {
            // Read required offsets (XANIM, XMODEL, XIMAGE)
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 6)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 8)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 0x12)));
            // Verify via first xmodel asset
            auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1] + 8));
            // Check
            if (FirstXModelName == "viewmodel_default")
            {
                // Verify string table, otherwise we are all set
                CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);
                // Read the first string
                if (!Strings::IsNullOrWhiteSpace(LoadStringEntry(2)))
                {
                    // Add ImagePackage offset
                    CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.ImagePackageTable);
                    // Read and apply sizes
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 6)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 8)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0x12)));
                    // Return success
                    return true;
                }
            }
            // Reset
            CoDAssets::GameOffsetInfos.clear();
        }

        // Attempt to locate via heuristic searching (Note: As of right now, none of the functions got inlined)
        auto DBAssetsScan = CoDAssets::GameInstance->Scan("48 63 C1 48 8D 15 ?? ?? ?? ?? 48 8B 8C C2");
        auto DBSizesScan = CoDAssets::GameInstance->Scan("72 ?? 48 63 C1 48 8D 0D ?? ?? ?? ?? 83 3C 81");
        auto StringTableScan = CoDAssets::GameInstance->Scan("55 56 57 48 83 EC ?? 48 8B F9 E8");
        auto PackagesTableScan = CoDAssets::GameInstance->Scan("41 55 41 56 48 83 EC ?? 44 8B F1 48 8D 05");

        // We need the base address of the IW Module for Heuristics
        auto BaseAddress = CoDAssets::GameInstance->GetMainModuleAddress();

        // Check that we had hits
        if (DBAssetsScan > 0 && DBSizesScan > 0 && StringTableScan > 0 && PackagesTableScan > 0)
        {
            // Load info and verify
            auto GameOffsets = DBGameInfo(
                // Resolve pool ptrs from RCX
                CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan + 0xE) + BaseAddress,
                // Resolve pool sizes from LEA
                CoDAssets::GameInstance->Read<uint32_t>(DBSizesScan + 0x8) + (DBSizesScan + 0xC),
                // Resolve strings from LEA
                CoDAssets::GameInstance->Read<uint32_t>(StringTableScan + 0x14) + (StringTableScan + 0x18),
                // Resolve packages from LEA
                CoDAssets::GameInstance->Read<uint32_t>(PackagesTableScan + 0xE) + (PackagesTableScan + 0x12)
                );

            // In debug, print the info for easy additions later!
#if _DEBUG
            // Format the output
            printf("Heuristic: { 0x%llX, 0x%llX, 0x%llX, 0x%llX }\n", (GameOffsets.DBAssetPools), (GameOffsets.DBPoolSizes), (GameOffsets.StringTable), (GameOffsets.ImagePackageTable));
#endif

            // Read required offsets (XANIM, XMODEL, XIMAGE)
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 6)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 8)));
            CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 0x12)));
            // Verify via first xmodel asset
            auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1] + 8));
            // Check
            if (FirstXModelName == "viewmodel_default")
            {
                // Verify string table, otherwise we are all set
                CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);
                // Read the first string
                if (!Strings::IsNullOrWhiteSpace(LoadStringEntry(2)))
                {
                    // Add ImagePackage offset
                    CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.ImagePackageTable);
                    // Read and apply sizes
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 6)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 8)));
                    CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0x12)));

                    // Return success
                    return true;
                }
            }
        }
    }

    // Failed
    return false;
}

bool GameInfiniteWarfare::LoadAssets()
{
    // Prepare to load game assets, into the AssetPool
    bool NeedsAnims = (SettingsManager::GetSetting("showxanim", "true") == "true");
    bool NeedsModels = (SettingsManager::GetSetting("showxmodel", "true") == "true");
    bool NeedsImages = (SettingsManager::GetSetting("showximage", "false") == "true");

    // Check if we need assets
    if (NeedsAnims)
    {
        // Animations are the first offset and first pool, skip 8 byte pointer to free head
        auto AnimationOffset = CoDAssets::GameOffsetInfos[0] + 8;
        auto AnimationCount = CoDAssets::GamePoolSizes[0];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (AnimationCount * sizeof(IWXAnim)) + AnimationOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[0];

        // Store the placeholder anim
        IWXAnim PlaceholderAnim;
        // Clear it out
        std::memset(&PlaceholderAnim, 0, sizeof(PlaceholderAnim));

        // Loop and read
        for (uint32_t i = 0; i < AnimationCount; i++)
        {
            // Read
            auto AnimResult = CoDAssets::GameInstance->Read<IWXAnim>(AnimationOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((AnimResult.NamePtr > MinimumPoolOffset && AnimResult.NamePtr < MaximumPoolOffset) || AnimResult.NamePtr == 0)
            {
                // Advance
                AnimationOffset += sizeof(IWXAnim);
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
            AnimationOffset += sizeof(IWXAnim);
        }
    }

    if (NeedsModels)
    {
        // Models are the second offset and second pool, skip 8 byte pointer to free head
        auto ModelOffset = CoDAssets::GameOffsetInfos[1] + 8;
        auto ModelCount = CoDAssets::GamePoolSizes[1];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (ModelCount * sizeof(IWXModel)) + ModelOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[1];

        // Store the placeholder model
        IWXModel PlaceholderModel;
        // Clear it out
        std::memset(&PlaceholderModel, 0, sizeof(PlaceholderModel));

        // Loop and read
        for (uint32_t i = 0; i < ModelCount; i++)
        {
            // Read
            auto ModelResult = CoDAssets::GameInstance->Read<IWXModel>(ModelOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((ModelResult.NamePtr > MinimumPoolOffset && ModelResult.NamePtr < MaximumPoolOffset) || ModelResult.NamePtr == 0)
            {
                // Advance
                ModelOffset += sizeof(IWXModel);
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

            // Check placeholder configuration, "empty_model" is the base xmodel swap
            if (ModelName == "empty_model")
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
            ModelOffset += sizeof(IWXModel);
        }
    }

    if (NeedsImages)
    {
        // Images are the third offset and third pool, skip 8 byte pointer to free head
        auto ImageOffset = CoDAssets::GameOffsetInfos[2] + 8;
        auto ImageCount = CoDAssets::GamePoolSizes[2];

        // Calculate maximum pool size
        auto MaximumPoolOffset = (ImageCount * sizeof(IWGfxImage)) + ImageOffset;
        // Store original offset
        auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[2];

        // Loop and read
        for (uint32_t i = 0; i < ImageCount; i++)
        {
            // Read
            auto ImageResult = CoDAssets::GameInstance->Read<IWGfxImage>(ImageOffset);

            // Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
            if ((ImageResult.NextHead > MinimumPoolOffset && ImageResult.NextHead < MaximumPoolOffset) || ImageResult.NamePtr == 0)
            {
                // Advance
                ImageOffset += sizeof(IWGfxImage);
                // Skip this asset
                continue;
            }

            // Validate and load if need be
            auto ImageName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(ImageResult.NamePtr));

            // Check if it's streamed
            if (ImageResult.Streamed > 0)
            {
                // Calculate the largest image mip
                uint32_t LargestMip = 0;
                uint32_t LargestWidth = ImageResult.Width;
                uint32_t LargestHeight = ImageResult.Height;

                // Loop and calculate
                for (uint32_t i = 0; i < 3; i++)
                {
                    // Compare widths
                    if (ImageResult.MipLevels[i].Width > LargestWidth)
                    {
                        LargestMip = (i + 1);
                        LargestWidth = ImageResult.MipLevels[i].Width;
                        LargestHeight = ImageResult.MipLevels[i].Height;
                    }
                }

                // Make and add
                auto LoadedImage = new CoDImage_t();
                // Set
                LoadedImage->AssetName = ImageName;
                LoadedImage->AssetPointer = ImageOffset;
                LoadedImage->Width = (uint16_t)LargestWidth;
                LoadedImage->Height = (uint16_t)LargestHeight;
                LoadedImage->Format = ImageResult.ImageFormat;
                LoadedImage->AssetStatus = WraithAssetStatus::Loaded;

                // Add
                CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
            }

            // Advance
            ImageOffset += sizeof(IWGfxImage);
        }
    }

    // Success, error only on specific load
    return true;
}

bool GameInfiniteWarfare::LoadAssetsPS()
{
    // Prepare to load game assets, into the AssetPool
    bool NeedsAnims = (SettingsManager::GetSetting("showxanim", "true") == "true");
    bool NeedsModels = (SettingsManager::GetSetting("showxmodel", "true") == "true");
    bool NeedsImages = (SettingsManager::GetSetting("showximage", "false") == "true");
    bool NeedsSounds = (SettingsManager::GetSetting("showxsounds", "false") == "true");
    bool NeedsMaterials = (SettingsManager::GetSetting("showxmtl", "false") == "true");

    // Check if we need assets
    if (NeedsModels)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 8 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto ModelResult = CoDAssets::GameInstance->Read<IWXModel>(Asset.Header);
            // Validate and load if need be
            auto ModelName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(ModelResult.NamePtr));
            // Make and add
            auto LoadedModel = new CoDModel_t();
            // Set
            LoadedModel->AssetName = ModelName;
            LoadedModel->AssetPointer = Asset.Header;
            LoadedModel->BoneCount = ModelResult.NumBones;
            LoadedModel->LodCount = ModelResult.NumLods;
            LoadedModel->AssetStatus = Asset.Temp == 1 ? WraithAssetStatus::Placeholder : WraithAssetStatus::Loaded;
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedModel);
        });
    }

    // Check if we need assets
    if (NeedsImages)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 18 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto ImageResult = CoDAssets::GameInstance->Read<IWGfxImage>(Asset.Header);
            // Validate and load if need be
            auto ImageName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(ImageResult.NamePtr));
            // Calculate the largest image mip
            uint32_t LargestWidth = ImageResult.Width;
            uint32_t LargestHeight = ImageResult.Height;
            // Check if it's streamed
            if (ImageResult.Streamed > 0)
            {
                // Loop and calculate
                for (uint32_t i = 0; i < 3; i++)
                {
                    // Compare widths
                    if (ImageResult.MipLevels[i].Width > LargestWidth)
                    {
                        LargestWidth = ImageResult.MipLevels[i].Width;
                        LargestHeight = ImageResult.MipLevels[i].Height;
                    }
                }
            }
            else
            {
                // Use loaded data
                LargestWidth = ImageResult.LoadedWidth;
                LargestHeight = ImageResult.LoadedHeight;
            }
            // Make and add
            auto LoadedImage = new CoDImage_t();
            // Set
            LoadedImage->AssetName = ImageName;
            LoadedImage->AssetPointer = Asset.Header;
            LoadedImage->Width = (uint16_t)LargestWidth;
            LoadedImage->Height = (uint16_t)LargestHeight;
            LoadedImage->Format = ImageResult.ImageFormat;
            LoadedImage->Streamed = ImageResult.Streamed > 0;
            LoadedImage->AssetStatus = WraithAssetStatus::Loaded;
            LoadedImage->AssetStatus = Asset.Temp == 1 ? WraithAssetStatus::Placeholder : WraithAssetStatus::Loaded;
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
        });
    }

    // Check if we need assets
    if (NeedsAnims)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 6 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto AnimResult = CoDAssets::GameInstance->Read<IWXAnim>(Asset.Header);
            // Validate and load if need be
            auto AnimName = CoDAssets::GameInstance->ReadNullTerminatedString(AnimResult.NamePtr);
            // Make and add
            auto LoadedAnim = new CoDAnim_t();
            // Set
            LoadedAnim->AssetName = AnimName;
            LoadedAnim->AssetPointer = Asset.Header;
            LoadedAnim->Framerate = AnimResult.Framerate;
            LoadedAnim->FrameCount = AnimResult.NumFrames;
            LoadedAnim->BoneCount = AnimResult.TotalBoneCount;
            LoadedAnim->AssetStatus = Asset.Temp == 1 ? WraithAssetStatus::Placeholder : WraithAssetStatus::Loaded;
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedAnim);
        });
    }

    // Success, error only on specific load
    return true;
}

std::unique_ptr<XAnim_t> GameInfiniteWarfare::ReadXAnim(const CoDAnim_t* Animation)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Prepare to read the xanim
        auto Anim = std::make_unique<XAnim_t>();

        // Read the XAnim structure
        auto AnimData = CoDAssets::GameInstance->Read<IWXAnim>(Animation->AssetPointer);

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
        // Check for looping
        Anim->LoopingAnimation = (AnimData.Flags & 1);

        // Read the delta data
        auto AnimDeltaData = CoDAssets::GameInstance->Read<IWXAnimDeltaParts>(AnimData.DeltaPartsPtr);

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

        // Set types, we use dividebysize for IW
        Anim->RotationType = AnimationKeyTypes::DivideBySize;
        Anim->TranslationType = AnimationKeyTypes::MinSizeTable;

        // Infinite Warfare supports inline indicies
        Anim->SupportsInlineIndicies = true;

        // Return it
        return Anim;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XModel_t> GameInfiniteWarfare::ReadXModel(const CoDModel_t* Model)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Read the XModel structure
        auto ModelData = CoDAssets::GameInstance->Read<IWXModel>(Model->AssetPointer);

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
                auto SurfaceInfo = CoDAssets::GameInstance->Read<IWXModelSurface>(XSurfacePtr);

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
                XSurfacePtr += sizeof(IWXModelSurface);
                ModelData.MaterialHandlesPtr += sizeof(uint64_t);
            }
        }

        // Return it
        return ModelAsset;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XImageDDS> GameInfiniteWarfare::ReadXImage(const CoDImage_t* Image)
{
    // Proxy the image off, determine type if need be
    auto Usage = ImageUsageType::Unknown;
    // Determine from name
    if (Strings::EndsWith(Image->AssetName, "_nog") || Strings::EndsWith(Image->AssetName, "_ng") || Strings::EndsWith(Image->AssetName, "_n") || Strings::EndsWith(Image->AssetName, "_nml"))
    {
        // Set to normal map
        Usage = ImageUsageType::NormalMap;
    }
    else if (Strings::EndsWith(Image->AssetName, "_cs"))
    {
        // Set to diffuse map
        Usage = ImageUsageType::DiffuseMap;
    }
    // Proxy off
    if (ps::state != nullptr)
        return LoadXImagePS(XImage_t(Usage, 0, Image->AssetPointer, Image->AssetName));
    else
        return LoadXImage(XImage_t(Usage, 0, Image->AssetPointer, Image->AssetName));
}

const XMaterial_t GameInfiniteWarfare::ReadXMaterial(uint64_t MaterialPointer)
{
    // Prepare to parse the material
    auto MaterialData = CoDAssets::GameInstance->Read<IWXMaterial>(MaterialPointer);

    // Allocate a new material with the given image count
    XMaterial_t Result(MaterialData.ImageCount);
    // Clean the name, then apply it
    Result.MaterialName = FileSystems::GetFileNameWithoutExtension(CoDAssets::GameInstance->ReadNullTerminatedString(MaterialData.NamePtr));

    // Iterate over material images, assign proper references if available
    for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
    {
        // Read the image info
        auto ImageInfo = CoDAssets::GameInstance->Read<IWXMaterialImage>(MaterialData.ImageTablePtr);
        // Read the image name (End of image - 8)
        auto ImageName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(ImageInfo.ImagePtr + (sizeof(IWGfxImage) - 8)));

        // Default type
        auto DefaultUsage = ImageUsageType::Unknown;
        // Check 
        switch (ImageInfo.SemanticHash)
        {
        case 0xA0AB1041:
            DefaultUsage = ImageUsageType::DiffuseMap;
            break;
        }

        // Assign the new image
        Result.Images.emplace_back(DefaultUsage, ImageInfo.SemanticHash, ImageInfo.ImagePtr, ImageName);

        // Advance
        MaterialData.ImageTablePtr += sizeof(IWXMaterialImage);
    }

    // Return it
    return Result;
}

std::unique_ptr<XImageDDS> GameInfiniteWarfare::LoadXImage(const XImage_t& Image)
{
    // Prepare to load an image, we only support PAK images
    uint32_t ResultSize = 0;

    // We must read the image data
    auto ImageInfo = CoDAssets::GameInstance->Read<IWGfxImage>(Image.ImagePtr);

    // Check if the image isn't streamed, if it isn't, just exit
    if (ImageInfo.Streamed <= 0) { return nullptr; }

    // Calculate the largest image mip
    uint32_t LargestMip = 0;
    uint32_t LargestWidth = ImageInfo.Width;
    uint32_t LargestHeight = ImageInfo.Height;

    // Loop and calculate
    for (uint32_t i = 0; i < 3; i++)
    {
        // Compare widths
        if (ImageInfo.MipLevels[i].Width > LargestWidth)
        {
            LargestMip = (i + 1);
            LargestWidth = ImageInfo.MipLevels[i].Width;
            LargestHeight = ImageInfo.MipLevels[i].Height;
        }
    }

    // Calculate table offset of the biggest mip
    uint64_t PAKTableOffset = (((Image.ImagePtr - (CoDAssets::GameOffsetInfos[2] + 8)) / sizeof(IWGfxImage)) * (sizeof(IWPAKImageEntry) * 4)) + CoDAssets::GameOffsetInfos[4] + (LargestMip * sizeof(IWPAKImageEntry));

    // Read info
    auto ImageStreamInfo = CoDAssets::GameInstance->Read<IWPAKImageEntry>(PAKTableOffset);

    // Read image package name
    auto ImagePackageName = CoDAssets::GameInstance->ReadNullTerminatedString(ImageStreamInfo.ImagePAKInfoPtr);

    // Attempt to extract the package asset
    auto ImageData = PAKSupport::IWExtractImagePackage(FileSystems::CombinePath(CoDAssets::GamePackageCache->GetPackagesPath(), ImagePackageName), ImageStreamInfo.ImageOffset, (ImageStreamInfo.ImageEndOffset - ImageStreamInfo.ImageOffset), ResultSize);

    // Check
    if (ImageData != nullptr)
    {
        // Prepare to create a MemoryDDS file
        auto Result = CoDRawImageTranslator::TranslateBC(ImageData, ResultSize, LargestWidth, LargestHeight, ImageInfo.ImageFormat, 1, (ImageInfo.MapType == (uint8_t)GfxImageMapType::MAPTYPE_CUBE));

        // Check for, and apply patch if required, if we got a raw result
        if (Result != nullptr && Image.ImageUsage == ImageUsageType::NormalMap && (SettingsManager::GetSetting("patchnormals", "true") == "true"))
        {
            // Set normal map patch
            Result->ImagePatchType = ImagePatch::Normal_COD_NOG;
        }
        else if (Result != nullptr && Image.ImageUsage == ImageUsageType::DiffuseMap && (SettingsManager::GetSetting("patchcolor", "true") == "true"))
        {
            // Set color patch
            Result->ImagePatchType = ImagePatch::Color_StripAlpha;
        }

        // Return it
        return Result;
    }

    // Failed to load the image
    return nullptr;
}

struct InfiniteWarfare_AssetFileRef
{
    uint64_t StartOffset;
    uint64_t EndOffset;
    uint16_t PackageIndex;
};

std::unique_ptr<XImageDDS> GameInfiniteWarfare::LoadXImagePS(const XImage_t& Image)
{
    // Prepare to load an image, we only support PAK images
    uint32_t ResultSize = 0;

    // We must read the image data
    auto ImageInfo = CoDAssets::GameInstance->Read<IWGfxImage>(Image.ImagePtr);
    // Buffer
    std::unique_ptr<uint8_t[]> ImageData = nullptr;
    // Calculate the largest image mip
    uint32_t LargestMip = 0;
    uint32_t LargestWidth = ImageInfo.Width;
    uint32_t LargestHeight = ImageInfo.Height;

    // Check if the image isn't streamed, if it isn't, just exit
    if (ImageInfo.Streamed > 0)
    {
        // Loop and calculate
        for (uint32_t i = 0; i < 3; i++)
        {
            // Compare widths
            if (ImageInfo.MipLevels[i].Width > LargestWidth)
            {
                LargestMip = (i + 1);
                LargestWidth = ImageInfo.MipLevels[i].Width;
                LargestHeight = ImageInfo.MipLevels[i].Height;
            }
        }
        // Read info
        auto ImageStreamInfo = CoDAssets::GameInstance->Read<InfiniteWarfare_AssetFileRef>(Image.ImagePtr + sizeof(IWGfxImage) + sizeof(InfiniteWarfare_AssetFileRef) * LargestMip);
        // Read image package name
        auto ImagePackageName = Strings::Format("imagefile%i.pak", ImageStreamInfo.PackageIndex);
        // Attempt to extract the package asset
        ImageData = PAKSupport::IWExtractImagePackage(FileSystems::CombinePath(CoDAssets::GamePackageCache->GetPackagesPath(), ImagePackageName), ImageStreamInfo.StartOffset, (ImageStreamInfo.EndOffset - ImageStreamInfo.StartOffset), ResultSize);
    }
    else
    {
        // Use loaded data
        LargestWidth = ImageInfo.LoadedWidth;
        LargestHeight = ImageInfo.LoadedHeight;

        ResultSize = CoDAssets::GameInstance->Read<uint32_t>(Image.ImagePtr + 40);
        ImageData = std::make_unique<uint8_t[]>(ResultSize);

        if (CoDAssets::GameInstance->Read(ImageData.get(), CoDAssets::GameInstance->Read<uint64_t>(Image.ImagePtr + 64), ResultSize) != ResultSize)
            return nullptr;
    }

    // Check
    if (ImageData != nullptr)
    {
        // Prepare to create a MemoryDDS file
        auto Result = CoDRawImageTranslator::TranslateBC(
            ImageData,
            ResultSize,
            LargestWidth,
            LargestHeight,
            ImageInfo.ImageFormat,
            1,
            (ImageInfo.MapType == (uint8_t)GfxImageMapType::MAPTYPE_CUBE));

        // Check for, and apply patch if required, if we got a raw result
        if (Result != nullptr && Image.ImageUsage == ImageUsageType::NormalMap && (SettingsManager::GetSetting("patchnormals", "true") == "true"))
        {
            // Set normal map patch
            Result->ImagePatchType = ImagePatch::Normal_COD_NOG;
        }
        else if (Result != nullptr && Image.ImageUsage == ImageUsageType::DiffuseMap && (SettingsManager::GetSetting("patchcolor", "true") == "true"))
        {
            // Set color patch
            Result->ImagePatchType = ImagePatch::Color_StripAlpha;
        }

        // Return it
        return Result;
    }

    // Failed to load the image
    return nullptr;
}


std::string GameInfiniteWarfare::LoadStringEntry(uint64_t Index)
{
    if (ps::state == nullptr)
    {
        // Read and return (Offsets[3] = StringTable)
        return CoDAssets::GameInstance->ReadNullTerminatedString((20 * Index) + CoDAssets::GameOffsetInfos[3] + 8);
    }
    else
    {
        return CoDAssets::GameInstance->ReadNullTerminatedString(ps::state->StringsAddress + Index);
    }
}