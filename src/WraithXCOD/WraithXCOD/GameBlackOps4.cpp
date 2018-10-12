#include "stdafx.h"

// The class we are implementing
#include "GameBlackOps4.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDRawImageTranslator.h"

// We need the following WraithX classes
#include "Strings.h"
#include "FileSystems.h"
#include "MemoryReader.h"
#include "SettingsManager.h"
#include "WraithNameIndex.h"
#include "HalfFloats.h"

// -- Initialize built-in game offsets databases

// Black Ops 4 SP
std::array<DBGameInfo, 1> GameBlackOps4::SinglePlayerOffsets =
{{
	{ 0x917FBD0, 0x0, 0x7FE3620, 0x0 }
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

// -- Black Ops 4 Pool Data Structure

struct BO4XAssetPoolData
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
static_assert(sizeof(BO4XAssetPoolData) == 0x20, "Invalid Pool Data Size (Expected 0x20)");

bool GameBlackOps4::LoadOffsets()
{
	// ----------------------------------------------------
	//	Black Ops 4 pools and sizes, XAssetPoolData is an array of pool info for each asset pool in the game
	//	The index of the assets we use are as follows: xanim (3), xmodel (4), ximage (0x9)
	//	Index * sizeof(BO4XAssetPoolData) = the offset of the asset info in this array of data, we can verify it using the xmodel pool and checking for the model hash (0x04647533e968c910)
	//  Notice: Black Ops 4 doesn't store a freePoolHandle at the beginning, so we just read on.
	//	On Black Ops 4, (0x04647533e968c910) will be the first xmodel
	//	Black Ops 4 stringtable, check entries, results may vary
	//	Reading is: (StringIndex * 16) + StringTablePtr + 16
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
			auto AnimPoolData = CoDAssets::GameInstance->Read<BO4XAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BO4XAssetPoolData) * 3));
			auto ModelPoolData = CoDAssets::GameInstance->Read<BO4XAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BO4XAssetPoolData) * 4));
			auto ImagePoolData = CoDAssets::GameInstance->Read<BO4XAssetPoolData>(BaseAddress + GameOffsets.DBAssetPools + (sizeof(BO4XAssetPoolData) * 0x9));

			// Apply game offset info
			CoDAssets::GameOffsetInfos.emplace_back(AnimPoolData.PoolPtr);
			CoDAssets::GameOffsetInfos.emplace_back(ModelPoolData.PoolPtr);
			CoDAssets::GameOffsetInfos.emplace_back(ImagePoolData.PoolPtr);

			// Verify via first xmodel asset, right now, we're using a hash
			auto FirstXModelHash = CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1] + 8);
			// Check
			if (FirstXModelHash == 0x04647533e968c910)
			{
				// Verify string table, otherwise we are all set
				CoDAssets::GameOffsetInfos.emplace_back(BaseAddress + GameOffsets.StringTable);

				// Read and apply sizes
				CoDAssets::GamePoolSizes.emplace_back(AnimPoolData.PoolSize);
				CoDAssets::GamePoolSizes.emplace_back(ModelPoolData.PoolSize);
				CoDAssets::GamePoolSizes.emplace_back(ImagePoolData.PoolSize);
				// Return success
				return true;
			}
			// Reset
			CoDAssets::GameOffsetInfos.clear();
		}

		// Attempt to locate via heuristic searching
		auto DBAssetsScan = CoDAssets::GameInstance->Scan("48 89 5C 24 ?? 57 48 83 EC ?? 0F B6 F9 48 8D 05 ?? ?? ?? ??");
		auto StringTableScan = CoDAssets::GameInstance->Scan("48 8B 53 ?? 48 85 D2 74 ?? 48 8B 03 48 89 02 4C 8D 1D ?? ?? ?? ??");

		// Check that we had hits
		if (DBAssetsScan > 0 && StringTableScan > 0)
		{
			// Load info and verify
			auto GameOffsets = DBGameInfo(
				// Resolve pool info from LEA
				CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan + 0x10) + (DBAssetsScan + 0x14),
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
			printf("Heuristic: { 0x%X, 0x0, 0x%X, 0x0 }\n", (GameOffsets.DBAssetPools - BaseAddress), (GameOffsets.StringTable - BaseAddress));
#endif

			// Read required offsets (XANIM, XMODEL, XIMAGE)
			auto AnimPoolData = CoDAssets::GameInstance->Read<BO4XAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BO4XAssetPoolData) * 3));
			auto ModelPoolData = CoDAssets::GameInstance->Read<BO4XAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BO4XAssetPoolData) * 4));
			auto ImagePoolData = CoDAssets::GameInstance->Read<BO4XAssetPoolData>(GameOffsets.DBAssetPools + (sizeof(BO4XAssetPoolData) * 0x9));

			// Apply game offset info
			CoDAssets::GameOffsetInfos.emplace_back(AnimPoolData.PoolPtr);
			CoDAssets::GameOffsetInfos.emplace_back(ModelPoolData.PoolPtr);
			CoDAssets::GameOffsetInfos.emplace_back(ImagePoolData.PoolPtr);

			// Verify via first xmodel asset, right now, we're using a hash
			auto FirstXModelHash = CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1] + 8);
			// Check
			if (FirstXModelHash == 0x04647533e968c910)
			{
				// Verify string table, otherwise we are all set
				CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);

				// Read and apply sizes
				CoDAssets::GamePoolSizes.emplace_back(AnimPoolData.PoolSize);
				CoDAssets::GamePoolSizes.emplace_back(ModelPoolData.PoolSize);
				CoDAssets::GamePoolSizes.emplace_back(ImagePoolData.PoolSize);

				// Return success
				return true;
			}
		}
	}

	// Failed
	return true;
}

