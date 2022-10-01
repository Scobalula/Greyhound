#include "stdafx.h"

// The class we are implementing
#include "GameModernWarfare5.h"
#include "GameModernWarfare5Structures.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDRawImageTranslator.h"
#include "CoDXPoolParser.h"
#include "DBGameFiles.h"
// We need the following WraithX classes
#include "Strings.h"
#include "FileSystems.h"
#include "MemoryReader.h"
#include "TextWriter.h"
#include "SettingsManager.h"
#include "HalfFloats.h"
#include "BinaryReader.h"

// -- Begin XModelStream structures

struct MW5GfxRigidVerts
{
    uint16_t BoneIndex;
    uint16_t VertexCount;
    uint16_t FacesCount;
    uint16_t FacesIndex;
    uint32_t Marv;
};

#pragma pack(push, 1)
struct MW5GfxStreamVertex
{
    uint64_t PackedPosition; // Packed 21bits, scale + offset in Mesh Info
    uint32_t BiNormal;
    uint16_t UVUPosition;
    uint16_t UVVPosition;
    uint32_t NormalQuaternion;
};
#pragma pack(pop)

struct MW5GfxStreamFace
{
    uint16_t Index1;
    uint16_t Index2;
    uint16_t Index3;
};

// -- End XModelStream structures

// -- Modern Warfare 5 Pool Data Structure

struct MW5XAssetPoolData
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

struct MW5SoundBankInfo
{
    uint64_t BankNamePointer;
    uint64_t StreamKey;
    uint8_t Padding[0x14];
    uint64_t BankFilePointer;
    uint64_t UnkPointer;
    uint32_t BankFileSize;
    uint32_t Unk;
};

struct MW5SoundAlias
{
    uint64_t NamePtr;
    uint32_t NameHash;
    uint32_t Padding;
    uint64_t EntriesPtr;
    uint64_t EntriesCount;
};

struct MW5SoundAliasEntry
{
    uint64_t FilePtr;
    uint32_t FileHash;
    uint32_t Unk;
    uint64_t NamingInfoPtr;
    uint64_t UnkPtr2;
};

// In the Season 5 patch the Name/Secondary pointers are no longer stored directly in the sound alias entry and are instead their own structure pointed to from the entry.
struct MW5SoundAliasNamingInfo
{
    uint64_t NamePtr;
    uint64_t SecondaryPtr;
};

std::map<uint32_t, std::string> Temp;


// Calculates the hash of a sound string
uint32_t MW5HashSoundString(const std::string& Value)
{
    uint32_t Result = 5381;

    for (auto& Character : Value)
        Result = (uint32_t)(tolower(Character) + (Result << 6) + (Result << 16)) - Result;

    return Result;
}

// Verify that our pool data is exactly 0x20
static_assert(sizeof(MW5XAssetPoolData) == 0x18, "Invalid Pool Data Size (Expected 0x18)");

bool GameModernWarfare5::LoadOffsets()
{
    // Failed
    return false;
}

