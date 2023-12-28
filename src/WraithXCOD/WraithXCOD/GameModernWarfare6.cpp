#include "stdafx.h"

// The class we are implementing
#include "GameModernWarfare6.h"
#include "GameModernWarfare6Structures.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDRawImageTranslator.h"
#include "CoDXPoolParser.h"
#include "DBGameFiles.h"
#include "CoDXModelMeshHelper.h"
#include "CoDXModelBonesHelper.h"
// We need the following WraithX classes
#include "Strings.h"
#include "FileSystems.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"
#include "TextWriter.h"
#include "SettingsManager.h"
#include "HalfFloats.h"
#include "BinaryReader.h"
#include "Sound.h"

// We need Opus
#include "..\..\External\Opus\include\opus.h"

#include "SABSupport.h"

// We need the QTangent
#include "CoDQTangent.h"

// -- Begin XModelStream structures

struct MW6GfxRigidVerts
{
    uint16_t BoneIndex;
    uint16_t VertexCount;
    uint16_t FacesCount;
    uint16_t FacesIndex;
    uint32_t Marv;
};

#pragma pack(push, 1)
struct MW6GfxStreamVertex
{
    uint64_t PackedPosition; // Packed 21bits, scale + offset in Mesh Info
    uint32_t BiNormal;
    uint16_t UVUPosition;
    uint16_t UVVPosition;
    uint32_t NormalQuaternion;
};
#pragma pack(pop)

struct MW6GfxStreamFace
{
    uint16_t Index1;
    uint16_t Index2;
    uint16_t Index3;
};

// -- End XModelStream structures

// -- Modern Warfare 5 Pool Data Structure

struct MW6XAssetPoolData
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

// Calculates the hash of a sound string
uint32_t MW6HashSoundString(const std::string& Value)
{
    uint32_t Result = 5381;

    for (auto& Character : Value)
        Result = (uint32_t)(tolower(Character) + (Result << 6) + (Result << 16)) - Result;

    return Result;
}

// Verify that our pool data is exactly 0x20
static_assert(sizeof(MW6XAssetPoolData) == 0x18, "Invalid Pool Data Size (Expected 0x18)");

bool GameModernWarfare6::LoadOffsets()
{
    // Failed
    return false;
}

bool GameModernWarfare6::LoadAssets()
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
        ps::PoolParser64(Pool.Root, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto ModelResult = CoDAssets::GameInstance->Read<MW6XModel>(Asset.Header);

            std::string ModelName;

            // Check if the asset actually has a name.
            if (ModelResult.NamePtr > 0)
                ModelName = FileSystems::GetFileName(Strings::Replace(CoDAssets::GameInstance->ReadNullTerminatedString(ModelResult.NamePtr), "::", "_"));
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
        });
    }

    if (NeedsImages)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 21 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.Root, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto ImageResult = CoDAssets::GameInstance->Read<MW6GfxImage>(Asset.Header);
            // Mask the name as hashes are 60Bit (Actually 63Bit but maintain with our existing tables)
            ImageResult.Hash &= 0xFFFFFFFFFFFFFFF;
            // Validate and load if need be
            auto ImageName = CoDAssets::GetHashedName("ximage", ImageResult.Hash);
            // Log it
            CoDAssets::LogXAsset("Image", ImageName);
            // Make and add
            auto LoadedImage = new CoDImage_t();
            // Set
            LoadedImage->AssetName = ImageName;
            LoadedImage->AssetPointer = Asset.Header;
            LoadedImage->Width = (uint16_t)ImageResult.Width;
            LoadedImage->Height = (uint16_t)ImageResult.Height;
            LoadedImage->Format = ImageResult.ImageFormat;
            LoadedImage->AssetStatus = WraithAssetStatus::Loaded;
            LoadedImage->Streamed = ImageResult.LoadedImagePtr == 0;
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
        });
    }

    if (NeedsAnims)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 7 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.Root, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto AnimResult = CoDAssets::GameInstance->Read<MW6XAnim>(Asset.Header);
            // Validate and load if need be
            auto AnimName = CoDAssets::GetHashedName("xanim", AnimResult.Hash);

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
        ps::PoolParser64(Pool.Root, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto MatResult = CoDAssets::GameInstance->Read<MW6XMaterial>(Asset.Header);
            // Validate and load if need be
            auto MaterialName = CoDAssets::GetHashedName("xmaterial", MatResult.Hash);

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

    if (NeedsSounds)
    {
        auto Pool = CoDAssets::GameInstance->Read<ps::XAssetPool64>(ps::state->PoolsAddress + 0xc1 * sizeof(ps::XAssetPool64));
        ps::PoolParser64(Pool.Root, CoDAssets::ParasyteRequest, [](ps::XAsset64& Asset)
        {
            // Read
            auto SoundResult = CoDAssets::GameInstance->Read<MW6SndAsset>(Asset.Header);
            // Mask the name as hashes are 60Bit (Actually 63Bit but maintain with our existing tables)
            SoundResult.Name &= 0xFFFFFFFFFFFFFFF;
            // Validate and load if need be
            auto SoundName = CoDAssets::GetHashedName("xsound", SoundResult.Name);

            // Log it
            CoDAssets::LogXAsset("Sound", SoundName);

            // Make and add
            auto LoadedSound = new CoDSound_t();
            // Set the name, but remove all extensions first
            LoadedSound->AssetName = FileSystems::GetFileNamePurgeExtensions(SoundName);
            LoadedSound->FullPath = FileSystems::GetDirectoryName(SoundName);
            LoadedSound->AssetPointer = Asset.Header;
            LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
            // Set various properties
            LoadedSound->FrameRate = SoundResult.FrameRate;
            LoadedSound->FrameCount = SoundResult.FrameCount;
            LoadedSound->ChannelsCount = SoundResult.ChannelCount;
            LoadedSound->AssetSize = -1;
            LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
            LoadedSound->IsFileEntry = false;
            LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));
            // Add
            CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
        });
    }

    // Success, error only on specific load
    return true;
}