bool GameBlackOps4::LoadAssets()
{
	// Prepare to load game assets, into the AssetPool
	bool NeedsAnims = (SettingsManager::GetSetting("showxanim", "true") == "true");
	bool NeedsModels = (SettingsManager::GetSetting("showxmodel", "true") == "true");
	bool NeedsImages = (SettingsManager::GetSetting("showximage", "false") == "true");
	bool NeedsRawFiles = (SettingsManager::GetSetting("showxrawfiles", "false") == "true");

	// Store the asset-specifc name-db
	auto NameDatabase = WraithNameIndex();

	// Check if we need assets
	if (NeedsAnims)
	{
		// Animations are the first offset and first pool
		auto AnimationOffset = CoDAssets::GameOffsetInfos[0];
		auto AnimationCount = CoDAssets::GamePoolSizes[0];

		// Load name database, if any
		NameDatabase = WraithNameIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo4_xanim.wni"));

		// Calculate maximum pool size
		auto MaximumPoolOffset = (AnimationCount * sizeof(BO4XAnim)) + AnimationOffset;
		// Store original offset
		auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[0];

		// Loop and read
		for (uint32_t i = 0; i < AnimationCount; i++)
		{
			// Read
			auto AnimResult = CoDAssets::GameInstance->Read<BO4XAnim>(AnimationOffset);

			// Check whether or not to skip, if the handle not 0, or, if the handle is a pointer within the current pool
			if ((AnimResult.NamePtr > MinimumPoolOffset && AnimResult.NamePtr < MaximumPoolOffset) || AnimResult.UnknownHash == 0)
			{
				// Advance
				AnimationOffset += sizeof(BO4XAnim);
				// Skip this asset
				continue;
			}

			// Validate and load if need be
			auto AnimName = Strings::Format("xanim_%llx", AnimResult.UnknownHash);
			// Check for an override in the name DB
			if (NameDatabase.NameDatabase.find(AnimResult.UnknownHash) != NameDatabase.NameDatabase.end())
				AnimName = NameDatabase.NameDatabase[AnimResult.UnknownHash];

			// Make and add
			auto LoadedAnim = new CoDAnim_t();
			// Set
			LoadedAnim->AssetName = AnimName;
			LoadedAnim->AssetPointer = AnimationOffset;
			LoadedAnim->Framerate = AnimResult.Framerate;
			LoadedAnim->FrameCount = AnimResult.NumFrames;
			LoadedAnim->AssetStatus = WraithAssetStatus::Loaded;

			// Add
			CoDAssets::GameAssets->LoadedAssets.push_back(LoadedAnim);

			// Advance
			AnimationOffset += sizeof(BO4XAnim);
		}
	}

	if (NeedsModels)
	{
		// Models are the second offset and second pool
		auto ModelOffset = CoDAssets::GameOffsetInfos[1];
		auto ModelCount = CoDAssets::GamePoolSizes[1];

		// Load name database, if any
		NameDatabase = WraithNameIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo4_xmodel.wni"));

		// Calculate maximum pool size
		auto MaximumPoolOffset = (ModelCount * sizeof(BO4XModel)) + ModelOffset;
		// Store original offset
		auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[1];

		// Loop and read
		for (uint32_t i = 0; i < ModelCount; i++)
		{
			// Read
			auto ModelResult = CoDAssets::GameInstance->Read<BO4XModel>(ModelOffset);

			// Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
			if ((ModelResult.NamePtr > MinimumPoolOffset && ModelResult.NamePtr < MaximumPoolOffset) || ModelResult.NamePtr == 0)
			{
				// Advance
				ModelOffset += sizeof(BO4XModel);
				// Skip this asset
				continue;
			}

			// Validate and load if need be
			auto ModelName = Strings::Format("xmodel_%llx", ModelResult.NamePtr);
			// Check for an override in the name DB
			if (NameDatabase.NameDatabase.find(ModelResult.NamePtr) != NameDatabase.NameDatabase.end())
				ModelName = NameDatabase.NameDatabase[ModelResult.NamePtr];

			// Make and add
			auto LoadedModel = new CoDModel_t();
			// Set
			LoadedModel->AssetName = ModelName;
			LoadedModel->AssetPointer = ModelOffset;
			LoadedModel->BoneCount = (ModelResult.NumBones + ModelResult.NumCosmeticBones);
			LoadedModel->LodCount = ModelResult.NumLods;
			LoadedModel->AssetStatus = WraithAssetStatus::Loaded;

			// Add
			CoDAssets::GameAssets->LoadedAssets.push_back(LoadedModel);

			// Advance
			ModelOffset += sizeof(BO4XModel);
		}
	}

	if (NeedsImages)
	{
		// Images are the third offset and third pool
		auto ImageOffset = CoDAssets::GameOffsetInfos[2];
		auto ImageCount = CoDAssets::GamePoolSizes[2];

		// Load name database, if any
		NameDatabase = WraithNameIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo4_ximage.wni"));

		// Calculate maximum pool size
		auto MaximumPoolOffset = (ImageCount * sizeof(BO4GfxImage)) + ImageOffset;
		// Store original offset
		auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[2];

		// Loop and read
		for (uint32_t i = 0; i < ImageCount; i++)
		{
			// Read
			auto ImageResult = CoDAssets::GameInstance->Read<BO4GfxImage>(ImageOffset);

			// Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
			if ((ImageResult.NamePtr > MinimumPoolOffset && ImageResult.NamePtr < MaximumPoolOffset) || ImageResult.UnknownHash == 0)
			{
				// Advance
				ImageOffset += sizeof(BO4GfxImage);
				// Skip this asset
				continue;
			}

			// Validate and load if need be
			auto ImageName = Strings::Format("ximage_%llx", ImageResult.UnknownHash);
			// Check for an override in the name DB
			if (NameDatabase.NameDatabase.find(ImageResult.UnknownHash) != NameDatabase.NameDatabase.end())
				ImageName = NameDatabase.NameDatabase[ImageResult.UnknownHash];

			// Check for loaded images
			if (ImageResult.LoadedMipPtr != 0 && ImageResult.MipLevels[0].HashID == 0)
			{
				// Make and add
				auto LoadedImage = new CoDImage_t();
				// Set
				LoadedImage->AssetName = ImageName;
				LoadedImage->AssetPointer = ImageOffset;
				LoadedImage->Width = (uint16_t)ImageResult.LoadedMipWidth;
				LoadedImage->Height = (uint16_t)ImageResult.LoadedMipHeight;
				LoadedImage->Format = (uint16_t)ImageResult.ImageFormat;
				LoadedImage->AssetStatus = WraithAssetStatus::Loaded;

				// Add
				CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
			}

			// Advance
			ImageOffset += sizeof(BO4GfxImage);
		}
	}

	if (NeedsRawFiles)
	{
		// Rawfiles (Lua) are the fourth offset and fourth pool
		auto RawfileOffset = CoDAssets::GameOffsetInfos[3];
		auto RawfileCount = CoDAssets::GamePoolSizes[3];

		// Calculate maximum pool size
		auto MaximumPoolOffset = (RawfileCount * sizeof(BO4XRawFile)) + RawfileOffset;
		// Store original offset
		auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[3];

		// Loop and read
		for (uint32_t i = 0; i < RawfileCount; i++)
		{
			// Read
			auto RawfileResult = CoDAssets::GameInstance->Read<BO4XRawFile>(RawfileOffset);

			// Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
			if ((RawfileResult.NamePtr > MinimumPoolOffset && RawfileResult.NamePtr < MaximumPoolOffset) || RawfileResult.NamePtr == 0)
			{
				// Advance
				RawfileOffset += sizeof(BO4XRawFile);
				// Skip this asset
				continue;
			}

			// Validate and load if need be
			auto RawfileName = Strings::Format("rawfile_%llx.lua", RawfileResult.NamePtr);

			// Make and add
			auto LoadedRawfile = new CoDRawFile_t();
			// Set
			LoadedRawfile->AssetName = RawfileName;
			LoadedRawfile->AssetPointer = RawfileOffset;
			LoadedRawfile->RawDataPointer = RawfileResult.RawDataPtr;
			LoadedRawfile->RawFilePath = "";
			LoadedRawfile->AssetSize = RawfileResult.AssetSize;
			LoadedRawfile->AssetStatus = WraithAssetStatus::Loaded;

			// Add
			CoDAssets::GameAssets->LoadedAssets.push_back(LoadedRawfile);

			// Advance
			RawfileOffset += sizeof(BO4XRawFile);
		}

		// AnimTrees are the fifth offset and fifth pool
		RawfileOffset = CoDAssets::GameOffsetInfos[4];
		RawfileCount = CoDAssets::GamePoolSizes[4];

		// Calculate maximum pool size
		MaximumPoolOffset = (RawfileCount * sizeof(BO4XAnimTree)) + RawfileOffset;
		// Store original offset
		MinimumPoolOffset = CoDAssets::GameOffsetInfos[4];

		// Loop and read
		for (uint32_t i = 0; i < RawfileCount; i++)
		{
			// Read
			auto RawfileResult = CoDAssets::GameInstance->Read<BO4XAnimTree>(RawfileOffset);

			// Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
			if ((RawfileResult.NamePtr > MinimumPoolOffset && RawfileResult.NamePtr < MaximumPoolOffset) || RawfileResult.NamePtr == 0)
			{
				// Advance
				RawfileOffset += sizeof(BO4XAnimTree);
				// Skip this asset
				continue;
			}

			// Validate and load if need be
			auto RawfileName = Strings::Format("rawfile_%llx.atr", RawfileResult.NamePtr);

			// Make and add
			auto LoadedRawfile = new CoDRawFile_t();
			// Set
			LoadedRawfile->AssetName = RawfileName;
			LoadedRawfile->AssetPointer = RawfileOffset;
			LoadedRawfile->RawDataPointer = RawfileResult.RawDataPtr;
			LoadedRawfile->RawFilePath = "";
			LoadedRawfile->AssetSize = RawfileResult.AssetSize;
			LoadedRawfile->AssetStatus = WraithAssetStatus::Loaded;

			// Add
			CoDAssets::GameAssets->LoadedAssets.push_back(LoadedRawfile);

			// Advance
			RawfileOffset += sizeof(BO4XAnimTree);
		}
	}

	// Success, error only on specific load
	return true;
}

