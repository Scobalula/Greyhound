#include "stdafx.h"

// The class we are implementing
#include "CoDAssets.h"

// We need the following classes
#include "ProcessReader.h"
#include "Systems.h"
#include "FileSystems.h"
#include "Strings.h"
#include "Image.h"
#include "Sound.h"
#include "BinaryReader.h"

// We need the main window for callbacks
#include "MainWindow.h"

// We need settings
#include "SettingsManager.h"

// We need the CoDTranslators
#include "CoDXAnimTranslator.h"
#include "CoDXModelTranslator.h"
#include "CoDEffectTranslator.h"
#include "CoDRawfileTranslator.h"
#include "CoDXConverter.h"

// We need the game support functions
#include "GameWorldAtWar.h"
#include "GameBlackOps.h"
#include "GameBlackOps2.h"
#include "GameBlackOps3.h"
#include "GameBlackOps4.h"
#include "GameModernWarfare.h"
#include "GameModernWarfare2.h"
#include "GameModernWarfare3.h"
#include "GameGhosts.h"
#include "GameAdvancedWarfare.h"
#include "GameModernWarfareRM.h"
#include "GameInfiniteWarfare.h"
#include "GameWorldWar2.h"

// We need the game cache functions
#include "IWDCache.h"
#include "IPAKCache.h"
#include "PAKCache.h"
#include "XPAKCache.h"
#include "SABCache.h"
#include "XPTOCCache.h"

// We need the game support functions
#include "PAKSupport.h"
#include "XPAKSupport.h"
#include "IPAKSupport.h"
#include "IWDSupport.h"
#include "SABSupport.h"

// We need the export formats
#include "SEAnimExport.h"
#include "SEModelExport.h"
#include "MayaExport.h"
#include "XMEExport.h"
#include "XNALaraExport.h"
#include "ValveSMDExport.h"
#include "OBJExport.h"
#include "XAnimRawExport.h"

// TODO: Use image usage/semantic hashes to determine image types instead when reading from XMaterials

// -- Setup global variables

// Set the default game instant pointer
std::unique_ptr<ProcessReader> CoDAssets::GameInstance = nullptr;

// Set the default game id
SupportedGames CoDAssets::GameID = SupportedGames::None;
// Set the default flags
SupportedGameFlags CoDAssets::GameFlags = SupportedGameFlags::None;

// Set game offsets
std::vector<uint64_t> CoDAssets::GameOffsetInfos = std::vector<uint64_t>();
// Set game sizes
std::vector<uint32_t> CoDAssets::GamePoolSizes = std::vector<uint32_t>();

// Set reader
std::unique_ptr<CoDGDTProcessor> CoDAssets::GameGDTProcessor = std::make_unique<CoDGDTProcessor>();

// Set loaded assets
std::unique_ptr<AssetPool> CoDAssets::GameAssets = nullptr;
// Set cache
std::unique_ptr<CoDPackageCache> CoDAssets::GamePackageCache = nullptr;

// Set the image read handler
LoadXImageHandler CoDAssets::GameXImageHandler = nullptr;
// Set the string read handler
LoadStringHandler CoDAssets::GameStringHandler = nullptr;

// Setup the cod mutex
std::mutex CoDAssets::CodMutex;

// Setup counts
std::atomic<uint32_t> CoDAssets::ExportedAssetsCount;
std::atomic<uint32_t> CoDAssets::AssetsToExportCount;
std::atomic<bool> CoDAssets::CanExportContinue;

// Setup export callbacks
ExportProgressHandler CoDAssets::OnExportProgress = nullptr;
ExportStatusHandler CoDAssets::OnExportStatus = nullptr;

// Set last path
std::string CoDAssets::LatestExportPath = "";

// -- Find game information

const std::vector<CoDGameProcess> CoDAssets::GameProcessInfo =
{
	// World at War
	{ "codwaw.exe", SupportedGames::WorldAtWar, SupportedGameFlags::SP },
	{ "codwawmp.exe", SupportedGames::WorldAtWar, SupportedGameFlags::MP },
	// Black Ops
	{ "blackops.exe", SupportedGames::BlackOps, SupportedGameFlags::SP },
	{ "blackopsmp.exe", SupportedGames::BlackOps, SupportedGameFlags::MP },
	// Black Ops 2
	{ "t6zm.exe", SupportedGames::BlackOps2, SupportedGameFlags::ZM },
	{ "t6mp.exe", SupportedGames::BlackOps2, SupportedGameFlags::MP },
	{ "t6sp.exe", SupportedGames::BlackOps2, SupportedGameFlags::SP },
	// Black Ops 3
	{ "blackops3.exe", SupportedGames::BlackOps3, SupportedGameFlags::SP },
	// Black Ops 4
	{ "blackops4.exe", SupportedGames::BlackOps4, SupportedGameFlags::SP },
	// Modern Warfare
	{ "iw3sp.exe", SupportedGames::ModernWarfare, SupportedGameFlags::SP },
	{ "iw3mp.exe", SupportedGames::ModernWarfare, SupportedGameFlags::MP },
	// Modern Warfare 2
	{ "iw4sp.exe", SupportedGames::ModernWarfare2, SupportedGameFlags::SP },
	{ "iw4mp.exe", SupportedGames::ModernWarfare2, SupportedGameFlags::MP },
	// Modern Warfare 3
	{ "iw5sp.exe", SupportedGames::ModernWarfare3, SupportedGameFlags::SP },
	{ "iw5mp.exe", SupportedGames::ModernWarfare3, SupportedGameFlags::MP },
	// Ghosts
	{ "iw6sp64_ship.exe", SupportedGames::Ghosts, SupportedGameFlags::SP },
	{ "iw6mp64_ship.exe", SupportedGames::Ghosts, SupportedGameFlags::MP },
	// Advanced Warfare
	{ "s1_sp64_ship.exe", SupportedGames::AdvancedWarfare, SupportedGameFlags::SP },
	{ "s1_mp64_ship.exe", SupportedGames::AdvancedWarfare, SupportedGameFlags::MP },
	// Modern Warfare Remastered
	{ "h1_sp64_ship.exe", SupportedGames::ModernWarfareRemastered, SupportedGameFlags::SP },
	{ "h1_mp64_ship.exe", SupportedGames::ModernWarfareRemastered, SupportedGameFlags::MP },
	// Infinite Warfare
	{ "iw7_ship.exe", SupportedGames::InfiniteWarfare, SupportedGameFlags::SP },
	// World War II
	{ "s2_sp64_ship.exe", SupportedGames::WorldWar2, SupportedGameFlags::SP },
	{ "s2_mp64_ship.exe", SupportedGames::WorldWar2, SupportedGameFlags::MP }
};

// -- End find game database

FindGameResult CoDAssets::BeginGameMode()
{
	// Aquire a lock
	std::lock_guard<std::mutex> Lock(CodMutex);

	// Result
	auto Result = FindGameResult::Success;

	// Check if we have a game
	if (GameInstance == nullptr || !GameInstance->IsRunning())
	{
		// Load the game
		Result = FindGame();
	}

	// If success, load assets
	if (Result == FindGameResult::Success)
	{
		// Load assets
		LoadGame();
	}
	else if (GameInstance != nullptr)
	{
		// Close out the game (Failed somehow)
		GameInstance.reset();
	}

	// Success unless failed
	return Result;
}

LoadGameFileResult CoDAssets::BeginGameFileMode(const std::string& FilePath)
{
	// Force clean up first, so we don't store redundant assets
	CoDAssets::CleanUpGame();

	// Aquire a lock
	std::lock_guard<std::mutex> Lock(CodMutex);

	// Result from load file
	return LoadFile(FilePath);
}

