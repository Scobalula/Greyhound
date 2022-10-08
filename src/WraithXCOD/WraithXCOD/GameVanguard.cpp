#include "stdafx.h"

// The class we are implementing
#include "GameVanguard.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDRawImageTranslator.h"
#include "CoDXPoolParser.h"
#include "DBGameFiles.h"

// We need the SAB class for decoding
#include "SABSupport.h"

// We need the following WraithX classes
#include "Strings.h"
#include "FileSystems.h"
#include "MemoryReader.h"
#include "TextWriter.h"
#include "SettingsManager.h"
#include "HalfFloats.h"

// -- Initialize built-in game offsets databases

WraithNameIndex GameVanguard::StringCache = WraithNameIndex();
WraithNameIndex GameVanguard::AssetNameCache = WraithNameIndex();

// Vanguard SP
std::array<DBGameInfo, 1> GameVanguard::SinglePlayerOffsets =
{{
    { 0xE1AF540, 0x0, 0xF9B1000, 0x0 }
}};

// -- Finished with databases

// -- Begin XModelStream structures

struct VGGfxRigidVerts
{
    uint16_t BoneIndex;
    uint16_t VertexCount;
    uint16_t FacesCount;
    uint16_t FacesIndex;
};

#pragma pack(push, 1)
struct VGGfxStreamVertex
{
    uint64_t PackedPosition; // Packed 21bits, scale + offset in Mesh Info
    uint32_t BiNormal;
    uint16_t UVUPosition;
    uint16_t UVVPosition;
    uint32_t NormalQuaternion;
};
#pragma pack(pop)

struct VGGfxStreamFace
{
    uint16_t Index1;
    uint16_t Index2;
    uint16_t Index3;
};

// -- End XModelStream structures

// Calculates the hash of a sound string
uint64_t VGHashSoundString(const std::string& Value)
{
    uint64_t Result = 0xCBF29CE484222325;

    for (uint32_t i = 0; i < Value.length(); i++)
    {
        Result ^= tolower(Value[i]);
        Result *= 0x100000001B3;
    }

    return Result;
}

// Verify that our pool data is exactly 0x20
static_assert(sizeof(VGXAssetPoolData) == 0x18, "Invalid Pool Data Size (Expected 0x18)");

struct VGGfxImage
{
    uint64_t NamePtr;
    uint8_t Unk00[12];
    uint8_t Unk010;
    uint8_t Unk011;
    uint8_t Unk012;
    uint8_t Unk013;
    uint32_t BufferSize;
    uint32_t Unk02;
    uint16_t Width;
    uint16_t Height;
    uint16_t Depth;
    uint16_t Levels;
    uint8_t UnkByte0;
    uint8_t UnkByte1;
    uint8_t ImageFormat;
    uint8_t UnkByte3;
    uint8_t UnkByte4;
    uint8_t UnkByte5;
    uint8_t UnkByte6;
    uint8_t UnkByte7;
    uint64_t Unk04;
    uint64_t MipMapsPtr;
    uint64_t PrimedMipPtr;
    uint64_t LoadedImagePtr;
};

#pragma pack(push, 1)
struct VGGfxMip
{
    uint64_t HashID;
    uint8_t Padding[8];
    uint32_t Size;
    uint16_t Width;
    uint16_t Height;
};
#pragma pack(pop)

template <size_t Size>
struct VGGfxMipArray
{
    VGGfxMip MipMaps[Size];

    size_t GetImageSize(size_t i)
    {
        if (i == 0)
            return (size_t)(MipMaps[i].Size >> 4);
        else
            return (size_t)((MipMaps[i].Size >> 4) - (MipMaps[i - 1].Size >> 4));
    }

    size_t GetMipCount()
    {
        return Size;
    }
};

bool GameVanguard::LoadOffsets()
{
    return false;
}

bool GameVanguard::LoadAssets()
{
    // Prepare to load game assets, into the AssetPool
    bool NeedsAnims = (SettingsManager::GetSetting("showxanim", "true") == "true");
    bool NeedsModels = (SettingsManager::GetSetting("showxmodel", "true") == "true");
    bool NeedsImages = (SettingsManager::GetSetting("showximage", "false") == "true");
    bool NeedsSounds = (SettingsManager::GetSetting("showxsounds", "false") == "true");
    bool NeedsRawFiles = (SettingsManager::GetSetting("showxrawfiles", "false") == "true");
    bool NeedsMaterials = (SettingsManager::GetSetting("showxmtl", "false") == "true");
    bool NeedsExtInfo = (SettingsManager::GetSetting("needsextinfo", "true") == "true");

    if (NeedsModels)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 9 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [&NeedsExtInfo](ps::XAsset64& Asset)
        {
            // Read
            auto ModelResult = CoDAssets::GameInstance->Read<VGXModel>(Asset.Header);
            // Validate and load if need be
            auto ModelName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(ModelResult.NamePtr));
            // Make and add
            auto LoadedModel = new CoDModel_t();
            // Set
            LoadedModel->AssetName = ModelName;
            LoadedModel->AssetPointer = Asset.Header;
            // Bone counts (check counts, since there's some weird models that we don't want, they have thousands of bones with no info)
            if ((ModelResult.NumBones + ModelResult.UnkBoneCount) > 1 && ModelResult.ParentListPtr == 0)
                LoadedModel->BoneCount = 0;
            else
                LoadedModel->BoneCount = ModelResult.NumBones + ModelResult.UnkBoneCount;
            LoadedModel->LodCount = ModelResult.NumLods;
            LoadedModel->AssetStatus = Asset.Temp == 1 ? WraithAssetStatus::Placeholder : WraithAssetStatus::Loaded;
            // Parse bone names if requested
            if (NeedsExtInfo)
            {
                for (size_t i = 0; i < LoadedModel->BoneCount; i++)
                {
                    auto BoneIndex = CoDAssets::GameInstance->Read<uint32_t>(ModelResult.BoneIDsPtr + i * 4);

                    LoadedModel->BoneNames.emplace_back(CoDAssets::GameStringHandler(BoneIndex));
                }
            }
            // Log it
            CoDAssets::LogXAsset("Model", ModelName);
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedModel);
        });
    }

    if (NeedsAnims)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 7 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto AnimResult = CoDAssets::GameInstance->Read<MW4XAnim>(Asset.Header);
            // Validate and load if need be
            auto AnimName = CoDAssets::GameInstance->ReadNullTerminatedString(AnimResult.NamePtr);

            // Log it
            CoDAssets::LogXAsset("Anim", AnimName);

            // Make and add
            auto LoadedAnim = new CoDAnim_t();
            // Set
            LoadedAnim->AssetName    = AnimName;
            LoadedAnim->AssetPointer = Asset.Header;
            LoadedAnim->Framerate    = AnimResult.Framerate;
            LoadedAnim->FrameCount   = AnimResult.NumFrames;
            LoadedAnim->AssetStatus  = Asset.Temp == 1 ? WraithAssetStatus::Placeholder : WraithAssetStatus::Loaded;
            LoadedAnim->BoneCount    = AnimResult.TotalBoneCount;
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedAnim);
        });
    }

    if (NeedsImages)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 19 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto ImageResult = CoDAssets::GameInstance->Read<VGGfxImage>(Asset.Header);
            // Validate and load if need be
            auto ImageName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(ImageResult.NamePtr));

            // Log it
            CoDAssets::LogXAsset("Image", ImageName);

            // Check if it's streamed
            if(ImageResult.MipMapsPtr > 0)
            {

                // Make and add
                auto LoadedImage = new CoDImage_t();
                // Set
                LoadedImage->AssetName = ImageName;
                LoadedImage->AssetPointer = Asset.Header;
                LoadedImage->Width = (uint16_t)ImageResult.Width;
                LoadedImage->Height = (uint16_t)ImageResult.Height;
                LoadedImage->Format = ImageResult.ImageFormat;
                LoadedImage->AssetStatus = WraithAssetStatus::Loaded;

                // Add
                CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
            }
        });
    }

    if (NeedsSounds)
    {
#if _DEBUG
        TextWriter Writer;
        Writer.Open("vg_sound_names.csv");
#endif

        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 22 * sizeof(ps::XAssetPool64));
