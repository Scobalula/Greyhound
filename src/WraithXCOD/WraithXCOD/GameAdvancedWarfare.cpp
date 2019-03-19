#include "stdafx.h"

// The class we are implementing
#include "GameAdvancedWarfare.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDRawImageTranslator.h"
#include "PAKSupport.h"

// We need the following WraithX classes
#include "Strings.h"
#include "FileSystems.h"
#include "SettingsManager.h"

// -- Initialize built-in game offsets databases

// Advanced Warfare SP
std::array<DBGameInfo, 1> GameAdvancedWarfare::SinglePlayerOffsets =
{{
	{ 0x140804690, 0x140804140, 0x14966D280, 0x14316FA00 }
}};
// Advanced Warfare MP
std::array<DBGameInfo, 1> GameAdvancedWarfare::MultiPlayerOffsets =
{{
	{ 0x1409B40D0, 0x1409B4B90, 0x147DB0080, 0x143C5B000 }
}};

// -- Finished with databases

// -- Structures for reading

enum class GfxImageMapType : uint8_t
{
	MAPTYPE_CUBE = 0x5
};

// -- End structures for reading

bool GameAdvancedWarfare::LoadOffsets()
{
	// ----------------------------------------------------
	//	Advanced Warfare pools, DBAssetPools is an array of uint64 (ptrs) of each asset pool in the game
	//	The index of the assets we use are as follows: xanim (5), xmodel (7), ximage (0x10)
	//	Index * 8 = the offset of the pool pointer in this array of pools, we can verify it using the xmodel pool and checking for "fx"
	//	On Advanced Warfare, "fx" will be the first xmodel
	//	Advanced Warfare stringtable, check entries, results may vary
	//	Reading is:
	//	MP: (StringIndex * 12) + StringTablePtr + 4
	//	SP: (StringIndex * 16) + StringTablePtr + 4
	// ----------------------------------------------------

	// Attempt to load the game offsets
	if (CoDAssets::GameInstance != nullptr)
	{
		// Check built-in offsets via game exe mode (SP/MP)
		for (auto& GameOffsets : (CoDAssets::GameFlags == SupportedGameFlags::SP) ? SinglePlayerOffsets : MultiPlayerOffsets)
		{
			// Read required offsets (XANIM, XMODEL, XIMAGE)
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 5)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 7)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 0x10)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 0x11)));
			// Verify via first xmodel asset
			auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1] + 8));
			// Check
			if (FirstXModelName == "fx")
			{
				// Verify string table, otherwise we are all set
				CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);
				// Read the first string
				if (!Strings::IsNullOrWhiteSpace(LoadStringEntry(2)))
				{
					// Add ImagePackage offset
					CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.ImagePackageTable);
					// Read and apply sizes
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 5)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 7)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0x10)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0x11)));
					// Return success
					return true;
				}
			}
			// Reset
			CoDAssets::GameOffsetInfos.clear();
		}

		// Attempt to locate via heuristic searching
		auto DBAssetsScan = CoDAssets::GameInstance->Scan("48 8B D8 48 85 C0 75 ?? F0 FF 0D");
		auto StringTableScan = CoDAssets::GameInstance->Scan("8B F2 48 8B E9 E8 ?? ?? ?? ?? 44 8B C0");
		auto PackagesTableScan = CoDAssets::GameInstance->Scan("41 56 48 83 EC 40 44 8B E1 48 8D 05");

		// We need the base address of the AW Module
		auto BaseAddress = CoDAssets::GameInstance->GetMainModuleAddress();

		// Check that we had hits
		if (DBAssetsScan > 0 && StringTableScan > 0 && PackagesTableScan > 0)
		{
			// Load info and verify
			auto GameOffsets = DBGameInfo(
				// Resolve pool ptrs from RSI
				CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan - 0xB) + BaseAddress,
				// Resolve pool sizes from RSI
				CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan + 0x34) + BaseAddress,
				// Resolve strings from LEA
				CoDAssets::GameInstance->Read<uint32_t>(StringTableScan + 0x10) + (StringTableScan + 0x14),
				// Resolve packages from LEA
				CoDAssets::GameInstance->Read<uint32_t>(PackagesTableScan + 0xC) + (PackagesTableScan + 0x10)
				);
			// Read required offsets (XANIM, XMODEL, XIMAGE)
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 5)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 7)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 0x10)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 0x11)));
			// Verify via first xmodel asset
			auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1] + 8));
			// Check
			if (FirstXModelName == "fx")
			{
				// Verify string table, otherwise we are all set
				CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);
				// Read the first string
				if (!Strings::IsNullOrWhiteSpace(LoadStringEntry(2)))
				{
					// Add ImagePackage offset
					CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.ImagePackageTable);
					// Read and apply sizes
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 5)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 7)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0x10)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0x11)));
					// Return success
					return true;
				}
			}
		}
	}

	// Failed
	return false;
}

