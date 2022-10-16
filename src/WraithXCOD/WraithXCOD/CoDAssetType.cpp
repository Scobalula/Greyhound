#include "stdafx.h"

// The class we are implementing
#include "CoDAssetType.h"

CoDAsset_t::CoDAsset_t() : WraithAsset()
{
    // Defaults
    AssetPointer = 0;
    AssetLoadedIndex = 0;
    IsFileEntry = false;
}

CoDAsset_t::~CoDAsset_t()
{
    // Defaults
}

bool CoDAsset_t::Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const
{
    // For easy copying from our existing sort method, store with same var names temp
    auto lhs = this;
    auto rhs = candidate;

    // By default, just check if it's none.
    if (compareMethod != AssetCompareMethod::None)
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
    }

    return false;
}

AssetPool::~AssetPool()
{
    // Clean up the assets
    for (auto Asset : LoadedAssets)
    {
        // Clean it up
        delete Asset;
    }
    // Clear
    LoadedAssets.clear();
    // Reset
    LoadedAssets.shrink_to_fit();
}

CoDAnim_t::CoDAnim_t()
{
    // Defaults
    Framerate = 30.0f;
    FrameCount = 0;
    BoneCount = 0;
    ShapeCount = 0;
    // Set type
    AssetType = WraithAssetType::Animation;
    // Size
    AssetSize = -1;
    // Siege
    isSiegeAnim = false;
}

CoDAnim_t::~CoDAnim_t()
{
    // Defaults
}

bool CoDAnim_t::Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const
{
    // For easy copying from our existing sort method, store with same var names temp
    auto lhs = this;
    auto rhs = candidate;

    switch (compareMethod)
    {
    case AssetCompareMethod::ByDetails:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;

        if (candidate->AssetType == WraithAssetType::Animation)
        {
            auto Compare1 = (CoDAnim_t*)lhs;
            auto Compare2 = (CoDAnim_t*)rhs;

            if (Compare1->FrameCount < Compare2->FrameCount) return false;
            if (Compare2->FrameCount < Compare1->FrameCount) return true;
        }

        // Fall back to a name comparison, it's still sorting by "details"
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    case AssetCompareMethod::ByName:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    }
    
    // Nothing, always return false.
    return false;
}

XMaterial_t::XMaterial_t(uint32_t ImageCount)
{
    // Defaults
    MaterialName = "wraith_material";
    // Preallocate images
    Images.reserve(ImageCount);
}

XImage_t::XImage_t(ImageUsageType Usage, uint32_t Hash, uint64_t Pointer, const std::string& Name)
{
    // Defaults
    ImageUsage = Usage;
    ImagePtr = Pointer;
    ImageName = Name;
    SemanticHash = Hash;
}

XModelSubmesh_t::XModelSubmesh_t()
{
    // Defaults
    VertListcount = 0;
    RigidWeightsPtr = 0;
    VertexCount = 0;
    FaceCount = 0;
    FacesPtr = 0;
    VertexPtr = 0;
    // Clear
    std::memset(&WeightCounts, 0, sizeof(WeightCounts));
    // Set
    WeightsPtr = 0;
    MaterialIndex = -1;
}

XModelLod_t::XModelLod_t(uint16_t SubmeshCount)
{
    // Reserve memory for the count we have
    Submeshes.reserve(SubmeshCount);
    Materials.reserve(SubmeshCount);
    // Defaults
    LodDistance = 100.0f;
    LodMaxDistance = 200.0f;
    LODStreamKey = 0;
    LODStreamInfoPtr = 0;
}

XModel_t::XModel_t(uint16_t LodCount)
{
    // Reserve memory for the count we have
    ModelLods.reserve(LodCount);
    // Defaults
    ModelName = "";
    BoneRotationData = BoneDataTypes::DivideBySize;
    BoneCount = 0;
    RootBoneCount = 0;
    CosmeticBoneCount = 0;
    BoneIDsPtr = 0;
    BoneIndexSize = 2;
    BoneParentsPtr = 0;
    BoneParentSize = 1;
    RotationsPtr = 0;
    TranslationsPtr = 0;
    BaseMatriciesPtr = 0;
    BoneInfoPtr = 0;
    IsModelStreamed = false;
}

CoDModel_t::CoDModel_t()
{
    // Defaults
    BoneCount = 0;
    LodCount = 0;
    CosmeticBoneCount = 0;
    // Set type
    AssetType = WraithAssetType::Model;
    // Size
    AssetSize = -1;
}