#if _DEBUG
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [&Writer](ps::XAsset64& Asset)
#else
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
#endif
        {
            // Read
            auto SoundResult = CoDAssets::GameInstance->Read<VGSoundBank>(Asset.Header);
            auto SoundBankInfo = CoDAssets::GameInstance->Read<VGSoundBankInfo>(SoundResult.SoundBankPtr);

            // Get Settings
            auto SkipBlankAudio = SettingsManager::GetSetting("skipblankaudio", "false") == "true";

            if (SoundBankInfo.BankFilePointer > 0)
            {
                // Parse the loaded header, and offset from the pointer to it
                auto Header = CoDAssets::GameInstance->Read<SABFileHeader>(SoundBankInfo.BankFilePointer);

                // Verify magic first, same for all SAB files
                // The magic is ('2UX#')
                if (Header.Magic != 0x23585532)
                {
                    return;
                }

                // Name offset
                auto NamesOffset = CoDAssets::GameInstance->Read<uint64_t>(SoundBankInfo.BankFilePointer + 0x250);

                // Prepare to loop and read entries
                for (size_t i = 0; i < Header.EntriesCount; i++)
                {
                    auto Name = CoDAssets::GameInstance->ReadNullTerminatedString(SoundBankInfo.BankFilePointer + NamesOffset + i * 128);
                    AssetNameCache.NameDatabase[VGHashSoundString(Name)] = Name;
                }

                // Prepare to loop and read entries
                for (uint32_t i = 0; i < Header.EntriesCount; i++)
                {
                    // Read each entry
                    auto Entry = CoDAssets::GameInstance->Read<SABv17Entry>(SoundBankInfo.BankFilePointer + Header.EntryTableOffset + i * sizeof(SABv17Entry));

                    // Check do we want to skip this
                    if (SkipBlankAudio && Entry.Size <= 0)
                    {
                        continue;
                    }

                    // Setup a new entry
                    auto LoadedSound = new CoDSound_t();

                    LoadedSound->FrameRate = Entry.FrameRate;
                    LoadedSound->FrameCount = Entry.FrameCount;
                    LoadedSound->ChannelsCount = Entry.ChannelCount;
                    LoadedSound->AssetPointer = SoundBankInfo.BankFilePointer + (Entry.Offset + Entry.SeekTableLength);
                    LoadedSound->AssetSize = Entry.Size;
                    LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
                    LoadedSound->IsFileEntry = false;
                    LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));
                    LoadedSound->DataType = SoundDataTypes::Opus_Interleaved;

                    auto NameResult = AssetNameCache.NameDatabase.find(Entry.Key);

                    if (NameResult != AssetNameCache.NameDatabase.end())
                    {
                        LoadedSound->AssetName = FileSystems::GetFileNamePurgeExtensions(NameResult->second);
                        LoadedSound->FullPath = FileSystems::GetDirectoryName(NameResult->second);
                    }
                    else
                    {
                        LoadedSound->AssetName = FileSystems::GetFileNamePurgeExtensions(Strings::Format("xsound_%llx", Entry.Key));
                        LoadedSound->FullPath = FileSystems::GetDirectoryName(LoadedSound->AssetName);
                    }


                    // Log it
                    CoDAssets::LogXAsset("Sound", FileSystems::CombinePath(LoadedSound->FullPath, LoadedSound->AssetName));

#if _DEBUG
                    Writer.WriteLineFmt("%llx,%s", Entry.Key, FileSystems::CombinePath(LoadedSound->FullPath, LoadedSound->AssetName));
#endif 

                    // Add
                    CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
                }
            }

            if (SoundResult.StreamedSoundsPtr > 0 && SoundResult.StreamedSoundCount > 0)
            {
                for (size_t i = 0; i < SoundResult.StreamedSoundCount; i++)
                {
                    // Read each entry
                    auto Entry = CoDAssets::GameInstance->Read<SABv17Entry>(SoundResult.StreamedSoundsPtr + i * sizeof(SABv17Entry));

                    // Check do we want to skip this
                    if (SkipBlankAudio && Entry.Size <= 0)
                    {
                        continue;
                    }

                    // Setup a new entry
                    auto LoadedSound = new CoDSound_t();

                    LoadedSound->FrameRate = Entry.FrameRate;
                    LoadedSound->FrameCount = Entry.FrameCount;
                    LoadedSound->ChannelsCount = Entry.ChannelCount;
                    LoadedSound->AssetPointer = SoundResult.StreamedSoundsPtr + i * sizeof(SABv17Entry);
                    LoadedSound->AssetSize = Entry.Size;
                    LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
                    LoadedSound->IsFileEntry = false;
                    LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));
                    LoadedSound->DataType = SoundDataTypes::Opus_Interleaved_Streamed;

                    auto NameResult = AssetNameCache.NameDatabase.find(Entry.Key);

                    if (NameResult != AssetNameCache.NameDatabase.end())
                    {
                        LoadedSound->AssetName = FileSystems::GetFileNamePurgeExtensions(NameResult->second);
                        LoadedSound->FullPath = FileSystems::GetDirectoryName(NameResult->second);
                    }
                    else
                    {
                        LoadedSound->AssetName = FileSystems::GetFileNamePurgeExtensions(Strings::Format("xsound_%llx", Entry.Key));
                        LoadedSound->FullPath = FileSystems::GetDirectoryName(LoadedSound->AssetName);
                    }


                    // Log it
                    CoDAssets::LogXAsset("Sound", NameResult->second);