bool GameAdvancedWarfare::LoadAssets()
{
	// Prepare to load game assets, into the AssetPool
	bool NeedsAnims = (SettingsManager::GetSetting("showxanim", "true") == "true");
	bool NeedsModels = (SettingsManager::GetSetting("showxmodel", "true") == "true");
	bool NeedsImages = (SettingsManager::GetSetting("showximage", "false") == "true");
	bool NeedsSounds = (SettingsManager::GetSetting("showxsounds", "false") == "true");

	// Check if we need assets
	if (NeedsAnims)
	{
		// Animations are the first offset and first pool, skip 8 byte pointer to free head
		auto AnimationOffset = CoDAssets::GameOffsetInfos[0] + 8;
		auto AnimationCount = CoDAssets::GamePoolSizes[0];

		// Calculate maximum pool size
		auto MaximumPoolOffset = (AnimationCount * sizeof(AWXAnim)) + AnimationOffset;
		// Store original offset
		auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[0];

		// Store the placeholder anim
		AWXAnim PlaceholderAnim;
		// Clear it out
		std::memset(&PlaceholderAnim, 0, sizeof(PlaceholderAnim));

		// Loop and read
		for (uint32_t i = 0; i < AnimationCount; i++)
		{
			// Read
			auto AnimResult = CoDAssets::GameInstance->Read<AWXAnim>(AnimationOffset);

			// Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
			if ((AnimResult.NamePtr > MinimumPoolOffset && AnimResult.NamePtr < MaximumPoolOffset) || AnimResult.NamePtr == 0)
			{
				// Advance
				AnimationOffset += sizeof(AWXAnim);
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
			AnimationOffset += sizeof(AWXAnim);
		}
	}

	if (NeedsModels)
	{
		// Models are the second offset and second pool, skip 8 byte pointer to free head
		auto ModelOffset = CoDAssets::GameOffsetInfos[1] + 8;
		auto ModelCount = CoDAssets::GamePoolSizes[1];

		// Calculate maximum pool size
		auto MaximumPoolOffset = (ModelCount * sizeof(AWXModel)) + ModelOffset;
		// Store original offset
		auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[1];

		// Store the placeholder model
		AWXModel PlaceholderModel;
		// Clear it out
		std::memset(&PlaceholderModel, 0, sizeof(PlaceholderModel));

		// Loop and read
		for (uint32_t i = 0; i < ModelCount; i++)
		{
			// Read
			auto ModelResult = CoDAssets::GameInstance->Read<AWXModel>(ModelOffset);

			// Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
			if ((ModelResult.NamePtr > MinimumPoolOffset && ModelResult.NamePtr < MaximumPoolOffset) || ModelResult.NamePtr == 0)
			{
				// Advance
				ModelOffset += sizeof(AWXModel);
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
			ModelOffset += sizeof(AWXModel);
		}
	}

	if (NeedsImages)
	{
		// Images are the third offset and third pool, skip 8 byte pointer to free head
		auto ImageOffset = CoDAssets::GameOffsetInfos[2] + 8;
		auto ImageCount = CoDAssets::GamePoolSizes[2];

		// Calculate maximum pool size
		auto MaximumPoolOffset = (ImageCount * sizeof(AWGfxImage)) + ImageOffset;
		// Store original offset
		auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[2];

		// Loop and read
		for (uint32_t i = 0; i < ImageCount; i++)
		{
			// Read
			auto ImageResult = CoDAssets::GameInstance->Read<AWGfxImage>(ImageOffset);

			// Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
			if ((ImageResult.NextHead > MinimumPoolOffset && ImageResult.NextHead < MaximumPoolOffset) || ImageResult.NamePtr == 0)
			{
				// Advance
				ImageOffset += sizeof(AWGfxImage);
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
			ImageOffset += sizeof(AWGfxImage);
		}
	}
	if (NeedsSounds)
	{
		// A temporary table for duplicates, since we are tracing from alias entries...
		std::set<uint64_t> UniqueEntries;

		// Sounds are the fourth offset and fourth pool, skip 8 byte pointer to free head
		auto LoadedSoundOffset = CoDAssets::GameOffsetInfos[3] + 8;
		auto LoadedSoundCount = CoDAssets::GamePoolSizes[3];

		// Calculate maximum pool size
		auto MaximumPoolOffset = (LoadedSoundCount * sizeof(AWSoundAlias)) + LoadedSoundOffset;
		// Store original offset
		auto MinimumPoolOffset = CoDAssets::GameOffsetInfos[3];

		// Loop and read
		for (uint32_t i = 0; i < LoadedSoundCount; i++)
		{
			// Read
			auto SoundResult = CoDAssets::GameInstance->Read<AWSoundAlias>(LoadedSoundOffset);

			// Check whether or not to skip, if the handle is 0, or, if the handle is a pointer within the current pool
			if ((SoundResult.NamePtr > MinimumPoolOffset && SoundResult.NamePtr < MaximumPoolOffset) || SoundResult.NamePtr == 0)
			{
				// Advance
				LoadedSoundOffset += sizeof(AWSoundAlias);
				// Skip this asset
				continue;
			}

			// Validate and load if need be
			auto SoundName = CoDAssets::GameInstance->ReadNullTerminatedString(SoundResult.NamePtr);

			for (uint32_t j = 0; j < SoundResult.EntryCount; j++)
			{
				// Load Alias
				auto SoundAliasEntry = CoDAssets::GameInstance->Read<AWSoundAliasEntry>(SoundResult.EntriesPtr + (j * sizeof(AWSoundAliasEntry)));
				// Load File Spec
				auto SoundFileSpec = CoDAssets::GameInstance->Read<AWSoundAliasFileSpec>(SoundAliasEntry.FileSpecPtr);
				// Check type
				if (SoundFileSpec.Type == 1)
				{
					// Read Pointer to Sound
					auto LoadedSoundPtr = CoDAssets::GameInstance->Read<uint64_t>(SoundAliasEntry.FileSpecPtr + 8);
					// Validate uniqueness
					if (UniqueEntries.insert(LoadedSoundPtr).second == false)
						continue;
					// Read Sound
					auto LoadedSoundInfo = CoDAssets::GameInstance->Read<AWLoadedSound>(LoadedSoundPtr);

					// Validate and load if need be
					auto LoadedSoundName = CoDAssets::GameInstance->ReadNullTerminatedString(LoadedSoundInfo.NamePtr);

					// Make and add
					auto LoadedSound = new CoDSound_t();
					// Set
					LoadedSound->AssetName = FileSystems::GetFileName(LoadedSoundName);
					LoadedSound->AssetPointer = LoadedSoundInfo.SoundDataPtr;
					LoadedSound->FrameRate = LoadedSoundInfo.FrameRate;
					LoadedSound->FrameCount = LoadedSoundInfo.FrameCount;
					LoadedSound->AssetSize = LoadedSoundInfo.SoundDataSize;
					LoadedSound->ChannelsCount = LoadedSoundInfo.Channels;
					LoadedSound->IsFileEntry = false;
					LoadedSound->FullPath = FileSystems::GetDirectoryName(LoadedSoundName);
					LoadedSound->DataType = (LoadedSoundInfo.Format == 7 || LoadedSoundInfo.Format == 6) ? SoundDataTypes::FLAC_WithHeader : SoundDataTypes::WAV_NeedsHeader;
					LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
					LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));
					// Add
					CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
				}
				else if (SoundFileSpec.Type == 2)
				{
					// Read Data
					auto StreamedSoundInfo = CoDAssets::GameInstance->Read<AWStreamedSound>(SoundAliasEntry.FileSpecPtr + 8);
					// Check does it exist
					{
						// Make and add
						auto LoadedSound = new CoDSound_t();
						// Set (we'll use the alias names since streamed audio is nameless)
						LoadedSound->AssetName = Strings::Format("%s_%i", Strings::ToLower(SoundName).c_str(), j);
						LoadedSound->IsFileEntry = true;
						LoadedSound->DataType = SoundDataTypes::FLAC_WithHeader;
						LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
						LoadedSound->PackageIndex = StreamedSoundInfo.PackageIndex;
						LoadedSound->AssetPointer = StreamedSoundInfo.Offset;
						LoadedSound->AssetSize = StreamedSoundInfo.Size;
						LoadedSound->Length = StreamedSoundInfo.Length;
						LoadedSound->IsLocalized = StreamedSoundInfo.Exists > 0;
						// Add
						CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
					}
				}
				else
				{
#if _DEBUG
					// Log on debug
					printf("Unknown sound type: %d\n", SoundFileSpec.Type);
#endif
				}
			}
			// Advance
			LoadedSoundOffset += sizeof(AWSoundAlias);
		}
	}


	// Success, error only on specific load
	return true;
}

std::unique_ptr<XAnim_t> GameAdvancedWarfare::ReadXAnim(const CoDAnim_t* Animation)
{
	// Verify that the program is running
	if (CoDAssets::GameInstance->IsRunning())
	{
		// Prepare to read the xanim
		auto Anim = std::make_unique<XAnim_t>();

		// Read the XAnim structure
		auto AnimData = CoDAssets::GameInstance->Read<AWXAnim>(Animation->AssetPointer);

		// Copy over default properties
		Anim->AnimationName = Animation->AssetName;
		// Frames and Rate
		Anim->FrameCount = AnimData.NumFrames;
		Anim->FrameRate = AnimData.Framerate;

		// Check for viewmodel animations
		if ((_strnicmp(Animation->AssetName.c_str(), "viewmodel_", 10) == 0) || (_strnicmp(Animation->AssetName.c_str(), "vm_", 3) == 0) || (_strnicmp(Animation->AssetName.c_str(), "va_", 3) == 0))
		{
			// This is a viewmodel animation
			Anim->ViewModelAnimation = true;
		}
		// Check for looping
		Anim->LoopingAnimation = (AnimData.Flags & 1);

		// Read the delta data
		auto AnimDeltaData = CoDAssets::GameInstance->Read<AWXAnimDeltaParts>(AnimData.DeltaPartsPtr);

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

		// Set types, we use dividebysize for AW
		Anim->RotationType = AnimationKeyTypes::DivideBySize;
		Anim->TranslationType = AnimationKeyTypes::MinSizeTable;

		// Advanced Warfare supports inline indicies
		Anim->SupportsInlineIndicies = true;

		// Return it
		return Anim;
	}
	// Not running
	return nullptr;
}

std::unique_ptr<XModel_t> GameAdvancedWarfare::ReadXModel(const CoDModel_t* Model)
{
	// Verify that the program is running
	if (CoDAssets::GameInstance->IsRunning())
	{
		// Read the XModel structure
		auto ModelData = CoDAssets::GameInstance->Read<AWXModel>(Model->AssetPointer);

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
				auto SurfaceInfo = CoDAssets::GameInstance->Read<AWXModelSurface>(XSurfacePtr);

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
				XSurfacePtr += sizeof(AWXModelSurface);
				ModelData.MaterialHandlesPtr += sizeof(uint64_t);
			}
		}

		// Return it
		return ModelAsset;
	}
	// Not running
	return nullptr;
}