// A structure to hold an xanim buffer state.
struct MW6XAnimBufferState
{
    // The packed per frame information.
    uint32_t* PackedPerFrameInfo;
    // The current buffer index requested from the packed buffer.
    size_t BufferIndex;
    // The offset within the buffer from the packed buffer.
    size_t BufferOffset;
    // The number of buffers.
    size_t OffsetCount;
    // The current offset within the packed buffer.
    size_t PackedPerFrameOffset;
};

// Handles calculating the offset within the buffer for the given key index.
size_t MW6XAnimCalculateBufferOffset(const MW6XAnimBufferState* animState, const size_t index, const size_t count)
{
    if (count == 0)
        return 0;

    size_t mask = index + count - 1;
    size_t start = index >> 5;
    size_t end = mask >> 5;
    size_t result = 0;

    for (size_t i = start; i <= end; i++)
    {
        if (animState->PackedPerFrameInfo[i] == 0)
            continue;

        uint32_t maskA = 0xFFFFFFFF;
        uint32_t maskB = 0xFFFFFFFF;

        // If we're at the start or end, we need to calculate trailing bit masks.
        if (i == start)
            maskA = 0xFFFFFFFF >> (index & 0x1F);
        if (i == end)
            maskB = 0xFFFFFFFF << (31 - (mask & 0x1F));

        result += __popcnt(animState->PackedPerFrameInfo[i] & maskA & maskB);
    }

    return result;
}

// Increments the buffers for the given bone.
template <typename T>
void MW6XAnimIncrementBuffers(MW6XAnimBufferState* animState, int tableSize, const size_t elemCount, std::vector<T*>& buffers)
{
    // Check if we are small enough to take it from the initial buffer.
    if (tableSize < 4 || animState->OffsetCount == 1)
    {
        buffers[0] += elemCount * tableSize;
    }
    else
    {
        for (size_t i = 0; i < animState->OffsetCount; i++)
        {
            buffers[i] += elemCount * MW6XAnimCalculateBufferOffset(animState, animState->PackedPerFrameOffset, tableSize);
            animState->PackedPerFrameOffset += tableSize;
        }
    }
}

// Calculates the buffer and offsets from the packed buffer.
void MW6XAnimCalculateBufferIndex(MW6XAnimBufferState* animState, const size_t tableSize, const size_t keyFrameIndex)
{
    // Check if we are small enough to take it from the initial buffer.
    if (tableSize < 4 || animState->OffsetCount == 1)
    {
        animState->BufferIndex = 0;
        animState->BufferOffset = keyFrameIndex;
    }
    else
    {
        // If we're at 0, we're definitely within the initial buffer, otherwise, we need to search
        // for our buffer.
        if (keyFrameIndex >= 0)
        {
            for (size_t i = 0; i < animState->OffsetCount; i++)
            {
                if (((0x80000000 >> ((keyFrameIndex + (i * tableSize) + animState->PackedPerFrameOffset) & 0x1F)) & animState->PackedPerFrameInfo[(keyFrameIndex + (i * tableSize) + animState->PackedPerFrameOffset) >> 5]) != 0)
                {
                    animState->BufferIndex = i;
                    animState->BufferOffset = MW6XAnimCalculateBufferOffset(animState, animState->PackedPerFrameOffset + tableSize * animState->BufferIndex, keyFrameIndex);
                    return;
                }
            }

            // Shouldn't happen on a valid xanim.
            throw std::exception("");
        }
    }
}