bool GameModernWarfare5::LoadAssets()
{
    // Prepare to load game assets, into the AssetPool
    bool NeedsAnims     = (SettingsManager::GetSetting("showxanim", "true") == "true");
    bool NeedsModels    = (SettingsManager::GetSetting("showxmodel", "true") == "true");
    bool NeedsImages    = (SettingsManager::GetSetting("showximage", "false") == "true");
    bool NeedsSounds    = (SettingsManager::GetSetting("showxsounds", "false") == "true");
    bool NeedsRawFiles  = (SettingsManager::GetSetting("showxrawfiles", "false") == "true");
    bool NeedsMaterials = (SettingsManager::GetSetting("showxmtl", "false") == "true");

    if (NeedsModels)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 9 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto ModelResult = CoDAssets::GameInstance->Read<MW5XModel>(Asset.Header);

            std::string ModelName;

            // Check if the asset actually has a name.
            if (ModelResult.NamePtr > 0)
                ModelName = FileSystems::GetFileName(Strings::SplitString(CoDAssets::GameInstance->ReadNullTerminatedString(ModelResult.NamePtr), ':', true)[0]);
            else
                ModelName = CoDAssets::GetHashedName("xmodel", ModelResult.Hash);

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
            // Log it
            CoDAssets::LogXAsset("Model", ModelName);
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedModel);

            for (size_t b = 0; b < LoadedModel->BoneCount; b++)
            {
                auto x = CoDAssets::GameStringHandler(CoDAssets::GameInstance->Read<uint32_t>(ModelResult.BoneIDsPtr + b * 8));
                auto y = CoDAssets::GameInstance->Read<uint32_t>(ModelResult.BoneIDsPtr + b * 8 + 4);
                Temp[CoDAssets::GameInstance->Read<uint32_t>(ModelResult.BoneIDsPtr + b * 8 + 4)] = CoDAssets::GameStringHandler(CoDAssets::GameInstance->Read<uint32_t>(ModelResult.BoneIDsPtr + b * 8));
            }
        });
    }

    if (NeedsImages)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 19 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto ImageResult = CoDAssets::GameInstance->Read<MW5GfxImage>(Asset.Header);
            // Get Loaded Image
            //uint64_t ImagePointer = *(uint64_t*)&ImageResult.Padding5[8];
            // Validate and load if need be
            auto ImageName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(ImageResult.NamePtr));
            // Log it
            CoDAssets::LogXAsset("Image", ImageName);
            // Check for loaded images
            //if (ImagePointer == (uint64_t)-1)
            //    return;
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
        });
    }

    if (NeedsAnims)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 7 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto AnimResult = CoDAssets::GameInstance->Read<MW5XAnim>(Asset.Header);
            // Validate and load if need be
            auto AnimName = CoDAssets::GameInstance->ReadNullTerminatedString(AnimResult.NamePtr);

            // Log it
            CoDAssets::LogXAsset("Anim", AnimName);

            // Make and add
            auto LoadedAnim = new CoDAnim_t();
            // Set
            LoadedAnim->AssetName = AnimName;
            LoadedAnim->AssetPointer = Asset.Header;
            LoadedAnim->Framerate = AnimResult.Framerate;
            LoadedAnim->FrameCount = AnimResult.FrameCount;
            LoadedAnim->AssetStatus = Asset.Temp == 1 ? WraithAssetStatus::Placeholder : WraithAssetStatus::Loaded;
            LoadedAnim->BoneCount = AnimResult.TotalBoneCount;
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedAnim);
        });
    }

    if (NeedsMaterials)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 11 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto MatResult = CoDAssets::GameInstance->Read<MW5XMaterial>(Asset.Header);
            // Validate and load if need be
            auto MaterialName = CoDAssets::GameInstance->ReadNullTerminatedString(MatResult.NamePtr);

            // Log it
            CoDAssets::LogXAsset("Material", MaterialName);

            // Make and add
            auto LoadedMaterial = new CoDMaterial_t();
            // Set
            LoadedMaterial->AssetName = FileSystems::GetFileName(MaterialName);
            LoadedMaterial->AssetPointer = Asset.Header;
            LoadedMaterial->ImageCount = MatResult.ImageCount;
            LoadedMaterial->AssetStatus = WraithAssetStatus::Loaded;

            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedMaterial);
        });
    }

    ////// Since MW now stores the entire SABL in Fast Files, we must essentially parse it in memory, SABS files can be loaded as usual.
    ////if (NeedsSounds)
    ////{
    ////    auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 21 * sizeof(ps::XAssetPool64));
    ////    ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
    ////    {
    ////        // Read
    ////        auto SoundResult = CoDAssets::GameInstance->Read<MW5SoundBank>(Asset.Header);
    ////        // Read Info
    ////        auto SoundBankInfo = CoDAssets::GameInstance->Read<MW5SoundBankInfo>(SoundResult.SoundBankPtr);

    ////        if (SoundBankInfo.BankFilePointer > 0)
    ////        {
    ////            // Names by Hash
    ////            std::map<uint32_t, std::string> SABFileNames;

    ////            // Parse the loaded header, and offset from the pointer to it
    ////            auto Header = CoDAssets::GameInstance->Read<SABFileHeader>(SoundBankInfo.BankFilePointer);

    ////            // Verify magic first, same for all SAB files
    ////            // The magic is ('2UX#')
    ////            if (Header.Magic != 0x23585532)
    ////            {
    ////                return;
    ////            }

    ////            // Get Settings
    ////            auto SkipBlankAudio = SettingsManager::GetSetting("skipblankaudio", "false") == "true";

    ////            // Name offset
    ////            auto NamesOffset = CoDAssets::GameInstance->Read<uint64_t>(SoundBankInfo.BankFilePointer + 0x250);

    ////            // Prepare to loop and read entries
    ////            for (size_t i = 0; i < Header.EntriesCount; i++)
    ////            {
    ////                auto Name = CoDAssets::GameInstance->ReadNullTerminatedString(SoundBankInfo.BankFilePointer + NamesOffset + i * 128);
    ////                SABFileNames[MW5HashSoundString(Name)] = Name;
    ////            }

    ////            // Prepare to loop and read entries
    ////            for (uint32_t i = 0; i < Header.EntriesCount; i++)
    ////            {
    ////                // Read each entry
    ////                auto Entry = CoDAssets::GameInstance->Read<SABv4Entry>(SoundBankInfo.BankFilePointer + Header.EntryTableOffset + i * sizeof(SABv4Entry));

    ////                // Prepare to parse the information to our generic structure
    ////                std::string EntryName = "";
    ////                // Check our options
    ////                if (SABFileNames.size() > 0)
    ////                {
    ////                    // We have it in file
    ////                    EntryName = SABFileNames[Entry.Key];
    ////                }
    ////                else
    ////                {
    ////                    // We don't have one
    ////                    EntryName = Strings::Format("_%llx", Entry.Key);
    ////                }

    ////                // Log it
    ////                CoDAssets::LogXAsset("Sound", EntryName);

    ////                // Setup a new entry
    ////                auto LoadedSound = new CoDSound_t();
    ////                // Set the name, but remove all extensions first
    ////                LoadedSound->AssetName = FileSystems::GetFileNamePurgeExtensions(EntryName);
    ////                LoadedSound->FullPath = FileSystems::GetDirectoryName(EntryName);

    ////                // Set various properties
    ////                LoadedSound->FrameRate = Entry.FrameRate;
    ////                LoadedSound->FrameCount = Entry.FrameCount;
    ////                LoadedSound->ChannelsCount = Entry.ChannelCount;
    ////                // The offset should be after the seek table, since it is not required
    ////                LoadedSound->AssetPointer = SoundBankInfo.BankFilePointer + (Entry.Offset + Entry.SeekTableLength);
    ////                LoadedSound->AssetSize = Entry.Size;
    ////                LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
    ////                LoadedSound->IsFileEntry = false;
    ////                LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));
    ////                // All Modern Warfare (v10) entries are FLAC's with no header
    ////                LoadedSound->DataType = SoundDataTypes::Opus_Interleaved;

    ////                // Check do we want to skip this
    ////                if (SkipBlankAudio && LoadedSound->AssetSize <= 0)
    ////                {
    ////                    delete LoadedSound;
    ////                    continue;
    ////                }

    ////                // Add
    ////                CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
    ////            }
    ////        }
    ////    });
    ////}

    ////if (NeedsRawFiles)
    ////{
    ////    auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 22 * sizeof(ps::XAssetPool64));
    ////    ps::PoolParser64(Pool.FirstXAsset, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
    ////    {
    ////        // Read
    ////        auto SoundResult = CoDAssets::GameInstance->Read<MW5SoundBank>(Asset.Header);
    ////        // Validate and load if need be
    ////        auto RawfileName = CoDAssets::GameInstance->ReadNullTerminatedString(SoundResult.NamePtr) + ".sabs";

    ////        // Note actually streamer info pool
    ////        auto Info = CoDAssets::GameInstance->Read<MW5SoundBankInfo>(SoundResult.SoundBankPtr);

    ////        // Make and add
    ////        auto LoadedRawfile = new CoDRawFile_t();
    ////        // Set
    ////        LoadedRawfile->AssetName = FileSystems::GetFileName(RawfileName);
    ////        LoadedRawfile->RawFilePath = FileSystems::GetDirectoryName(RawfileName);
    ////        LoadedRawfile->AssetPointer = Asset.Header;
    ////        LoadedRawfile->AssetSize = Info.BankFileSize;
    ////        LoadedRawfile->RawDataPointer = Info.BankFilePointer;
    ////        LoadedRawfile->AssetStatus = WraithAssetStatus::Loaded;

    ////        // Add
    ////        CoDAssets::GameAssets->LoadedAssets.push_back(LoadedRawfile);
    ////    });
    ////}

    // Success, error only on specific load
    return true;
}