std::unique_ptr<XImageDDS> GameAdvancedWarfare::ReadXImage(const CoDImage_t* Image)
{
	// Proxy the image off, determine type if need be
	auto Usage = (Image->Format == 84) ? ImageUsageType::NormalMap : ImageUsageType::DiffuseMap;
	// Proxy off
	return LoadXImage(XImage_t(Usage, 0, Image->AssetPointer, Image->AssetName));
}

const XMaterial_t GameAdvancedWarfare::ReadXMaterial(uint64_t MaterialPointer)
{
	// Prepare to parse the material
	auto MaterialData = CoDAssets::GameInstance->Read<AWXMaterial>(MaterialPointer);

	// Allocate a new material with the given image count
	XMaterial_t Result(MaterialData.ImageCount);
	// Clean the name, then apply it
	Result.MaterialName = FileSystems::GetFileNameWithoutExtension(CoDAssets::GameInstance->ReadNullTerminatedString(MaterialData.NamePtr));
	
	// Iterate over material images, assign proper references if available
	for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
	{
		// Read the image info
		auto ImageInfo = CoDAssets::GameInstance->Read<AWXMaterialImage>(MaterialData.ImageTablePtr);
		// Read the image name (End of image - 8)
		auto ImageName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(ImageInfo.ImagePtr + (sizeof(AWGfxImage) - 8)));

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
		MaterialData.ImageTablePtr += sizeof(AWXMaterialImage);
	}

	// Return it
	return Result;
}