std::unique_ptr<XAnim_t> GameModernWarfare6::ReadXAnim(const CoDAnim_t* Animation)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Read the XAnim structure
        auto AnimData = CoDAssets::GameInstance->Read<MW6XAnim>(Animation->AssetPointer);

        // No stream info, haven't encountered any yet.
        if (AnimData.DataInfo.StreamInfoPtr == 0)
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

        // Check for looping
        Anim->LoopingAnimation = (AnimData.Padding2[4] & 1) != 0;
        Anim->AdditiveAnimation = AnimData.AssetType == 0x6;

        // Read the delta data
        auto AnimDeltaData = CoDAssets::GameInstance->Read<MW4XAnimDeltaParts>(AnimData.DeltaPartsPtr);

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
        Anim->NotificationCount            = AnimData.NotetrackCount;

        // We use a custom reader function, MW2 is not standard (streamed/custom packed info).
        Anim->ReaderFunction = LoadXAnim;
        Anim->ReaderInformationPointer = Animation->AssetPointer;
        Anim->Reader = std::make_unique<CoDXAnimReader>();

        // Consume bones
        for (size_t b = 0; b < AnimData.TotalBoneCount; b++)
        {
            Anim->Reader->BoneNames.push_back(CoDAssets::GetHashedString("bone", (uint64_t)CoDAssets::GameInstance->Read<uint32_t>(AnimData.BoneIDsPtr + b * 4)));
            uint32_t boneid = CoDAssets::GameInstance->Read<uint32_t>(AnimData.BoneIDsPtr + b * 4);
        }

        // Consume notetracks
        for (size_t n = 0; n < AnimData.NotetrackCount; n++)
        {
            auto noteTrack = CoDAssets::GameInstance->Read<MW6XAnimNotetrack>(AnimData.NotificationsPtr + n * sizeof(MW6XAnimNotetrack));
            Anim->Reader->Notetracks.push_back({ CoDAssets::GameStringHandler(noteTrack.Name), (size_t)(noteTrack.Time * (float)Anim->FrameCount) });
        }

        // Copy delta
        Anim->DeltaTranslationPtr = AnimDeltaData.DeltaTranslationsPtr;
        Anim->Delta2DRotationsPtr = AnimDeltaData.Delta2DRotationsPtr;
        Anim->Delta3DRotationsPtr = AnimDeltaData.Delta3DRotationsPtr;

        // Set types, we use dividebysize for MW6
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

std::unique_ptr<XModel_t> GameModernWarfare6::ReadXModel(const CoDModel_t* Model)
{
    // Verify that the program is running
    if (CoDAssets::GameInstance->IsRunning())
    {
        // Read the XModel structure
        auto ModelData = CoDAssets::GameInstance->Read<MW6XModel>(Model->AssetPointer);

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

        // Prepare to parse lods
        for (uint32_t i = 0; i < ModelData.NumLods; i++)
        {
            // Read the XModel Lod
            auto ModelLod = CoDAssets::GameInstance->Read<MW6XModelLod>(ModelData.ModelLods + i * sizeof(MW6XModelLod));
            // Create the lod and grab reference
            ModelAsset->ModelLods.emplace_back(ModelLod.NumSurfs);
            // Grab reference
            auto& LodReference = ModelAsset->ModelLods[i];

            // Set distance (use index, "distance" we see may be bounds? doesn't line up)
            LodReference.LodDistance = i;

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
                auto SurfaceInfo = CoDAssets::GameInstance->Read<MW6XModelSurface>(XSurfacePtr);

                // Apply surface info
                SubmeshReference.RigidWeightsPtr              = SurfaceInfo.RigidWeightsPtr;
                SubmeshReference.VertListcount                = SurfaceInfo.VertListCount;
                SubmeshReference.VertexCount                  = SurfaceInfo.VertexCount;
                SubmeshReference.FaceCount                    = SurfaceInfo.FacesCount;
                SubmeshReference.PackedIndexTableCount        = SurfaceInfo.PackedIndexTableCount;
                SubmeshReference.VertexPtr                    = SurfaceInfo.Offsets[0];
                SubmeshReference.VertexUVsPtr                 = SurfaceInfo.Offsets[1];
                SubmeshReference.VertexNormalsPtr             = SurfaceInfo.Offsets[2];
                SubmeshReference.FacesPtr                     = SurfaceInfo.Offsets[3];
                SubmeshReference.VertexColorPtr               = SurfaceInfo.Offsets[6];
                SubmeshReference.PackedIndexTablePtr          = SurfaceInfo.Offsets[4];
                SubmeshReference.PackedIndexBufferPtr         = SurfaceInfo.Offsets[5];
                SubmeshReference.VertexColorPtr               = SurfaceInfo.Offsets[6];
                SubmeshReference.WeightsPtr                   = SurfaceInfo.Offsets[11];

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
                XSurfacePtr += sizeof(MW6XModelSurface);
                ModelData.MaterialHandlesPtr += sizeof(uint64_t);
            }
        }

        // Return it
        return ModelAsset;
    }
    // Not running
    return nullptr;
}

std::unique_ptr<XImageDDS> GameModernWarfare6::ReadXImage(const CoDImage_t* Image)
{
    // Proxy off
    return LoadXImage(XImage_t(ImageUsageType::DiffuseMap, 0, Image->AssetPointer, Image->AssetName));
}