#if _DEBUG
                    Writer.WriteLineFmt("%llx,%s", Entry.Key, FileSystems::CombinePath(LoadedSound->FullPath, LoadedSound->AssetName));
#endif 

                    // Add
                    CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
                }
            }
        });
    }

    if (NeedsRawFiles)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 23 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto SoundResult = CoDAssets::GameInstance->Read<VGSoundBank>(Asset.Header);
            // Validate and load if need be
            auto RawfileName = CoDAssets::GameInstance->ReadNullTerminatedString(SoundResult.NamePtr) + ".sabs";

            // Note actually streamer info pool
            auto Info = CoDAssets::GameInstance->Read<VGSoundBankInfo>(SoundResult.SoundBankPtr);

            // Log it
            CoDAssets::LogXAsset("RawFile", RawfileName);

            // Make and add
            auto LoadedRawfile = new CoDRawFile_t();
            // Set
            LoadedRawfile->AssetName = FileSystems::GetFileName(RawfileName);
            LoadedRawfile->RawFilePath = FileSystems::GetDirectoryName(RawfileName);
            LoadedRawfile->AssetPointer = Asset.Header;
            LoadedRawfile->AssetSize = Info.BankFileSize;
            LoadedRawfile->RawDataPointer = Info.BankFilePointer;
            LoadedRawfile->AssetStatus = WraithAssetStatus::Loaded;

            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedRawfile);
        });
    }

    if (NeedsMaterials)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 11 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto MaterialResult = CoDAssets::GameInstance->Read<VGXMaterial>(Asset.Header);
            // Validate and load if need be
            auto MaterialName = CoDAssets::GameInstance->ReadNullTerminatedString(MaterialResult.NamePtr);

            // Log it
            CoDAssets::LogXAsset("Material", MaterialName);

            // Make and add
            auto LoadedMaterial = new CoDMaterial_t();
            // Set
            LoadedMaterial->AssetName = FileSystems::GetFileName(Strings::Replace(MaterialName, "*", ""));
            LoadedMaterial->AssetPointer = Asset.Header;
            LoadedMaterial->ImageCount = MaterialResult.ImageCount;
            LoadedMaterial->AssetStatus = WraithAssetStatus::Loaded;
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedMaterial);
        });
    }

    // Success, error only on specific load
    return true;
}