FindGameResult CoDAssets::FindGame()
{
	// Attempt to locate one of the supported games
	auto Processes = Systems::GetProcesses();

	// Reset it
	GameInstance.reset();
	// Clear out existing offsets
	GameOffsetInfos.clear();
	// Clear out existing sizes
	GamePoolSizes.clear();

	// Loop and check
	for (auto& Process : Processes)
	{
		// Loop over game process info
		for (auto& GameInfo : GameProcessInfo)
		{
			// Compare name
			if (_stricmp(Process.ProcessName.c_str(), GameInfo.ProcessName) == 0)
			{
				// Make a new game instance
				GameInstance = std::make_unique<ProcessReader>();
				// Attempt to load
				if (GameInstance->Attach(Process.ProcessID))
				{
					// Loaded set it up
					GameID = GameInfo.GameID;
					GameFlags = GameInfo.GameFlags;
					// Attempt to locate game offsets
					if (LocateGameInfo())
					{
						// Success
						return FindGameResult::Success;
					}
					else
					{
						// Failed to locate
						return FindGameResult::FailedToLocateInfo;
					}
				}
			}
		}
	}

	// Reset
	if (GameInstance != nullptr)
	{
		// Clean up
		GameInstance.reset();
	}

	// Failed
	return FindGameResult::NoGamesRunning;
}

LoadGameResult CoDAssets::LoadGame()
{
	// Make sure the process is running
	if (GameInstance->IsRunning())
	{
		// Setup the assets
		GameAssets.reset(new AssetPool());
		// Whether or not we loaded assets
		bool Success = false;

		// Load assets from the game
		switch (GameID)
		{
		case SupportedGames::WorldAtWar: Success = GameWorldAtWar::LoadAssets(); break;
		case SupportedGames::BlackOps: Success = GameBlackOps::LoadAssets(); break;
		case SupportedGames::BlackOps2: Success = GameBlackOps2::LoadAssets(); break;
		case SupportedGames::BlackOps3: Success = GameBlackOps3::LoadAssets(); break;
		case SupportedGames::BlackOps4: Success = GameBlackOps4::LoadAssets(); break;
		case SupportedGames::ModernWarfare: Success = GameModernWarfare::LoadAssets(); break;
		case SupportedGames::ModernWarfare2: Success = GameModernWarfare2::LoadAssets(); break;
		case SupportedGames::ModernWarfare3: Success = GameModernWarfare3::LoadAssets(); break;
		case SupportedGames::Ghosts: Success = GameGhosts::LoadAssets(); break;
		case SupportedGames::AdvancedWarfare: Success = GameAdvancedWarfare::LoadAssets(); break;
		case SupportedGames::ModernWarfareRemastered: Success = GameModernWarfareRM::LoadAssets(); break;
		case SupportedGames::InfiniteWarfare: Success = GameInfiniteWarfare::LoadAssets(); break;
		case SupportedGames::WorldWar2: Success = GameWorldWar2::LoadAssets(); break;
		}

		// Result check
		if (Success)
		{
			// Sort the assets
			std::stable_sort(GameAssets->LoadedAssets.begin(), GameAssets->LoadedAssets.end(), SortAssets);

			// Success
			return LoadGameResult::Success;
		}
		else
		{
			// Failed to load
			return LoadGameResult::NoAssetsFound;
		}
	}

	// Reset
	if (GameAssets != nullptr)
	{
		// Clean up
		GameAssets.reset();
	}

	// Failed
	return LoadGameResult::ProcessNotRunning;
}

LoadGameFileResult CoDAssets::LoadFile(const std::string& FilePath)
{
	// Setup the assets
	GameAssets.reset(new AssetPool());

	// Load result
	auto LoadResult = false;

	// Determine based on file extension first
	auto FileExt = Strings::ToLower(FileSystems::GetExtension(FilePath));

	// Check known extensions
	if (FileExt == ".xpak")
	{
		// Pass off to XPAK File Parser (And Cache the File)
		LoadResult = XPAKSupport::ParseXPAK(FilePath);

		// Cache if success
		if (LoadResult)
		{
			// Allocate a new XPAK Cache
			GamePackageCache = std::make_unique<XPAKCache>();
			// Cache package entries for this specific file
			GamePackageCache->LoadPackageAsync(FilePath);

			// Set game mode
			GameFlags = SupportedGameFlags::Files;
		}
	}
	else if (FileExt == ".ipak")
	{
		// Pass off to IPAK File Parser
		LoadResult = IPAKSupport::ParseIPAK(FilePath);

		// Cache if success
		if (LoadResult)
		{
			// Allocate a new IPAK Cache
			GamePackageCache = std::make_unique<IPAKCache>();
			// Cache package entries for this specific file
			GamePackageCache->LoadPackageAsync(FilePath);

			// Set game mode
			GameID = SupportedGames::BlackOps2;
			GameFlags = SupportedGameFlags::Files;
		}
	}
	else if (FileExt == ".iwd")
	{
		// Pass off to IWD File Parser
		LoadResult = IWDSupport::ParseIWD(FilePath);

		// Cache if success
		if (LoadResult)
		{
			// Allocate a new IWD Cache
			GamePackageCache = std::make_unique<IWDCache>();
			// Cache package entries for this specific file
			GamePackageCache->LoadPackageAsync(FilePath);

			// Set game mode (Determine export path from the file's path)
			auto GamePath = Strings::ToLower(FileSystems::GetDirectoryName(FilePath));

			// Compare
			if (Strings::Contains(GamePath, "black ops"))
			{
				GameID = SupportedGames::BlackOps;
			}
			else if (Strings::Contains(GamePath, "call of duty 4"))
			{
				GameID = SupportedGames::ModernWarfare;
			}
			else if (Strings::Contains(GamePath, "modern warfare 2"))
			{
				GameID = SupportedGames::ModernWarfare2;
			}
			else if (Strings::Contains(GamePath, "modern warfare 3"))
			{
				GameID = SupportedGames::ModernWarfare3;
			}
			else
			{
				// Default to WAW path
				GameID = SupportedGames::WorldAtWar;
			}	

			// Set file mode
			GameFlags = SupportedGameFlags::Files;
		}
	}
	else if (FileExt == ".sabs" || FileExt == ".sabl")
	{
		// Pass off to SAB File Parser
		LoadResult = SABSupport::ParseSAB(FilePath);
		
		// Cache if success
		if (LoadResult)
		{
			// Allocate a new SAB Cache
			GamePackageCache = std::make_unique<SABCache>();
			// Cache file path for faster extraction
			GamePackageCache->LoadPackageCacheAsync(FilePath);

			// Set file mode
			GameFlags = SupportedGameFlags::Files;
		}
	}
	else
	{
		// Unknown
		return LoadGameFileResult::UnknownError;
	}

	// Check result, then sort
	if (LoadResult)
	{
		// Sort the assets
		std::stable_sort(GameAssets->LoadedAssets.begin(), GameAssets->LoadedAssets.end(), SortAssets);
		// Success
		return LoadGameFileResult::Success;
	}

	// Reset
	if (GameAssets != nullptr)
	{
		// Clean up
		GameAssets.reset();
	}

	// Return it
	return LoadGameFileResult::InvalidFile;
}