std::unique_ptr<XAnim_t> GameBlackOps4::ReadXAnim(const CoDAnim_t* Animation)
{
	// Verify that the program is running
	if (CoDAssets::GameInstance->IsRunning())
	{
		// Prepare to read the xanim
		auto Anim = std::make_unique<XAnim_t>();

		// Read the XAnim structure
		auto AnimData = CoDAssets::GameInstance->Read<BO4XAnim>(Animation->AssetPointer);

		// Copy over default properties
		Anim->AnimationName = Animation->AssetName;
		// Frames and Rate
		Anim->FrameCount = AnimData.NumFrames;
		Anim->FrameRate = AnimData.Framerate;

		// TODO: Determine viewmodel anims
		// TODO: Determine additive anims
		// TODO: On first bo4 attach, copy oodle dll automatically

		////// Check for viewmodel animations
		////if ((_strnicmp(Animation->AssetName.c_str(), "viewmodel_", 10) == 0) || (_strnicmp(Animation->AssetName.c_str(), "vm_", 3) == 0))
		////{
		////	// This is a viewmodel animation
		////	Anim->ViewModelAnimation = true;
		////}
		////// Check for additive animations
		////if (AnimData.AssetType == 0x6)
		////{
		////	// This is a additive animation
		////	Anim->AdditiveAnimation = true;
		////}
		////// Check for looping
		////Anim->LoopingAnimation = (AnimData.LoopingFlag > 0);

		// Read the delta data
		auto AnimDeltaData = CoDAssets::GameInstance->Read<BO4XAnimDeltaParts>(AnimData.DeltaPartsPtr);

		// Copy over pointers
		Anim->BoneIDsPtr = AnimData.BoneIDsPtr;
		Anim->DataBytesPtr = AnimData.DataBytePtr;
		Anim->DataShortsPtr = AnimData.DataShortPtr;
		Anim->DataIntsPtr = AnimData.DataIntPtr;
		Anim->RandomDataBytesPtr = AnimData.RandomDataBytePtr;
		Anim->RandomDataShortsPtr = AnimData.RandomDataShortPtr;
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

		// Set types, we use quata for BO4
		Anim->RotationType = AnimationKeyTypes::QuatPackingA;
		Anim->TranslationType = AnimationKeyTypes::MinSizeTable;

		// Black Ops 4 doesn't support inline indicies
		Anim->SupportsInlineIndicies = false;

		// Return it
		return Anim;
	}
	// Not running
	return nullptr;
}