std::unique_ptr<XAnim_t> GameVanguard::ReadXAnim(const CoDAnim_t* Animation)
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

        // Check for additive animations
        // No point, breaks it in SETools, wait for Cast to implement full Additive support
        if (AnimData.AssetType == 0x6)
        {
            // This is a additive animation
            Anim->AdditiveAnimation = true;
        }
        // Check for looping
        Anim->LoopingAnimation = (AnimData.Flags & 1);

        // Read the delta data
        auto AnimDeltaData = CoDAssets::GameInstance->Read<MW4XAnimDeltaParts>(*(uint64_t*)&AnimData.PaddingNew[0]);

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
        Anim->BlendShapeWeightCount        = *(uint16_t*)&AnimData.Padding3[3];
        Anim->BlendShapeNamesPtr           = *(uint64_t*)&AnimData.Padding3[11];
        Anim->BlendShapeWeightsPtr         = *(uint64_t*)&AnimData.Padding3[19];

        // Copy delta
        Anim->DeltaTranslationPtr = AnimDeltaData.DeltaTranslationsPtr;
        Anim->Delta2DRotationsPtr = AnimDeltaData.Delta2DRotationsPtr;
        Anim->Delta3DRotationsPtr = AnimDeltaData.Delta3DRotationsPtr;

        // Set types, we use dividebysize for VG
        Anim->RotationType = AnimationKeyTypes::DivideBySize;
        Anim->TranslationType = AnimationKeyTypes::MinSizeTable;

        // Vanguard supports inline indicies
        Anim->SupportsInlineIndicies = true;

        // Return it
        return Anim;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XModel_t> GameVanguard::ReadXModel(const CoDModel_t* Model)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Read the XModel structure
        auto ModelData = CoDAssets::GameInstance->Read<VGXModel>(Model->AssetPointer);

        // Prepare to read the xmodel (Reserving space for lods)
        auto ModelAsset = std::make_unique<XModel_t>(ModelData.NumLods);

        // Copy over default properties
        ModelAsset->ModelName = Model->AssetName;

        // Bone counts (check counts, since there's some weird models that we don't want, they have thousands of bones with no info)
        if ((ModelData.NumBones + ModelData.UnkBoneCount) > 1 && ModelData.ParentListPtr == 0)
        {
            ModelAsset->BoneCount = 0;
            ModelAsset->RootBoneCount = 0;
        }
        else
        {
            ModelAsset->BoneCount = ModelData.NumBones + ModelData.UnkBoneCount;
            ModelAsset->RootBoneCount = ModelData.NumRootBones;
        }

        // Bone data type
        ModelAsset->BoneRotationData = BoneDataTypes::DivideBySize;

        // We are streamed
        ModelAsset->IsModelStreamed = true;

        // Bone id info
        ModelAsset->BoneIDsPtr = ModelData.BoneIDsPtr;
        ModelAsset->BoneIndexSize = 4;

        // Bone parent info
        ModelAsset->BoneParentsPtr = ModelData.ParentListPtr;
        ModelAsset->BoneParentSize = 2;

        // Local bone pointers
        ModelAsset->RotationsPtr = ModelData.RotationsPtr;
        ModelAsset->TranslationsPtr = ModelData.TranslationsPtr;

        // Global matricies
        ModelAsset->BaseMatriciesPtr = ModelData.BaseMatriciesPtr;

        // Blend shape info
        ModelAsset->BlendShapeNamesPtr = *(uint64_t*)&ModelData.Padding4[64];

        // Prepare to parse lods
        for (uint32_t i = 0; i < ModelData.NumLods; i++)
        {
            // Read Load
            auto ModelLod = CoDAssets::GameInstance->Read<VGXModelLod>(ModelData.ModelLodsPtr + i * sizeof(VGXModelLod));

            // Create the lod and grab reference
            ModelAsset->ModelLods.emplace_back(ModelLod.NumSurfs);
            // Grab reference
            auto& LodReference = ModelAsset->ModelLods[i];

            // Set distance
            LodReference.LodDistance = ModelLod.LodDistance;

            // Set stream key and info ptr
            LodReference.LODStreamInfoPtr = ModelLod.MeshPtr;

            // Grab pointer from the lod itself
            auto XSurfacePtr = ModelLod.SurfsPtr;

            // Load surfaces
            for (uint32_t s = 0; s < ModelLod.NumSurfs; s++)
            {
                // Create the surface and grab reference
                LodReference.Submeshes.emplace_back();
                // Grab reference
                auto& SubmeshReference = LodReference.Submeshes[s];

                // Read the surface data
                auto SurfaceInfo = CoDAssets::GameInstance->Read<VGXModelSurface>(XSurfacePtr);

                // Apply surface info
                SubmeshReference.RigidWeightsPtr  = SurfaceInfo.RigidWeightsPtr;
                SubmeshReference.VertListcount    = SurfaceInfo.VertListCount;
                SubmeshReference.VertexCount      = SurfaceInfo.VertexCount;
                SubmeshReference.FaceCount        = SurfaceInfo.FacesCount;
                SubmeshReference.VertexPtr        = SurfaceInfo.Offsets[0];
                SubmeshReference.VertexUVsPtr     = SurfaceInfo.Offsets[1];
                SubmeshReference.VertexNormalsPtr = SurfaceInfo.Offsets[2];
                SubmeshReference.FacesPtr         = SurfaceInfo.Offsets[3];
                SubmeshReference.VertexColorPtr   = SurfaceInfo.Offsets[6];

                if (SurfaceInfo.NewScale != -1)
                {
                    SubmeshReference.Scale = SurfaceInfo.NewScale;
                    SubmeshReference.XOffset = 0;
                    SubmeshReference.YOffset = 0;
                    SubmeshReference.ZOffset = 0;
                }
                else
                {
                    SubmeshReference.Scale = fmaxf(fmaxf(SurfaceInfo.Min, SurfaceInfo.Scale), SurfaceInfo.Max);
                    SubmeshReference.XOffset = SurfaceInfo.XOffset;
                    SubmeshReference.YOffset = SurfaceInfo.YOffset;
                    SubmeshReference.ZOffset = SurfaceInfo.ZOffset;
                }

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
                SubmeshReference.BlendShapesPtr = *(uint64_t*)&SurfaceInfo.Padding7[8];

                // Read this submesh's material handle
                auto MaterialHandle = CoDAssets::GameInstance->Read<uint64_t>(ModelData.MaterialHandlesPtr);
                // Create the material and add it
                LodReference.Materials.emplace_back(ReadXMaterial(MaterialHandle));

                // Advance
                XSurfacePtr += sizeof(VGXModelSurface);
                ModelData.MaterialHandlesPtr += sizeof(uint64_t);
            }
        }

        // Return it
        return ModelAsset;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XImageDDS> GameVanguard::ReadXImage(const CoDImage_t* Image)
{
    // Proxy off
    return LoadXImage(XImage_t(ImageUsageType::DiffuseMap, 0, Image->AssetPointer, Image->AssetName));
}

std::unique_ptr<XSound> GameVanguard::ReadXSound(const CoDSound_t* Sound)
{
    // Buffer
    std::unique_ptr<uint8_t[]> SoundBuffer = nullptr;
    size_t OpusOffset = 0;

    if (Sound->DataType == SoundDataTypes::Opus_Interleaved_Streamed)
    {
        // Use entry and get streamed item
        auto Entry = CoDAssets::GameInstance->Read<SABv17Entry>(Sound->AssetPointer);

        // Result size
        uint32_t ResultSize = 0;
        // We must read from the cache
        SoundBuffer = CoDAssets::GamePackageCache->ExtractPackageObject(Entry.Offset, Entry.Size + Entry.SeekTableLength, ResultSize);
        // Set offset, skip seek table
        OpusOffset = Entry.SeekTableLength;
    }
    else
    {
        // Buffer
        SoundBuffer = std::make_unique<uint8_t[]>((size_t)Sound->AssetSize);

        // Validate
        if (CoDAssets::GameInstance->Read(SoundBuffer.get(), Sound->AssetPointer, Sound->AssetSize) != (size_t)Sound->AssetSize)
            return nullptr;
    }

    if (SoundBuffer != nullptr)
    {
        return SABSupport::DecodeOpusInterleaved(SoundBuffer.get(), Sound->AssetSize, OpusOffset, Sound->FrameRate, Sound->ChannelsCount, Sound->FrameCount);
    }
    
    return nullptr;
}