bool CoDAssets::SortAssets(const CoDAsset_t* lhs, const CoDAsset_t* rhs)
{
	// Sort by type
	if (lhs->AssetType < rhs->AssetType) return true;
	if (rhs->AssetType < lhs->AssetType) return false;

	// Check if we want to sort this by asset data and that they have same asset type
	if (lhs->AssetType == rhs->AssetType && SettingsManager::GetSetting("sortbydetails", "true") == "true")
	{
		// Sory by specific counts
		switch (lhs->AssetType)
		{
		case WraithAssetType::Model:
		{
			// Models
			auto Compare1 = (CoDModel_t*)lhs;
			auto Compare2 = (CoDModel_t*)rhs;

			// Sort by Bone Counts
			if (Compare1->BoneCount < Compare2->BoneCount) return false;
			if (Compare2->BoneCount < Compare1->BoneCount) return true;

			// Done
			break;
		}
		case WraithAssetType::Animation:
		{
			// Animations
			auto Compare1 = (CoDAnim_t*)lhs;
			auto Compare2 = (CoDAnim_t*)rhs;

			// Sort by Frame Counts
			if (Compare1->FrameCount < Compare2->FrameCount) return false;
			if (Compare2->FrameCount < Compare1->FrameCount) return true;

			// Done
			break;
		}
		case WraithAssetType::Image:
		{
			// Images
			auto Compare1 = (CoDImage_t*)lhs;
			auto Compare2 = (CoDImage_t*)rhs;

			// Pixel Counts
			auto PixelCount1 = Compare1->Width * Compare1->Height;
			auto PixelCount2 = Compare2->Width * Compare2->Height;

			// Sort by Pixel Counts
			if (PixelCount1 < PixelCount2) return false;
			if (PixelCount2 < PixelCount1) return true;

			// Done
			break;
		}
		case WraithAssetType::RawFile:
		{
			// Rawfiles
			auto Compare1 = (CoDRawFile_t*)lhs;
			auto Compare2 = (CoDRawFile_t*)rhs;

			// Sort by pure Size
			if (Compare1->AssetSize < Compare2->AssetSize) return false;
			if (Compare2->AssetSize < Compare1->AssetSize) return true;

			// Done
			break;
		}
		case WraithAssetType::Sound:
		{
			// Sounds
			auto Compare1 = (CoDSound_t*)lhs;
			auto Compare2 = (CoDSound_t*)rhs;

			// Sort by duration
			if (Compare1->Length < Compare2->Length) return false;
			if (Compare2->Length < Compare1->Length) return true;

			// Done
			break;
		}
		case WraithAssetType::Effect:
		{
			// Effects
			auto Compare1 = (CoDEffect_t*)lhs;
			auto Compare2 = (CoDEffect_t*)rhs;

			// Sort by Elements
			if (Compare1->ElementCount < Compare2->ElementCount) return false;
			if (Compare2->ElementCount < Compare1->ElementCount) return true;

			// Done
			break;
		}
		default:
			// Default, match by name
			break;
		}
	}

	// Sort by name
	if (lhs->AssetName < rhs->AssetName) return true;
	if (rhs->AssetName < lhs->AssetName) return false;

	// Should basically never happen
	return false;
}

ExportGameResult CoDAssets::ExportAsset(const CoDAsset_t* Asset)
{
	// Prepare to export the asset
	auto ExportPath = BuildExportPath(Asset);
	// Create it, if not exists
	FileSystems::CreateDirectory(ExportPath);

	// Build image export path
	auto ImagesPath = (SettingsManager::GetSetting("global_images", "true") == "true") ? FileSystems::CombinePath(FileSystems::GetDirectoryName(ExportPath), "_images") : FileSystems::CombinePath(ExportPath, "_images");
	// Build images path
	auto ImageRelativePath = (SettingsManager::GetSetting("global_images", "true") == "true") ? "..\\\\_images\\\\" : "_images\\\\";
	// Build image ext
	auto ImageExtension = "." + Strings::ToLower(SettingsManager::GetSetting("exportimg"));
	// Build sound ext
	auto SoundExtension = "." + Strings::ToLower(SettingsManager::GetSetting("exportsnd"));

	// Result
	auto Result = ExportGameResult::Success;

	// Send to specific export handler
	switch (Asset->AssetType)
	{
		// Export an animation
	case WraithAssetType::Animation: Result = ExportAnimationAsset((CoDAnim_t*)Asset, ExportPath); break;
		// Export a model, combine the name of the model with the export path!
	case WraithAssetType::Model: Result = ExportModelAsset((CoDModel_t*)Asset, ExportPath, ImagesPath, ImageRelativePath, ImageExtension); break;
		// Export an image
	case WraithAssetType::Image: Result = ExportImageAsset((CoDImage_t*)Asset, ExportPath, ImageExtension); break;
		// Export a sound
	case WraithAssetType::Sound: Result = ExportSoundAsset((CoDSound_t*)Asset, ExportPath, SoundExtension); break;
		// Export a effect
	case WraithAssetType::Effect: Result = ExportEffectAsset((CoDEffect_t*)Asset, ExportPath); break;
		// Export a rawfile
	case WraithAssetType::RawFile: Result = ExportRawfileAsset((CoDRawFile_t*)Asset, ExportPath); break;
	}

	// Success, unless specific error
	return Result;
}

std::unique_ptr<WraithModel> CoDAssets::GetModelForPreview(const CoDModel_t* Model)
{
	// Attempt to load the model
	auto GenericModel = CoDAssets::LoadGenericModelAsset(Model);

	// If loaded, continue
	if (GenericModel != nullptr)
		return CoDXModelTranslator::TranslateXModel(GenericModel, CoDXModelTranslator::CalculateBiggestLodIndex(GenericModel));

	// Failed somehow
	return nullptr;
}