std::unique_ptr<XAnim_t> GameModernWarfare5::ReadXAnim(const CoDAnim_t* Animation)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Read the XAnim structure
        auto AnimData = CoDAssets::GameInstance->Read<MW5XAnim>(Animation->AssetPointer);

        // No stream info, not supported atm.
        if (AnimData.StreamInfoPtr == 0)
        {
            return nullptr;
        }

        // Prepare to read the xanim
        auto Anim = std::make_unique<XAnim_t>();

        // Copy over default properties
        Anim->AnimationName = Animation->AssetName;
        // Frames and Rate
        Anim->FrameCount = AnimData.FrameCount;
        Anim->FrameRate = AnimData.Framerate;

        // Check for viewmodel animations
        //if ((_strnicmp(Animation->AssetName.c_str(), "viewmodel_", 10) == 0) || (_strnicmp(Animation->AssetName.c_str(), "vm_", 3) == 0))
        //{
        //    // This is a viewmodel animation
        //    Anim->ViewModelAnimation = true;
        //}

        // Check for additive animations
        // No point, breaks it in SETools, wait for Cast to implement full Additive support
        //if (AnimData.AssetType == 0x6)
        //{
        //    // This is a additive animation
        //    Anim->AdditiveAnimation = true;
        //}
        // Check for looping
        Anim->LoopingAnimation = false /*(AnimData.Flags & 1)*/;

        // Read the delta data
        // auto AnimDeltaData = CoDAssets::GameInstance->Read<MW4XAnimDeltaParts>(AnimData.DeltaPartsPtr);

        std::unique_ptr<uint8_t[]> AnimBuffer = nullptr;
        uint32_t AnimBufferSize = 0;
        uint64_t AnimBufferOffset = 0;

        // Check for stream info, should be a flag but this will do
        if (AnimData.StreamInfoPtr != 0)
        {
            // We'll need to extract it from the streamed packages
            // What we'll get back is a complete buffer we can work with.
            // TODO: Support big anims (multiple buffers)
            auto StreamInfo = CoDAssets::GameInstance->Read<MW5XAnimStreamInfo>(AnimData.StreamInfoPtr);

            AnimBuffer = CoDAssets::GamePackageCache->ExtractPackageObject(StreamInfo.StreamKey, StreamInfo.Size, AnimBufferSize);

            if (AnimBufferSize != StreamInfo.Size)
            {
                return nullptr;
            }

            // Debug, TODO: Multiple xanim buffers
            //for (size_t i = 0; i < AnimData.OffsetCount; i++)
            //{
            //    auto StreamInfo2 = CoDAssets::GameInstance->Read<MW5XAnimStreamInfo>(AnimData.StreamInfoPtr + i * sizeof(MW5XAnimStreamInfo));
            //    uint32_t AnimBufferSize2 = 0;
            //    auto AnimBuffer2 = CoDAssets::GamePackageCache->ExtractPackageObject(StreamInfo2.StreamKey, StreamInfo2.Size, AnimBufferSize2);

            //    BinaryWriter Writer;

            //    Writer.Create(Strings::Format("anim_%llu.dat", i));

            //    Writer.Write(AnimBuffer2.get(), StreamInfo.Size);
            //}
        }

        // Create a generic xanim reader and assign it to the new xanim translator
        Anim->Reader = std::make_unique<CoDXAnimReader>(AnimBuffer.release(), (size_t)AnimBufferSize, true);

        // Now consume the chicken dinner.
        Anim->Reader->DataBytes = Anim->Reader->GetBuffer();
        Anim->Reader->DataShorts = Anim->Reader->GetBuffer() + (size_t)AnimData.DataShortOffset;
        Anim->Reader->DataInts = Anim->Reader->GetBuffer() + (size_t)AnimData.DataIntOffset;
        Anim->Reader->RandomDataShorts = Anim->Reader->GetBuffer() + CoDAssets::GameInstance->Read<uint32_t>(AnimData.OffsetPtr);
        Anim->Reader->RandomDataBytes = Anim->Reader->GetBuffer() + CoDAssets::GameInstance->Read<uint32_t>(AnimData.OffsetPtr2);
        Anim->Reader->RandomDataInts = Anim->Reader->GetBuffer();

        // Consume Bones (TODO: Hashes are now stored, need to build a table and to figure out algorithm)
        // Building from XModels on fly atm.
        for (size_t b = 0; b < AnimData.TotalBoneCount; b++)
        {
            uint32_t boneid = CoDAssets::GameInstance->Read<uint32_t>(AnimData.BoneIDsPtr + b * 4);
            if (Temp.find(boneid) != Temp.end())
            {
                Anim->Reader->BoneNames.push_back(Temp[boneid]);
            }
            else
            {
                Anim->Reader->BoneNames.push_back(Strings::Format("bone_%llx", boneid));
            }
        }

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
        //Anim->NotificationCount            = AnimData.NotificationCount;

        // Copy delta
        //Anim->DeltaTranslationPtr = AnimDeltaData.DeltaTranslationsPtr;
        //Anim->Delta2DRotationsPtr = AnimDeltaData.Delta2DRotationsPtr;
        //Anim->Delta3DRotationsPtr = AnimDeltaData.Delta3DRotationsPtr;

        // Set types, we use dividebysize for MW5
        Anim->RotationType = AnimationKeyTypes::DivideBySize;
        Anim->TranslationType = AnimationKeyTypes::MinSizeTable;

        // Modern Warfare 5 supports inline indicies
        Anim->SupportsInlineIndicies = true;

        // Return it
        return Anim;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XModel_t> GameModernWarfare5::ReadXModel(const CoDModel_t* Model)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Read the XModel structure
        auto ModelData = CoDAssets::GameInstance->Read<MW5XModel>(Model->AssetPointer);

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
        ModelAsset->BoneIndexSize = 8;

        // Bone parent info
        ModelAsset->BoneParentsPtr = ModelData.ParentListPtr;
        ModelAsset->BoneParentSize = 2;

        // Local bone pointers
        ModelAsset->RotationsPtr = ModelData.RotationsPtr;
        ModelAsset->TranslationsPtr = ModelData.TranslationsPtr;

        // Global matricies
        ModelAsset->BaseMatriciesPtr = ModelData.BaseMatriciesPtr;

        // Prepare to parse lods
        for (uint32_t i = 0; i < ModelData.NumLods; i++)
        {
            // Read the XModel Lod
            auto ModelLod = CoDAssets::GameInstance->Read<MW5XModelLod>(ModelData.ModelLods + i * sizeof(MW5XModelLod));
            // Create the lod and grab reference
            ModelAsset->ModelLods.emplace_back(ModelLod.NumSurfs);
            // Grab reference
            auto& LodReference = ModelAsset->ModelLods[i];

            // Set distance
            LodReference.LodDistance = ModelLod.LodDistance[0];

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
                auto SurfaceInfo = CoDAssets::GameInstance->Read<MW5XModelSurface>(XSurfacePtr);

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
                SubmeshReference.WeightsPtr       = SurfaceInfo.Offsets[10];

                // Check for new single scale value.
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

                // Read this submesh's material handle
                auto MaterialHandle = CoDAssets::GameInstance->Read<uint64_t>(ModelData.MaterialHandlesPtr);
                // Create the material and add it
                LodReference.Materials.emplace_back(ReadXMaterial(MaterialHandle));

                // Advance
                XSurfacePtr += sizeof(MW5XModelSurface);
                ModelData.MaterialHandlesPtr += sizeof(uint64_t);
            }
        }

        // Return it
        return ModelAsset;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XImageDDS> GameModernWarfare5::ReadXImage(const CoDImage_t* Image)
{
    // Proxy off
    return LoadXImage(XImage_t(ImageUsageType::DiffuseMap, 0, Image->AssetPointer, Image->AssetName));
}