std::unique_ptr<XModel_t> GameBlackOps4::ReadXModel(const CoDModel_t* Model)
{
	// Verify that the program is running
	if (CoDAssets::GameInstance->IsRunning())
	{
		// Read the XModel structure
		auto ModelData = CoDAssets::GameInstance->Read<BO4XModel>(Model->AssetPointer);

		// Prepare to read the xmodel (Reserving space for lods)
		auto ModelAsset = std::make_unique<XModel_t>(ModelData.NumLods);

		// Copy over default properties
		ModelAsset->ModelName = Model->AssetName;
		// Bone counts
		ModelAsset->BoneCount = ModelData.NumBones;
		ModelAsset->RootBoneCount = ModelData.NumRootBones;
		ModelAsset->CosmeticBoneCount = ModelData.NumCosmeticBones;

		// Bone data type
		ModelAsset->BoneRotationData = BoneDataTypes::QuatPackingA;

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

		// Prepare to parse lods
		for (uint32_t i = 0; i < ModelData.NumLods; i++)
		{
			// Read the lod
			auto LODInfo = CoDAssets::GameInstance->Read<BO4XModelLod>(ModelData.ModelLodPtrs[i]);
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
			// Advance 8 and skip 16 bytes
			ModelData.MaterialHandlesPtr += 0x18;

			// Load surfaces
			for (uint32_t s = 0; s < LODInfo.NumSurfs; s++)
			{
				// Create the surface and grab reference
				LodReference.Submeshes.emplace_back();
				// Grab reference
				auto& SubmeshReference = LodReference.Submeshes[s];

				// Read the surface data
				auto SurfaceInfo = CoDAssets::GameInstance->Read<BO4XModelSurface>(XSurfacePtr);

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
				XSurfacePtr += sizeof(BO4XModelSurface);
				MaterialHandlesPtr += sizeof(uint64_t);
			}
		}

		// Return it
		return ModelAsset;
	}
	// Not running
	return nullptr;
}

