#include "stdafx.h"

// The class we are implementing
#include "GameWorldWar2.h"

// We need the CoDAssets class
#include "CoDAssets.h"
#include "CoDRawImageTranslator.h"
#include "CoDXModelTranslator.h"
#include "CoDXPoolParser.h"
#include "PAKSupport.h"

// We need the following WraithX classes
#include "Strings.h"
#include "FileSystems.h"
#include "MemoryReader.h"
#include "BinaryReader.h"
#include "SettingsManager.h"
#include "HalfFloats.h"
#include "Sound.h"
#include "Hashing.h"

// Include generic structures
#include "DBGameGenerics.h"

// -- Initialize built-in game offsets databases

// World War 2 SP
std::array<DBGameInfo, 8> GameWorldWar2::SinglePlayerOffsets =
{{
	{ 0x9483F0, 0xBCC5E0, 0x9E02080, 0x6CC4C00 },	// LATEST
	{ 0x9483D0, 0xBCC5E0, 0x9D9E080, 0x6C60C00 },	
	{ 0x947260, 0xBCB5D0, 0x9C62800, 0x6B49580 },
	{ 0x947100, 0xBCB590, 0x9AAD000, 0x69CA080 },
	{ 0x945FA0, 0xBCA580, 0x9FA8680, 0x6EE5180 },
	{ 0x94A280, 0xBE0580, 0x9EDDE80, 0x6E2ED80 },
	{ 0x94A280, 0xBE0580, 0x9EDDE80, 0x6E2ED80 },
	{ 0x94A280, 0xBE0580, 0x9EDDE80, 0x6E2ED80 },	// MIN (Required)
}};

// World War 2 MP
std::array<DBGameInfo, 8> GameWorldWar2::MultiPlayerOffsets =
{{
	{ 0xC05370, 0xEACC40, 0xADF3E80, 0x19E9580 },	// LATEST
	{ 0xC04360, 0xEABC40, 0xACD9100, 0x19E0D00 },
	{ 0xC03420, 0xEA9C40, 0xAB2CB00, 0x19CF100 },
	{ 0xC01330, 0xEA7C40, 0xA80FC00, 0x198B280 },
	{ 0xBFF3D0, 0xEA4C40, 0xABD8800, 0x197B700 },
	{ 0xBF73A0, 0xEB6C00, 0xA6C4D80, 0x191CB80 },
	{ 0xBF7370, 0xEB6C00, 0xA6C4D80, 0x191CB00 },
	{ 0xBF72C0, 0xEB6C00, 0xA6C3700, 0x191CB00 },	// MIN (Required)
}};

// Localization prefixes
const char* LocalizationFolders[] = { "english", "spanish" };
const char* LocalizationPrefixes[] = { "eng_", "spa_" };

// -- Finished with databases

