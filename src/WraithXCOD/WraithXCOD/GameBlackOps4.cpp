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
#include "HalfFloats.h"

// -- Initialize Asset Name Cache

WraithNameIndex GameBlackOps4::AssetNameCache = WraithNameIndex();

// -- Initialize built-in game offsets databases

// Black Ops 4 SP
std::array<DBGameInfo, 1> GameBlackOps4::SinglePlayerOffsets =
{{
	{ 0xA1684D0, 0x0, 0x8E030F0, 0x0 }
}};

// Encryption Map
struct EncryptionMap
{
	uint8_t Keys[255];
};

// Precalculated Encryption Keys
std::map<uint8_t, EncryptionMap> EncryptionKeys =
{
	{
		139,
		{
			165, 164, 162, 165, 161, 164, 158, 165,
			157, 164, 154, 165, 153, 164, 150, 165,
			149, 164, 146, 165, 145, 164, 142, 165,
			141, 164, 138, 165, 137, 164, 134, 165,
			133, 164, 130, 165, 129, 164, 126, 165,
			125, 164, 122, 165, 121, 164, 118, 165,
			117, 164, 114, 165, 113, 164, 110, 165,
			109, 164, 106, 165, 105, 164, 102, 165,
			101, 164,  98, 165,  97, 164,  94, 165,
			 93, 164,  90, 165,  89, 164,  86, 165,
			 85, 164,  82, 165,  81, 164,  78, 165,
			 77, 164,  74, 165,  73, 164,  70, 165,
			 69, 164,  66, 165,  65, 164,  62, 165,
			 61, 164,  58, 165,  57, 164,  54, 165,
			 53, 164,  50, 165,  49, 164,  46, 165,
			 45, 164,  42, 165,  41, 164,  38, 165,
			 37, 164,  34, 165,  33, 164,  30, 165,
			 29, 164,  26, 165,  25, 164,  22, 165,
			 21, 164,  18, 165,  17, 164,  14, 165,
			 13, 164,  10, 165,   9, 164,   6, 165,
			  5, 164,   2, 165,   1, 164, 254, 165,
			253, 164, 250, 165, 249, 164, 246, 165,
			245, 164, 242, 165, 241, 164, 238, 165,
			237, 164, 234, 165, 233, 164, 230, 165,
			229, 164, 226, 165, 225, 164, 222, 165,
			221, 164, 218, 165, 217, 164, 214, 165,
			213, 164, 210, 165, 209, 164, 206, 165,
			205, 164, 202, 165, 201, 164, 198, 165,
			197, 164, 194, 165, 193, 164, 190, 165,
			189, 164, 186, 165, 185, 164, 182, 165,
			181, 164, 178, 165, 177, 164, 174, 165,
			173, 164, 170, 165, 169, 164, 166,
		}
	},
	{ 
		152, 
		{
			189, 190, 192, 195, 199, 204, 210, 217,
			225, 234, 244, 255,  11,  24,  38,  53,
			 69,  86, 104, 123, 143, 164, 186, 209,
			233,   2,  28,  55,  83, 112, 142, 173,
			205, 238,  16,  51,  87, 124, 162, 201,
			241,  26,  68, 111, 155, 200, 246,  37,
			 85, 134, 184, 235,  31,  84, 138, 193,
			249,  50, 108, 167, 227,  32,  94, 157,
			221,  30,  96, 163, 231,  44, 114, 185,
			  1,  74, 148, 223,  43, 120, 198,  21,
			101, 182,   8,  91, 175,   4,  90, 177,
			  9,  98, 188,  23, 115, 208,  46, 141,
			237,  78, 176,  19, 119, 220,  66, 169,
			 17, 122, 228,  79, 187,  40, 150,   5,
			117, 230,  88, 203,  63, 180,  42, 161,
			 25, 146,  12, 135,   3, 128, 254, 125,
			253, 126,   0, 131,   7, 140,  18, 153,
			 33, 170,  52, 191,  75, 216, 102, 245,
			133,  22, 168,  59, 207, 100, 250, 145,
			 41, 194,  92, 247, 147,  48, 206, 109,
			 13, 174,  80, 243, 151,  60, 226, 137,
			 49, 218, 132,  47, 219, 136,  54, 229,
			149,  70, 248, 171,  95,  20, 202, 129,
			 57, 242, 172, 103,  35, 224, 158,  93,
			 29, 222, 160,  99,  39, 236, 178, 121,
			 65,  10, 212, 159, 107,  56,   6, 213,
			165, 118,  72,  27, 239, 196, 154, 113,
			 73,  34, 252, 215, 179, 144, 110,  77,
			 45,  14, 240, 211, 183, 156, 130, 105,
			 81,  58,  36,  15, 251, 232, 214, 197,
			181, 166, 152, 139, 127, 116, 106,  97,
			 89,  82,  76,  71,  67,  64,  62,
		}
	},
	{
		171,
		{
			143, 144, 145, 146, 147, 148, 149, 150,
			151, 152, 153, 154, 155, 156, 157, 158,
			159, 160, 161, 162, 163, 164, 165, 166,
			167, 168, 169, 170, 171, 172, 173, 174,
			175, 176, 177, 178, 179, 180, 181, 182,
			183, 184, 185, 186, 187, 188, 189, 190,
			191, 192, 193, 194, 195, 196, 197, 198,
			199, 200, 201, 202, 203, 204, 205, 206,
			207, 208, 209, 210, 211, 212, 213, 214,
			215, 216, 217, 218, 219, 220, 221, 222,
			223, 224, 225, 226, 227, 228, 229, 230,
			231, 232, 233, 234, 235, 236, 237, 238,
			239, 240, 241, 242, 243, 244, 245, 246,
			247, 248, 249, 250, 251, 252, 253, 254,
			255,   0,   1,   2,   3,   4,   5,   6,
			  7,   8,   9,  10,  11,  12,  13,  14,
			 15,  16,  17,  18,  19,  20,  21,  22,
			 23,  24,  25,  26,  27,  28,  29,  30,
			 31,  32,  33,  34,  35,  36,  37,  38,
			 39,  40,  41,  42,  43,  44,  45,  46,
			 47,  48,  49,  50,  51,  52,  53,  54,
			 55,  56,  57,  58,  59,  60,  61,  62,
			 63,  64,  65,  66,  67,  68,  69,  70,
			 71,  72,  73,  74,  75,  76,  77,  78,
			 79,  80,  81,  82,  83,  84,  85,  86,
			 87,  88,  89,  90,  91,  92,  93,  94,
			 95,  96,  97,  98,  99, 100, 101, 102,
			103, 104, 105, 106, 107, 108, 109, 110,
			111, 112, 113, 114, 115, 116, 117, 118,
			119, 120, 121, 122, 123, 124, 125, 126,
			127, 128, 129, 130, 131, 132, 133, 134,
			135, 136, 137, 138, 139, 140, 141,
		}
	},
	{
		189,
		{
			170, 172, 167, 171, 166, 172, 163, 171,
			162, 172, 159, 171, 158, 172, 155, 171,
			154, 172, 151, 171, 150, 172, 147, 171,
			146, 172, 143, 171, 142, 172, 139, 171,
			138, 172, 135, 171, 134, 172, 131, 171,
			130, 172, 127, 171, 126, 172, 123, 171,
			122, 172, 119, 171, 118, 172, 115, 171,
			114, 172, 111, 171, 110, 172, 107, 171,
			106, 172, 103, 171, 102, 172,  99, 171,
			 98, 172,  95, 171,  94, 172,  91, 171,
			 90, 172,  87, 171,  86, 172,  83, 171,
			 82, 172,  79, 171,  78, 172,  75, 171,
			 74, 172,  71, 171,  70, 172,  67, 171,
			 66, 172,  63, 171,  62, 172,  59, 171,
			 58, 172,  55, 171,  54, 172,  51, 171,
			 50, 172,  47, 171,  46, 172,  43, 171,
			 42, 172,  39, 171,  38, 172,  35, 171,
			 34, 172,  31, 171,  30, 172,  27, 171,
			 26, 172,  23, 171,  22, 172,  19, 171,
			 18, 172,  15, 171,  14, 172,  11, 171,
			 10, 172,   7, 171,   6, 172,   3, 171,
			  2, 172, 255, 171, 254, 172, 251, 171,
			250, 172, 247, 171, 246, 172, 243, 171,
			242, 172, 239, 171, 238, 172, 235, 171,
			234, 172, 231, 171, 230, 172, 227, 171,
			226, 172, 223, 171, 222, 172, 219, 171,
			218, 172, 215, 171, 214, 172, 211, 171,
			210, 172, 207, 171, 206, 172, 203, 171,
			202, 172, 199, 171, 198, 172, 195, 171,
			194, 172, 191, 171, 190, 172, 187, 171,
			186, 172, 183, 171, 182, 172, 179, 171,
			178, 172, 175, 171, 174, 172, 171,
		}
	},
	{
		191,
		{
			153, 154, 156, 159, 163, 168, 174, 181,
			189, 198, 208, 219, 231, 244,   2,  17,
			 33,  50,  68,  87, 107, 128, 150, 173,
			197, 222, 248,  19,  47,  76, 106, 137,
			169, 202, 236,  15,  51,  88, 126, 165,
			205, 246,  32,  75, 119, 164, 210,   1,
			 49,  98, 148, 199, 251,  48, 102, 157,
			213,  14,  72, 131, 191, 252,  58, 121,
			185, 250,  60, 127, 195,   8,  78, 149,
			221,  38, 112, 187,   7,  84, 162, 241,
			 65, 146, 228,  55, 139, 224,  54, 141,
			229,  62, 152, 243,  79, 172,  10, 105,
			201,  42, 140, 239,  83, 184,  30, 133,
			237,  86, 192,  43, 151,   4, 114, 225,
			 81, 194,  52, 167,  27, 144,   6, 125,
			245, 110, 232,  99, 223,  92, 218,  89,
			217,  90, 220,  95, 227, 104, 238, 117,
			253, 134,  16, 155,  39, 180,  66, 209,
			 97, 242, 132,  23, 171,  64, 214, 109,
			  5, 158,  56, 211, 111,  12, 170,  73,
			233, 138,  44, 207, 115,  24, 190, 101,
			 13, 182,  96,  11, 183, 100,  18, 193,
			113,  34, 212, 135,  59, 240, 166,  93,
			 21, 206, 136,  67, 255, 188, 122,  57,
			249, 186, 124,  63,   3, 200, 142,  85,
			 29, 230, 176, 123,  71,  20, 226, 177,
			129,  82,  36, 247, 203, 160, 118,  77,
			 37, 254, 216, 179, 143, 108,  74,  41,
			  9, 234, 204, 175, 147, 120,  94,  69,
			 45,  22,   0, 235, 215, 196, 178, 161,
			145, 130, 116, 103,  91,  80,  70,  61,
			 53,  46,  40,  35,  31,  28,  26,
		}
	}
};

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
			auto FirstXModelHash = CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameOffsetInfos[1]);
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
		auto StringTableScan = CoDAssets::GameInstance->Scan("48 8B 53 ?? 48 85 D2 74 ?? 48 8B 03 48 89 02");

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

	/*
		This was implemented as a fix for a specific user who requested it, as the search box is capped at 32767 by Windows
		and this is a workaround, if you're interested in using it, any hashes in this filters file will be ignored on load,
		essentially acting as an excluder, consider it a hidden feature with no support as it was made for a specific use
		case. If you cannot get it to work, do not ask me.
	*/
	auto Filters = WraithNameIndex();
	Filters.LoadIndex("package_index\\bo4_filters.wni");

	// Check if we need assets
	if (NeedsAnims)
	{
		// Animations are the first offset and first pool
		auto AnimationOffset = CoDAssets::GameOffsetInfos[0];
		auto AnimationCount = CoDAssets::GamePoolSizes[0];

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

			// Mask the name (some bits are used for other stuffs)
			AnimResult.UnknownHash &= 0xFFFFFFFFFFFFFFF;

			// Check for filters
			if (Filters.NameDatabase.size() > 0)
			{
				// Check for this asset in DB
				if (Filters.NameDatabase.find(AnimResult.UnknownHash) != Filters.NameDatabase.end())
				{
					// Advance
					AnimationOffset += sizeof(BO4XAnim);
					// Skip this asset
					continue;
				}
			}

			// Validate and load if need be
			auto AnimName = Strings::Format("xanim_%llx", AnimResult.UnknownHash);
			// Check for an override in the name DB
			if (AssetNameCache.NameDatabase.find(AnimResult.UnknownHash) != AssetNameCache.NameDatabase.end())
				AnimName = AssetNameCache.NameDatabase[AnimResult.UnknownHash];

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

			// Mask the name (some bits are used for other stuffs)
			ModelResult.NamePtr &= 0xFFFFFFFFFFFFFFF;

			// Check for filters
			if (Filters.NameDatabase.size() > 0)
			{
				// Check for this asset in DB
				if (Filters.NameDatabase.find(ModelResult.NamePtr) != Filters.NameDatabase.end())
				{
					// Advance
					ModelOffset += sizeof(BO4XModel);
					// Skip this asset
					continue;
				}
			}

			// Validate and load if need be
			auto ModelName = Strings::Format("xmodel_%llx", ModelResult.NamePtr);

			// Check for an override in the name DB
			if (AssetNameCache.NameDatabase.find(ModelResult.NamePtr) != AssetNameCache.NameDatabase.end())
				ModelName = AssetNameCache.NameDatabase[ModelResult.NamePtr];

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

			// Mask the name (some bits are used for other stuffs)
			ImageResult.UnknownHash &= 0xFFFFFFFFFFFFFFF;

			// Check for filters
			if (Filters.NameDatabase.size() > 0)
			{
				// Check for this asset in DB
				if (Filters.NameDatabase.find(ImageResult.UnknownHash) != Filters.NameDatabase.end())
				{
					// Advance
					ImageOffset += sizeof(BO4GfxImage);
					// Skip this asset
					continue;
				}
			}

			// Validate and load if need be
			auto ImageName = Strings::Format("ximage_%llx", ImageResult.UnknownHash);
			// Check for an override in the name DB
			if (AssetNameCache.NameDatabase.find(ImageResult.UnknownHash) != AssetNameCache.NameDatabase.end())
				ImageName = AssetNameCache.NameDatabase[ImageResult.UnknownHash];

			// Check for loaded images
			if (ImageResult.GfxMipsPtr != 0)
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

		// Check for viewmodel animations
		if ((_strnicmp(Animation->AssetName.c_str(), "viewmodel_", 10) == 0) || (_strnicmp(Animation->AssetName.c_str(), "vm_", 3) == 0))
		{
			// This is a viewmodel animation
			Anim->ViewModelAnimation = true;
		}
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

	// Mask the name (some bits are used for other stuffs)
	MaterialData.Hash &= 0xFFFFFFFFFFFFFFF;
	// Allocate a new material with the given image count
	XMaterial_t Result(MaterialData.ImageCount);
	// Clean the name, then apply it
	Result.MaterialName = Strings::Format("xmaterial_%llx", MaterialData.Hash);

	// Check for an override in the name DB
	if (AssetNameCache.NameDatabase.find(MaterialData.Hash) != AssetNameCache.NameDatabase.end())
		Result.MaterialName = AssetNameCache.NameDatabase[MaterialData.Hash];

	// Iterate over material images, assign proper references if available
	for (uint32_t m = 0; m < MaterialData.ImageCount; m++)
	{
		// Read the image info
		auto ImageInfo = CoDAssets::GameInstance->Read<BO4XMaterialImage>(MaterialData.ImageTablePtr);

		// Get Hash and mask it (some bits are used for other stuffs)
		auto ImageHash = CoDAssets::GameInstance->Read<uint64_t>(ImageInfo.ImagePtr + 0x28) & 0xFFFFFFFFFFFFFFF;

		// Get the image name
		auto ImageName = Strings::Format("ximage_%llx", ImageHash);

		// Check for an override in the name DB
		if (AssetNameCache.NameDatabase.find(ImageHash) != AssetNameCache.NameDatabase.end())
			ImageName = AssetNameCache.NameDatabase[ImageHash];

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
	for (uint32_t i = 0; i < ImageInfo.GfxMipMaps; i++)
	{
		// Load Mip Map
		auto MipMap = CoDAssets::GameInstance->Read<BO4GfxMip>(ImageInfo.GfxMipsPtr);
		// Compare widths
		if (MipMap.Width > LargestWidth)
		{
			LargestMip = i;
			LargestWidth = MipMap.Width;
			LargestHeight = MipMap.Height;
			LargestHash = MipMap.HashID;
		}
		// Advance Mip Map Pointer
		ImageInfo.GfxMipsPtr += sizeof(BO4GfxMip);
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

std::string GameBlackOps4::LoadStringEntry(uint64_t Index)
{
	// Check if we have an index to use
	if (Index > 0)
	{
		// Calculate Offset to String
		auto Offset = CoDAssets::GameOffsetInfos[3] + ((Index * 16) >> 2) + (Index * 16);
		// Read and return (Offsets[3] = StringTable)
		uint64_t BytesRead = 0;
		// XOR Key to decrypt the string if necessary
		auto XORKey = CoDAssets::GameInstance->Read<uint8_t>(Offset + 16);
		// String Size (includes terminating null character, -1 for just the string)
		auto StringSize = CoDAssets::GameInstance->Read<uint8_t>(Offset + 17) - 1;
		// Resulting String
		auto Result = CoDAssets::GameInstance->Read(Offset + 18, StringSize, BytesRead);
		// Check for key in encryption map
		if (EncryptionKeys.find(XORKey) != EncryptionKeys.end())
		{
			// Get map
			auto Map = EncryptionKeys[XORKey];
			// Loop through string
			for (uint8_t i = 0; i < StringSize; i++)
			{
				// Decrpyt it if the key and input differ
				Result[i] = Result[i] != Map.Keys[i] ? Result[i] ^ Map.Keys[i] : Result[i];
			}
		}
		else
		{
			// These are weird, results change, some patterns can be seen, need more time to analyze the exe...
			// We appear to "reset" in some way every 8 bytes, there might be shifting by index up to 8 or some shit
			switch (XORKey)
			{
			// For now, read their hash
			case 185:
			case 165:
			case 153:
			case 143:
				return Strings::Format("xstring_%llx", CoDAssets::GameInstance->Read<uint64_t>(Offset + 8));
			}
		}
		// Convert to string and return
		return std::string(reinterpret_cast<char const*>(Result), StringSize);
	}
	// Return blank string
	return "";
}
void GameBlackOps4::PerformInitialSetup()
{
	// Load Caches
	AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo4_xanim.wni"));
	AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo4_ximage.wni"));
	AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo4_xmaterial.wni"));
	AssetNameCache.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo4_xmodel.wni"));

	// Prepare to copy the oodle dll
	auto OurPath = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "oo2core_6_win64.dll");

	// Copy if not exists
	if (!FileSystems::FileExists(OurPath))
		FileSystems::CopyFile(FileSystems::CombinePath(FileSystems::GetDirectoryName(CoDAssets::GameInstance->GetProcessPath()), "oo2core_6_win64.dll"), OurPath);
}