std::unique_ptr<XImageDDS> GameBlackOps4::ReadXImage(const CoDImage_t* Image)
{
	// Proxy off
	return LoadXImage(XImage_t(ImageUsageType::DiffuseMap, Image->AssetPointer, Image->AssetName));
}

const XMaterial_t GameBlackOps4::ReadXMaterial(uint64_t MaterialPointer)
{
	// Prepare to parse the material
	auto MaterialData = CoDAssets::GameInstance->Read<BO4XMaterial>(MaterialPointer);

	// Allocate a new material with the given image count
	XMaterial_t Result(MaterialData.ImageCount);
	// Clean the name, then apply it
	Result.MaterialName = Strings::Format("xmaterial_%llx", MaterialData.NamePtr);

	// Iterate over material images, assign proper references if available
	for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
	{
		// Read the image info
		auto ImageInfo = CoDAssets::GameInstance->Read<BO4XMaterialImage>(MaterialData.ImageTablePtr);
		// Get the image name
		auto ImageName = Strings::Format("ximage_%llx", CoDAssets::GameInstance->Read<uint64_t>(ImageInfo.ImagePtr + 0x28));

		// Default type
		auto DefaultUsage = ImageUsageType::Unknown;

		// Check usage byte
		switch (ImageInfo.Usage)
		{
		case 1: DefaultUsage = ImageUsageType::DiffuseMap; break;
		case 2: DefaultUsage = ImageUsageType::NormalMap; break;
		}

		// Assign the new image
		Result.Images.emplace_back(DefaultUsage, ImageInfo.ImagePtr, ImageName);

		// Advance
		MaterialData.ImageTablePtr += sizeof(BO4XMaterialImage);
	}

	// Return it
	return Result;
}