std::unique_ptr<XSound> GameModernWarfare6::ReadXSound(const CoDSound_t* Sound)
{
    // Read the Sound Asset structure
    auto SoundData = CoDAssets::GameInstance->Read<MW6SndAsset>(Sound->AssetPointer);

    if (SoundData.StreamKey != 0)
    {
        // Buffer
        std::unique_ptr<uint8_t[]> SoundBuffer = nullptr;
        // Extract buffer, these are compressed, pad the audio size since the size in sound asset is literal, but XPAK data is padded.
        uint32_t SoundMemoryResult = 0;
        SoundBuffer = CoDAssets::GamePackageCache->ExtractPackageObject(SoundData.StreamKey, (int32_t)(((uint64_t)SoundData.Size + (size_t)SoundData.SeekTableSize + 4095) & 0xFFFFFFFFFFFFFFF0), SoundMemoryResult);

        if (SoundMemoryResult == 0)
            return nullptr;

        if (SoundBuffer == nullptr)
            return nullptr;

        return SABSupport::DecodeOpusInterleaved(SoundBuffer.get() + SoundData.SeekTableSize, (size_t)SoundMemoryResult - SoundData.SeekTableSize, 0, SoundData.FrameRate, SoundData.ChannelCount, SoundData.FrameCount);
    }
    else
    {
        // Buffer
        std::unique_ptr<uint8_t[]> SoundBuffer = nullptr;
        // Extract buffer, these are compressed, pad the audio size since the size in sound asset is literal, but XPAK data is padded.
        uint32_t SoundMemoryResult = 0;
        SoundBuffer = CoDAssets::GamePackageCache->ExtractPackageObject(SoundData.StreamKeyEx, (int32_t)(((uint64_t)SoundData.LoadedSize + (size_t)SoundData.SeekTableSize + 4095) & 0xFFFFFFFFFFFFFFF0), SoundMemoryResult);

        if (SoundMemoryResult == 0)
            return nullptr;

        if (SoundBuffer == nullptr)
            return nullptr;

        return SABSupport::DecodeOpusInterleaved(SoundBuffer.get() + 32 + SoundData.SeekTableSize, (size_t)SoundMemoryResult - 32 - SoundData.SeekTableSize, 0, SoundData.FrameRate, SoundData.ChannelCount, SoundData.FrameCount);
    }

}

void GameModernWarfare6::TranslateRawfile(const CoDRawFile_t * Rawfile, const std::string & ExportPath)
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
    auto Bank = CoDAssets::GameInstance->Read<MW6SoundBank>(Rawfile->AssetPointer);

    // Always stream, data in memory gets bamboozled like Thomas Cat's death
    // Size read
    uint32_t ResultSize = 0;
    // Buffer
    std::unique_ptr<uint8_t[]> Data = CoDAssets::GamePackageCache->ExtractPackageObject(Bank.StreamKey, Bank.SoundBankSize, ResultSize);

    // Prepare if we have it
    if (Data != nullptr)
    {
        // New writer instance
        auto Writer = BinaryWriter();
        // Create the file
        if (Writer.Create(FileSystems::CombinePath(ExportFolder, Rawfile->AssetName)))
        {
            // Write the raw data
            Writer.Write(Data.get(), (uint32_t)Bank.SoundBankSize);
        }
    }
}