void GameModernWarfare5::TranslateRawfile(const CoDRawFile_t * Rawfile, const std::string & ExportPath)
{
    //// Build the export path
    //std::string ExportFolder = ExportPath;

    //// Check to preserve the paths
    //if (SettingsManager::GetSetting("keeprawpath", "true") == "true")
    //{
    //    // Apply the base path
    //    ExportFolder = FileSystems::CombinePath(ExportFolder, Rawfile->RawFilePath);
    //}

    //// Make the directory
    //FileSystems::CreateDirectory(ExportFolder);

    //// Read Bank
    //auto Bank = CoDAssets::GameInstance->Read<MW5SoundBank>(Rawfile->AssetPointer);

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
    //        auto Alias = CoDAssets::GameInstance->Read<MW5SoundAlias>(Bank.AliasesPtr + j * sizeof(MW5SoundAlias));

    //        for (uint64_t k = 0; k < Alias.EntriesCount; k++)
    //        {
    //            auto Entry = CoDAssets::GameInstance->Read<MW5SoundAliasEntry>(Alias.EntriesPtr + k * sizeof(MW5SoundAliasEntry));

    //            auto NamingInfo = CoDAssets::GameInstance->Read<MW5SoundAliasNamingInfo>(Entry.NamingInfoPtr);
    //            auto Name      = CoDAssets::GameInstance->ReadNullTerminatedString(NamingInfo.NamePtr);
    //            auto Secondary = CoDAssets::GameInstance->ReadNullTerminatedString(NamingInfo.SecondaryPtr);
    //            auto FileName  = CoDAssets::GameInstance->ReadNullTerminatedString(Entry.FilePtr);

    //            AliasWriter.WriteLineFmt("%s,%s,%s", Name.c_str(), Secondary.c_str(), FileName.c_str());
    //        }
    //    }
    //}

    //// Note actually streamer info pool
    //auto Info = CoDAssets::GameInstance->Read<MW5SoundBankInfo>(Bank.SoundBankPtr);

    //// Always stream, data in memory gets bamboozled like Thomas Cat's death
    //// Size read
    //uint32_t ResultSize = 0;
    //// Buffer
    //std::unique_ptr<uint8_t[]> Data = CoDAssets::GamePackageCache->ExtractPackageObject(Info.StreamKey, ResultSize);

    //// Prepare if we have it
    //if (Data != nullptr)
    //{
    //    // New writer instance
    //    auto Writer = BinaryWriter();
    //    // Create the file
    //    if (Writer.Create(FileSystems::CombinePath(ExportFolder, Rawfile->AssetName)))
    //    {
    //        // Write the raw data
    //        Writer.Write(Data.get(), (uint32_t)Info.BankFileSize);
    //    }
    //}
}