bool CoDAssets::LocateGameInfo()
{
	// Whether or not we found what we need
	bool Success = false;

	// GDTShorthand
	auto GDTShorthand = "WAW";

	// Attempt to find the loaded game's offsets, either via DB or heuristics
	// Also, apply proper handlers for various game read functions (Non-inlinable functions only)
	switch (GameID)
	{
	case SupportedGames::WorldAtWar:
		// Load game offset info
		Success = GameWorldAtWar::LoadOffsets();
		// Set shorthand
		GDTShorthand = "WAW";
		// Set game ximage handler
		GameXImageHandler = GameWorldAtWar::LoadXImage;
		// Set game string handler
		GameStringHandler = GameWorldAtWar::LoadStringEntry;
		// Allocate a new IWD Mega Cache
		GamePackageCache = std::make_unique<IWDCache>();
		// Set the IWD path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::CombinePath(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()), "main"));
		break;
	case SupportedGames::BlackOps:
		// Load game offset info
		Success = GameBlackOps::LoadOffsets();
		// Set shorthand
		GDTShorthand = "BO1";
		// Set game ximage handler
		GameXImageHandler = GameBlackOps::LoadXImage;
		// Set game string handler
		GameStringHandler = GameBlackOps::LoadStringEntry;
		// Allocate a new IWD Mega Cache
		GamePackageCache = std::make_unique<IWDCache>();
		// Set the IWD path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::CombinePath(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()), "main"));
		break;
	case SupportedGames::BlackOps2:
		// Load game offset info
		Success = GameBlackOps2::LoadOffsets();
		// Set shorthand
		GDTShorthand = "BO2";
		// Set game ximage handler
		GameXImageHandler = GameBlackOps2::LoadXImage;
		// Set game string handler
		GameStringHandler = GameBlackOps2::LoadStringEntry;
		// Allocate a new IPAK Mega Cache
		GamePackageCache = std::make_unique<IPAKCache>();
		// Set the IPAK path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::CombinePath(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()), "zone\\all"));
		break;
	case SupportedGames::BlackOps3:
		// Load game offset info
		Success = GameBlackOps3::LoadOffsets();
		// Set shorthand
		GDTShorthand = "BO3";
		// Set game ximage handler
		GameXImageHandler = GameBlackOps3::LoadXImage;
		// Set game string handler
		GameStringHandler = GameBlackOps3::LoadStringEntry;
		// Allocate a new XPAK Mega Cache
		GamePackageCache = std::make_unique<XPAKCache>();
		// Set the XPAK path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::CombinePath(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()), "zone"));
		break;
	case SupportedGames::BlackOps4:
	{
		// Get Zone Folder
		auto ZoneFolder = FileSystems::CombinePath(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()), "zone");
		// Check if it exists, if it doesn't, fall back to if it exists in our dir
		if (!FileSystems::DirectoryExists(ZoneFolder))
			// Set it
#if _DEBUG
			ZoneFolder = "E:\\Tools\\Greyhound\\Default\\zone";
#else
			ZoneFolder = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "zone");
#endif

		// Check if it exists
		if (!FileSystems::DirectoryExists(ZoneFolder))
			MessageBoxA(NULL, "Greyhound failed to find the Zone folder. Information on extracting the Zone folder is available on the Github Repo. Models and Images will not export correctly until you do this.", "Greyhound", MB_OK | MB_ICONWARNING);
		// Initial setup required for BO4
		GameBlackOps4::PerformInitialSetup();
		// Load game offset info
		Success = GameBlackOps4::LoadOffsets();
		// Set shorthand
		GDTShorthand = "BO4";
		// Set game ximage handler
		GameXImageHandler = GameBlackOps4::LoadXImage;
		// Set game string handler
		GameStringHandler = GameBlackOps4::LoadStringEntry;
		// Allocate a new XPAK Mega Cache
		GamePackageCache = std::make_unique<XPAKCache>();
		// Set the XPAK path
		GamePackageCache->LoadPackageCacheAsync(ZoneFolder);
		break;
	}
	case SupportedGames::ModernWarfare:
		// Load game offset info
		Success = GameModernWarfare::LoadOffsets();
		// Set shorthand
		GDTShorthand = "MW";
		// Set game ximage handler
		GameXImageHandler = GameModernWarfare::LoadXImage;
		// Set game string handler
		GameStringHandler = GameModernWarfare::LoadStringEntry;
		// Allocate a new IWD Mega Cache
		GamePackageCache = std::make_unique<IWDCache>();
		// Set the IWD path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::CombinePath(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()), "main"));
		break;
	case SupportedGames::ModernWarfare2:
		// Load game offset info
		Success = GameModernWarfare2::LoadOffsets();
		// Set shorthand
		GDTShorthand = "MW2";
		// Set game ximage handler
		GameXImageHandler = GameModernWarfare2::LoadXImage;
		// Set game string handler
		GameStringHandler = GameModernWarfare2::LoadStringEntry;
		// Allocate a new IWD Mega Cache
		GamePackageCache = std::make_unique<IWDCache>();
		// Set the IWD path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::CombinePath(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()), "main"));
		break;
	case SupportedGames::ModernWarfare3:
		// Load game offset info
		Success = GameModernWarfare3::LoadOffsets();
		// Set shorthand
		GDTShorthand = "MW3";
		// Set game ximage handler
		GameXImageHandler = GameModernWarfare3::LoadXImage;
		// Set game string handler
		GameStringHandler = GameModernWarfare3::LoadStringEntry;
		// Allocate a new IWD Mega Cache
		GamePackageCache = std::make_unique<IWDCache>();
		// Set the IWD path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::CombinePath(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()), "main"));
		break;
	case SupportedGames::Ghosts:
		// Load game offset info
		Success = GameGhosts::LoadOffsets();
		// Set shorthand
		GDTShorthand = "Ghosts";
		// Set game ximage handler
		GameXImageHandler = GameGhosts::LoadXImage;
		// Set game string handler
		GameStringHandler = GameGhosts::LoadStringEntry;
		// Allocate a new PAK Mega Cache
		GamePackageCache = std::make_unique<PAKCache>();
		// Set the PAK path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()));
		break;
	case SupportedGames::AdvancedWarfare:
		// Load game offset info
		Success = GameAdvancedWarfare::LoadOffsets();
		// Set shorthand
		GDTShorthand = "AW";
		// Set game ximage handler
		GameXImageHandler = GameAdvancedWarfare::LoadXImage;
		// Set game string handler
		GameStringHandler = GameAdvancedWarfare::LoadStringEntry;
		// Allocate a new PAK Mega Cache
		GamePackageCache = std::make_unique<PAKCache>();
		// Set the PAK path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()));
		break;
	case SupportedGames::ModernWarfareRemastered:
		// Load game offset info
		Success = GameModernWarfareRM::LoadOffsets();
		// Set shorthand
		GDTShorthand = "MWR";
		// Set game ximage handler
		GameXImageHandler = GameModernWarfareRM::LoadXImage;
		// Set game string handler
		GameStringHandler = GameModernWarfareRM::LoadStringEntry;
		// Allocate a new PAK Mega Cache
		GamePackageCache = std::make_unique<PAKCache>();
		// Set the PAK path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()));
		break;
	case SupportedGames::InfiniteWarfare:
		// Load game offset info
		Success = GameInfiniteWarfare::LoadOffsets();
		// Set shorthand
		GDTShorthand = "IW";
		// Set game ximage handler
		GameXImageHandler = GameInfiniteWarfare::LoadXImage;
		// Set game string handler
		GameStringHandler = GameInfiniteWarfare::LoadStringEntry;
		// Allocate a new PAK Mega Cache
		GamePackageCache = std::make_unique<PAKCache>();
		// Set the PAK path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()));
		break;
	case SupportedGames::WorldWar2:
		// Load game offset info
		Success = GameWorldWar2::LoadOffsets();
		// Set shorthand
		GDTShorthand = "WWII";
		// Set game ximage handler
		GameXImageHandler = GameWorldWar2::LoadXImage;
		// Set game string handler
		GameStringHandler = GameWorldWar2::LoadStringEntry;
		// Allocate a new PAK Mega Cache
		GamePackageCache = std::make_unique<XPTOCCache>();
		// Set the PAK path
		GamePackageCache->LoadPackageCacheAsync(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()));
		break;
	}

	// Setup the game's cache
	GameGDTProcessor->SetupProcessor(GDTShorthand);

	// Validate the results, every game should have at least 1 offset and 1 size, and success must be true
	if (Success && (GameOffsetInfos.size() > 0 && GamePoolSizes.size() > 0))
	{
		// We succeeded
		return true;
	}

	// We failed to load
	return false;
}

std::string CoDAssets::BuildExportPath(const CoDAsset_t* Asset)
{
	// Build the export path
	auto ApplicationPath = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "exported_files");

	// Append the game directory
	switch (GameID)
	{
	case SupportedGames::WorldAtWar: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "world_at_war"); break;
	case SupportedGames::BlackOps: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "black_ops_1"); break;
	case SupportedGames::BlackOps2: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "black_ops_2"); break;
	case SupportedGames::BlackOps3: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "black_ops_3"); break;
	case SupportedGames::BlackOps4: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "black_ops_4"); break;
	case SupportedGames::ModernWarfare: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare"); break;
	case SupportedGames::ModernWarfare2: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare_2"); break;
	case SupportedGames::ModernWarfare3: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare_3"); break;
	case SupportedGames::Ghosts: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "ghosts"); break;
	case SupportedGames::AdvancedWarfare: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "advanced_warfare"); break;
	case SupportedGames::ModernWarfareRemastered: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare_rm"); break;
	case SupportedGames::InfiniteWarfare: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "infinite_warfare"); break;
	case SupportedGames::WorldWar2: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "world_war_2"); break;
	}

	// Append the asset type folder (Some assets have specific folder names)
	switch (Asset->AssetType)
	{
	case WraithAssetType::Animation:
		// Default directory
		ApplicationPath = FileSystems::CombinePath(ApplicationPath, "xanims");
		break;
	case WraithAssetType::Model:
		// Directory with asset name
		ApplicationPath = FileSystems::CombinePath(FileSystems::CombinePath(ApplicationPath, "xmodels"), Asset->AssetName);
		break;
	case WraithAssetType::Image:
		// Default directory
		ApplicationPath = FileSystems::CombinePath(ApplicationPath, "ximages");
		break;
	case WraithAssetType::Sound:
		// Default directory, OR, Merged with path, check setting
		ApplicationPath = FileSystems::CombinePath(ApplicationPath, "sounds");
		// Check setting for merged paths
		if (SettingsManager::GetSetting("keepsndpath", "true") == "true")
		{
			// Merge it
			ApplicationPath = FileSystems::CombinePath(ApplicationPath, ((CoDSound_t*)Asset)->FullPath);
		}
		break;
	case WraithAssetType::Effect:
		// Default directory
		ApplicationPath = FileSystems::CombinePath(ApplicationPath, "fxt");
		break;
	case WraithAssetType::RawFile:
		// Default directory
		ApplicationPath = FileSystems::CombinePath(ApplicationPath, "xrawfiles");
		break;
	}

	// Return it
	return ApplicationPath;
}