const XMaterial_t GameModernWarfare6::ReadXMaterial(uint64_t MaterialPointer)
{
    //// Check for SP Files
    //if (CoDAssets::GameFlags == SupportedGameFlags::SP)
    //{
    //    // Prepare to parse the material
    //    auto MaterialData = CoDAssets::GameInstance->Read<MW6XMaterialSP>(MaterialPointer);

    //    // Allocate a new material with the given image count
    //    XMaterial_t Result(MaterialData.ImageCount);
    //    // Clean the name, then apply it
    //    if (MaterialData.NamePtr > 0)
    //        Result.MaterialName = Strings::Replace(FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(MaterialData.NamePtr)), "*", "");
    //    else
    //        Result.MaterialName = CoDAssets::GetHashedName("xmaterial", MaterialData.Hash);

    //    // Iterate over material images, assign proper references if available
    //    for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
    //    {
    //        // Read the image info
    //        auto ImageInfo = CoDAssets::GameInstance->Read<MW6XMaterialImage>(MaterialData.ImageTablePtr);
    //        // Read the image name (End of image - 8)
    //        auto ImageName = CoDAssets::GetHashedName("ximage", CoDAssets::GameInstance->Read<uint64_t>(ImageInfo.ImagePtr));

    //        // Default type
    //        auto DefaultUsage = ImageUsageType::Unknown;
    //        // Check 
    //        switch (ImageInfo.Type)
    //        {
    //        case 0:
    //            DefaultUsage = ImageUsageType::DiffuseMap;
    //            break;
    //        }

    //        // Assign the new image
    //        Result.Images.emplace_back(DefaultUsage, ImageInfo.Type, ImageInfo.ImagePtr, ImageName);

    //        // Advance
    //        MaterialData.ImageTablePtr += sizeof(IWXMaterialImage);
    //    }

    //    // Return it
    //    return Result;
    //}
    //else
    {
        // Prepare to parse the material
        auto MaterialData = CoDAssets::GameInstance->Read<MW6XMaterial>(MaterialPointer);

        // Allocate a new material with the given image count
        XMaterial_t Result(MaterialData.ImageCount);
        // Clean the name, then apply it
        Result.MaterialName = CoDAssets::GetHashedName("xmaterial", MaterialData.Hash);

        // Iterate over material images, assign proper references if available
        for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
        {
            // Read the image info
            auto ImageInfo = CoDAssets::GameInstance->Read<MW6XMaterialImage>(MaterialData.ImageTablePtr);
            // Read the image name (End of image - 8)
            auto ImageName = CoDAssets::GetHashedName("ximage", CoDAssets::GameInstance->Read<uint64_t>(ImageInfo.ImagePtr));

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
}

void GameModernWarfare6::PrepareVertexWeights(MemoryReader& ComplexReader, std::vector<WeightsData>& Weights, const XModelSubmesh_t & Submesh)
{
    // The index of read weight data
    uint32_t WeightDataIndex = 0;

    // Prepare the simple, rigid weights
    for (uint32_t i = 0; i < Submesh.VertListcount; i++)
    {
        // Simple weights build, rigid, just apply the proper bone id
        auto RigidInfo = CoDAssets::GameInstance->Read<MW6GfxRigidVerts>(Submesh.RigidWeightsPtr + (i * sizeof(MW6GfxRigidVerts)));
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

std::unique_ptr<XImageDDS> GameModernWarfare6::LoadXImage(const XImage_t& Image)
{
    // We must read the image data
    auto ImageInfo = CoDAssets::GameInstance->Read<MW6GfxImage>(Image.ImagePtr);
    // Buffer
    std::unique_ptr<uint8_t[]> ImageData = nullptr;
    size_t ImageSize = 0;

    if (ImageInfo.LoadedImagePtr != 0)
    {
        ImageData = std::make_unique<uint8_t[]>(ImageInfo.BufferSize);
        ImageSize = (size_t)ImageInfo.BufferSize;

        if (CoDAssets::GameInstance->Read(ImageData.get(), ImageInfo.LoadedImagePtr, ImageSize) != ImageSize)
            return nullptr;

        // Prepare to create a MemoryDDS file
        auto Result = CoDRawImageTranslator::TranslateBC(
            ImageData,
            ImageSize,
            ImageInfo.Width,
            ImageInfo.Height,
            MW6DXGIFormats[ImageInfo.ImageFormat]);

        // Return it
        return Result;
    }
    else
    {
        if (ImageInfo.MipMaps == 0)
            return nullptr;

        // Read Array of Mip Maps
        MW6GfxMipArray<32> Mips{};
        size_t MipCount = std::min((size_t)ImageInfo.MipCount, (size_t)32);

        if (CoDAssets::GameInstance->Read((uint8_t*)&Mips.MipMaps, ImageInfo.MipMaps, MipCount * sizeof(MW6GfxMip)) != (MipCount * sizeof(MW6GfxMip)))
            return nullptr;

        // An initial loop to find the fallback to use in case of CDN not being
        // a viable option.
        size_t Fallback = 0;
        size_t HighestIndex = (size_t)ImageInfo.MipCount - 1;

        for (size_t i = 0; i < MipCount; i++)
        {
            if (CoDAssets::GamePackageCache->Exists(Mips.MipMaps[i].HashID))
            {
                Fallback = i;
            }
        }

        // Calculate game specific format to DXGI
        ImageInfo.ImageFormat = MW6DXGIFormats[ImageInfo.ImageFormat];

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
                ImageInfo.Width >> (MipCount - HighestIndex - 1),
                ImageInfo.Height >> (MipCount - HighestIndex - 1),
                ImageInfo.ImageFormat);

            // Return it
            return Result;
        }
    }

    // failed to load the image
    return nullptr;
}

void GameModernWarfare6::LoadXModel(const std::unique_ptr<XModel_t>& Model, const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel)
{
    // Scale to use
    auto ScaleConstant = (1.0f / 0x1FFFFF) * 2.0f;
    // Check if we want Vertex Colors
    bool ExportColors = (SettingsManager::GetSetting("exportvtxcolor", "true") == "true");
    // Read the mesh information
    auto MeshInfo = CoDAssets::GameInstance->Read<MW6XModelMesh>(ModelLOD.LODStreamInfoPtr);

    // Read Buffer Info
    auto BufferInfo = CoDAssets::GameInstance->Read<MW6XModelMeshBufferInfo>(MeshInfo.MeshBufferPointer);

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

    // Grab our xmodel bones
    CoDXModelBonesHelper::ReadXModelBones(Model, ModelLOD, ResultModel);

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

                // Read and assign position
                Vertex.Position = CoDXModelMeshHelper::Unpack21BitVertex(VertexPosReader.Read<uint64_t>(), Submesh.Scale, { Submesh.XOffset, Submesh.YOffset, Submesh.ZOffset });
                Vertex.Normal = VertexNormReader.Read<CoDQTangent>().Unpack(nullptr, nullptr);

                // Read and set UVs
                auto UVU = HalfFloats::ToFloat(VertexUVReader.Read<uint16_t>());
                auto UVV = HalfFloats::ToFloat(VertexUVReader.Read<uint16_t>());

                // Set it
                Vertex.AddUVLayer(UVU, UVV);

                // Apply Color (some models don't store colors, so we need to check ptr below)
                Vertex.Color[0] = 255;
                Vertex.Color[1] = 255;
                Vertex.Color[2] = 255;
                Vertex.Color[3] = 255;

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

            uint32_t x[3]{};
            uint8_t* table = MeshDataBuffer.get() + Submesh.PackedIndexTablePtr;
            uint8_t* packed = MeshDataBuffer.get() + Submesh.PackedIndexBufferPtr;
            uint8_t* indices = MeshDataBuffer.get() + Submesh.FacesPtr;

            for (uint16_t i = 0; i < Submesh.FaceCount; i++)
            {
                CoDXModelMeshHelper::UnpackFaceIndices(table, Submesh.PackedIndexTableCount, packed, (uint16_t*)indices, i, x);

                // Add the face
                Mesh.AddFace(x[0], x[1], x[2]);
            }
        }
    }
}