const XMaterial_t GameModernWarfare5::ReadXMaterial(uint64_t MaterialPointer)
{
    // Prepare to parse the material
    auto MaterialData = CoDAssets::GameInstance->Read<MW5XMaterial>(MaterialPointer);

    // Allocate a new material with the given image count
    XMaterial_t Result(MaterialData.ImageCount);
    // Clean the name, then apply it
    Result.MaterialName = FileSystems::GetFileNameWithoutExtension(CoDAssets::GameInstance->ReadNullTerminatedString(MaterialData.NamePtr));

    // Iterate over material images, assign proper references if available
    for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
    {
        // Read the image info
        auto ImageInfo = CoDAssets::GameInstance->Read<MW5XMaterialImage>(MaterialData.ImageTablePtr);
        // Read the image name (End of image - 8)
        auto ImageName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(ImageInfo.ImagePtr + 8));

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

void GameModernWarfare5::PrepareVertexWeights(MemoryReader& ComplexReader, std::vector<WeightsData>& Weights, const XModelSubmesh_t & Submesh)
{
    // The index of read weight data
    uint32_t WeightDataIndex = 0;

    // Prepare the simple, rigid weights
    for (uint32_t i = 0; i < Submesh.VertListcount; i++)
    {
        // Simple weights build, rigid, just apply the proper bone id
        auto RigidInfo = CoDAssets::GameInstance->Read<MW5GfxRigidVerts>(Submesh.RigidWeightsPtr + (i * sizeof(MW5GfxRigidVerts)));
        // Apply bone ids properly
        for (uint32_t w = 0; w < RigidInfo.VertexCount; w++)
        {
            // Apply
            Weights[WeightDataIndex].BoneValues[0] = RigidInfo.BoneIndex;
            // Advance
            WeightDataIndex++;
        }
    }

    // Check for complex weights
    if (ComplexReader.GetLength() > 0)
    {
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
                    Weights[LocalWeightDataIndex].BoneValues[k] = ComplexReader.Read<uint16_t>();

                    // Process weight if applicable
                    if (k > 0)
                    {
                        // Read value and subtract from first bone
                        Weights[LocalWeightDataIndex].WeightValues[k] = ((float)ComplexReader.Read<uint16_t>() / 65536.0f);
                        Weights[LocalWeightDataIndex].WeightValues[0] -= Weights[LocalWeightDataIndex].WeightValues[k];
                    }
                    else
                    {
                        // Skip 2 padding
                        ComplexReader.Advance(2);
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

std::unique_ptr<XImageDDS> GameModernWarfare5::LoadXImage(const XImage_t& Image)
{
    // We must read the image data
    auto ImageInfo = CoDAssets::GameInstance->Read<MW5GfxImage>(Image.ImagePtr);

    // Read Array of Mip Maps
    MW5GfxMipArray<32> Mips{};
    size_t MipCount = (size_t)std::min((size_t)ImageInfo.MipCount, Mips.GetMipCount());

    if (CoDAssets::GameInstance->Read((uint8_t*)&Mips.MipMaps, ImageInfo.MipMaps, MipCount * sizeof(MW5GfxMip)) != (MipCount * sizeof(MW5GfxMip)))
        return nullptr;

    // An initial loop to find the fallback to use in case of CDN not being
    // a viable option.
    size_t Fallback = 0;
    size_t HighestIndex = MipCount - 1;

    for (size_t i = 0; i < MipCount; i++)
    {
        if (CoDAssets::GamePackageCache->Exists(Mips.MipMaps[i].HashID))
        {
            Fallback = i;
        }
    }

    // Calculate game specific format to DXGI
    ImageInfo.ImageFormat = MW5DXGIFormats[ImageInfo.ImageFormat];

    // Buffer
    std::unique_ptr<uint8_t[]> ImageData = nullptr;
    size_t ImageSize = 0;

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
        ImageData = CoDAssets::GamePackageCache->ExtractPackageObject(Mips.MipMaps[Fallback].HashID, Mips.GetImageSize(HighestIndex), PackageSize);
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
            ImageInfo.Width >> (MipCount - HighestIndex - 1),
            ImageInfo.Height >> (MipCount - HighestIndex - 1),
            ImageInfo.ImageFormat);

        // Return it
        return Result;
    }

    // failed to load the image
    return nullptr;
}

// Transforms a Normal by the Rotation
Vector3 MW5TransformNormal(Quaternion quat, Vector3 up)
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
Vector3 MW5UnpackNormalQuat(uint32_t packedQuat)
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
    case 0: return MW5TransformNormal(Quaternion(w, x, y, z), Up);
    case 1: return MW5TransformNormal(Quaternion(x, w, y, z), Up);
    case 2: return MW5TransformNormal(Quaternion(x, y, w, z), Up);
    case 3: return MW5TransformNormal(Quaternion(x, y, z, w), Up);
    default: return Vector3(1.0f, 0.0f, 0.0f);
    }
}