std::unique_ptr<XAnim_t> CoDAssets::LoadGenericAnimAsset(const CoDAnim_t* Animation)
{
	// Read from game
	switch (CoDAssets::GameID)
	{
	case SupportedGames::WorldAtWar: return GameWorldAtWar::ReadXAnim(Animation); break;
	case SupportedGames::BlackOps: return GameBlackOps::ReadXAnim(Animation); break;
	case SupportedGames::BlackOps2: return GameBlackOps2::ReadXAnim(Animation); break;
	case SupportedGames::BlackOps3: return GameBlackOps3::ReadXAnim(Animation); break;
	case SupportedGames::BlackOps4: return GameBlackOps4::ReadXAnim(Animation); break;
	case SupportedGames::ModernWarfare: return GameModernWarfare::ReadXAnim(Animation); break;
	case SupportedGames::ModernWarfare2: return GameModernWarfare2::ReadXAnim(Animation); break;
	case SupportedGames::ModernWarfare3: return GameModernWarfare3::ReadXAnim(Animation); break;
	case SupportedGames::Ghosts: return GameGhosts::ReadXAnim(Animation); break;
	case SupportedGames::AdvancedWarfare: return GameAdvancedWarfare::ReadXAnim(Animation); break;
	case SupportedGames::ModernWarfareRemastered: return GameModernWarfareRM::ReadXAnim(Animation); break;
	case SupportedGames::InfiniteWarfare: return GameInfiniteWarfare::ReadXAnim(Animation); break;
	case SupportedGames::WorldWar2: return GameWorldWar2::ReadXAnim(Animation); break;
	}

	// Unknown game
	return nullptr;
}

ExportGameResult CoDAssets::ExportAnimationAsset(const CoDAnim_t* Animation, const std::string& ExportPath)
{
	// Quit if we should not export this model (files already exist)
	if (!ShouldExportAnim(FileSystems::CombinePath(ExportPath, Animation->AssetName)))
		return ExportGameResult::Success;

	// Prepare to export the animation
	std::unique_ptr<XAnim_t> GenericAnimation = CoDAssets::LoadGenericAnimAsset(Animation);

	// Check
	if (GenericAnimation != nullptr)
	{
		// Translate generic animation to a WraithAnim, then export
		auto Result = CoDXAnimTranslator::TranslateXAnim(GenericAnimation);

		// Check result and export
		if (Result != nullptr)
		{
			// Prepare to export to the formats specified in settings

			// Check for DirectXAnim format
			if (SettingsManager::GetSetting("export_directxanim") == "true")
			{
				// Determine export file version
				auto XAnimVer = (SettingsManager::GetSetting("directxanim_ver") == "17") ? XAnimRawVersion::WorldAtWar : XAnimRawVersion::BlackOps;
				// Export a XAnim Raw
				XAnimRaw::ExportXAnimRaw(*Result.get(), FileSystems::CombinePath(ExportPath, Result->AssetName), XAnimVer);
			}

			// The following formats are scaled
			Result->ScaleAnimation(2.54f);

			// Check for SEAnim format
			if (SettingsManager::GetSetting("export_seanim") == "true")
			{
				// Export a SEAnim
				SEAnim::ExportSEAnim(*Result.get(), FileSystems::CombinePath(ExportPath, Result->AssetName + ".seanim"));
			}
		}
		else
		{
			// We failed
			return ExportGameResult::UnknownError;
		}
	}
	else
	{
		// We failed
		return ExportGameResult::UnknownError;
	}

	// Success, unless specific error
	return ExportGameResult::Success;
}

std::unique_ptr<XModel_t> CoDAssets::LoadGenericModelAsset(const CoDModel_t* Model)
{
	// Read from game
	switch (CoDAssets::GameID)
	{
	case SupportedGames::WorldAtWar: return GameWorldAtWar::ReadXModel(Model); break;
	case SupportedGames::BlackOps: return GameBlackOps::ReadXModel(Model); break;
	case SupportedGames::BlackOps2: return GameBlackOps2::ReadXModel(Model); break;
	case SupportedGames::BlackOps3: return GameBlackOps3::ReadXModel(Model); break;
	case SupportedGames::BlackOps4: return GameBlackOps4::ReadXModel(Model); break;
	case SupportedGames::ModernWarfare: return GameModernWarfare::ReadXModel(Model); break;
	case SupportedGames::ModernWarfare2: return GameModernWarfare2::ReadXModel(Model); break;
	case SupportedGames::ModernWarfare3: return GameModernWarfare3::ReadXModel(Model); break;
	case SupportedGames::Ghosts: return GameGhosts::ReadXModel(Model); break;
	case SupportedGames::AdvancedWarfare: return GameAdvancedWarfare::ReadXModel(Model); break;
	case SupportedGames::ModernWarfareRemastered: return GameModernWarfareRM::ReadXModel(Model); break;
	case SupportedGames::InfiniteWarfare: return GameInfiniteWarfare::ReadXModel(Model); break;
	case SupportedGames::WorldWar2: return GameWorldWar2::ReadXModel(Model); break;
	}

	// Unknown game
	return nullptr;
}

bool CoDAssets::ShouldExportAnim(std::string ExportPath)
{
	// Check if we want to skip previous anims
	auto SkipPrevAnims = SettingsManager::GetSetting("skipprevanim") == "true";

	// If we don't want to skip previously exported anims, then we will continue
	if (!SkipPrevAnims)
		return true;

	// Initialize result
	bool Result = false;

	// Check it
	if (SettingsManager::GetSetting("export_directxanim") == "true" && !FileSystems::FileExists(ExportPath))
		Result = true;
	// Check it
	if (SettingsManager::GetSetting("export_seanim") == "true" && !FileSystems::FileExists(ExportPath + ".seanim"))
		Result = true;

	// Done
	return Result;
}

bool CoDAssets::ShouldExportModel(std::string ExportPath)
{
	// Check if we want to skip previous models
	auto SkipPrevModels = SettingsManager::GetSetting("skipprevmodel") == "true";

	// If we don't want to skip previously exported models, then we will continue
	if (!SkipPrevModels)
		return true;

	// Initialize result
	bool Result = false;

	// Check it
	if (SettingsManager::GetSetting("export_xmexport") == "true" && !FileSystems::FileExists(ExportPath + ".XMODEL_EXPORT"))
		Result = true;
	// Check it
	if (SettingsManager::GetSetting("export_smd") == "true" && !FileSystems::FileExists(ExportPath + ".smd"))
		Result = true;
	// Check it
	if (SettingsManager::GetSetting("export_obj") == "true" && !FileSystems::FileExists(ExportPath + ".obj"))
		Result = true;
	// Check it
	if (SettingsManager::GetSetting("export_ma") == "true" && !FileSystems::FileExists(ExportPath + ".ma"))
		Result = true;
	// Check it
	if (SettingsManager::GetSetting("export_xna") == "true" && !FileSystems::FileExists(ExportPath + ".mesh.ascii"))
		Result = true;
	// Check it
	if (SettingsManager::GetSetting("export_semodel") == "true" && !FileSystems::FileExists(ExportPath + ".semodel"))
		Result = true;

	// Done
	return Result;
}