void GameModernWarfare6::LoadXAnim(const std::unique_ptr<XAnim_t>& Anim, std::unique_ptr<WraithAnim>& ResultAnim)
{
    // We're going to need the animation header, we'll be streaming in each data buffer.
    auto animHeader = CoDAssets::GameInstance->Read<MW6XAnim>(Anim->ReaderInformationPointer);
    auto animBuffers = std::vector<std::unique_ptr<uint8_t[]>>();
    auto indicesBuffer = std::make_unique<uint8_t[]>(animHeader.FrameCount >= 0x100 ? (size_t)animHeader.IndexCount * 2 : (size_t)animHeader.IndexCount);

    // Determine animation type
    ResultAnim->AnimType = WraithAnimationType::Relative;

    // The above array is a view over the entire unpacked XPAK object, but we also need to store
    // pointers to each data type, DataBytes and DataShorts will always be in buffer 0, but we will
    // have multiple random data across each buffer.
    uint8_t* dataByte = (uint8_t*)animBuffers.data();
    int16_t* dataShort = (int16_t*)animBuffers.data();
    int32_t* dataInt = (int32_t*)animBuffers.data();
    uint16_t* indices = (uint16_t*)indicesBuffer.get();
    auto randomDataBytes = std::vector<uint8_t*>();
    auto randomDataShorts = std::vector<int16_t*>();

    // We need this if the xanim has multiple buffers, as it contains per-channel info on data offsets/buffers.
    auto animPackedInfo = std::make_unique<uint32_t[]>(animHeader.DataInfo.PackedInfoCount);
    CoDAssets::GameInstance->Read((uint8_t*)animPackedInfo.get(), animHeader.DataInfo.PackedInfoPtr, (size_t)animHeader.DataInfo.PackedInfoCount * 4);

    // Calculate Indices Size
    CoDAssets::GameInstance->Read(indicesBuffer.get(), animHeader.IndicesPtr, animHeader.FrameCount >= 0x100 ? (size_t)animHeader.IndexCount * 2 : (size_t)animHeader.IndexCount);

    uint32_t bufferSize = 0;

    for (size_t i = 0; i < animHeader.DataInfo.OffsetCount; i++)
    {
        auto StreamInfo = CoDAssets::GameInstance->Read<MW6XAnimStreamInfo>(animHeader.DataInfo.StreamInfoPtr + i * 16);

        animBuffers.push_back(CoDAssets::GamePackageCache->ExtractPackageObject(StreamInfo.StreamKey, StreamInfo.Size, bufferSize));

        // If this is the first data buffer, we'll need to grab extra stuff from it.
        if (i == 0)
        {
            // Check if we have the following, if we get -1, we don't have that data to copy, sad times indeed.
            if (animHeader.DataInfo.DataByteOffset != -1)
                dataByte = animBuffers[i].get() + animHeader.DataInfo.DataByteOffset;
            if (animHeader.DataInfo.DataShortOffset != -1)
                dataShort = (int16_t*)(animBuffers[i].get() + animHeader.DataInfo.DataShortOffset);
            if (animHeader.DataInfo.DataIntOffset != -1)
                dataInt = (int32_t*)(animBuffers[i].get() + animHeader.DataInfo.DataIntOffset);
        }

        // Now grab our random data offsets.
        auto RandomDataAOffset = CoDAssets::GameInstance->Read<uint32_t>(animHeader.DataInfo.OffsetPtr2 + i * 4);
        auto RandomDataBOffset = CoDAssets::GameInstance->Read<uint32_t>(animHeader.DataInfo.OffsetPtr + i * 4);

        if (RandomDataAOffset != -1)
            randomDataBytes.push_back(animBuffers[i].get() + RandomDataAOffset);
        else
            randomDataBytes.push_back(nullptr);

        if (RandomDataBOffset != -1)
            randomDataShorts.push_back((int16_t*)(animBuffers[i].get() + RandomDataBOffset));
        else
            randomDataShorts.push_back(nullptr);
    }

    MW6XAnimBufferState state{};

    state.OffsetCount = animHeader.DataInfo.OffsetCount;
    state.PackedPerFrameInfo = animPackedInfo.get();

    // Calculate the size of frames and inline bone indicies
    uint32_t FrameSize = (Anim->FrameCount > 255) ? 2 : 1;
    uint32_t BoneTypeSize = (Anim->TotalBoneCount > 255) ? 2 : 1;

    size_t currentBoneIndex = 0;
    size_t currentSize = Anim->NoneRotatedBoneCount;
    bool byteFrames = Anim->FrameCount < 0x100;

    // Stage 0: Zero-Rotated bones
    // Zero-rotated bones must be reset to identity, they are the first set of bones
    // Note: NoneTranslated bones aren't reset, they remain scene position
    while (currentBoneIndex < currentSize)
    {
        // Add the keyframe
        ResultAnim->AddRotationKey(Anim->Reader->BoneNames[currentBoneIndex++], 0, 0, 0, 0, 1.0f);
    }

    currentSize += Anim->TwoDRotatedBoneCount;

    // 2D Bones
    while (currentBoneIndex < currentSize)
    {
        auto tableSize = *dataShort++;

        if (tableSize >= 0x40 && !byteFrames)
            dataShort += (tableSize - 1 >> 8) + 2;

        for (int i = 0; i < tableSize + 1; i++)
        {
            uint32_t frame = 0;

            if (byteFrames)
            {
                frame = *dataByte++;
            }
            else
            {
                frame = tableSize >= 0x40 ? *indices++ : *dataShort++;
            }

            MW6XAnimCalculateBufferIndex(&state, (size_t)tableSize + 1, i);

            auto randomDataShort = randomDataShorts[state.BufferIndex] + 2 * state.BufferOffset;

            float RZ = (float)randomDataShort[0] * 0.000030518509f;
            float RW = (float)randomDataShort[1] * 0.000030518509f;

            ResultAnim->AddRotationKey(Anim->Reader->BoneNames[currentBoneIndex], frame, 0, 0, RZ, RW);
        }

        MW6XAnimIncrementBuffers(&state, tableSize + 1, 2, randomDataShorts);
        currentBoneIndex++;
    }

    currentSize += Anim->NormalRotatedBoneCount;

    // 3D Rotations
    while (currentBoneIndex < currentSize)
    {
        auto tableSize = *dataShort++;

        if (tableSize >= 0x40 && !byteFrames)
            dataShort += (tableSize - 1 >> 8) + 2;

        for (int i = 0; i < tableSize + 1; i++)
        {
            uint32_t frame = 0;

            if (byteFrames)
            {
                frame = *dataByte++;
            }
            else
            {
                frame = tableSize >= 0x40 ? *indices++ : *dataShort++;
            }

            MW6XAnimCalculateBufferIndex(&state, (size_t)tableSize + 1, i);

            auto randomDataShort = randomDataShorts[state.BufferIndex] + 4 * state.BufferOffset;

            float RX = (float)randomDataShort[0] * 0.000030518509f;
            float RY = (float)randomDataShort[1] * 0.000030518509f;
            float RZ = (float)randomDataShort[2] * 0.000030518509f;
            float RW = (float)randomDataShort[3] * 0.000030518509f;

            ResultAnim->AddRotationKey(Anim->Reader->BoneNames[currentBoneIndex], frame, RX, RY, RZ, RW);
        }

        MW6XAnimIncrementBuffers(&state, tableSize + 1, 4, randomDataShorts);
        currentBoneIndex++;
    }

    currentSize += Anim->TwoDStaticRotatedBoneCount;

    // Stage 3: 2D Static Rotations
    // 2D Static Rotations appear directly after the "3D Rotations"
    while (currentBoneIndex < currentSize)
    {
        float RZ = (float)*dataShort++ * 0.000030518509f;
        float RW = (float)*dataShort++ * 0.000030518509f;
        ResultAnim->AddRotationKey(Anim->Reader->BoneNames[currentBoneIndex], 0, 0, 0, RZ, RW);
        currentBoneIndex++;
    }

    currentSize += Anim->NormalStaticRotatedBoneCount;

    // Stage 4: 3D Static Rotations
    // 3D Static Rotations appear directly after the "2D Static Rotations"
    while (currentBoneIndex < currentSize)
    {
        float RX = (float)*dataShort++ * 0.000030518509f;
        float RY = (float)*dataShort++ * 0.000030518509f;
        float RZ = (float)*dataShort++ * 0.000030518509f;
        float RW = (float)*dataShort++ * 0.000030518509f;
        ResultAnim->AddRotationKey(Anim->Reader->BoneNames[currentBoneIndex], 0, RX, RY, RZ, RW);
        currentBoneIndex++;
    }

    currentBoneIndex = 0;
    currentSize = Anim->NormalTranslatedBoneCount;

    while (currentBoneIndex++ < currentSize)
    {
        auto boneIndex = *dataShort++; // TODO: Allow for different sizes. Atm specific to MW2.
        auto tableSize = *dataShort++;

        if (tableSize >= 0x40 && !byteFrames)
            dataShort += (tableSize - 1 >> 8) + 2;

        float minsVecX = *(float*)dataInt++;
        float minsVecY = *(float*)dataInt++;
        float minsVecZ = *(float*)dataInt++;
        float frameVecX = *(float*)dataInt++;
        float frameVecY = *(float*)dataInt++;
        float frameVecZ = *(float*)dataInt++;

        for (int i = 0; i < tableSize + 1; i++)
        {
            int frame = 0;

            if (byteFrames)
            {
                frame = *dataByte++;
            }
            else
            {
                frame = tableSize >= 0x40 ? *indices++ : *dataShort++;
            }

            MW6XAnimCalculateBufferIndex(&state, (size_t)tableSize + 1, i);

            auto randomDataByte = randomDataBytes[state.BufferIndex] + 3 * state.BufferOffset;

            // Calculate translation
            float TranslationX = (frameVecX * (float)randomDataByte[0]) + minsVecX;
            float TranslationY = (frameVecY * (float)randomDataByte[1]) + minsVecY;
            float TranslationZ = (frameVecZ * (float)randomDataByte[2]) + minsVecZ;
            // Add
            ResultAnim->AddTranslationKey(Anim->Reader->BoneNames[boneIndex], frame, TranslationX, TranslationY, TranslationZ);
        }

        MW6XAnimIncrementBuffers(&state, tableSize + 1, 3, randomDataBytes);
    }

    currentBoneIndex = 0;
    currentSize = Anim->PreciseTranslatedBoneCount;

    while (currentBoneIndex++ < currentSize)
    {
        auto boneIndex = *dataShort++; // TODO: Allow for different sizes. Atm specific to MW2.
        auto tableSize = *dataShort++;

        if (tableSize >= 0x40 && !byteFrames)
            dataShort += (tableSize - 1 >> 8) + 2;

        float minsVecX = *(float*)dataInt++;
        float minsVecY = *(float*)dataInt++;
        float minsVecZ = *(float*)dataInt++;
        float frameVecX = *(float*)dataInt++;
        float frameVecY = *(float*)dataInt++;
        float frameVecZ = *(float*)dataInt++;

        for (int i = 0; i < tableSize + 1; i++)
        {
            int frame = 0;

            if (byteFrames)
            {
                frame = *dataByte++;
            }
            else
            {
                frame = tableSize >= 0x40 ? *indices++ : *dataShort++;
            }

            MW6XAnimCalculateBufferIndex(&state, (size_t)tableSize + 1, i);

            auto randomDataShort = randomDataShorts[state.BufferIndex] + 3 * state.BufferOffset;

            // Calculate translation
            float TranslationX = (frameVecX * (float)(uint16_t)randomDataShort[0]) + minsVecX;
            float TranslationY = (frameVecY * (float)(uint16_t)randomDataShort[1]) + minsVecY;
            float TranslationZ = (frameVecZ * (float)(uint16_t)randomDataShort[2]) + minsVecZ;
            // Add
            ResultAnim->AddTranslationKey(Anim->Reader->BoneNames[boneIndex], frame, TranslationX, TranslationY, TranslationZ);
        }

        MW6XAnimIncrementBuffers(&state, tableSize + 1, 3, randomDataShorts);
    }

    // Stage 7: Static Translations
    // Static Translations appear directly after the "Precise Translations"
    currentBoneIndex = 0;
    currentSize = Anim->StaticTranslatedBoneCount;

    while (currentBoneIndex++ < currentSize)
    {
        // Read translation data
        auto vec = *(Vector3*)dataInt; dataInt += 3;
        auto boneIndex = *dataShort++; // TODO: Allow for different sizes. Atm specific to MW2.

        // Build the translation key
        ResultAnim->AddTranslationKey(Anim->Reader->BoneNames[boneIndex], 0, vec.X, vec.Y, vec.Z);
    }
}

