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

XMaterial_t::XMaterial_t(uint32_t ImageCount)
{
	// Defaults
	MaterialName = "wraith_material";
	// Preallocate images
	Images.reserve(ImageCount);
}

XImage_t::XImage_t(ImageUsageType Usage, uint64_t Pointer, const std::string& Name)
{
	// Defaults
	ImageUsage = Usage;
	ImagePtr = Pointer;
	ImageName = Name;
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
	// Set type
	AssetType = WraithAssetType::Model;
	// Size
	AssetSize = -1;
}

CoDModel_t::~CoDModel_t()
{
	// Defaults
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

CoDSound_t::CoDSound_t()
{
	// Defaults
	FullPath = "";
	FrameRate = 0;
	FrameCount = 0;
	ChannelsCount = 0;
	PackageIndex = 0;
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

CoDEffect_t::CoDEffect_t()
{
	// Defaults
	ElementCount = 0;
	// Set type
	AssetType = WraithAssetType::Effect;
	// Size
	AssetSize = -1;
}

CoDEffect_t::~CoDEffect_t()
{
	// Defaults
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