std::unique_ptr<XImageDDS> GameAdvancedWarfare::LoadXImage(const XImage_t& Image)
{
	// Prepare to load an image, we only support PAK images
	uint32_t ResultSize = 0;

	// We must read the image data
	auto ImageInfo = CoDAssets::GameInstance->Read<AWGfxImage>(Image.ImagePtr);

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
	uint64_t PAKTableOffset = (((Image.ImagePtr - (CoDAssets::GameOffsetInfos[2] + 8)) / sizeof(AWGfxImage)) * (sizeof(AWPAKImageEntry) * 4)) + CoDAssets::GameOffsetInfos[5] + (LargestMip * sizeof(AWPAKImageEntry));

	// Read info
	auto ImageStreamInfo = CoDAssets::GameInstance->Read<AWPAKImageEntry>(PAKTableOffset);

	// Read image package name
	auto ImagePackageName = CoDAssets::GameInstance->ReadNullTerminatedString(ImageStreamInfo.ImagePAKInfoPtr + 0x18);

	// Attempt to extract the package asset
	auto ImageData = PAKSupport::AWExtractImagePackage(FileSystems::CombinePath(CoDAssets::GamePackageCache->GetPackagesPath(), ImagePackageName + ".pak"), ImageStreamInfo.ImageOffset, (ImageStreamInfo.ImageEndOffset - ImageStreamInfo.ImageOffset), ResultSize);

	// Check
	if (ImageData != nullptr)
	{
		// If we have a certain format, prepare to swizzle the data
		if (ImageInfo.ImageFormat == 84)
		{
			// We must swap the R and G channels (for each 16 bytes, swap the 8-byte parts) (A2XY)
			auto ImageDataParts = (uint64_t*)ImageData.get();

			// Store position and blocks to go
			uint32_t Position = 0;
			uint32_t TotalBlocks = ResultSize / 8;

			// Iterate and swap
			do
			{
				// Hold the first one
				auto ImagePartUpper = ImageDataParts[Position];

				// Place second in first
				ImageDataParts[Position] = ImageDataParts[Position + 1];
				ImageDataParts[Position + 1] = ImagePartUpper;

				// Advance 2
				Position += 2;

				// Loop until end
			} while (Position < TotalBlocks);
		}

		// Prepare to create a MemoryDDS file
		auto Result = CoDRawImageTranslator::TranslateBC(ImageData, ResultSize, LargestWidth, LargestHeight, ImageInfo.ImageFormat, 1, (ImageInfo.MapType == (uint8_t)GfxImageMapType::MAPTYPE_CUBE));

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

std::string GameAdvancedWarfare::LoadStringEntry(uint64_t Index)
{
	// Read and return (Offsets[4] = StringTable), sizes differ in SP and MP
	return (CoDAssets::GameFlags == SupportedGameFlags::SP) ? CoDAssets::GameInstance->ReadNullTerminatedString((16 * Index) + CoDAssets::GameOffsetInfos[4] + 4) : CoDAssets::GameInstance->ReadNullTerminatedString((12 * Index) + CoDAssets::GameOffsetInfos[4] + 4);
}