void GameModernWarfare5::LoadXModel(const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel)
{
    // Scale to use
    auto ScaleConstant = (1.0f / 0x1FFFFF) * 2.0f;
    // Check if we want Vertex Colors
    bool ExportColors = (SettingsManager::GetSetting("exportvtxcolor", "true") == "true");
    // Read the mesh information
    auto MeshInfo = CoDAssets::GameInstance->Read<MW5XModelMesh>(ModelLOD.LODStreamInfoPtr);

    // Read Buffer Info
    auto BufferInfo = CoDAssets::GameInstance->Read<MW5XModelMeshBufferInfo>(MeshInfo.MeshBufferPointer);

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
        MeshDataBuffer = CoDAssets::GamePackageCache->ExtractPackageObject(MeshInfo.LODStreamKey, BufferInfo.BufferSize, ResultSize);
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

        // Readers for all data types, global so that we can use them when streaming...
        MemoryReader VertexPosReader;
        MemoryReader VertexNormReader;
        MemoryReader VertexUVReader;
        MemoryReader VertexColorReader;
        MemoryReader VertexWeightReader;
        MemoryReader SimpleWeightReader;
        MemoryReader FaceIndiciesReader;

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

            // Vertex Readers
            VertexPosReader.Setup((int8_t*)MeshDataBuffer.get() + Submesh.VertexPtr, Submesh.VertexCount * sizeof(uint64_t), true);
            VertexNormReader.Setup((int8_t*)MeshDataBuffer.get() + Submesh.VertexNormalsPtr, Submesh.VertexCount * sizeof(uint32_t), true);
            VertexUVReader.Setup((int8_t*)MeshDataBuffer.get() + Submesh.VertexUVsPtr, Submesh.VertexCount * sizeof(uint32_t), true);
            FaceIndiciesReader.Setup((int8_t*)MeshDataBuffer.get() + Submesh.FacesPtr, Submesh.FaceCount * sizeof(uint16_t) * 3, true);

            // Calculate Weights Size
            auto WeightsDataLength = (
                (4 * Submesh.WeightCounts[0]) +
                (8 * Submesh.WeightCounts[1]) +
                (12 * Submesh.WeightCounts[2]) +
                (16 * Submesh.WeightCounts[3]) +
                (20 * Submesh.WeightCounts[4]) +
                (24 * Submesh.WeightCounts[5]) +
                (28 * Submesh.WeightCounts[6]) +
                (32 * Submesh.WeightCounts[7]));


            // Setup weights if we have any bones
            if (ResultModel->BoneCount() > 1)
            {
                VertexWeightReader.Setup((int8_t*)MeshDataBuffer.get() + Submesh.WeightsPtr, WeightsDataLength, true);
                PrepareVertexWeights(VertexWeightReader, VertexWeights, Submesh);
            }

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
                Vertex.Normal = MW5UnpackNormalQuat(QTangent);

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
                auto Face = FaceIndiciesReader.Read<MW5GfxStreamFace>();

                // Add the face
                Mesh.AddFace(Face.Index1, Face.Index2, Face.Index3);
            }
        }
    }
}

std::string GameModernWarfare5::LoadStringEntry(uint64_t Index)
{
    // Read Info
    auto StringHash = CoDAssets::GameInstance->Read<uint64_t>(ps::state->StringsAddress + Index) & 0xFFFFFFFFFFFFFFF;
    // Attempt to locate string
    auto StringEntry = CoDAssets::StringCache.NameDatabase.find(StringHash);
    // Not Encrypted
    if (StringEntry != CoDAssets::StringCache.NameDatabase.end())
        return StringEntry->second;
    else
        return Strings::Format("xstring_%llx", StringHash);
}
void GameModernWarfare5::PerformInitialSetup()
{
    // Prepare to copy the oodle dll
    auto OurPath = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "oo2core_6_win64.dll");

    // Load Caches
    CoDAssets::StringCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\fnv1a_string.wni"));
    CoDAssets::AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\fnv1a_xsounds_unverified.wni"));

    // Copy if not exists
    if (!FileSystems::FileExists(OurPath))
        FileSystems::CopyFile(FileSystems::CombinePath(ps::state->GameDirectory, "oo2core_7_win64.dll"), OurPath);
}
