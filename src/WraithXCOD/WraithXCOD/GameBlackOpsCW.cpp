#include "stdafx.h"

// The class we are implementing
#include "GameBlackOpsCW.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDRawImageTranslator.h"
#include "CoDXPoolParser.h"

// We need the following WraithX classes
#include "Strings.h"
#include "FileSystems.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"
#include "SettingsManager.h"
#include "HalfFloats.h"
#include "Sound.h"

// We need Opus
#include "..\..\External\Opus\include\opus.h"

// -- Initialize Asset Name Cache

WraithNameIndex GameBlackOpsCW::AssetNameCache = WraithNameIndex();
WraithNameIndex GameBlackOpsCW::StringCache = WraithNameIndex();

// -- Initialize built-in game offsets databases

// Black Ops CW SP
std::array<DBGameInfo, 1> GameBlackOpsCW::SinglePlayerOffsets =
{{
    { 0x10CFDE80, 0x0, 0xC97EAB0, 0x0 }
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

// -- Black Ops CW Pool Data Structure

struct BOCWXAssetPoolData
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

uint32_t GetFrameRate(uint8_t Index)
{
    switch (Index)
    {
    case 0: return 8000;
    case 1: return 12000;
    case 2: return 16000;
    case 3: return 24000;
    case 4: return 32000;
    case 5: return 44100;
    case 6: return 48000;
    case 7: return 96000;
    case 8: return 192000;
    default: return 48000;
    }
}

#pragma pack(push, 1)
struct BOCWStreamInfo
{
    uint64_t NamePtr;
    uint64_t UnknownZero;
    uint64_t StreamKey;
};
#pragma pack(pop)


// Verify that our pool data is exactly 0x20
static_assert(sizeof(BOCWXAssetPoolData) == 0x20, "Invalid Pool Data Size (Expected 0x20)");

bool GameBlackOpsCW::LoadOffsets()
{
    // ----------------------------------------------------
    //    Black Ops CW pools and sizes, XAssetPoolData is an array of pool info for each asset pool in the game
    //    Index * sizeof(BOCWXAssetPoolData) = the offset of the asset info in this array of data, we can verify it using the xmodel pool and checking for the model hash (0x04647533e968c910)
    //    Notice: Black Ops CW doesn't store a freePoolHandle at the beginning, so we just read on.
    //    On Black Ops CW, (0x04647533e968c910) will be the first xmodel
    //    Black Ops CW stringtable, check entries, results may vary
    // ----------------------------------------------------

    // Attempt to load the game offsets
    if (CoDAssets::GameInstance != nullptr)
    {
        // We need the base address of the BO4 Module for ASLR + Heuristics
        auto BaseAddress = CoDAssets::GameInstance->GetMainModuleAddress();

        // Check built-in offsets via game exe mode (SP)
        for (auto& GameOffsets : SinglePlayerOffsets)
        {
            // Read required offsets (XANIM, XMODEL, XIMAGE, RAWFILE RELATED...)
            auto AnimPoolData       = CoDAssets::GameInstance->Read<BOCWXAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BOCWXAssetPoolData) * 5));
            auto ModelPoolData      = CoDAssets::GameInstance->Read<BOCWXAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BOCWXAssetPoolData) * 6));
            auto ImagePoolData      = CoDAssets::GameInstance->Read<BOCWXAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BOCWXAssetPoolData) * 0x10));
            auto MaterialPoolData   = CoDAssets::GameInstance->Read<BOCWXAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BOCWXAssetPoolData) * 10));
            auto SoundAssetPoolData = CoDAssets::GameInstance->Read<BOCWXAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BOCWXAssetPoolData) * 19));

            // Apply game offset info
            CoDAssets::GameOffsetInfos.emplace_back(AnimPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ModelPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ImagePoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(MaterialPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(SoundAssetPoolData.PoolPtr);

            // Verify via first xmodel asset, right now, we're using a hash
            auto FirstXModelHash = CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1]);
            // Check
            if (FirstXModelHash == 0x04647533e968c910)
            {
                // Validate sizes
                if (
                    AnimPoolData.AssetSize == sizeof(BOCWXAnim) &&
                    ModelPoolData.AssetSize == sizeof(BOCWXModel) &&
                    ImagePoolData.AssetSize == sizeof(BOCWGfxImage))
                {
                    // Verify string table, otherwise we are all set
                    CoDAssets::GameOffsetInfos.emplace_back(BaseAddress + GameOffsets.StringTable);
                    // Read and apply sizes
                    CoDAssets::GamePoolSizes.emplace_back(AnimPoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(ModelPoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(ImagePoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(MaterialPoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(SoundAssetPoolData.PoolSize);
                    // Return success
                    return true;
                }
            }
            // Reset
            CoDAssets::GameOffsetInfos.clear();
        }

        // Attempt to locate via heuristic searching
        auto DBAssetsScan = CoDAssets::GameInstance->Scan("40 53 48 83 EC ?? 0F B6 ?? 48 8D 05 ?? ?? ?? ?? 48 C1 E2 05");
        auto StringTableScan = CoDAssets::GameInstance->Scan("48 8B 53 ?? 48 85 D2 74 ?? 48 8B 03 48 89 02");

        // Check that we had hits
        if (DBAssetsScan > 0 && StringTableScan > 0)
        {
            // Load info and verify
            auto GameOffsets = DBGameInfo(
                // Resolve pool info from LEA
                CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan + 0xC) + (DBAssetsScan + 0x10),
                // We don't use size offsets
                0,
                // Resolve strings from LEA
                CoDAssets::GameInstance->Read<uint32_t>(StringTableScan + 0x12) + (StringTableScan + 0x16),
                // We don't use package offsets
                0
            );

            // In debug, print the info for easy additions later!
#if _DEBUG
            // Format the output
            printf("Heuristic: { 0x%llX, 0x0, 0x%llX, 0x0 }\n", (GameOffsets.DBAssetPools - BaseAddress), (GameOffsets.StringTable - BaseAddress));
#endif


            // Read required offsets (XANIM, XMODEL, XIMAGE)
            auto AnimPoolData = CoDAssets::GameInstance->Read<BOCWXAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BOCWXAssetPoolData) * 5));
            auto ModelPoolData = CoDAssets::GameInstance->Read<BOCWXAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BOCWXAssetPoolData) * 6));
            auto ImagePoolData = CoDAssets::GameInstance->Read<BOCWXAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BOCWXAssetPoolData) * 0x10));
            auto MaterialPoolData = CoDAssets::GameInstance->Read<BOCWXAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BOCWXAssetPoolData) * 10));
            auto SoundAssetPoolData = CoDAssets::GameInstance->Read<BOCWXAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BOCWXAssetPoolData) * 19));

            // Apply game offset info
            CoDAssets::GameOffsetInfos.emplace_back(AnimPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ModelPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(ImagePoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(MaterialPoolData.PoolPtr);
            CoDAssets::GameOffsetInfos.emplace_back(SoundAssetPoolData.PoolPtr);

            // Verify via first xmodel asset, right now, we're using a hash
            auto FirstXModelHash = CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1]);

            // Check
            if (FirstXModelHash == 0x04647533e968c910)
            {
                // Validate sizes
                if (
                    AnimPoolData.AssetSize  == sizeof(BOCWXAnim) && 
                    ModelPoolData.AssetSize == sizeof(BOCWXModel) && 
                    ImagePoolData.AssetSize == sizeof(BOCWGfxImage))
                {
                    // Verify string table, otherwise we are all set
                    CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);

                    // Read and apply sizes
                    CoDAssets::GamePoolSizes.emplace_back(AnimPoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(ModelPoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(ImagePoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(MaterialPoolData.PoolSize);
                    CoDAssets::GamePoolSizes.emplace_back(SoundAssetPoolData.PoolSize);

                    // Return success
                    return true;
                }
            }
        }
    }

    // Failed
    return false;
}