std::unique_ptr<XImageDDS> GameBlackOps4::LoadXImage(const XImage_t& Image)
{
	// Prepare to load an image, we need to rip loaded and streamed ones
	uint32_t ResultSize = 0;

	// We must read the image data
	auto ImageInfo = CoDAssets::GameInstance->Read<BO4GfxImage>(Image.ImagePtr);

	// Calculate the largest image mip
	uint32_t LargestMip = 0;
	uint32_t LargestWidth = 0;
	uint32_t LargestHeight = 0;
	uint64_t LargestHash = 0;

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
		ImageData = CoDAssets::GamePackageCache->ExtractPackageObject(LargestHash, ResultSize);
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

void GameBlackOps4::LoadXModel(const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel)
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
				int32_t PackedX = (((VertexData.VertexNormal >> 0) & ((1 << 10) - 1)) - 512);
				int32_t PackedY = (((VertexData.VertexNormal >> 10) & ((1 << 10) - 1)) - 512);
				int32_t PackedZ = (((VertexData.VertexNormal >> 20) & ((1 << 10) - 1)) - 512);
				// Add Colors
				Vertex.Color[0] = ExportColors ? VertexData.Color[0] : 255;
				Vertex.Color[1] = ExportColors ? VertexData.Color[1] : 255;
				Vertex.Color[2] = ExportColors ? VertexData.Color[2] : 255;
				Vertex.Color[3] = ExportColors ? VertexData.Color[3] : 255;
				// Calculate
				Vertex.Normal.X = ((float)PackedX / 511.0f);
				Vertex.Normal.Y = ((float)PackedY / 511.0f);
				Vertex.Normal.Z = ((float)PackedZ / 511.0f);

				// Skip extended vertex information
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
				if ((MeshInfo.WeightCount > TotalReadWeights) && ((uint8_t)Submesh.WeightCounts[0] & (1 << 1)) > 0)
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
				StreamBone.TagName = Strings::Format("smod_bone%d", i);
				StreamBone.BoneParent = 0;
			}

			// Ensure root is tag_origin
			ResultModel->Bones[0].TagName = "tag_origin";
		}
	}
}