ExportGameResult CoDAssets::ExportModelAsset(const CoDModel_t* Model, const std::string& ExportPath, const std::string& ImagesPath, const std::string& ImageRelativePath, const std::string& ImageExtension)
{
	// Prepare to export the model
	std::unique_ptr<XModel_t> GenericModel = CoDAssets::LoadGenericModelAsset(Model);

	// Grab the image format type
	auto ImageFormatType = ImageFormat::Standard_PNG;
	// Check setting
	auto ImageSetting = SettingsManager::GetSetting("exportimg", "PNG");
	// Check if we even need images
	auto ExportImages = SettingsManager::GetSetting("exportmodelimg") == "true";
	// Check if we want image names
	auto ExportImageNames = SettingsManager::GetSetting("exportimgnames") == "true";

	// Create if not exists
	FileSystems::CreateDirectory(ImagesPath);

	// Check it
	if (ImageSetting == "DDS")
	{
		ImageFormatType = ImageFormat::DDS_WithHeader;
	}
	else if (ImageSetting == "TGA")
	{
		ImageFormatType = ImageFormat::Standard_TGA;
	}
	else if (ImageSetting == "TIFF")
	{
		ImageFormatType = ImageFormat::Standard_TIFF;
	}

	// Check
	if (GenericModel != nullptr)
	{
		// Prepare material images
		for (auto& LOD : GenericModel->ModelLods)
		{
			// Iterate over all materials for the lod
			for (auto& Material : LOD.Materials)
			{
				// Export image names if needed
				if (ExportImageNames)
				{
					// Process Image Names
					ExportMaterialImageNames(Material, ExportPath);
				}
				if (ExportImages)
				{
					// Process the material
					ExportMaterialImages(Material, ImagesPath, ImageExtension, ImageFormatType);
				}

				// Apply image paths
				for (auto& Image : Material.Images)
				{
					// Append the relative path and image extension here, since we are done with these images
					Image.ImageName = ImageRelativePath + Image.ImageName + ImageExtension;
				}
			}
		}

		// Determine lod export type
		if (SettingsManager::GetSetting("exportalllods") == "true")
		{
			// We should export all loaded lods from this xmodel
			auto LodCount = (uint32_t)GenericModel->ModelLods.size();

			// Iterate and convert
			for (uint32_t i = 0; i < LodCount; i++)
			{
				// Continue if we should not export this model (files already exist)
				if (ShouldExportModel(FileSystems::CombinePath(ExportPath, Model->AssetName + Strings::Format("_LOD%d", i))))
				{
					// Translate generic model to a WraithModel, then export
					auto Result = CoDXModelTranslator::TranslateXModel(GenericModel, i);

					// Apply lod name
					Result->AssetName += Strings::Format("_LOD%d", i);

					// Check result and export
					if (Result != nullptr)
					{
						// Send off to exporter
						ExportWraithModel(Result, ExportPath);
					}
					else
					{
						// We failed
						return ExportGameResult::UnknownError;
					}
				}
			}
		}
		else
		{
			// We should export the biggest we can find
			auto BiggestLodIndex = CoDXModelTranslator::CalculateBiggestLodIndex(GenericModel);

			// If the biggest > -1 translate
			if (BiggestLodIndex > -1)
			{
				// Check if we should not export this model (files already exist)
				if (ShouldExportModel(FileSystems::CombinePath(ExportPath, Model->AssetName + "_LOD0")))
				{
					// Translate generic model to a WraithModel, then export
					auto Result = CoDXModelTranslator::TranslateXModel(GenericModel, BiggestLodIndex);

					// Apply lod name (_LOD0)
					Result->AssetName += "_LOD0";

					// Check result and export
					if (Result != nullptr)
					{
						// Send off to exporter
						ExportWraithModel(Result, ExportPath);
					}
					else
					{
						// We failed
						return ExportGameResult::UnknownError;
					}
				}
			}
			else
			{
				// Failed, no loaded lods (Should be marked placeholder anyways)
				return ExportGameResult::UnknownError;
			}
		}

		// Check whether or not to export the hitbox model
		if (SettingsManager::GetSetting("exporthitbox") == "true")
		{
			// The hitbox result, if any
			std::unique_ptr<WraithModel> Result = nullptr;
			// Check the game
			switch (CoDAssets::GameID)
			{
			case SupportedGames::BlackOps: Result = CoDXModelTranslator::TranslateXModelHitbox(GenericModel); break;
			}

			// Export if we have a reslt
			if (Result != nullptr)
			{
				// Export it
				Result->AssetName += "_HITBOX";
				ExportWraithModel(Result, ExportPath);
			}
		}
	}
	else
	{
		// We failed
		return ExportGameResult::UnknownError;
	}

	// Success, unless specific error
	return ExportGameResult::Success;
}

ExportGameResult CoDAssets::ExportImageAsset(const CoDImage_t* Image, const std::string& ExportPath, const std::string& ImageExtension)
{
	// Grab the full image path, if it doesn't exist convert it!
	auto FullImagePath = FileSystems::CombinePath(ExportPath, Image->AssetName + ImageExtension);
	// Check if we want to skip previous images
	auto SkipPrevImages = SettingsManager::GetSetting("skipprevimg") == "true";

	// Check if it exists and if we want to skip it or not
	if (!FileSystems::FileExists(FullImagePath) || !SkipPrevImages)
	{
		// Buffer for the image (Loaded via the global game handler)
		std::unique_ptr<XImageDDS> ImageData = nullptr;

		// Check what mode we're in
		if (Image->IsFileEntry)
		{
			// Read from specific handler (By game)
			switch (CoDAssets::GameID)
			{
			case SupportedGames::WorldAtWar:
			case SupportedGames::ModernWarfare:
			case SupportedGames::ModernWarfare2:
			case SupportedGames::ModernWarfare3:
			case SupportedGames::BlackOps:
				ImageData = IWDSupport::ReadImageFile(Image);
				break;
				
			case SupportedGames::BlackOps2: 
				ImageData = IPAKSupport::ReadImageFile(Image);
				break;

			case SupportedGames::BlackOps3: 
			case SupportedGames::BlackOps4:
				ImageData = XPAKSupport::ReadImageFile(Image);
				break;
			}
		}
		else
		{
			// Read from game
			switch (CoDAssets::GameID)
			{
			case SupportedGames::BlackOps3: ImageData = GameBlackOps3::ReadXImage(Image); break;
			case SupportedGames::BlackOps4: ImageData = GameBlackOps4::ReadXImage(Image); break;
			case SupportedGames::Ghosts: ImageData = GameGhosts::ReadXImage(Image); break;
			case SupportedGames::AdvancedWarfare: ImageData = GameAdvancedWarfare::ReadXImage(Image); break;
			case SupportedGames::ModernWarfareRemastered: ImageData = GameModernWarfareRM::ReadXImage(Image); break;
			case SupportedGames::InfiniteWarfare: ImageData = GameInfiniteWarfare::ReadXImage(Image); break;
			case SupportedGames::WorldWar2: ImageData = GameWorldWar2::ReadXImage(Image); break;
			}
		}

		// Grab the image format type
		auto ImageFormatType = ImageFormat::Standard_PNG;
		// Check setting
		auto ImageSetting = SettingsManager::GetSetting("exportimg", "PNG");

		// Check it
		if (ImageSetting == "DDS")
		{
			ImageFormatType = ImageFormat::DDS_WithHeader;
		}
		else if (ImageSetting == "TGA")
		{
			ImageFormatType = ImageFormat::Standard_TGA;
		}
		else if (ImageSetting == "TIFF")
		{
			ImageFormatType = ImageFormat::Standard_TIFF;
		}

		// Check if we got it
		if (ImageData != nullptr)
		{
			// Convert it to a file or just write the DDS data raw
			if (ImageFormatType == ImageFormat::DDS_WithHeader)
			{
				// Since this can throw, wrap it in an exception handler
				try
				{
					// Just write the buffer
					auto Writer = BinaryWriter();
					// Make the file
					Writer.Create(FullImagePath);
					// Write the DDS buffer
					Writer.Write((const int8_t*)ImageData->DataBuffer, ImageData->DataSize);
				}
				catch (...)
				{
					// Nothing, this means that something is already accessing the image
				}
			}
			else
			{
				// Convert it, this method is a nothrow
				Image::ConvertImageMemory(ImageData->DataBuffer, ImageData->DataSize, ImageFormat::DDS_WithHeader, FullImagePath, ImageFormatType, ImageData->ImagePatchType);
			}
		}
	}

	// Success, unless specific error
	return ExportGameResult::Success;
}