bool GameWorldWar2::LoadOffsets()
{
	// ----------------------------------------------------
	//	World War 2 pools, DBAssetPools is an array of uint64 (ptrs) of each asset pool in the game
	//	The index of the assets we use are as follows: xanim (0x7), xmodel (0xA), ximage (0x15), sound (0x16)
	//	Index * 8 = the offset of the pool pointer in this array of pools, we can verify it using the xmodel pool and checking for "empty_model"
	//	On World War 2, "empty_model" will be the first xmodel
	//	World War 2 stringtable, check entries, results may vary
	//	Reading is: (StringIndex * 16) + StringTablePtr + 8
	// ----------------------------------------------------

	// Attempt to load the game offsets
	if (CoDAssets::GameInstance != nullptr)
	{
		// We need the base address of the WWII Module for ASLR + Heuristics
		auto BaseAddress = CoDAssets::GameInstance->GetMainModuleAddress();

		// Check built-in offsets via game exe mode (SP/MP)
		for (auto& GameOffsets : (CoDAssets::GameFlags == SupportedGameFlags::SP) ? SinglePlayerOffsets : MultiPlayerOffsets)
		{
			// Read required offsets (XANIM, XMODEL, XIMAGE, SOUND)
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(BaseAddress + GameOffsets.DBAssetPools + (8 * 0x7)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(BaseAddress + GameOffsets.DBAssetPools + (8 * 0xA)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(BaseAddress + GameOffsets.DBAssetPools + (8 * 0x15)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(BaseAddress + GameOffsets.DBAssetPools + (8 * 0x16)));
			// Verify via first xmodel asset
			auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1] + 8));
			// Check
			if (FirstXModelName == "empty_model")
			{
				// Verify string table, otherwise we are all set
				CoDAssets::GameOffsetInfos.emplace_back(BaseAddress + GameOffsets.StringTable);
				// Read the first string
				if (!Strings::IsNullOrWhiteSpace(LoadStringEntry(2)))
				{
					// Add ImagePackage offset
					CoDAssets::GameOffsetInfos.emplace_back(BaseAddress + GameOffsets.ImagePackageTable);
					// Read and apply sizes
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(BaseAddress + GameOffsets.DBPoolSizes + (4 * 0x7)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(BaseAddress + GameOffsets.DBPoolSizes + (4 * 0xA)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(BaseAddress + GameOffsets.DBPoolSizes + (4 * 0x15)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(BaseAddress + GameOffsets.DBPoolSizes + (4 * 0x16)));
					// Return success
					return true;
				}
			}
			// Reset
			CoDAssets::GameOffsetInfos.clear();
		}

		// Attempt to locate via heuristic searching (tested on MP and SP across 2 updates)
		auto DBAssetsScan = CoDAssets::GameInstance->Scan("4A 8B AC ?? ?? ?? ?? ?? 48 85 ED");
		auto DBPoolSizesScan = CoDAssets::GameInstance->Scan("?? 83 BC ?? ?? ?? ?? ?? 01 7F 48");
		auto StringTableScan = CoDAssets::GameInstance->Scan("48 C1 E0 04 48 03 C1 48 83 C4 28 C3");
		auto PackagesTableScan = CoDAssets::GameInstance->Scan("49 89 6B 18 48 8B CA 49 89 73 E0");

		// Check that we had hits
		if (DBAssetsScan > 0 && StringTableScan > 0 && PackagesTableScan > 0)
		{
			// Load info and verify
			auto GameOffsets = DBGameInfo(
				// Resolve pool ptrs from RCX
				CoDAssets::GameInstance->Read<uint32_t>(DBAssetsScan + 0x4) + BaseAddress,
				// Resolve pool sizes from EDX
				CoDAssets::GameInstance->Read<uint32_t>(DBPoolSizesScan + 0x4) + BaseAddress,
				// Resolve strings from LEA
				CoDAssets::GameInstance->Read<uint32_t>(StringTableScan - 0x4) + (StringTableScan),
				// Resolve packages from LEA
				CoDAssets::GameInstance->Read<uint32_t>(PackagesTableScan - 0x1F) + (PackagesTableScan - 0x1B)
				);

			// In debug, print the info for easy additions later!
#if _DEBUG
			// Format the output
			printf("Heuristic: { 0x%X, 0x%X, 0x%X, 0x%X }\n", (GameOffsets.DBAssetPools - BaseAddress), (GameOffsets.DBPoolSizes - BaseAddress), (GameOffsets.StringTable - BaseAddress), (GameOffsets.ImagePackageTable - BaseAddress));
#endif

			// Read required offsets (XANIM, XMODEL, XIMAGE, SOUND)
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 0x7)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 0xA)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 0x15)));
			CoDAssets::GameOffsetInfos.emplace_back(CoDAssets::GameInstance->Read<uint64_t>(GameOffsets.DBAssetPools + (8 * 0x16)));
			// Verify via first xmodel asset
			auto FirstXModelName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1] + 8));
			// Check
			if (FirstXModelName == "empty_model")
			{
				// Verify string table, otherwise we are all set
				CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.StringTable);
				// Read the first string
				if (!Strings::IsNullOrWhiteSpace(LoadStringEntry(2)))
				{
					// Add ImagePackage offset
					CoDAssets::GameOffsetInfos.emplace_back(GameOffsets.ImagePackageTable);
					// Read and apply sizes
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0x7)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0xA)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0x15)));
					CoDAssets::GamePoolSizes.emplace_back(CoDAssets::GameInstance->Read<uint32_t>(GameOffsets.DBPoolSizes + (4 * 0x16)));
					// Return success
					return true;
				}
			}
		}
	}

	// Failed
	return false;
}