bool GameBlackOpsCW::LoadAssets()
{
    // Prepare to load game assets, into the AssetPool
    bool NeedsAnims     = (SettingsManager::GetSetting("showxanim",     "true")         == "true");
    bool NeedsModels    = (SettingsManager::GetSetting("showxmodel",    "true")         == "true");
    bool NeedsImages    = (SettingsManager::GetSetting("showximage",    "false")        == "true");
    bool NeedsRawFiles  = (SettingsManager::GetSetting("showxrawfiles", "false")        == "true");
    bool NeedsMaterials = (SettingsManager::GetSetting("showxmtl",      "false")        == "true");
    bool NeedsSounds    = (SettingsManager::GetSetting("showxsounds",   "false")        == "true");

    /*
        This was implemented as a fix for a specific user who requested it, as the search box is capped at 32767 by Windows
        and this is a workaround, if you're interested in using it, any hashes in this filters file will be ignored on load,
        essentially acting as an excluder, consider it a hidden feature with no support as it was made for a specific use
        case. If you cannot get it to work, do not ask me.
    */
    auto Filters = WraithNameIndex();
    Filters.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bocw_filters.wni"));

    // Check if we need assets
    if (NeedsAnims)
    {
        // Parse the XAnim pool
        CoDXPoolParser<uint64_t, BOCWXAnim>((CoDAssets::GameOffsetInfos[0]), CoDAssets::GamePoolSizes[0], [Filters](BOCWXAnim& Asset, uint64_t& AssetOffset)
        {
            // Mask the name as hashes are 60Bit
            Asset.NamePtr &= 0xFFFFFFFFFFFFFFF;

            // Check for filters
            if (Filters.NameDatabase.size() > 0)
            {
                // Check for this asset in DB
                if (Filters.NameDatabase.find(Asset.NamePtr) != Filters.NameDatabase.end())
                {
                    // Skip this asset
                    return;
                }
            }

            // Validate and load if need be
            auto AnimName = Strings::Format("xanim_%llx", Asset.NamePtr);

            // Check for an override in the name DB
            if (AssetNameCache.NameDatabase.find(Asset.NamePtr) != AssetNameCache.NameDatabase.end())
                AnimName = AssetNameCache.NameDatabase[Asset.NamePtr];

            // Log it
            CoDAssets::LogXAsset("Anim", AnimName);

            // Make and add
            auto LoadedAnim = new CoDAnim_t();
            // Set
            LoadedAnim->AssetName    = AnimName;
            LoadedAnim->AssetPointer = AssetOffset;
            LoadedAnim->Framerate    = Asset.Framerate;
            LoadedAnim->FrameCount   = Asset.NumFrames;
            LoadedAnim->BoneCount    = Asset.TotalBoneCount;
            LoadedAnim->AssetStatus  = WraithAssetStatus::Loaded;
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedAnim);
        });
    }

    if (NeedsModels)
    {
        // Parse the XModel pool
        CoDXPoolParser<uint64_t, BOCWXModel>((CoDAssets::GameOffsetInfos[1]), CoDAssets::GamePoolSizes[1], [Filters](BOCWXModel& Asset, uint64_t& AssetOffset)
        {
            // Mask the name as hashes are 60Bit
            Asset.NamePtr &= 0xFFFFFFFFFFFFFFF;

            // Check for filters
            if (Filters.NameDatabase.size() > 0)
            {
                // Check for this asset in DB
                if (Filters.NameDatabase.find(Asset.NamePtr) != Filters.NameDatabase.end())
                {
                    // Skip this asset
                    return;
                }
            }

            // XSkeleton for bone info
            auto XSkeleton = CoDAssets::GameInstance->Read<BOCWXSkeleton>(Asset.XSkeletonPtr);

            // Validate and load if need be
            auto ModelName = Strings::Format("xmodel_%llx", Asset.NamePtr);

            // Check for an override in the name DB
            if (AssetNameCache.NameDatabase.find(Asset.NamePtr) != AssetNameCache.NameDatabase.end())
                ModelName = AssetNameCache.NameDatabase[Asset.NamePtr];

            // Log it
            CoDAssets::LogXAsset("Model", ModelName);

            // Make and add
            auto LoadedModel = new CoDModel_t();
            // Set
            LoadedModel->AssetName         = ModelName;
            LoadedModel->AssetPointer      = AssetOffset;
            LoadedModel->BoneCount         = XSkeleton.BoneCounts[0] + XSkeleton.BoneCounts[1];
            LoadedModel->LodCount          = Asset.NumLods;
            LoadedModel->AssetStatus       = WraithAssetStatus::Loaded;
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedModel);
        });
    }

    if (NeedsImages)
    {
        // Parse the XModel pool
        CoDXPoolParser<uint64_t, BOCWGfxImage>((CoDAssets::GameOffsetInfos[2]), CoDAssets::GamePoolSizes[2], [Filters](BOCWGfxImage& Asset, uint64_t& AssetOffset)
        {
            // Mask the name as hashes are 60Bit
            Asset.NamePtr &= 0xFFFFFFFFFFFFFFF;

            // Check for filters
            if (Filters.NameDatabase.size() > 0)
            {
                // Check for this asset in DB
                if (Filters.NameDatabase.find(Asset.NamePtr) != Filters.NameDatabase.end())
                {
                    // Skip this asset
                    return;
                }
            }

            // Validate and load if need be
            auto ImageName = Strings::Format("ximage_%llx", Asset.NamePtr);

            // Check for an override in the name DB
            if (AssetNameCache.NameDatabase.find(Asset.NamePtr) != AssetNameCache.NameDatabase.end())
                ImageName = AssetNameCache.NameDatabase[Asset.NamePtr];

            // Log it
            CoDAssets::LogXAsset("Image", ImageName);

            // Check for loaded images
            if (Asset.GfxMipsPtr != 0)
            {
                // Make and add
                auto LoadedImage = new CoDImage_t();
                // Set
                LoadedImage->AssetName    = ImageName;
                LoadedImage->AssetPointer = AssetOffset;
                LoadedImage->Width        = (uint16_t)Asset.LoadedMipWidth;
                LoadedImage->Height       = (uint16_t)Asset.LoadedMipHeight;
                LoadedImage->Format       = (uint16_t)Asset.ImageFormat;
                LoadedImage->AssetStatus  = WraithAssetStatus::Loaded;
                // Add
                CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
            }
        });
    }

    if (NeedsMaterials)
    {
        // Parse the XModel pool
        CoDXPoolParser<uint64_t, BOCWXMaterial>((CoDAssets::GameOffsetInfos[3]), CoDAssets::GamePoolSizes[3], [Filters](BOCWXMaterial& Asset, uint64_t& AssetOffset)
        {
            // Mask the name as hashes are 60Bit
            Asset.NamePtr &= 0xFFFFFFFFFFFFFFF;

            // Check for filters
            if (Filters.NameDatabase.size() > 0)
            {
                // Check for this asset in DB
                if (Filters.NameDatabase.find(Asset.NamePtr) != Filters.NameDatabase.end())
                {
                    // Skip this asset
                    return;
                }
            }

            // Validate and load if need be
            auto MaterialName = Strings::Format("xmaterial_%llx", Asset.NamePtr);

            // Check for an override in the name DB
            if (AssetNameCache.NameDatabase.find(Asset.NamePtr) != AssetNameCache.NameDatabase.end())
                MaterialName = AssetNameCache.NameDatabase[Asset.NamePtr];

            // Log it
            CoDAssets::LogXAsset("Material", MaterialName);

            // Make and add
            auto LoadedImage = new CoDMaterial_t();
            // Set
            LoadedImage->AssetName    = MaterialName;
            LoadedImage->AssetPointer = AssetOffset;
            LoadedImage->ImageCount   = Asset.ImageCount;
            LoadedImage->AssetStatus  = WraithAssetStatus::Loaded;
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
        });
    }

    if (NeedsSounds)
    {
        // Parse the XModel pool
        CoDXPoolParser<uint64_t, BOCWSoundAsset>((CoDAssets::GameOffsetInfos[4]), CoDAssets::GamePoolSizes[4], [](BOCWSoundAsset& Asset, uint64_t& AssetOffset)
        {
            // Mask the name as hashes are 60Bit
            Asset.NamePtr &= 0xFFFFFFFFFFFFFFF;

            // Validate and load if need be
            auto SoundName = Strings::Format("xsound_%llx", Asset.NamePtr);

            // Check for an override in the name DB
            if (AssetNameCache.NameDatabase.find(Asset.NamePtr) != AssetNameCache.NameDatabase.end())
                SoundName = AssetNameCache.NameDatabase[Asset.NamePtr];

            // Log it
            CoDAssets::LogXAsset("Sound", SoundName);

            // Make and add
            auto LoadedSound = new CoDSound_t();
            // Set the name, but remove all extensions first
            LoadedSound->AssetName    = FileSystems::GetFileNamePurgeExtensions(SoundName);
            LoadedSound->FullPath     = FileSystems::GetDirectoryName(SoundName);
            LoadedSound->AssetPointer = AssetOffset;
            LoadedSound->AssetStatus  = WraithAssetStatus::Loaded;
            // Set various properties
            LoadedSound->FrameRate     = GetFrameRate(Asset.FrameRateIndex);
            LoadedSound->FrameCount    = Asset.FrameCount;
            LoadedSound->ChannelsCount = Asset.ChannelCount;
            LoadedSound->AssetSize     = -1;
            LoadedSound->AssetStatus   = WraithAssetStatus::Loaded;
            LoadedSound->IsFileEntry   = false;
            LoadedSound->Length        = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
           
        });
    }

    // Success, error only on specific load
    return true;
}