std::string GameModernWarfare6::LoadStringEntry(uint64_t Index)
{
    return CoDAssets::GameInstance->ReadNullTerminatedString(ps::state->StringsAddress + Index);
}

void GameModernWarfare6::PerformInitialSetup()
{
    // Prepare to copy the oodle dll
    auto OurPath = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "oo2core_8_win64.dll");

    // Load Caches
    CoDAssets::StringCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\fnv1a_string.wni"));
    CoDAssets::StringCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\fnv1a_bones.wni"));
    CoDAssets::AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\fnv1a_v2_xanims.wni"));
    CoDAssets::AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\fnv1a_v2_ximages.wni"));
    CoDAssets::AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\fnv1a_v2_xmodels.wni"));
    CoDAssets::AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\fnv1a_v2_xmaterials.wni"));
    CoDAssets::AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\fnv1a_v2_xanims_test.wni"));

    // Copy if not exists
    if (!FileSystems::FileExists(OurPath))
    {
        // Check for Battlenet
        if (FileSystems::FileExists(FileSystems::CombinePath(ps::state->GameDirectory, "oo2core_8_win64.dll")))
        {
            FileSystems::CopyFile(FileSystems::CombinePath(ps::state->GameDirectory, "oo2core_8_win64.dll"), OurPath);
        }
        else
        {
            FileSystems::CopyFile(FileSystems::CombinePath(ps::state->GameDirectory, "_retail_\\oo2core_8_win64.dll"), OurPath);
        }
    }
}

void GameModernWarfare6::PerformShutDown()
{
}