bool GameWorldWar2::LoadAssets()
{
	// Prepare to load game assets, into the AssetPool
	bool NeedsAnims = (SettingsManager::GetSetting("showxanim", "true") == "true");
	bool NeedsModels = (SettingsManager::GetSetting("showxmodel", "true") == "true");
	bool NeedsImages = (SettingsManager::GetSetting("showximage", "false") == "true");
	bool NeedsSounds = (SettingsManager::GetSetting("showxsounds", "false") == "true");

	if (NeedsAnims)
	{
		// Store the placeholder model
		WWIIXAnim PlaceholderAnim;
		// Clear it out
		std::memset(&PlaceholderAnim, 0, sizeof(PlaceholderAnim));
		
		// Parse the XAnim pool
		CoDXPoolParser<uint64_t, WWIIXAnim>((CoDAssets::GameOffsetInfos[0] + 8), CoDAssets::GamePoolSizes[0], [&PlaceholderAnim](WWIIXAnim& Asset, uint64_t& AssetOffset)
		{
			// Validate and load if need be
			auto AnimName = CoDAssets::GameInstance->ReadNullTerminatedString(Asset.NamePtr);

			// Make and add
			auto LoadedAnim = new CoDAnim_t();
			// Set
			LoadedAnim->AssetName = AnimName;
			LoadedAnim->AssetPointer = AssetOffset;
			LoadedAnim->Framerate = Asset.Framerate;
			LoadedAnim->FrameCount = Asset.NumFrames;

			// Check placeholder configuration, "void" is the base xanim
			if (AnimName == "void")
			{
				// Set as placeholder animation
				PlaceholderAnim = Asset;
				LoadedAnim->AssetStatus = WraithAssetStatus::Placeholder;
			}
			else if (Asset.BoneIDsPtr == PlaceholderAnim.BoneIDsPtr && Asset.DataBytePtr == PlaceholderAnim.DataBytePtr && Asset.DataShortPtr == PlaceholderAnim.DataShortPtr && Asset.DataIntPtr == PlaceholderAnim.DataIntPtr && Asset.RandomDataBytePtr == PlaceholderAnim.RandomDataBytePtr && Asset.RandomDataIntPtr == PlaceholderAnim.RandomDataIntPtr && Asset.RandomDataShortPtr == PlaceholderAnim.RandomDataShortPtr && Asset.NotificationsPtr == PlaceholderAnim.NotificationsPtr && Asset.DeltaPartsPtr == PlaceholderAnim.DeltaPartsPtr)
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
		});
	}

	if (NeedsModels)
	{
		// Store the placeholder model
		WWIIXModel PlaceholderModel;
		// Clear it out
		std::memset(&PlaceholderModel, 0, sizeof(PlaceholderModel));

		// Parse the XModel pool
		CoDXPoolParser<uint64_t, WWIIXModelBase>((CoDAssets::GameOffsetInfos[1] + 8), CoDAssets::GamePoolSizes[1], [&PlaceholderModel](WWIIXModelBase& Asset, uint64_t& AssetOffset)
		{
			// Validate and load if need be
			auto ModelName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(Asset.NamePtr));

			// Continue if we only have one part
			if (Asset.XModelPartCount == 0)
			{
				// Validate and load if need be
				auto ModelResult = CoDAssets::GameInstance->Read<WWIIXModel>(Asset.XModelPtr);

				// Make and add
				auto LoadedModel = new CoDModel_t();
				// Set
				LoadedModel->AssetName = ModelName;
				LoadedModel->AssetPointer = AssetOffset;
				LoadedModel->BoneCount = ModelResult.NumBones;
				LoadedModel->LodCount = ModelResult.NumLods;

				// Check placeholder configuration, "empty_model" is the base xmodel swap
				if (ModelName == "empty_model")
				{
					// Set as placeholder model
					PlaceholderModel = ModelResult;
					LoadedModel->AssetStatus = WraithAssetStatus::Placeholder;
				}
				else if ((ModelResult.BoneIDsPtr == PlaceholderModel.BoneIDsPtr && ModelResult.ParentListPtr == PlaceholderModel.ParentListPtr && ModelResult.RotationsPtr == PlaceholderModel.RotationsPtr && ModelResult.TranslationsPtr == PlaceholderModel.TranslationsPtr && ModelResult.PartClassificationPtr == PlaceholderModel.PartClassificationPtr && ModelResult.BaseMatriciesPtr == PlaceholderModel.BaseMatriciesPtr && ModelResult.NumLods == PlaceholderModel.NumLods && ModelResult.NumBones == PlaceholderModel.NumBones) || ModelResult.NumLods == 0)
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
			}
			else
			{
#ifdef _DEBUG
				// Log this
				printf("XModel has multiple parts: %s\n", ModelName.c_str());
#endif
			}
		});
	}

	if (NeedsImages)
	{
		// Parse the XImage pool
		CoDXPoolParser<uint64_t, WWIIGfxImage>((CoDAssets::GameOffsetInfos[2] + 8), CoDAssets::GamePoolSizes[2], [](WWIIGfxImage& Asset, uint64_t& AssetOffset)
		{
			// Load the asset name, we must be valid
			auto ImageName = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(Asset.NamePtr));

			// Check if it's streamed
			if (Asset.LoadedImagePtr == 0 && Asset.Width > 0)
			{
				// Calculate the largest image mip
				uint32_t LargestMip = 0;
				uint32_t LargestWidth = Asset.Width;
				uint32_t LargestHeight = Asset.Height;

				// Loop and calculate
				for (uint32_t i = 0; i < 3; i++)
				{
					// Compare widths
					if (Asset.MipLevels[i].Width > LargestWidth)
					{
						LargestMip = (i + 1);
						LargestWidth = Asset.MipLevels[i].Width;
						LargestHeight = Asset.MipLevels[i].Height;
					}
				}

				// Make and add
				auto LoadedImage = new CoDImage_t();
				// Set
				LoadedImage->AssetName = ImageName;
				LoadedImage->AssetPointer = AssetOffset;
				LoadedImage->Width = (uint16_t)LargestWidth;
				LoadedImage->Height = (uint16_t)LargestHeight;
				LoadedImage->Format = Asset.ImageFormat;
				LoadedImage->AssetStatus = WraithAssetStatus::Loaded;

				// Add
				CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
			}
		});
	}

	if (NeedsSounds)
	{
		// A temporary table for duplicates, since we are tracing from alias entries...
		std::set<uint64_t> UniqueEntries;

		// Parse the Sound pool
		CoDXPoolParser<uint64_t, WWIISoundAlias>((CoDAssets::GameOffsetInfos[3] + 8), CoDAssets::GamePoolSizes[3], [&UniqueEntries](WWIISoundAlias& Asset, uint64_t& AssetOffset)
		{
			// If the table is valid and we have > 1 entry, prepare to parse
			for (uint64_t i = 0; i < Asset.AliasEntryCount; i++)
			{
				// Read the entry header, then read the base
				auto SoundEntryHeader = CoDAssets::GameInstance->Read<WWIISoundEntry>(Asset.AliasEntrysPtr);
				auto SoundFileBase = CoDAssets::GameInstance->Read<WWIISoundFileBase>(SoundEntryHeader.SoundFilePtr);

				// Advance
				SoundEntryHeader.SoundFilePtr += sizeof(WWIISoundFileBase);

				// Skip if doesn't exist
				if (SoundFileBase.SoundFileExists == 0)
					continue;

				// Read based on type, 1 = loaded, 2 = primed, 3 = streamed
				if (SoundFileBase.SoundFileType == 1)
				{
					// Read the loaded sound data
					auto UniqueSoundEntry = CoDAssets::GameInstance->Read<uint64_t>(SoundEntryHeader.SoundFilePtr);

					// Validate uniqueness
					if (UniqueEntries.insert(UniqueSoundEntry).second == false)
						continue;

					// Load the header because it's unique
					auto LoadedSoundHeader = CoDAssets::GameInstance->Read<WWIILoadedSoundFile>(UniqueSoundEntry);

					// Add the loaded entry
					auto SoundName = CoDAssets::GameInstance->ReadNullTerminatedString(LoadedSoundHeader.SoundFileName);

					// Make and add
					auto LoadedSound = new CoDSound_t();
					// Set
					LoadedSound->AssetName = FileSystems::GetFileName(SoundName);
					LoadedSound->FullPath = FileSystems::GetDirectoryName(SoundName);
					LoadedSound->FrameCount = LoadedSoundHeader.FrameCount;
					LoadedSound->FrameRate = LoadedSoundHeader.FrameRate;
					LoadedSound->AssetPointer = LoadedSoundHeader.SoundDataPtr;
					LoadedSound->AssetSize = LoadedSoundHeader.SoundDataSize;
					LoadedSound->ChannelsCount = LoadedSoundHeader.ChannelCount;
					LoadedSound->DataType = (LoadedSoundHeader.Format == 6) ? SoundDataTypes::FLAC_WithHeader : SoundDataTypes::WAV_NeedsHeader;
					LoadedSound->IsFileEntry = false;
					LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
					LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));

					// Add
					CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
				}
				else if (SoundFileBase.SoundFileType == 2)
				{
					// Validate uniqueness
					if (UniqueEntries.insert(SoundEntryHeader.SoundFilePtr).second == false)
						continue;

					// Load the header because it's unique
					auto LoadedSoundHeader = CoDAssets::GameInstance->Read<WWIIPrimedSoundFile>(SoundEntryHeader.SoundFilePtr);

					// Read path and name
					auto SoundPath = CoDAssets::GameInstance->ReadNullTerminatedString(LoadedSoundHeader.SoundFilePath);
					auto SoundName = CoDAssets::GameInstance->ReadNullTerminatedString(LoadedSoundHeader.SoundFileName);

					// Make and add
					auto LoadedSound = new CoDSound_t();
					// Set
					LoadedSound->AssetName = SoundName;
					LoadedSound->FullPath = SoundName;
					LoadedSound->FrameCount = LoadedSoundHeader.FrameCount;
					LoadedSound->FrameRate = LoadedSoundHeader.FrameRate;
					LoadedSound->AssetPointer = LoadedSoundHeader.PackFileOffset;
					LoadedSound->AssetSize = LoadedSoundHeader.PackFileSize;
					LoadedSound->ChannelsCount = LoadedSoundHeader.ChannelCount;
					LoadedSound->DataType = (LoadedSoundHeader.Format == 6) ? SoundDataTypes::FLAC_WithHeader : SoundDataTypes::WAV_NeedsHeader;
					LoadedSound->IsFileEntry = true;
					LoadedSound->IsLocalized = (LoadedSoundHeader.IsLocalized > 0);
					LoadedSound->PackageIndex = LoadedSoundHeader.PackFileIndex;
					LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
					LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));

					// Add
					CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
				}
				else if (SoundFileBase.SoundFileType == 3)
				{
					// Validate uniqueness
					if (UniqueEntries.insert(SoundEntryHeader.SoundFilePtr).second == false)
						continue;

					// Load the header because it's unique
					auto LoadedSoundHeader = CoDAssets::GameInstance->Read<WWIIStreamedSoundFile>(SoundEntryHeader.SoundFilePtr);
					auto ExtendedSoundInfo = CoDAssets::GameInstance->Read<WWIIStreamedSoundInfo>(LoadedSoundHeader.ExtendedInfoPtr);

					// Read path and name
					auto SoundPath = CoDAssets::GameInstance->ReadNullTerminatedString(LoadedSoundHeader.SoundFilePath);
					auto SoundName = CoDAssets::GameInstance->ReadNullTerminatedString(LoadedSoundHeader.SoundFileName);

					// Make and add
					auto LoadedSound = new CoDSound_t();
					// Set
					LoadedSound->AssetName = SoundName;
					LoadedSound->FullPath = SoundPath;
					LoadedSound->FrameCount = ExtendedSoundInfo.FrameCount;
					LoadedSound->FrameRate = ExtendedSoundInfo.FrameRate;
					LoadedSound->AssetPointer = LoadedSoundHeader.PackFileOffset;
					LoadedSound->AssetSize = LoadedSoundHeader.PackFileSize;
					LoadedSound->ChannelsCount = ExtendedSoundInfo.ChannelCount;
					LoadedSound->DataType = (ExtendedSoundInfo.Format == 6) ? SoundDataTypes::FLAC_WithHeader : SoundDataTypes::WAV_NeedsHeader;
					LoadedSound->IsFileEntry = true;
					LoadedSound->IsLocalized = (LoadedSoundHeader.IsLocalized > 0);
					LoadedSound->PackageIndex = LoadedSoundHeader.PackFileIndex;
					LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
					LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));

					// Add
					CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
				}
				else
				{
#if _DEBUG
					// Log on debug
					printf("Unknown sound type: %d\n", SoundFileBase.SoundFileType);
#endif
				}

				// Advance
				Asset.AliasEntrysPtr += sizeof(WWIISoundEntry);
			}
		});
	}

	// Success, error only on specific load
	return true;
}