CoDModel_t::~CoDModel_t()
{
    // Defaults
}

bool CoDModel_t::Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const
{
    // For easy copying from our existing sort method, store with same var names temp
    auto lhs = this;
    auto rhs = candidate;

    switch (compareMethod)
    {
    case AssetCompareMethod::ByDetails:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;

        if (candidate->AssetType == WraithAssetType::Model)
        {
            auto Compare1 = (CoDModel_t*)lhs;
            auto Compare2 = (CoDModel_t*)rhs;

            if (Compare1->BoneCount < Compare2->BoneCount) return false;
            if (Compare2->BoneCount < Compare1->BoneCount) return true;
        }

        // Fall back to a name comparison, it's still sorting by "details"
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    case AssetCompareMethod::ByName:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    }

    // Nothing, always return false.
    return false;
}

CoDImage_t::CoDImage_t()
{
    // Defaults
    Width = 0;
    Height = 0;
    Format = 0;
    // Set type
    AssetType = WraithAssetType::Image;
    // Size
    AssetSize = -1;
}

CoDImage_t::~CoDImage_t()
{
    // Defaults
}

bool CoDImage_t::Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const
{
    // For easy copying from our existing sort method, store with same var names temp
    auto lhs = this;
    auto rhs = candidate;

    switch (compareMethod)
    {
    case AssetCompareMethod::ByDetails:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;

        if (candidate->AssetType == WraithAssetType::Image)
        {
            auto Compare1 = (CoDImage_t*)lhs;
            auto Compare2 = (CoDImage_t*)rhs;

            auto PixelCount1 = Compare1->Width * Compare1->Height;
            auto PixelCount2 = Compare2->Width * Compare2->Height;

            if (PixelCount1 < PixelCount2) return false;
            if (PixelCount2 < PixelCount1) return true;
        }

        // Fall back to a name comparison, it's still sorting by "details"
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    case AssetCompareMethod::ByName:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    }

    // Nothing, always return false.
    return false;
}

CoDSound_t::CoDSound_t()
{
    // Defaults
    FullPath = "";
    FrameRate = 0;
    FrameCount = 0;
    ChannelsCount = 0;
    PackageIndex = 0;
    Length = 0;
    IsLocalized = false;
    DataType = SoundDataTypes::WAV_WithHeader;
    // Set type
    AssetType = WraithAssetType::Sound;
    // Size
    AssetSize = -1;
}

CoDSound_t::~CoDSound_t()
{
    // Defaults
}

bool CoDSound_t::Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const
{
    // For easy copying from our existing sort method, store with same var names temp
    auto lhs = this;
    auto rhs = candidate;

    switch (compareMethod)
    {
    case AssetCompareMethod::ByDetails:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;

        if (candidate->AssetType == WraithAssetType::Sound)
        {
            auto Compare1 = (CoDSound_t*)lhs;
            auto Compare2 = (CoDSound_t*)rhs;

            if (Compare1->Length < Compare2->Length) return false;
            if (Compare2->Length < Compare1->Length) return true;
        }

        // Fall back to a name comparison, it's still sorting by "details"
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    case AssetCompareMethod::ByName:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    }

    // Nothing, always return false.
    return false;
}

CoDRawFile_t::CoDRawFile_t()
{
    // Defaults
    RawFilePath = "";
    // The data pointer
    RawDataPointer = 0;
    // Set type
    AssetType = WraithAssetType::RawFile;
    // Size
    AssetSize = -1;
}

CoDRawFile_t::~CoDRawFile_t()
{
    // Defaults
}

bool CoDRawFile_t::Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const
{
    // For easy copying from our existing sort method, store with same var names temp
    auto lhs = this;
    auto rhs = candidate;

    switch (compareMethod)
    {
    case AssetCompareMethod::ByDetails:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;

        if (candidate->AssetType == WraithAssetType::RawFile)
        {
            auto Compare1 = (CoDRawFile_t*)lhs;
            auto Compare2 = (CoDRawFile_t*)rhs;

            if (Compare1->AssetSize < Compare2->AssetSize) return false;
            if (Compare2->AssetSize < Compare1->AssetSize) return true;
        }

        // Fall back to a name comparison, it's still sorting by "details"
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    case AssetCompareMethod::ByName:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    }

    // Nothing, always return false.
    return false;
}