void GameVanguard::TranslateRawfile(const CoDRawFile_t * Rawfile, const std::string & ExportPath)
{
    // Build the export path
    std::string ExportFolder = ExportPath;

    // Check to preserve the paths
    if (SettingsManager::GetSetting("keeprawpath", "true") == "true")
    {
        // Apply the base path
        ExportFolder = FileSystems::CombinePath(ExportFolder, Rawfile->RawFilePath);
    }

    // Make the directory
    FileSystems::CreateDirectory(ExportFolder);

    // Read Bank
    auto Bank = CoDAssets::GameInstance->Read<VGSoundBank>(Rawfile->AssetPointer);

    //// Text writer for aliases
    //TextWriter AliasWriter;

    //// Create the file
    //if (AliasWriter.Create(FileSystems::CombinePath(ExportFolder, Rawfile->AssetName + ".csv")))
    //{
    //    // Write head
    //    AliasWriter.WriteLine("Name,Secondary,File");
    //    AliasWriter.WriteLine("# Note: Secondary is triggered by this alias, so a chain of secondaries are played at the same time");
    //    AliasWriter.WriteLine("# Note: There are other settings that cannot be pulled, so these sounds may not sound the same as in-game without editing");

    //    for (uint64_t j = 0; j < Bank.AliasCount; j++)
    //    {
    //        auto Alias = CoDAssets::GameInstance->Read<VGSoundAlias>(Bank.AliasesPtr + j * sizeof(VGSoundAlias));

    //        for (uint64_t k = 0; k < Alias.EntriesCount; k++)
    //        {
    //            auto Entry = CoDAssets::GameInstance->Read<VGSoundAliasEntry>(Alias.EntriesPtr + k * sizeof(VGSoundAliasEntry));

    //            auto NamingInfo = CoDAssets::GameInstance->Read<VGSoundAliasNamingInfo>(Entry.NamingInfoPtr);
    //            auto Name      = CoDAssets::GameInstance->ReadNullTerminatedString(NamingInfo.NamePtr);
    //            auto Secondary = CoDAssets::GameInstance->ReadNullTerminatedString(NamingInfo.SecondaryPtr);
    //            auto FileName  = CoDAssets::GameInstance->ReadNullTerminatedString(Entry.FilePtr);

    //            AliasWriter.WriteLineFmt("%s,%s,%s", Name.c_str(), Secondary.c_str(), FileName.c_str());
    //        }
    //    }
    //}

    // Note actually streamer info pool
    auto Info = CoDAssets::GameInstance->Read<VGSoundBankInfo>(Bank.SoundBankPtr);

    // Always stream, data in memory gets bamboozled like Thomas Cat's death
    // Size read
    uint32_t ResultSize = 0;
    // Buffer
    std::unique_ptr<uint8_t[]> Data = CoDAssets::GamePackageCache->ExtractPackageObject(Info.StreamKey, ResultSize);

    // Prepare if we have it
    if (Data != nullptr)
    {
        // New writer instance
        auto Writer = BinaryWriter();
        // Create the file
        if (Writer.Create(FileSystems::CombinePath(ExportFolder, Rawfile->AssetName)))
        {
            // Write the raw data
            Writer.Write(Data.get(), (uint32_t)Info.BankFileSize);
        }
    }
}