std::unique_ptr<XModel_t> GameWorldWar2::ReadXModel(const CoDModel_t* Model)
{
	// Verify that the program is running
	if (CoDAssets::GameInstance->IsRunning())
	{
		// Read the XModel structure
		auto ModelHeaderData = CoDAssets::GameInstance->Read<WWIIXModelBase>(Model->AssetPointer);
		auto ModelData = CoDAssets::GameInstance->Read<WWIIXModel>(ModelHeaderData.XModelPtr);

		// Prepare to read the xmodel (Reserving space for lods)
		auto ModelAsset = std::make_unique<XModel_t>(ModelData.NumLods);

		// Copy over default properties
		ModelAsset->ModelName = Model->AssetName;
		// Bone counts
		ModelAsset->BoneCount = ModelData.NumBones;
		ModelAsset->RootBoneCount = ModelData.NumRootBones;

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
				auto SurfaceInfo = CoDAssets::GameInstance->Read<WWIIXModelSurface>(XSurfacePtr);

				// Apply surface info
				SubmeshReference.VertListcount = SurfaceInfo.VertListCount;
				SubmeshReference.RigidWeightsPtr = SurfaceInfo.RigidWeightsPtr;
				SubmeshReference.VertexCount = SurfaceInfo.VertexCount;
				SubmeshReference.FaceCount = SurfaceInfo.FacesCount;

				// Assign stream pointer here for use later since we can't store everything
				SubmeshReference.VertexPtr = SurfaceInfo.XStreamSurfacePtr;

				// Read this submesh's material handle
				auto MaterialHandle = CoDAssets::GameInstance->Read<uint64_t>(ModelHeaderData.MaterialHandlesPtr);
				// Create the material and add it
				LodReference.Materials.emplace_back(ReadXMaterial(MaterialHandle));

				// Advance
				XSurfacePtr += sizeof(WWIIXModelSurface);
				ModelHeaderData.MaterialHandlesPtr += sizeof(uint64_t);
			}
		}

		// Return it
		return ModelAsset;
	}
	// Not running
	return nullptr;
}