ExportGameResult CoDAssets::ExportSoundAsset(const CoDSound_t* Sound, const std::string& ExportPath, const std::string& SoundExtension)
{
	// Grab the full sound path, if it doesn't exist convert it!
	auto FullSoundPath = FileSystems::CombinePath(ExportPath, Sound->AssetName + SoundExtension);
	// Check if we want to skip previous Sounds
	auto SkipPrevSound = SettingsManager::GetSetting("skipprevsound") == "true";

	// Check if it exists
	if (!FileSystems::FileExists(FullSoundPath) || !SkipPrevSound)
	{
		// Holds universal sound data
		std::unique_ptr<XSound> SoundData = nullptr;

		// Attempt to load it based on game
		switch (CoDAssets::GameID)
		{
		case SupportedGames::BlackOps2:
		case SupportedGames::BlackOps3:
		case SupportedGames::BlackOps4:
		case SupportedGames::InfiniteWarfare:
			SoundData = SABSupport::LoadSound(Sound);
			break;
		case SupportedGames::Ghosts:
		case SupportedGames::AdvancedWarfare:
		case SupportedGames::ModernWarfareRemastered:
		case SupportedGames::WorldWar2:
			SoundData = GameWorldWar2::ReadXSound(Sound);
			break;
		}

		// Grab the image format type
		auto SoundFormatType = SoundFormat::Standard_WAV;
		// Check setting
		auto SoundSetting = SettingsManager::GetSetting("exportsnd", "WAV");

		// Check it
		if (SoundSetting == "FLAC")
		{
			SoundFormatType = SoundFormat::Standard_FLAC;
		}

		// Check if we got it
		if (SoundData != nullptr)
		{
			// Check what format the DATA is, and see if we need to transcode
			if ((SoundData->DataType == SoundDataTypes::FLAC_WithHeader && SoundFormatType == SoundFormat::Standard_FLAC) || SoundData->DataType == SoundDataTypes::WAV_WithHeader && SoundFormatType == SoundFormat::Standard_WAV)
			{
				// We have an already-prepared sound buffer, just write it
				// Since this can throw, wrap it in an exception handler
				try
				{
					// Just write the buffer
					auto Writer = BinaryWriter();
					// Make the file
					Writer.Create(FullSoundPath);
					// Write the Sound buffer
					Writer.Write((const int8_t*)SoundData->DataBuffer, SoundData->DataSize);
				}
				catch (...)
				{
					// Nothing, this means that something is already accessing the sound
				}
			}
			else
			{
				// We must convert it
				auto InFormat = (SoundData->DataType == SoundDataTypes::FLAC_WithHeader) ? SoundFormat::FLAC_WithHeader : SoundFormat::WAV_WithHeader;
				// Convert the asset
				Sound::ConvertSoundMemory(SoundData->DataBuffer, SoundData->DataSize, InFormat, FullSoundPath, SoundFormatType);
			}
		}
	}

	// Success, unless specific error
	return ExportGameResult::Success;
}

ExportGameResult CoDAssets::ExportEffectAsset(const CoDEffect_t* Effect, const std::string& ExportPath)
{
	// Read from specific handler (By game)
	switch (CoDAssets::GameID)
	{
	case SupportedGames::BlackOps:
	case SupportedGames::BlackOps2:
	case SupportedGames::BlackOps3:
		// Send to generic translator
		CoDEffectTranslator::TranslateEffect(Effect, ExportPath);
		break;
	}

	// Success, unless specific error
	return ExportGameResult::Success;
}

ExportGameResult CoDAssets::ExportRawfileAsset(const CoDRawFile_t* Rawfile, const std::string& ExportPath)
{
	// Read from specific handler (By game)
	switch (CoDAssets::GameID)
	{
	case SupportedGames::BlackOps:
		// Send to generic translator, Black Ops does not compress the anim trees, but does compress the GSCs
		CoDRawfileTranslator::TranslateRawfile(Rawfile, ExportPath, false, true);
		break;
	case SupportedGames::BlackOps2:
	case SupportedGames::BlackOps3:
	case SupportedGames::BlackOps4:
		// Send to generic translator, these games compress the anim trees
		CoDRawfileTranslator::TranslateRawfile(Rawfile, ExportPath, true, false);
		break;
	}

	// Success, unless specific error
	return ExportGameResult::Success;
}

void CoDAssets::ExportWraithModel(const std::unique_ptr<WraithModel>& Model, const std::string& ExportPath)
{
	// Prepare to export to the formats specified in settings

	// Check for XME format
	if (SettingsManager::GetSetting("export_xmexport") == "true")
	{
		// Export a XME file
		CodXME::ExportXME(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".XMODEL_EXPORT"));
	}
	// Check for SMD format
	if (SettingsManager::GetSetting("export_smd") == "true")
	{
		// Export a SMD file
		ValveSMD::ExportSMD(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".smd"));
	}

	// The following formats are scaled
	Model->ScaleModel(2.54f);

	// Check for Obj format
	if (SettingsManager::GetSetting("export_obj") == "true")
	{
		// Export a Obj file
		WavefrontOBJ::ExportOBJ(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".obj"));
	}
	// Check for Maya format
	if (SettingsManager::GetSetting("export_ma") == "true")
	{
		// Export a Maya file
		Maya::ExportMaya(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".ma"));
	}
	// Check for XNALara format
	if (SettingsManager::GetSetting("export_xna") == "true")
	{
		// Export a XNALara file
		XNALara::ExportXNA(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".mesh.ascii"));
	}
	// Check for SEModel format
	if (SettingsManager::GetSetting("export_semodel") == "true")
	{
		// Export a SEModel file
		SEModel::ExportSEModel(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".semodel"));
	}

	// Prepare GDT info
	CoDAssets::GameGDTProcessor->ProcessModelGDT(Model);
}

void CoDAssets::CleanupPackageCache()
{
	// Check if even loaded
	if (GamePackageCache != nullptr)
	{
		// Wait until load completes
		GamePackageCache->WaitForPackageCacheLoad();

		// Clean up
		GamePackageCache.reset();
	}
}

void CoDAssets::CleanUpGame()
{
	// Aquire a lock
	std::lock_guard<std::mutex> Lock(CodMutex);

	// Prepare to clean up game resources
	CleanupPackageCache();

	// Clean up assets cache
	if (GameAssets != nullptr)
	{
		GameAssets.reset();
	}

	// Clean up offsets
	GameOffsetInfos.clear();
	GamePoolSizes.clear();

	// Clean up Bo4 Asset Cache
	GameBlackOps4::AssetNameCache.NameDatabase.clear();

	// Set load handler
	GameXImageHandler = nullptr;

	// Set flags
	GameFlags = SupportedGameFlags::None;
	GameID = SupportedGames::None;

	// Clean up game instance
	if (GameInstance != nullptr)
	{
		GameInstance.reset();
	}
}

void CoDAssets::ExportMaterialImageNames(const XMaterial_t& Material, const std::string& ExportPath)
{
	// Try write the image name
	try
	{
		// Image Names Output
		TextWriter ImageNames;

		// Get File Name
		auto ImageNamesPath = FileSystems::CombinePath(ExportPath, Material.MaterialName + "_images.txt");

		// Create File
		ImageNames.Create(ImageNamesPath);

		// Write each name
		for (auto& Image : Material.Images)
			ImageNames.WriteLineFmt("%s", Image.ImageName.c_str());
	}
	catch (...)
	{
		// Failed
	}
}