static byte Decrypt(uint8_t input, uint8_t key)
{
	// If our key equals the input we're not encrpyted
	if (input == key)
		return input;

	// Result
	uint8_t result = input ^ key;

	// Redundancy check for valid character
	if ((result > 64 && result < 91) || (result > 96 && result < 123) || (result > 47 && result < 58) || result == 95)
		return result;

	// Return default '_'
	return 95;
}

std::string GameBlackOps4::LoadStringEntry(uint64_t Index)
{
	// Check if we have an index to use
	if (Index > 0)
	{
		// Read and return (Offsets[3] = StringTable)
		uint64_t BytesRead = 0;
		// XOR Key to decrypt the string if necessary
		auto XORKey = CoDAssets::GameInstance->Read<uint8_t>((16 * Index) + CoDAssets::GameOffsetInfos[3] + 16);
		// String Size (includes terminating null character, -1 for just the string)
		auto StringSize = CoDAssets::GameInstance->Read<uint8_t>((16 * Index) + CoDAssets::GameOffsetInfos[3] + 17) - 1;
		// Resulting String
		auto Result = CoDAssets::GameInstance->Read((16 * Index) + CoDAssets::GameOffsetInfos[3] + 18, StringSize, BytesRead);
		// Decrypt string, increment/decrement depending on key. If not these, assume not encrypted.
		switch (XORKey)
		{
		case 165:
			for (uint8_t x = 0; x < StringSize; x++, XORKey--) Result[x] = Decrypt(Result[x], XORKey);
			break;
		case 175:
			for (uint8_t x = 0; x < StringSize; x++, XORKey++) Result[x] = Decrypt(Result[x], XORKey);
			break;
		case 185:
			for (uint8_t x = 0; x < StringSize; x++, XORKey -= (x - 1 + 1)) Result[x] = Decrypt(Result[x], XORKey);
			break;
		case 189:
			for (uint8_t x = 0; x < StringSize; x++, XORKey += (x - 1 + 1)) Result[x] = Decrypt(Result[x], XORKey);
			break;
		}
		// Convert to string and return
		return std::string(reinterpret_cast<char const*>(Result), StringSize);
	}
	// Return blank string
	return "";
}
void GameBlackOps4::PerformInitialSetup()
{
	// Prepare to copy the oodle dll
	auto OurPath = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "oo2core_6_win64.dll");

	// Copy if not exists
	if (!FileSystems::FileExists(OurPath))
		FileSystems::CopyFile(FileSystems::CombinePath(FileSystems::GetDirectoryName(CoDAssets::GameInstance->GetProcessPath()), "oo2core_6_win64.dll"), OurPath);
}