std::unique_ptr<XImageDDS> GameWorldWar2::ReadXImage(const CoDImage_t* Image)
{
	// Proxy the image off, determine type if need be
	auto Usage = ImageUsageType::Unknown;
	// Determine from name
	if (Strings::Contains(Image->AssetName, "_nml"))
	{
		// Set to normal map
		Usage = ImageUsageType::NormalMap;
	}
	// Proxy off
	return LoadXImage(XImage_t(Usage, Image->AssetPointer, Image->AssetName));
}

std::unique_ptr<XSound> GameWorldWar2::ReadXSound(const CoDSound_t* Sound)
{
	// Prepare to read the sound data, for WAV, we must include a WAV header...
	auto Result = std::make_unique<XSound>();

	// The offset of which to store the data
	uint32_t DataOffset = 0;

	// Allocate a buffer, if we're WAV, add size of WAV header
	if (Sound->DataType == SoundDataTypes::WAV_NeedsHeader)
	{
		Result->DataBuffer = new int8_t[Sound->AssetSize + Sound::GetMaximumWAVHeaderSize()];
		Result->DataType = SoundDataTypes::WAV_WithHeader;
		Result->DataSize = (uint32_t)(Sound->AssetSize + Sound::GetMaximumWAVHeaderSize());

		DataOffset += Sound::GetMaximumWAVHeaderSize();

		// Make the header
		Sound::WriteWAVHeaderToStream(Result->DataBuffer, (uint32_t)Sound->FrameRate, (uint32_t)Sound->ChannelsCount, (uint32_t)Sound->AssetSize);
	}
	else
	{
		Result->DataBuffer = new int8_t[Sound->AssetSize];
		Result->DataType = SoundDataTypes::FLAC_WithHeader;
		Result->DataSize = (uint32_t)(Sound->AssetSize);
	}

	// Set the format
	if (!Sound->IsFileEntry)
	{
		// Load the buffer from memory and prepare to copy it...
		uintptr_t ResultRead = 0;
		// Read it
		auto TempBuffer = CoDAssets::GameInstance->Read(Sound->AssetPointer, Sound->AssetSize, ResultRead);

		// Prepare to copy
		if (TempBuffer != nullptr)
		{
			// Copy it
			std::memcpy(Result->DataBuffer + DataOffset, TempBuffer, ResultRead);

			// Clean up
			delete[] TempBuffer;
		}
		else
		{
			// Failed, reset
			return nullptr;
		}
	}
	else
	{
		// Grab the base path
		auto PackagePath = CoDAssets::GamePackageCache->GetPackagesPath();

		// If we're not localized, it's just default
		if (!Sound->IsLocalized)
		{
			PackagePath = FileSystems::CombinePath(PackagePath, Strings::Format("soundfile%d.pak", Sound->PackageIndex));
		}
		else
		{
			// Iterate through all known folders
			for (uint32_t i = 0; i < _ARRAYSIZE(LocalizationFolders); i++)
			{
				auto DirectoryCheck = FileSystems::CombinePath(PackagePath, LocalizationFolders[i]);

				// Check for it
				if (FileSystems::DirectoryExists(DirectoryCheck))
				{
					// Set them
					PackagePath = DirectoryCheck;
					PackagePath = FileSystems::CombinePath(PackagePath, Strings::Format("%ssoundfile%d.pak", LocalizationPrefixes[i], Sound->PackageIndex));

					// Found it...
					break;
				}
			}
		}
		
		// If it doesn't exist, fail out
		if (!FileSystems::FileExists(PackagePath))
			return nullptr;

		// Open the file and read the entry
		auto Reader = BinaryReader();
		// Open the file
		Reader.Open(PackagePath);

		// Jump to the offset
		Reader.SetPosition(Sound->AssetPointer);

		// Read the data
		uint64_t ResultRead = 0;
		// Read it to the buffer
		Reader.Read((uint8_t*)Result->DataBuffer + DataOffset, Sound->AssetSize, ResultRead);
	}

	// Return the result
	return Result;
}