std::unique_ptr<XAnim_t> GameBlackOpsCW::ReadXAnim(const CoDAnim_t* Animation)
{
    // TODO: Look at streamed XAnims

    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Prepare to read the xanim
        auto Anim = std::make_unique<XAnim_t>();
        // Read the XAnim structure
        auto AnimData = CoDAssets::GameInstance->Read<BOCWXAnim>(Animation->AssetPointer);
        // Copy over default properties
        Anim->AnimationName = Animation->AssetName;
        // Frames and Rate
        Anim->FrameCount = AnimData.NumFrames;
        Anim->FrameRate = AnimData.Framerate;

        //// Check for viewmodel animations
        //if ((_strnicmp(Animation->AssetName.c_str(), "viewmodel_", 10) == 0) || (_strnicmp(Animation->AssetName.c_str(), "vm_", 3) == 0))
        //{
        //    // This is a viewmodel animation
        //    Anim->ViewModelAnimation = true;
        //}
        ////// Check for additive animations
        ////if (AnimData.AssetType == 0x6)
        ////{
        ////    // This is a additive animation
        ////    Anim->AdditiveAnimation = true;
        ////}
        ////// Check for looping
        ////Anim->LoopingAnimation = (AnimData.LoopingFlag > 0);

        // Read the delta data
        auto AnimDeltaData = CoDAssets::GameInstance->Read<BO4XAnimDeltaParts>(AnimData.DeltaPartsPtr);

        // Copy over pointers
        Anim->BoneIDsPtr          = AnimData.BoneIDsPtr;
        Anim->DataBytesPtr        = AnimData.DataBytePtr;
        Anim->DataShortsPtr       = AnimData.DataShortPtr;
        Anim->DataIntsPtr         = AnimData.DataIntPtr;
        Anim->RandomDataBytesPtr  = AnimData.RandomDataBytePtr;
        Anim->RandomDataShortsPtr = AnimData.RandomDataShortPtr;
        Anim->NotificationsPtr    = AnimData.NotificationsPtr;

        // Bone ID index size
        Anim->BoneIndexSize = 4;

        // Copy over counts
        Anim->NoneRotatedBoneCount         = AnimData.NoneRotatedBoneCount;
        Anim->TwoDRotatedBoneCount         = AnimData.TwoDRotatedBoneCount;
        Anim->NormalRotatedBoneCount       = AnimData.NormalRotatedBoneCount;
        Anim->TwoDStaticRotatedBoneCount   = AnimData.TwoDStaticRotatedBoneCount;
        Anim->NormalStaticRotatedBoneCount = AnimData.NormalStaticRotatedBoneCount;
        Anim->NormalTranslatedBoneCount    = AnimData.NormalTranslatedBoneCount;
        Anim->PreciseTranslatedBoneCount   = AnimData.PreciseTranslatedBoneCount;
        Anim->StaticTranslatedBoneCount    = AnimData.StaticTranslatedBoneCount;
        Anim->NoneTranslatedBoneCount      = AnimData.NoneTranslatedBoneCount;
        Anim->TotalBoneCount               = AnimData.TotalBoneCount;
        Anim->NotificationCount            = AnimData.NotificationCount;

        // Copy delta
        Anim->DeltaTranslationPtr = AnimDeltaData.DeltaTranslationsPtr;
        Anim->Delta2DRotationsPtr = AnimDeltaData.Delta2DRotationsPtr;
        Anim->Delta3DRotationsPtr = AnimDeltaData.Delta3DRotationsPtr;

        // Set types, we use quata for BO4
        Anim->RotationType = AnimationKeyTypes::QuatPackingA;
        Anim->TranslationType = AnimationKeyTypes::MinSizeTable;

        // Black Ops CW doesn't support inline indicies
        Anim->SupportsInlineIndicies = false;

        // Return it
        return Anim;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XModel_t> GameBlackOpsCW::ReadXModel(const CoDModel_t* Model)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Read the XModel structure
        auto ModelData = CoDAssets::GameInstance->Read<BOCWXModel>(Model->AssetPointer);
        // XSkeleton for bone info
        auto XSkeleton = CoDAssets::GameInstance->Read<BOCWXSkeleton>(ModelData.XSkeletonPtr);

        // Prepare to read the xmodel (Reserving space for lods)
        auto ModelAsset = std::make_unique<XModel_t>(ModelData.NumLods);

        // Copy over default properties
        ModelAsset->ModelName = Model->AssetName;
        // Bone counts
        ModelAsset->BoneCount = XSkeleton.BoneCounts[1];
        ModelAsset->RootBoneCount = XSkeleton.BoneCounts[2];
        ModelAsset->CosmeticBoneCount = XSkeleton.BoneCounts[0];

        // Bone data type
        ModelAsset->BoneRotationData = BoneDataTypes::QuatPackingA;

        // We are streamed
        ModelAsset->IsModelStreamed = true;

        // Bone id info
        ModelAsset->BoneIDsPtr = XSkeleton.BoneIDsPtr;
        ModelAsset->BoneIndexSize = 4;

        // Bone parent info
        ModelAsset->BoneParentsPtr = XSkeleton.ParentListPtr;
        ModelAsset->BoneParentSize = 2;

        // Local bone pointers
        ModelAsset->RotationsPtr = XSkeleton.RotationsPtr;
        ModelAsset->TranslationsPtr = XSkeleton.TranslationsPtr;

        // Global matricies
        ModelAsset->BaseMatriciesPtr = XSkeleton.BaseMatriciesPtr;

        // Prepare to parse lods
        for (uint32_t i = 0; i < ModelData.NumLods; i++)
        {
            // Read the lod
            auto LODInfo = CoDAssets::GameInstance->Read<BOCWXModelLod>(ModelData.ModelLodPtrs[i]);
            // Create the lod and grab reference
            ModelAsset->ModelLods.emplace_back(LODInfo.NumSurfs);
            // Grab reference
            auto& LodReference = ModelAsset->ModelLods[i];

            // Set distance
            LodReference.LodDistance = LODInfo.LodDistance;

            // Set stream key and info ptr
            LodReference.LODStreamKey = LODInfo.LODStreamKey;
            LodReference.LODStreamInfoPtr = LODInfo.XModelMeshPtr;

            // Grab pointer from the lod itself
            auto XSurfacePtr = LODInfo.XSurfacePtr;

            // Skip 8 bytes in materials
            ModelData.MaterialHandlesPtr += 8;
            // Read material handles ptr
            auto MaterialHandlesPtr = CoDAssets::GameInstance->Read<uint64_t>(ModelData.MaterialHandlesPtr);
            // Advance 8 and skip 24 bytes
            ModelData.MaterialHandlesPtr += 0x18;

            // Load surfaces
            for (uint32_t s = 0; s < LODInfo.NumSurfs; s++)
            {
                // Create the surface and grab reference
                LodReference.Submeshes.emplace_back();
                // Grab reference
                auto& SubmeshReference = LodReference.Submeshes[s];

                // Read the surface data
                auto SurfaceInfo = CoDAssets::GameInstance->Read<BOCWXModelSurface>(XSurfacePtr);
                // Apply surface info
                SubmeshReference.VertexCount = SurfaceInfo.VertexCount;
                SubmeshReference.FaceCount = SurfaceInfo.FacesCount;
                SubmeshReference.VertexPtr = SurfaceInfo.VerticiesIndex;
                SubmeshReference.FacesPtr = SurfaceInfo.FacesIndex;

                // Assign weight info to the count slots, to save memory
                SubmeshReference.WeightCounts[0] = SurfaceInfo.Flag1;
                SubmeshReference.WeightCounts[1] = SurfaceInfo.Flag2;
                //SubmeshReference.WeightCounts[2] = SurfaceInfo.Flag3;
                //SubmeshReference.WeightCounts[3] = SurfaceInfo.Flag4;

                // Read this submesh's material handle
                auto MaterialHandle = CoDAssets::GameInstance->Read<uint64_t>(MaterialHandlesPtr);
                // Create the material and add it
                LodReference.Materials.emplace_back(ReadXMaterial(MaterialHandle));

                // Advance
                XSurfacePtr += sizeof(BOCWXModelSurface);
                MaterialHandlesPtr += sizeof(uint64_t);
            }
        }

        // Return it
        return ModelAsset;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XImageDDS> GameBlackOpsCW::ReadXImage(const CoDImage_t* Image)
{
    // Proxy off
    return LoadXImage(XImage_t(ImageUsageType::DiffuseMap, 0, Image->AssetPointer, Image->AssetName));
}

std::unique_ptr<XSound> GameBlackOpsCW::ReadXSound(const CoDSound_t * Sound)
{
    // Read the Sound Asset structure
    auto SoundData = CoDAssets::GameInstance->Read<BOCWSoundAsset>(Sound->AssetPointer);
    // Buffer
    std::unique_ptr<uint8_t[]> SoundBuffer = nullptr;

    // Offset to the data, depending on the buffer
    uint32_t OpusDataSize       = 0;
    uint32_t OpusDataOffset     = 0;
    uint32_t OpusConsumed       = 0;
    uint32_t PCMDataSize        = 0;

    // Check if we need raw data or standard xpak
    if (SoundData.StreamKey != 0)
    {
        // Load raw buffer, these aren't compressed
        uint32_t SoundMemoryResult = 0;
        SoundBuffer = CoDAssets::GamePackageCache->ExtractPackageObjectRaw(SoundData.StreamKey, SoundMemoryResult);

        if (SoundMemoryResult == 0)
            return nullptr;

        OpusDataSize = SoundMemoryResult;
    }
    else
    {
        // Extract buffer, these are compressed
        uint32_t SoundMemoryResult = 0;
        SoundBuffer = CoDAssets::GamePackageCache->ExtractPackageObject(CoDAssets::GameInstance->Read<uint64_t>(SoundData.StreamInfoPtr + 0x8), SoundMemoryResult);

        if (SoundMemoryResult == 0)
            return nullptr;

        OpusDataSize = *(uint32_t*)(SoundBuffer.get() + 280);
        OpusDataOffset = 288;
    }

    
    if(SoundBuffer == nullptr)
        return nullptr;

    // Initialize Opus
    int ErrorCode;
    auto Decoder = opus_decoder_create(GetFrameRate(SoundData.FrameRateIndex), SoundData.ChannelCount, &ErrorCode);

    if (ErrorCode != OPUS_OK)
        return nullptr;

    // Create new buffer for each 960 frame block
    auto PCMBuffer = std::make_unique<opus_int16[]>(960 * 2 * (size_t)SoundData.ChannelCount);

    // Output Writer (Note: Frame Count in Asset isn't 100% accurate, so we add on some padding
    //                      to account for 960 frame split so that we can avoid realloc)
    MemoryWriter TempWriter((SoundData.FrameCount + 4096) * 2 * SoundData.ChannelCount);

    // Consume Opus Data
    while (OpusConsumed < OpusDataSize)
    {
        uint32_t BlockSize = *(uint32_t*)(SoundBuffer.get() + OpusConsumed + OpusDataOffset);
        OpusConsumed += 4;

        auto DecoderResult = opus_decode(
            Decoder,
            (SoundBuffer.get() + OpusConsumed + OpusDataOffset),
            BlockSize,
            PCMBuffer.get(),
            960,
            0);

        // Any negative is a failure in Opus
        if (DecoderResult < 0)
            return nullptr;

        // Output to Buffer
        TempWriter.Write((uint8_t*)PCMBuffer.get(), 960 * 2 * SoundData.ChannelCount);

        // Advance Info
        OpusConsumed       += BlockSize;
        PCMDataSize        += 960 * 2 * SoundData.ChannelCount;
    }

    // Prepare to read the sound data, for WAV, we must include a WAV header...
    auto Result = std::make_unique<XSound>();

    // The offset of which to store the data
    uint32_t DataOffset = 0;

    // We'll convert all BOCW sounds to 16bit PCM WAV
    Result->DataBuffer = new int8_t[PCMDataSize + (size_t)Sound::GetMaximumWAVHeaderSize()];
    Result->DataType = SoundDataTypes::WAV_WithHeader;
    Result->DataSize = (uint32_t)(PCMDataSize + Sound::GetMaximumWAVHeaderSize());

    DataOffset += Sound::GetMaximumWAVHeaderSize();

    // Make the header
    Sound::WriteWAVHeaderToStream(Result->DataBuffer, (uint32_t)Sound->FrameRate, (uint32_t)Sound->ChannelsCount, PCMDataSize);

    // Copy output
    std::memcpy(Result->DataBuffer + DataOffset, TempWriter.GetCurrentStream(), PCMDataSize);

    return Result;
}

#pragma pack(push, 1)
struct BOCWXMaterialImage
{
    uint64_t ImagePtr;
    uint32_t SemanticHash;
    uint8_t Padding[0xC];
};
#pragma pack(pop)

const XMaterial_t GameBlackOpsCW::ReadXMaterial(uint64_t MaterialPointer)
{
    // Prepare to parse the material
    auto MaterialData = CoDAssets::GameInstance->Read<BOCWXMaterial>(MaterialPointer);
    // Mask the name (some bits are used for other stuffs)
    MaterialData.NamePtr &= 0xFFFFFFFFFFFFFFF;
    // Allocate a new material with the given image count
    XMaterial_t Result(MaterialData.ImageCount);
    // Clean the name, then apply it
    Result.MaterialName = Strings::Format("xmaterial_%llx", MaterialData.NamePtr);
    // Clean the tech name, then apply it
    Result.TechsetName = Strings::Format("xtechset_%llx", CoDAssets::GameInstance->Read<uint64_t>(MaterialData.TechsetPtr));

    // Check for an override in the name DB
    if (AssetNameCache.NameDatabase.find(MaterialData.NamePtr) != AssetNameCache.NameDatabase.end())
        Result.MaterialName = FileSystems::GetFileNamePurgeExtensions(AssetNameCache.NameDatabase[MaterialData.NamePtr]);

    // Iterate over material images, assign proper references if available
    for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
    {
        // Read the image info
        auto ImageInfo = CoDAssets::GameInstance->Read<BOCWXMaterialImage>(MaterialData.ImageTablePtr);

        // Get Hash and mask it (some bits are used for other stuffs)
        auto ImageHash = CoDAssets::GameInstance->Read<uint64_t>(ImageInfo.ImagePtr) & 0xFFFFFFFFFFFFFFF;

        // Get the image name
        auto ImageName = Strings::Format("ximage_%llx", ImageHash);

        // Check for an override in the name DB
        if (AssetNameCache.NameDatabase.find(ImageHash) != AssetNameCache.NameDatabase.end())
            ImageName = AssetNameCache.NameDatabase[ImageHash];

        // Default type
        auto DefaultUsage = ImageUsageType::Unknown;
        // Check 
        switch (CoDAssets::GameInstance->Read<uint8_t>(ImageInfo.ImagePtr + 180))
        {
        case 0x2:
            DefaultUsage = ImageUsageType::DiffuseMap;
            break;
        case 0x4:
            DefaultUsage = ImageUsageType::NormalMap;
            break;
        case 0x6:
            DefaultUsage = ImageUsageType::SpecularMap;
            break;
        case 0x7:
            DefaultUsage = ImageUsageType::GlossMap;
            break;
        }

        // Assign the new image
        Result.Images.emplace_back(DefaultUsage, ImageInfo.SemanticHash, ImageInfo.ImagePtr, ImageName);

        // Advance
        MaterialData.ImageTablePtr += sizeof(BOCWXMaterialImage);
    }

    // Return it
    return Result;
}

std::unique_ptr<XImageDDS> GameBlackOpsCW::LoadXImage(const XImage_t& Image)
{
    // Prepare to load an image, we need to rip loaded and streamed ones
    uint32_t ResultSize = 0;

    // We must read the image data
    auto ImageInfo = CoDAssets::GameInstance->Read<BOCWGfxImage>(Image.ImagePtr);

    // Calculate the largest image mip
    uint32_t LargestMip = 0;
    uint32_t LargestSize = 0;
    uint32_t LargestWidth = 0;
    uint32_t LargestHeight = 0;
    uint64_t LargestHash = 0;

    // Loop and calculate
    for (uint32_t i = 0; i < ImageInfo.GfxMipMaps; i++)
    {
        // Load Mip Map
        auto MipMap = CoDAssets::GameInstance->Read<BOCWGfxMip>(ImageInfo.GfxMipsPtr);
        // Compare widths, checking if it exists for users without HD Texture Packs
        if (MipMap.Size > LargestSize && MipMap.HashID != 0 && CoDAssets::GamePackageCache->Exists(MipMap.HashID))
        {
            LargestMip    = i;
            LargestSize   = MipMap.Size;
            LargestHash   = MipMap.HashID;
            LargestWidth  = ImageInfo.LoadedMipWidth >> (ImageInfo.GfxMipMaps - i - 1);
            LargestHeight = ImageInfo.LoadedMipHeight >> (ImageInfo.GfxMipMaps - i - 1);
        }
        // Advance Mip Map Pointer
        ImageInfo.GfxMipsPtr += sizeof(BOCWGfxMip);
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
    if (LargestHash == 0)
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
        ImageData = CoDAssets::GamePackageCache->ExtractPackageObject(LargestHash, ResultSize);
    }

    // Prepare if we have it
    if (ImageData != nullptr)
    {
        // Prepare to create a MemoryDDS file
        auto Result = CoDRawImageTranslator::TranslateBC(ImageData, ResultSize, LargestWidth, LargestHeight, ImageInfo.ImageFormat);

        // Check for, and apply patch if required, if we got a raw result
        if (Result != nullptr)
        {
            if (ImageInfo.ImageSemantic == 0x4 && (SettingsManager::GetSetting("patchnormals", "true") == "true"))
            {
                // Set normal map patch
                Result->ImagePatchType = ImagePatch::Normal_Expand;
            }
            else if (ImageInfo.ImageSemantic == 0x7 || ImageInfo.ImageSemantic == 0x1C) //&& (SettingsManager::GetSetting("patchgloss", "true") == "true")
            {
                // Set gloss map patch
                Result->ImagePatchType = ImagePatch::Gloss_Roughness;
            }
        }

        // Return it
        return Result;
    }

    // Failed to load the image
    return nullptr;
}

void GameBlackOpsCW::LoadXModel(const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel)
{
    // Check if we want Vertex Colors
    bool ExportColors = (SettingsManager::GetSetting("exportvtxcolor", "true") == "true");
    // Read the mesh information
    auto MeshInfo = CoDAssets::GameInstance->Read<BO4XModelMeshInfo>(ModelLOD.LODStreamInfoPtr);

    // A buffer for the mesh data
    std::unique_ptr<uint8_t[]> MeshDataBuffer = nullptr;
    // Resulting size
    uint64_t MeshDataBufferSize = 0;

    // Vertex has extended vertex information
    bool HasExtendedVertexInfo = (MeshInfo.StatusFlag & 64) != 0;

    // Determine if we need to load the mesh or not (Seems flag == 8 is loaded)
    if ((MeshInfo.StatusFlag & 0x3F) == 8)
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
        MeshDataBuffer = CoDAssets::GamePackageCache->ExtractPackageObject(ModelLOD.LODStreamKey, ResultSize);
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

            // Jump to vertex info data, advance to this submeshes info, seek further for extended vertex info
            MeshReader.SetPosition(MeshInfo.UVOffset + (Submesh.VertexPtr * (HasExtendedVertexInfo ? 24 : 16)));

            // Iterate over verticies
            for (uint32_t i = 0; i < Submesh.VertexCount; i++)
            {
                // Grab the reference
                auto& Vertex = Mesh.Verticies[i];

                // Read vertex data
                auto VertexData = MeshReader.Read<GfxStreamVertex>();

                // Add UV layer
                Vertex.AddUVLayer(HalfFloats::ToFloat(VertexData.UVUPosition), HalfFloats::ToFloat(VertexData.UVVPosition));
                // Unpack normal
                Vertex.Normal.X = ((float)(((VertexData.VertexNormal >> 00) & ((1 << 10) - 1)) - 512) / 511.0f);
                Vertex.Normal.Y = ((float)(((VertexData.VertexNormal >> 10) & ((1 << 10) - 1)) - 512) / 511.0f);
                Vertex.Normal.Z = ((float)(((VertexData.VertexNormal >> 20) & ((1 << 10) - 1)) - 512) / 511.0f);

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

                // Skip extended vertex information (first 4 bytes seems to be UV, possibly for better camo UV Mapping)
                if (HasExtendedVertexInfo)
                    MeshReader.Advance(8);
            }

            // Jump to vertex weight data, advance to this submeshes info
            MeshReader.SetPosition(MeshInfo.WeightsOffset + (Submesh.VertexPtr * 12));

            // Iterate over verticies
            for (uint32_t i = 0; i < Submesh.VertexCount; i++)
            {
                // Grab the reference
                auto& Vertex = Mesh.Verticies[i];
                
                // Check if we're a complex weight, up to four weights
                if (((uint8_t)Submesh.WeightCounts[0] & 2) > 0)
                {
                    // Read weight data
                    auto VertexWeight = MeshReader.Read<GfxStreamWeight>();

                    // Add weights for each index if it exists
                    // Mask off bit, seems to be blendshape related
                    Vertex.AddVertexWeight(VertexWeight.WeightID1 & ~32768, (VertexWeight.WeightVal1 / 255.0f));

                    if (VertexWeight.WeightVal2 > 0)
                        Vertex.AddVertexWeight(VertexWeight.WeightID2 & ~32768, (VertexWeight.WeightVal2 / 255.0f));
                    if (VertexWeight.WeightVal3 > 0)
                        Vertex.AddVertexWeight(VertexWeight.WeightID3 & ~32768, (VertexWeight.WeightVal3 / 255.0f));
                    if (VertexWeight.WeightVal4 > 0)
                        Vertex.AddVertexWeight(VertexWeight.WeightID4 & ~32768, (VertexWeight.WeightVal4 / 255.0f));
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
    }
}

std::string GameBlackOpsCW::LoadStringEntry(uint64_t Index)
{
    // Calculate Offset to String (Offsets[3] = StringTable)
    auto Offset = CoDAssets::GameOffsetInfos[5] + (Index * 16);
    // Read Info
    auto StringHash = CoDAssets::GameInstance->Read<uint64_t>(Offset + 16) & 0xFFFFFFFFFFFFFFF;

    // Attempt to locate string
    auto StringEntry = StringCache.NameDatabase.find(StringHash);

    // Not Encrypted
    if (StringEntry != StringCache.NameDatabase.end())
        return StringEntry->second;
    else
        return Strings::Format("xstring_%llx", StringHash);
}
void GameBlackOpsCW::PerformInitialSetup()
{
    // Load Caches
    AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(),    "package_index\\fnv1a_xmaterials.wni"));
    AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(),    "package_index\\fnv1a_ximages.wni"));
    AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(),    "package_index\\fnv1a_xsounds.wni"));
    AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(),    "package_index\\fnv1a_xmodels.wni"));
    AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(),    "package_index\\fnv1a_xanims.wni"));
    StringCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(),       "package_index\\fnv1a_string.wni"));
    // Prepare to copy the oodle dll
    auto OurPath = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "oo2core_8_win64.dll");
    // Copy if not exists
    if (!FileSystems::FileExists(OurPath))
        FileSystems::CopyFile(FileSystems::CombinePath(FileSystems::GetDirectoryName(CoDAssets::GameInstance->GetProcessPath()), "oo2core_8_win64.dll"), OurPath);
}