XAnim_t::XAnim_t()
{
    // Defaults
    AnimationName = "";
    FrameRate = 30.0f;
    FrameCount = 0;
    ViewModelAnimation = false;
    LoopingAnimation = false;
    AdditiveAnimation = false;
    SupportsInlineIndicies = true;
    BoneIDsPtr = 0;
    BoneIndexSize = 2;
    BoneTypeSize = 0;
    DataBytesPtr = 0;
    DataShortsPtr = 0;
    DataIntsPtr = 0;
    RandomDataBytesPtr = 0;
    RandomDataShortsPtr = 0;
    RandomDataIntsPtr = 0;
    LongIndiciesPtr = 0;
    NotificationsPtr = 0;
    DeltaTranslationPtr = 0;
    Delta2DRotationsPtr = 0;
    Delta3DRotationsPtr = 0;
    RotationType = AnimationKeyTypes::DivideBySize;
    TranslationType = AnimationKeyTypes::MinSizeTable;
}

XImageDDS::XImageDDS()
{
    // Defaults
    DataBuffer = nullptr;
    DataSize = 0;
    ImagePatchType = ImagePatch::NoPatch;
}

XImageDDS::~XImageDDS()
{
    // Clean up if need be
    if (DataBuffer != nullptr)
    {
        // Delete it
        delete[] DataBuffer;
    }
}

XSound::XSound()
{
    // Defaults
    DataBuffer = nullptr;
    DataSize = 0;
    DataType = SoundDataTypes::FLAC_WithHeader;
}

XSound::~XSound()
{
    // Clean up if need be
    if (DataBuffer != nullptr)
    {
        // Delete it
        delete[] DataBuffer;
    }
}

CoDMaterial_t::CoDMaterial_t()
{
    ImageCount = 0;
    // Set type
    AssetType = WraithAssetType::Material;
}

CoDMaterial_t::~CoDMaterial_t()
{
}

bool CoDMaterial_t::Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const
{
    // For easy copying from our existing sort method, store with same var names temp
    auto lhs = this;
    auto rhs = candidate;

    switch (compareMethod)
    {
    case AssetCompareMethod::ByDetails:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;

        if (candidate->AssetType == WraithAssetType::Material)
        {
            // Materials
            auto Compare1 = (CoDMaterial_t*)lhs;
            auto Compare2 = (CoDMaterial_t*)rhs;

            // Sort by Image Count
            if (Compare1->ImageCount < Compare2->ImageCount) return false;
            if (Compare2->ImageCount < Compare1->ImageCount) return true;
        }

        // Fall back to a name comparison, it's still sorting by "details"
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    case AssetCompareMethod::ByName:
    {
        if (lhs->AssetType < rhs->AssetType) return true;
        if (rhs->AssetType < lhs->AssetType) return false;
        if (lhs->AssetName < rhs->AssetName) return true;
        if (rhs->AssetName < lhs->AssetName) return false;
        break;
    }
    }

    // Nothing, always return false.
    return false;
}

XMaterialSetting_t::XMaterialSetting_t(
    const char* name,
    const char* type,
    const float* data,
    const size_t numElements)
{
    Name = name;
    Type = type;

    std::memset(Data, 0, sizeof(Data));
    std::memcpy(Data, data, numElements * sizeof(float));

    // Append element count to match HLSL names
    if(numElements > 1)
        Type += std::to_string(numElements);
}

XMaterialSetting_t::XMaterialSetting_t(
    const char* name,
    const char* type,
    const int32_t* data,
    const size_t numElements)
{
    Name = name;
    Type = type;

    std::memset(Data, 0, sizeof(Data));

    for (size_t i = 0; i < numElements; i++)
    {
        Data[i] = (float)data[i];
    }

    // Append element count to match HLSL names
    if (numElements > 1)
        Type += std::to_string(numElements);
}

XMaterialSetting_t::XMaterialSetting_t(
    const char* name,
    const char* type,
    const uint32_t* data,
    const size_t numElements)
{
    Name = name;
    Type = type;

    std::memset(Data, 0, sizeof(Data));

    for (size_t i = 0; i < numElements; i++)
    {
        Data[i] = (float)data[i];
    }

    // Append element count to match HLSL names
    if (numElements > 1)
        Type += std::to_string(numElements);
}

AssetCompareMethod AssetCompareMethodHelper::CalculateCompareMethod(const std::string& compareMethodStr)
{
    if (compareMethodStr == "Name")
        return AssetCompareMethod::ByName;
    else if (compareMethodStr == "Details")
        return AssetCompareMethod::ByDetails;
    else
        return AssetCompareMethod::None;
}