std::unique_ptr<XAnim_t> GameWorldWar2::ReadXAnim(const CoDAnim_t* Animation)
{
	// Verify that the program is running
	if (CoDAssets::GameInstance->IsRunning())
	{
		// Prepare to read the xanim
		auto Anim = std::make_unique<XAnim_t>();

		// Read the XAnim structure
		auto AnimData = CoDAssets::GameInstance->Read<WWIIXAnim>(Animation->AssetPointer);

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
		// Check for additive animations
		if (AnimData.AssetType == 0x6)
		{
			// This is a additive animation
			Anim->AdditiveAnimation = true;
		}
		// Check for looping
		Anim->LoopingAnimation = (AnimData.Flags & 1);

		// Read the delta data
		auto AnimDeltaData = CoDAssets::GameInstance->Read<WWIIXAnimDeltaParts>(AnimData.DeltaPartsPtr);

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
		// Inline bone type size (All bone type indicies are ushort now)
		Anim->BoneTypeSize = 2;

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

		// Set types, we use dividebysize for WWII
		Anim->RotationType = AnimationKeyTypes::DivideBySize;
		Anim->TranslationType = AnimationKeyTypes::MinSizeTable;

		// World War II supports inline indicies
		Anim->SupportsInlineIndicies = true;

		// Return it
		return Anim;
	}
	// Not running
	return nullptr;
}

const XMaterial_t GameWorldWar2::ReadXMaterial(uint64_t MaterialPointer)
{
	// Prepare to parse the material
	auto MaterialData = CoDAssets::GameInstance->Read<WWIIXMaterial>(MaterialPointer);

	// Allocate a new material with the given image count
	XMaterial_t Result(MaterialData.ImageCount);
	// Clean the name, then apply it
	Result.MaterialName = FileSystems::GetFileNameWithoutExtension(CoDAssets::GameInstance->ReadNullTerminatedString(MaterialData.NamePtr));

	// Iterate over material images, assign proper references if available
	for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
	{
		// Read the image info
		auto ImageInfo = CoDAssets::GameInstance->Read<WWIIXMaterialImage>(MaterialData.ImageTablePtr);
		// Read the image name (First pointer in image)
		auto ImageName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(ImageInfo.ImagePtr));

		// Default type
		auto DefaultUsage = ImageUsageType::Unknown;
		// Check 
		switch (ImageInfo.Hash)
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
		Result.Images.emplace_back(DefaultUsage, ImageInfo.ImagePtr, ImageName);

		// Advance
		MaterialData.ImageTablePtr += sizeof(WWIIXMaterialImage);
	}

	// Return it
	return Result;
}