void CoDAssets::ExportMaterialImages(const XMaterial_t& Material, const std::string& ImagesPath, const std::string& ImageExtension, ImageFormat ImageFormatType)
{
	// Prepare to export material images
	for (auto& Image : Material.Images)
	{
		// Grab the full image path, if it doesn't exist convert it!
		auto FullImagePath = FileSystems::CombinePath(ImagesPath, Image.ImageName + ImageExtension);
		// Check if we want to skip previous images
		auto SkipPrevImages = SettingsManager::GetSetting("skipprevimg") == "true";
		// Check if it exists
		if (!FileSystems::FileExists(FullImagePath) || !SkipPrevImages)
		{
			// Buffer for the image (Loaded via the global game handler)
			std::unique_ptr<XImageDDS> ImageData = GameXImageHandler(Image);

			// Check if we got it
			if (ImageData != nullptr)
			{
				// Convert it to a file or just write the DDS data raw
				if (ImageFormatType == ImageFormat::DDS_WithHeader)
				{
					// Since this can throw, wrap it in an exception handler
					try
					{
						// Just write the buffer
						auto Writer = BinaryWriter();
						// Make the file
						Writer.Create(FullImagePath);
						// Write the DDS buffer
						Writer.Write((const int8_t*)ImageData->DataBuffer, ImageData->DataSize);
					}
					catch (...)
					{
						// Nothing, this means that something is already accessing the image
					}
				}
				else
				{
					// Convert it, this method is a nothrow
					Image::ConvertImageMemory(ImageData->DataBuffer, ImageData->DataSize, ImageFormat::DDS_WithHeader, FullImagePath, ImageFormatType, ImageData->ImagePatchType);
				}
			}
		}
	}
}

void CoDAssets::ExportAllAssets(void* Caller)
{
	// Prepare to export all available assets
	auto AssetsToExport = std::make_unique<std::vector<CoDAsset_t*>>();

	// Grab the parent window and check mode
	auto MainView = (MainWindow*)Caller;
	// Resolve the asset list
	auto& LoadedAssets = (MainView->SearchMode) ? MainView->SearchResults : CoDAssets::GameAssets->LoadedAssets;

	// An index for the element position
	uint32_t AssetPosition = 0;
	// Load up the assets, just passing the array
	for (auto& Asset : LoadedAssets)
	{
		// Add and set
		Asset->AssetLoadedIndex = AssetPosition;
		AssetsToExport->emplace_back(Asset);

		// Advance
		AssetPosition++;
	}

	// Setup export logic
	AssetsToExportCount = (uint32_t)AssetsToExport->size();
	ExportedAssetsCount = 0;
	CanExportContinue = true;

	// Pass off to the export logic thread
	ExportSelectedAssets(Caller, AssetsToExport);

	// Force clean up
	AssetsToExport->shrink_to_fit();
	AssetsToExport.reset();
}

void CoDAssets::ExportSelection(const std::vector<uint32_t>& Indicies, void* Caller)
{
	// Prepare to export all available assets
	auto AssetsToExport = std::make_unique<std::vector<CoDAsset_t*>>();

	// Grab the parent window and check mode
	auto MainView = (MainWindow*)Caller;
	// Resolve the asset list
	auto& LoadedAssets = (MainView->SearchMode) ? MainView->SearchResults : CoDAssets::GameAssets->LoadedAssets;

	// Load up the assets, just passing the array
	for (auto& AssetIndex : Indicies)
	{
		// Grab and set
		auto& AssetUse = LoadedAssets[AssetIndex];

		// Set
		AssetUse->AssetLoadedIndex = AssetIndex;
		AssetsToExport->emplace_back(AssetUse);
	}

	// Setup export logic
	AssetsToExportCount = (uint32_t)AssetsToExport->size();
	ExportedAssetsCount = 0;
	CanExportContinue = true;

	// Pass off to the export logic thread
	ExportSelectedAssets(Caller, AssetsToExport);

	// Force clean up
	AssetsToExport->shrink_to_fit();
	AssetsToExport.reset();
}

void CoDAssets::ExportSelectedAssets(void* Caller, const std::unique_ptr<std::vector<CoDAsset_t*>>& Assets)
{
	// We must wait until the package cache is fully loaded before exporting
	if (GamePackageCache != nullptr)
	{
		// Wait for it to finish
		GamePackageCache->WaitForPackageCacheLoad();
	}

	// At this point, all of the assets are loaded into the queue, we can do this in async
	// We are gonna set the popup directory here, either the game's export path, or single export path.

	// Make sure we have one asset
	if (Assets->size() == 1)
	{
		// Set from this one asset
		LatestExportPath = BuildExportPath(Assets->at(0));
	}
	else if (Assets->size() > 0)
	{
		// Build the game specific path
		if (Assets->at(0)->AssetType == WraithAssetType::Model)
		{
			// Go back two
			LatestExportPath = FileSystems::GetDirectoryName(FileSystems::GetDirectoryName(BuildExportPath(Assets->at(0))));
		}
		else
		{
			// Go back one
			LatestExportPath = FileSystems::GetDirectoryName(BuildExportPath(Assets->at(0)));
		}
	}

	// The asset index we're on
	std::atomic<uint32_t> AssetIndex = 0;
	// The assets we need to convert
	uint32_t AssetsToConvert = (uint32_t)Assets->size();

	// Detect the number of conversion threads
	SYSTEM_INFO SystemInfo;
	// Fetch system info
	GetSystemInfo(&SystemInfo);
	// Get count
	auto NumberOfCores = SystemInfo.dwNumberOfProcessors;
	
	// Clamp it, no less than 1, no more than 3
	auto DegreeOfConverter = VectorMath::Clamp<uint32_t>(NumberOfCores, 1, 3);

	// Prepare to convert the assets in async
	CoDXConverter([&AssetIndex, &Caller, &Assets, &AssetsToConvert]
	{
		// Begin the image thread
		Image::SetupConversionThread();

		// Loop until we've done all assets or, until we cancel
		while (AssetIndex < AssetsToConvert && CoDAssets::CanExportContinue)
		{
			// Grab our index
			auto AssetToConvert = AssetIndex++;

			// Ensure still valid
			if (AssetToConvert >= Assets->size())
				continue;

			// Get the asset we need
			auto& Asset = Assets->at(AssetToConvert);
			// Make sure it's not a placeholder
			if (Asset->AssetStatus != WraithAssetStatus::Placeholder)
			{
				// Set the status
				Asset->AssetStatus = WraithAssetStatus::Processing;
				// Report asset status
				if (OnExportStatus != nullptr)
				{
					OnExportStatus(Caller, Asset->AssetLoadedIndex);
				}

				// Export it
				auto Result = ExportGameResult::UnknownError;

				// Export it
				try
				{
					Result = CoDAssets::ExportAsset(Asset);
				}
				catch (...)
				{
					// Unknown issue
				}

				// Set the status
				Asset->AssetStatus = (Result == ExportGameResult::Success) ? WraithAssetStatus::Exported : WraithAssetStatus::Error;
				// Report asset status
				if (OnExportStatus != nullptr)
				{
					OnExportStatus(Caller, Asset->AssetLoadedIndex);
				}
			}

			// Advance
			CoDAssets::ExportedAssetsCount++;

			// Report
			if (OnExportProgress != nullptr)
			{
				OnExportProgress(Caller, (uint32_t)(((float)CoDAssets::ExportedAssetsCount / (float)CoDAssets::AssetsToExportCount) * 100.f));
			}
		}

		// End image conversion thread
		Image::DisableConversionThread();

		// We are spinning up a maximum of 3 threads for conversion
	}, DegreeOfConverter);
}