const XMaterial_t GameVanguard::ReadXMaterial(uint64_t MaterialPointer)
{
    // Prepare to parse the material
    auto MaterialData = CoDAssets::GameInstance->Read<VGXMaterial>(MaterialPointer);

    // Allocate a new material with the given image count
    XMaterial_t Result(MaterialData.ImageCount);
    // Clean the name, then apply it
    Result.MaterialName = FileSystems::GetFileNameWithoutExtension(CoDAssets::GameInstance->ReadNullTerminatedString(MaterialData.NamePtr));
    Result.MaterialName = Strings::Replace(Result.MaterialName, "*", "");

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

void GameVanguard::PrepareVertexWeights(std::vector<WeightsData>& Weights, const XModelSubmesh_t & Submesh)
{
    // The index of read weight data
    uint32_t WeightDataIndex = 0;

    if (Submesh.RigidWeightsPtr > 0)
    {
        // Prepare the simple, rigid weights
        for (uint32_t i = 0; i < Submesh.VertListcount; i++)
        {
            // Simple weights build, rigid, just apply the proper bone id
            auto RigidInfo = CoDAssets::GameInstance->Read<VGGfxRigidVerts>(Submesh.RigidWeightsPtr + (i * sizeof(VGGfxRigidVerts)));
            // Apply bone ids properly
            for (uint32_t w = 0; w < RigidInfo.VertexCount; w++)
            {
                // Apply
                Weights[WeightDataIndex].BoneValues[0] = RigidInfo.BoneIndex;
                // Advance
                WeightDataIndex++;
            }
        }
    }

    if (Submesh.WeightsPtr > 0)
    {
        // Total weight data read
        uintptr_t ReadDataSize = 0;
        // Calculate the size of weights buffer
        auto WeightsDataLength = ((4 * Submesh.WeightCounts[0]) + (8 * Submesh.WeightCounts[1]) + (12 * Submesh.WeightCounts[2]) + (16 * Submesh.WeightCounts[3]) + (20 * Submesh.WeightCounts[4]) + (24 * Submesh.WeightCounts[5]) + (28 * Submesh.WeightCounts[6]) + (32 * Submesh.WeightCounts[7]));
        // Read the weight data
        auto WeightsData = MemoryReader(CoDAssets::GameInstance->Read(Submesh.WeightsPtr, WeightsDataLength, ReadDataSize), WeightsDataLength);

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
}

std::unique_ptr<XImageDDS> GameVanguard::LoadXImage(const XImage_t& Image)
{
    // We must read the image data
    auto ImageInfo = CoDAssets::GameInstance->Read<VGGfxImage>(Image.ImagePtr);

    if (ImageInfo.MipMapsPtr == 0)
        return nullptr;

    // Read Array of Mip Maps
    VGGfxMipArray<32> Mips{};
    size_t MipCount = std::min((size_t)ImageInfo.UnkByte6, (size_t)32);

    if (CoDAssets::GameInstance->Read((uint8_t*)Mips.MipMaps, ImageInfo.MipMapsPtr, MipCount * sizeof(VGGfxMip)) != (MipCount * sizeof(VGGfxMip)))
        return nullptr;

    // An initial loop to find the fallback to use in case of CDN not being
    // a viable option.
    size_t Fallback = 0;
    size_t HighestIndex = (size_t)ImageInfo.UnkByte6 - 1;

    for (size_t i = 0; i < MipCount; i++)
    {
        if (CoDAssets::GamePackageCache->Exists(Mips.MipMaps[i].HashID))
        {
            Fallback = i;
        }
    }

    // Calculate proper image format
    switch (ImageInfo.ImageFormat)
    {
    case 0:  ImageInfo.ImageFormat = 71; break;
    case 1:  ImageInfo.ImageFormat = 61; break;
    case 2:  ImageInfo.ImageFormat = 65; break;
    case 3:  ImageInfo.ImageFormat = 65; break;
    case 4:  ImageInfo.ImageFormat = 49; break;
    case 5:  ImageInfo.ImageFormat = 49; break;
    case 6:  ImageInfo.ImageFormat = 28; break;
    case 7:  ImageInfo.ImageFormat = 28; break;
    case 8:  ImageInfo.ImageFormat = 71; break;
    case 9:  ImageInfo.ImageFormat = 71; break;
    case 10: ImageInfo.ImageFormat = 63; break;
    case 11: ImageInfo.ImageFormat = 51; break;
    case 12: ImageInfo.ImageFormat = 56; break;
    case 13: ImageInfo.ImageFormat = 35; break;
    case 14: ImageInfo.ImageFormat = 11; break;
    case 15: ImageInfo.ImageFormat = 58; break;
    case 16: ImageInfo.ImageFormat = 54; break;
    case 17: ImageInfo.ImageFormat = 34; break;
    case 18: ImageInfo.ImageFormat = 10; break;
    case 19: ImageInfo.ImageFormat = 41; break;
    case 20: ImageInfo.ImageFormat = 16; break;
    case 21: ImageInfo.ImageFormat = 2;  break;
    case 22: ImageInfo.ImageFormat = 40; break;
    case 23: ImageInfo.ImageFormat = 20; break;
    case 24: ImageInfo.ImageFormat = 62; break;
    case 25: ImageInfo.ImageFormat = 57; break;
    case 26: ImageInfo.ImageFormat = 42; break;
    case 27: ImageInfo.ImageFormat = 17; break;
    case 28: ImageInfo.ImageFormat = 3;  break;
    case 29: ImageInfo.ImageFormat = 25; break;
    case 30: ImageInfo.ImageFormat = 85; break;
    case 31: ImageInfo.ImageFormat = 85; break;
    case 32: ImageInfo.ImageFormat = 24; break;
    case 33: ImageInfo.ImageFormat = 67; break;
    case 34: ImageInfo.ImageFormat = 67; break;
    case 35: ImageInfo.ImageFormat = 71; break;
    case 36: ImageInfo.ImageFormat = 71; break;
    case 37: ImageInfo.ImageFormat = 71; break;
    case 38: ImageInfo.ImageFormat = 74; break;
    case 39: ImageInfo.ImageFormat = 77; break;
    case 40: ImageInfo.ImageFormat = 77; break;
    case 41: ImageInfo.ImageFormat = 80; break;
    case 42: ImageInfo.ImageFormat = 84; break;
    case 43: ImageInfo.ImageFormat = 84; break;
    case 44: ImageInfo.ImageFormat = 95; break;
    case 45: ImageInfo.ImageFormat = 96; break;
    case 46: ImageInfo.ImageFormat = 98; break;
    case 47: ImageInfo.ImageFormat = 98; break;
    // Fall back to BC1
    default: ImageInfo.ImageFormat = 71; break;
    }

    // Buffer
    std::unique_ptr<uint8_t[]> ImageData = nullptr;
    size_t ImageSize = 0;

    // First check the local game CDN cache.
    if (Fallback != HighestIndex && CoDAssets::OnDemandCache != nullptr)
    {
        uint32_t PackageSize = 0;
        ImageData = CoDAssets::OnDemandCache->ExtractPackageObject(Mips.MipMaps[HighestIndex].HashID, Mips.GetImageSize(HighestIndex), PackageSize);
        ImageSize = PackageSize;
    }

    // If we still have mips above our fallback, then we have to attempt
    // to load the on-demand version from the CDN.
    if (Fallback != HighestIndex && CoDAssets::CDNDownloader != nullptr)
    {
        ImageData = CoDAssets::CDNDownloader->ExtractCDNObject(Mips.MipMaps[HighestIndex].HashID, Mips.GetImageSize(HighestIndex), ImageSize);
    }

    // If that failed, fallback to the normal image cache.
    if (ImageData == nullptr)
    {
        uint32_t PackageSize = 0;
        ImageData = CoDAssets::GamePackageCache->ExtractPackageObject(Mips.MipMaps[Fallback].HashID, Mips.GetImageSize(Fallback), PackageSize);
        ImageSize = PackageSize;
        HighestIndex = Fallback;
    }

    // Prepare if we have it
    if (ImageData != nullptr)
    {
        // Prepare to create a MemoryDDS file
        auto Result = CoDRawImageTranslator::TranslateBC(
            ImageData,
            ImageSize,
            Mips.MipMaps[HighestIndex].Width,
            Mips.MipMaps[HighestIndex].Height,
            ImageInfo.ImageFormat);

        // Return it
        return Result;
    }

    // failed to load the image
    return nullptr;
}

// Transforms a Normal by the Rotation
Vector3 VanguardTransformNormal(Quaternion quat, Vector3 up)
{
    // Generate the normal by rotating up around the normal value
    const Vector3 a(
        quat.Y * up.Z - quat.Z * up.Y + up.X * quat.W,
        quat.Z * up.X - quat.X * up.Z + up.Y * quat.W,
        quat.X * up.Y - quat.Y * up.X + up.Z * quat.W);
    const Vector3 b(
        quat.Y * a.Z - quat.Z * a.Y,
        quat.Z * a.X - quat.X * a.Z,
        quat.X * a.Y - quat.Y * a.X);
    return Vector3(
        up.X + b.X + b.X,
        up.Y + b.Y + b.Y,
        up.Z + b.Z + b.Z);
}

// Unpacks a 4-D Quaternion Normal
Vector3 VanguardUnpackNormalQuat(uint32_t packedQuat)
{
    auto Up = Vector3(0, 0, 1.0f);
    auto LargestComponent = packedQuat >> 30;

    // Unpack the values and compute the largest component
    auto x = ((((packedQuat >> 00) & 0x3FF) / 511.5f) - 1.0f) / 1.4142135f;
    auto y = ((((packedQuat >> 10) & 0x3FF) / 511.5f) - 1.0f) / 1.4142135f;
    auto z = ((((packedQuat >> 20) & 0x1FF) / 255.5f) - 1.0f) / 1.4142135f;
    auto w = sqrtf(1 - x * x - y * y - z * z);

    // Determine largest
    switch (LargestComponent)
    {
    case 0: return VanguardTransformNormal(Quaternion(w, x, y, z), Up);
    case 1: return VanguardTransformNormal(Quaternion(x, w, y, z), Up);
    case 2: return VanguardTransformNormal(Quaternion(x, y, w, z), Up);
    case 3: return VanguardTransformNormal(Quaternion(x, y, z, w), Up);
    default: return Vector3(1.0f, 0.0f, 0.0f);
    }
}

struct VGBlendShapeInfo
{
    uint16_t ShapeCount;
    uint16_t NameCount;
    uint16_t Unk1;
    uint64_t WeightNames;
    uint64_t UnkPtr;
};

struct VGBlendShapeMap
{
    uint16_t NameIndex;
    uint16_t ShapeIndex;
    float UnknownOne;
};

struct MeshBlendShapesData
{
    uint64_t VertexShapes;
    uint64_t UnkDataPtr;
    uint32_t VertexShapesDataSize;
    uint16_t ShapeCount;
    uint16_t UnkCount;
    uint16_t NumVertexExtended;
    uint16_t NumVerts;
    uint16_t UnkValue0;
    uint16_t UnkValue;
};

void GameVanguard::LoadXModel(const std::unique_ptr<XModel_t>& Model, const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel)
{
    // Scale to use
    auto ScaleConstant = (1.0f / 0x1FFFFF) * 2.0f;
    // Check if we want Vertex Colors
    bool ExportColors = (SettingsManager::GetSetting("exportvtxcolor", "true") == "true");
    // Read the mesh information
    auto MeshInfo = CoDAssets::GameInstance->Read<VGXModelMesh>(ModelLOD.LODStreamInfoPtr);
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
        // Allocate safe
        MeshDataBuffer = std::make_unique<uint8_t[]>(BufferInfo.BufferSize);
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
        MeshDataBuffer = CoDAssets::GamePackageCache->ExtractPackageObject(MeshInfo.LODStreamKey, BufferInfo.BufferSize, ResultSize);
        // Set size
        MeshDataBufferSize = ResultSize;
    }

    // Continue on success
    if (MeshDataBuffer != nullptr)
    {
        // The total weighted verticies
        uint32_t TotalReadWeights = 0;
        // The maximum weight index
        uint32_t MaximumWeightIndex = ResultModel->BoneCount() - 1;

        // Prepare it for submeshes
        ResultModel->PrepareSubmeshes((uint32_t)ModelLOD.Submeshes.size());

        // Readers for all data types, global so that we can use them when streaming...
        MemoryReader VertexPosReader;
        MemoryReader VertexNormReader;
        MemoryReader VertexUVReader;
        MemoryReader VertexColorReader;
        MemoryReader VertexWeightReader;
        MemoryReader FaceIndiciesReader;

        // Start with blendshapes
        std::unique_ptr<uint32_t[]> ShapeIndices = nullptr;

        if (Model->BlendShapeNamesPtr != 0)
        {
            auto BlendShapeInfo = CoDAssets::GameInstance->Read<VGBlendShapeInfo>(Model->BlendShapeNamesPtr);

            ShapeIndices = std::make_unique<uint32_t[]>(BlendShapeInfo.ShapeCount);

            for (size_t i = 0; i < BlendShapeInfo.NameCount; i++)
            {
                ResultModel->BlendShapes.push_back(CoDAssets::GameStringHandler(CoDAssets::GameInstance->Read<uint32_t>(BlendShapeInfo.WeightNames + i * sizeof(uint32_t))));
            }

            for (size_t i = 0; i < BlendShapeInfo.ShapeCount; i++)
            {
                auto ShapeMap = CoDAssets::GameInstance->Read<VGBlendShapeMap>(BlendShapeInfo.UnkPtr + i * sizeof(VGBlendShapeMap));
                ShapeIndices[ShapeMap.ShapeIndex] = ShapeMap.NameIndex;
            }
        }

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

            // Setup weights if we have any bones
            if (ResultModel->BoneCount() > 1) PrepareVertexWeights(VertexWeights, Submesh);

            // Vertex Readers
            VertexPosReader.Setup((int8_t*)MeshDataBuffer.get() + Submesh.VertexPtr, Submesh.VertexCount * sizeof(uint64_t), true);
            VertexNormReader.Setup((int8_t*)MeshDataBuffer.get() + Submesh.VertexNormalsPtr, Submesh.VertexCount * sizeof(uint32_t), true);
            VertexUVReader.Setup((int8_t*)MeshDataBuffer.get() + Submesh.VertexUVsPtr, Submesh.VertexCount * sizeof(uint32_t), true);
            FaceIndiciesReader.Setup((int8_t*)MeshDataBuffer.get() + Submesh.FacesPtr, Submesh.FaceCount * sizeof(uint16_t) * 3, true);

            // Iterate over verticies
            for (uint32_t i = 0; i < Submesh.VertexCount; i++)
            {
                // Make a new vertex
                auto& Vertex = Mesh.AddVertex();

                // Read Vertex
                auto VertexPosition = VertexPosReader.Read<uint64_t>();

                // Read and assign position
                Vertex.Position = Vector3(
                    (((((VertexPosition >> 00) & 0x1FFFFF) * ScaleConstant) - 1.0f) * Submesh.Scale) + Submesh.XOffset,
                    (((((VertexPosition >> 21) & 0x1FFFFF) * ScaleConstant) - 1.0f) * Submesh.Scale) + Submesh.YOffset,
                    (((((VertexPosition >> 42) & 0x1FFFFF) * ScaleConstant) - 1.0f) * Submesh.Scale) + Submesh.ZOffset);

                // Read Tangent/Normal
                auto QTangent = VertexNormReader.Read<uint32_t>();

                // Add normal
                Vertex.Normal = VanguardUnpackNormalQuat(QTangent);

                // Apply Color (some models don't store colors, so we need to check ptr below)
                Vertex.Color[0] = 255;
                Vertex.Color[1] = 255;
                Vertex.Color[2] = 255;
                Vertex.Color[3] = 255;

                // Read and set UVs
                auto UVU = HalfFloats::ToFloat(VertexUVReader.Read<uint16_t>());
                auto UVV = HalfFloats::ToFloat(VertexUVReader.Read<uint16_t>());

                // Set it
                Vertex.AddUVLayer(UVU, UVV);

                // Assign weights
                auto& WeightValue = VertexWeights[i];

                //// Iterate
                for (uint32_t w = 0; w < WeightValue.WeightCount; w++)
                {
                    // Add new weight
                    Vertex.AddVertexWeight(WeightValue.BoneValues[w], WeightValue.WeightValues[w]);
                }
            }

            // Jump to color data, if it's stored and we want it
            if (ExportColors && Submesh.VertexColorPtr != 0xFFFFFFFF)
            {
                VertexColorReader.Setup((int8_t*)MeshDataBuffer.get() + Submesh.VertexColorPtr, Submesh.VertexCount * sizeof(uint32_t), true);

                // Iterate over verticies
                for (uint32_t i = 0; i < Submesh.VertexCount; i++)
                {
                    // Apply Color (some models don't store colors, so we need to check ptr below)
                    Mesh.Verticies[i].Color[0] = VertexColorReader.Read<uint8_t>();
                    Mesh.Verticies[i].Color[1] = VertexColorReader.Read<uint8_t>();
                    Mesh.Verticies[i].Color[2] = VertexColorReader.Read<uint8_t>();
                    Mesh.Verticies[i].Color[3] = VertexColorReader.Read<uint8_t>();
                }
            }

            // Iterate over faces
            for (uint32_t i = 0; i < Submesh.FaceCount; i++)
            {
                // Read data
                auto Face = FaceIndiciesReader.Read<VGGfxStreamFace>();

                // Add the face
                Mesh.AddFace(Face.Index1, Face.Index2, Face.Index3);
            }

            // Check for blendshapes
            // cs_blendshapes_no_tangent_frame.hlsl/cs_blendshapes_unified_main
            if (Submesh.BlendShapesPtr != 0)
            {
                uintptr_t ResultSize = 0;
                auto MeshBlendShapeInfo = CoDAssets::GameInstance->Read<MeshBlendShapesData>(Submesh.BlendShapesPtr);
                auto BlendShapeReader   = MemoryReader(CoDAssets::GameInstance->Read(MeshBlendShapeInfo.VertexShapes, MeshBlendShapeInfo.VertexShapesDataSize, ResultSize), MeshBlendShapeInfo.VertexShapesDataSize);
                auto NumVerts           = (size_t)MeshBlendShapeInfo.NumVerts;
                auto NumVertsExtended   = (size_t)MeshBlendShapeInfo.NumVertexExtended;
                auto NumVertsTotal      = NumVerts + NumVertsExtended;

                // Not sure if correct, I've never seen a model contain shape verts with extended data AND just positions
                for (size_t i = 0; i < NumVertsTotal; i++)
                {
                    BlendShapeReader.SetPosition(i * 8);

                    auto VertexIndex = BlendShapeReader.Read<uint16_t>();
                    auto ShapeCount  = BlendShapeReader.Read<uint16_t>();
                    auto DataOffset  = BlendShapeReader.Read<uint32_t>();

                    BlendShapeReader.SetPosition((uint64_t)DataOffset * (i < NumVertsExtended ? 16 : 8));

                    for (size_t j = 0; j < ShapeCount; j++)
                    {
                        auto VertexShapeIndex = BlendShapeReader.Read<uint16_t>();
                        auto X = HalfFloats::ToFloat(BlendShapeReader.Read<uint16_t>());
                        auto Y = HalfFloats::ToFloat(BlendShapeReader.Read<uint16_t>());
                        auto Z = HalfFloats::ToFloat(BlendShapeReader.Read<uint16_t>());

                        // TODO
                        if (i < NumVertsExtended)
                            BlendShapeReader.Advance(8);

                        Mesh.Verticies[VertexIndex].BlendShapeDeltas.push_back(std::make_pair(
                            (uint32_t)ShapeIndices[VertexShapeIndex],
                            Vector3(
                                Z,
                                Y,
                                X
                            )));
                    }
                }
            }
        }
    }
}

std::string GameVanguard::LoadStringEntry(uint64_t Index)
{
    // Read Info
    auto StringHash = CoDAssets::GameInstance->Read<uint64_t>(ps::state->StringsAddress + Index) & 0xFFFFFFFFFFFFFFF;
    // Attempt to locate string
    auto StringEntry = StringCache.NameDatabase.find(StringHash);
    // Not Encrypted
    if (StringEntry != StringCache.NameDatabase.end())
        return StringEntry->second;
    else
        return Strings::Format("xstring_%llx", StringHash);
}
void GameVanguard::PerformInitialSetup()
{
    // Load Caches
    StringCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\fnv1a_string.wni"));
    AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\fnv1a_xsounds_unverified.wni"));
    // Prepare to copy the oodle dll
    auto OurPath = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "oo2core_8_win64.dll");
    // Copy if not exists
    if (!FileSystems::FileExists(OurPath))
        FileSystems::CopyFile(FileSystems::CombinePath(ps::state->GameDirectory, "oo2core_8_win64.dll"), OurPath);
}