void GameWorldWar2::LoadXModel(const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel)
{
	// Check if we want to read vertex colors
	bool ExportColors = (SettingsManager::GetSetting("exportvtxcolor", "true") == "true");

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

		// Readers for all data types, global so that we can use them when streaming...
		MemoryReader VertexPosReader;
		MemoryReader VertexNormReader;
		MemoryReader VertexUVReader;
		MemoryReader VertexColorReader;
		MemoryReader VertexWeightReader;
		MemoryReader FaceIndiciesReader;

		// A pointer to the streamed data, if any
		std::unique_ptr<uint8_t[]> StreamDataBuffer = nullptr;

		// Read the stream submesh
		auto StreamMeshInfo = CoDAssets::GameInstance->Read<WWIIXStreamSurface>(Submesh.VertexPtr);
		auto StreamMeshExtended = CoDAssets::GameInstance->Read<WWIIXStreamSurfaceInfo>(StreamMeshInfo.LoadedSurfaceInfoPtr);

		// Determine if the mesh is loaded or not
		if (StreamMeshInfo.LoadedSurfaceInfoPtr == 0)
		{
			// The mesh is not loaded, prepare to stream it
			// We use a temporary buffer, then set the proper pointers in the stream struct for FAST loading :)
			// Buffer unallocates after loop
			auto Key = Hashing::HashXXHashString(std::string(StreamMeshInfo.SubmeshKeyHash));

			uint32_t ResultSize = 0;
			// Extract the buffer
			StreamDataBuffer = CoDAssets::GamePackageCache->ExtractPackageObject(Key, ResultSize);

			// Ensure success
			if (StreamDataBuffer != nullptr && ResultSize > 0)
			{
				// Prepare the data buffers
				uint32_t CurrentPosition = 0;

				// Faces
				FaceIndiciesReader.Setup((int8_t*)(StreamDataBuffer.get() + CurrentPosition), Submesh.FaceCount * sizeof(uint16_t) * 3, true);
				// Advance and skip padding
				CurrentPosition += (Submesh.FaceCount * sizeof(uint16_t) * 3);
				CurrentPosition = (CurrentPosition + 0x7) & 0xFFFFFFFFFFFFFFF8;

				// Positions
				VertexPosReader.Setup((int8_t*)(StreamDataBuffer.get() + CurrentPosition), Submesh.VertexCount * sizeof(Vector3), true);
				// Advance and skip padding
				CurrentPosition += (Submesh.VertexCount * sizeof(Vector3));
				CurrentPosition = (CurrentPosition + 0x7) & 0xFFFFFFFFFFFFFFF8;

				// Advance and skip tangents w/ padding
				CurrentPosition += (Submesh.VertexCount * sizeof(uint32_t));
				CurrentPosition = (CurrentPosition + 0x7) & 0xFFFFFFFFFFFFFFF8;

				// Colors
				VertexColorReader.Setup((int8_t*)(StreamDataBuffer.get() + CurrentPosition), Submesh.VertexCount * sizeof(uint32_t), true);
				// Advance and skip colors w/ padding
				CurrentPosition += (Submesh.VertexCount * 4);
				CurrentPosition = (CurrentPosition + 0x7) & 0xFFFFFFFFFFFFFFF8;

				// UVs
				VertexUVReader.Setup((int8_t*)(StreamDataBuffer.get() + CurrentPosition), Submesh.VertexCount * sizeof(uint32_t), true);
				// Advance and skip padding
				CurrentPosition += (Submesh.VertexCount * sizeof(uint32_t));
				CurrentPosition = (CurrentPosition + 0x7) & 0xFFFFFFFFFFFFFFF8;

				// Normals
				VertexNormReader.Setup((int8_t*)(StreamDataBuffer.get() + CurrentPosition), Submesh.VertexCount * sizeof(uint32_t), true);
				// Advance and skip padding
				CurrentPosition += (Submesh.VertexCount * sizeof(uint32_t));
				CurrentPosition = (CurrentPosition + 0x7) & 0xFFFFFFFFFFFFFFF8;

				// Advance and skip binormals w/ padding
				CurrentPosition += (Submesh.VertexCount * sizeof(uint32_t));
				CurrentPosition = (CurrentPosition + 0x7) & 0xFFFFFFFFFFFFFFF8;

				// Complex weights, if any
				if (Submesh.VertListcount == 0)
				{
					// Read them, 0x10 per vertex
					VertexWeightReader.Setup((int8_t*)(StreamDataBuffer.get() + CurrentPosition), Submesh.VertexCount * 0x10, true);
					// Advance to the end
					CurrentPosition += (Submesh.VertexCount * 0x10);
				}
			}
			else
			{
				// Skip this mesh, we failed to find it
#if _DEBUG
				printf("Found submesh that isn't loaded: %s\n", std::string(StreamMeshInfo.SubmeshKeyHash).c_str());
#endif
				continue;
			}
		}
		else
		{
			// Load the data from in-memory
			uintptr_t MemoryResult = 0;

			// Positions
			VertexPosReader.Setup(CoDAssets::GameInstance->Read(StreamMeshExtended.VertexPositionsPtr, Submesh.VertexCount * sizeof(Vector3), MemoryResult), MemoryResult);
			// Normals
			VertexNormReader.Setup(CoDAssets::GameInstance->Read(StreamMeshExtended.VertexNormalsPtr, Submesh.VertexCount * sizeof(uint32_t), MemoryResult), MemoryResult);
			// UVs
			VertexUVReader.Setup(CoDAssets::GameInstance->Read(StreamMeshExtended.VertexUVsPtr, Submesh.VertexCount * sizeof(uint32_t), MemoryResult), MemoryResult);
			// Colors
			VertexColorReader.Setup(CoDAssets::GameInstance->Read(StreamMeshExtended.VertexColorsPtr, Submesh.VertexCount * sizeof(uint32_t), MemoryResult), MemoryResult);

			// Complex weights, if any
			if (Submesh.VertListcount == 0)
			{
				// Read them, 0x10 per vertex
				VertexWeightReader.Setup(CoDAssets::GameInstance->Read(StreamMeshExtended.VertexWeightsPtr, Submesh.VertexCount * 0x10, MemoryResult), MemoryResult);
			}

			// Faces
			FaceIndiciesReader.Setup(CoDAssets::GameInstance->Read(StreamMeshExtended.FacesPtr, Submesh.FaceCount * sizeof(uint16_t) * 3, MemoryResult), MemoryResult);
		}

		// Pre-allocate vertex weights (Data defaults to weight 1.0 on bone 0)
		auto VertexWeights = std::vector<WeightsData>(Submesh.VertexCount);

		// Weight index
		uint32_t WeightDataIndex = 0;

		// Prepare the simple, rigid weights
		for (uint32_t i = 0; i < Submesh.VertListcount; i++)
		{
			// Simple weights build, rigid, just apply the proper bone id
			auto RigidInfo = CoDAssets::GameInstance->Read<GfxRigidVerts64>(Submesh.RigidWeightsPtr + (i * sizeof(GfxRigidVerts64)));
			// Apply bone ids properly
			for (uint32_t w = 0; w < RigidInfo.VertexCount; w++)
			{
				// Apply
				VertexWeights[WeightDataIndex].BoneValues[0] = (RigidInfo.BoneIndex / 64);
				// Advance
				WeightDataIndex++;
			}
		}

		// Prepare the complex weights, blends
		if (Submesh.VertListcount == 0)
		{
			// Loop and read them
			for (uint32_t i = 0; i < Submesh.VertexCount; i++)
			{
				// Apply IDs
				VertexWeights[i].BoneValues[0] = VertexWeightReader.Read<uint16_t>();
				VertexWeights[i].BoneValues[1] = VertexWeightReader.Read<uint16_t>();
				VertexWeights[i].BoneValues[2] = VertexWeightReader.Read<uint16_t>();
				VertexWeights[i].BoneValues[3] = VertexWeightReader.Read<uint16_t>();

				// Apply weight values
				VertexWeights[i].WeightValues[1] = VertexWeightReader.Read<uint16_t>() / 65536.0f;
				VertexWeights[i].WeightValues[2] = VertexWeightReader.Read<uint16_t>() / 65536.0f;
				VertexWeights[i].WeightValues[3] = VertexWeightReader.Read<uint16_t>() / 65536.0f;
				// Calculate first value
				VertexWeights[i].WeightValues[0] = (1.0f - (VertexWeights[i].WeightValues[1] + VertexWeights[i].WeightValues[2] + VertexWeights[i].WeightValues[3]));

				// Set weighted bone count
				VertexWeights[i].WeightCount = VertexWeightReader.Read<uint8_t>();
				// Skip 1 byte
				VertexWeightReader.Advance(1);
			}
		}

		// Iterate over verticies
		for (uint32_t i = 0; i < Submesh.VertexCount; i++)
		{
			// Make a new vertex
			auto& Vertex = Mesh.AddVertex();

			// Read and assign position
			Vertex.Position = VertexPosReader.Read<Vector3>();

			// Read and assign normals (Unpack first)
			auto PackedNormal = VertexNormReader.Read<int32_t>();

			// Unpack and set normal
			Vertex.Normal = Vector3(
				(float)((float)((float)(PackedNormal & 0x3FF) / 1023.0) * 2.0) - 1.0f,
				(float)((float)((float)((PackedNormal >> 10) & 0x3FF) / 1023.0) * 2.0) - 1.0f,
				(float)((float)((float)((PackedNormal >> 20) & 0x3FF) / 1023.0) * 2.0) - 1.0f);

			// Set Vertex Colors
			Vertex.Color[0] = ExportColors ? VertexColorReader.Read<uint8_t>() : 0xFF;
			Vertex.Color[1] = ExportColors ? VertexColorReader.Read<uint8_t>() : 0xFF;
			Vertex.Color[2] = ExportColors ? VertexColorReader.Read<uint8_t>() : 0xFF;
			Vertex.Color[3] = ExportColors ? VertexColorReader.Read<uint8_t>() : 0xFF;

			// Read and set UVs
			auto UVU = HalfFloats::ToFloat(VertexUVReader.Read<uint16_t>());
			auto UVV = HalfFloats::ToFloat(VertexUVReader.Read<uint16_t>());

			// Set it
			Vertex.AddUVLayer(UVU, UVV);

			// Assign weights
			auto& WeightValue = VertexWeights[i];

			// Iterate
			for (uint32_t w = 0; w < WeightValue.WeightCount; w++)
			{
				// Add new weight
				Vertex.AddVertexWeight(WeightValue.BoneValues[w], WeightValue.WeightValues[w]);
			}
		}

		// Iterate over faces
		for (uint32_t i = 0; i < Submesh.FaceCount; i++)
		{
			// Read the face
			auto& Face = Mesh.AddFace();

			// Assign indicies
			Face.Index1 = FaceIndiciesReader.Read<uint16_t>();
			Face.Index2 = FaceIndiciesReader.Read<uint16_t>();
			Face.Index3 = FaceIndiciesReader.Read<uint16_t>();
		}
	}
}

std::unique_ptr<XImageDDS> GameWorldWar2::LoadXImage(const XImage_t& Image)
{
	// Prepare to load an image, we only support PAK images
	uint32_t ResultSize = 0;

	// We must read the image data
	auto ImageInfo = CoDAssets::GameInstance->Read<WWIIGfxImage>(Image.ImagePtr);

	// Check if the image isn't streamed, if it isn't, just exit
	if (ImageInfo.LoadedImagePtr > 0 || ImageInfo.Width == 0) { return nullptr; }

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

	// Determine image format
	uint8_t ImageFormat = 71;

	// Determine from struct
	switch (ImageInfo.ImageFormat)
	{
	case 1: ImageFormat = 28; break;	// RGBA
	case 7:
	case 8: ImageFormat = 71; break;	// BC1
	case 9:
	case 10: ImageFormat = 74; break;	// BC2
	case 11:
	case 12: ImageFormat = 77; break;	// BC3
	case 13: ImageFormat = 80; break;	// BC4
	case 15: ImageFormat = 84; break;	// BC5
	case 16: ImageFormat = 95; break;	// BC6
	case 18:
	case 19: ImageFormat = 98; break;	// BC7

	default:
#ifdef _DEBUG
		printf("[WWII] Unknown image format: 0x%X\n", ImageInfo.ImageFormat);
#endif
		break;
	}

	// Calculate table offset of the biggest mip
	uint64_t PAKTableOffset = (((Image.ImagePtr - (CoDAssets::GameOffsetInfos[2] + 8)) / sizeof(WWIIGfxImage)) * (sizeof(WWIIPAKImageEntry) * 4)) + CoDAssets::GameOffsetInfos[5] + (LargestMip * sizeof(WWIIPAKImageEntry));

	// Read info
	auto ImageStreamInfo = CoDAssets::GameInstance->Read<WWIIPAKImageEntry>(PAKTableOffset);

	// Unpack the packed data, offset, size is not needed as it's in-file
	uint64_t ImageOffset = ImageStreamInfo.ImageInfoPacked & 0xFFFFFFFFF;

	// Read image package name
	auto ImagePackageName = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(ImageStreamInfo.ImagePAKInfoPtr) + 0x9);

	// Attempt to extract the package asset
	auto ImageData = PAKSupport::AWExtractImagePackage(FileSystems::CombinePath(CoDAssets::GamePackageCache->GetPackagesPath(), ImagePackageName), ImageOffset, 0, ResultSize);

	// Check
	if (ImageData != nullptr)
	{
		// Prepare to create a MemoryDDS file
		auto Result = CoDRawImageTranslator::TranslateBC(ImageData, ResultSize, LargestWidth, LargestHeight, ImageFormat);

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

std::string GameWorldWar2::LoadStringEntry(uint64_t Index)
{
	// Read and return (Offsets[4] = StringTable)
	return CoDAssets::GameInstance->ReadNullTerminatedString((16 * Index) + CoDAssets::GameOffsetInfos[4] + 8);
}