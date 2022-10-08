#pragma once

#include <cstdint>
#include <memory>
#include <atomic>

// We need the following WraithX classes
#include "ProcessReader.h"
#include "TextWriter.h"
#include "WraithModel.h"
#include "Image.h"
#include "WraithNameIndex.h"

// The Call of Duty asset type
#include "CoDAssetType.h"
#include "CoDPackageCache.h"
#include "CoDXConverter.h"
#include "CoDGDTProcessor.h"
#include "CoDCDNCache.h"
#include "CoDCDNDownloader.h"

// Parasyte
#include "Parasyte.h"

// A list of supported Call of Duty games, or none for nothing loaded
enum class SupportedGames
{
    None,
    WorldAtWar,
    BlackOps,
    BlackOps2,
    BlackOps3,
    BlackOps4,
    BlackOpsCW,
    ModernWarfare,
    ModernWarfare2,
    ModernWarfare3,
    ModernWarfare4,
    ModernWarfare5,
    QuantumSolace,
    ModernWarfareRemastered,
    ModernWarfare2Remastered,
    Ghosts,
    InfiniteWarfare,
    AdvancedWarfare,
    WorldWar2,
    Vanguard,
    Parasyte,
};

// A list of supported game flags for Call of Duty
enum class SupportedGameFlags
{
    None,
    Files,
    SP,
    MP,
    ZM
};

// A set of FindGame() results
enum class FindGameResult
{
    Success,
    NoGamesRunning,
    FailedToLocateInfo,
    UnknownError
};

// A set of LoadFile() results
enum class LoadGameFileResult
{
    Success,
    InvalidFile,
    UnknownError
};

// A set of LoadGame() results
enum class LoadGameResult
{
    Success,
    NoAssetsFound,
    ProcessNotRunning,
    UnknownError
};

// A set of ExportAsset() results
enum class ExportGameResult
{
    Success,
    Placeholder,
    UnknownError
};

// Delegate for reading a ximage
typedef std::unique_ptr<XImageDDS>(*LoadXImageHandler)(const XImage_t& Image);
// Delegate for reading a string entry
typedef std::string(*LoadStringHandler)(uint64_t Index);

// Delegate callback for export progress
typedef void(*ExportProgressHandler)(void* Caller, uint32_t Progress);
typedef void(*ExportStatusHandler)(void* Caller, uint32_t AssetIndex);

// Represents a game process info
struct CoDGameProcess
{
    // The name of the process
    const char* ProcessName;
    // The game id of this process
    SupportedGames GameID;
    // The game flags of this process
    SupportedGameFlags GameFlags;

    CoDGameProcess(const char* Name, SupportedGames Game, SupportedGameFlags Flags)
    {
        // Set data
        ProcessName = Name;
        GameID = Game;
        GameFlags = Flags;
    }
};

// A class that handles reading assets from COD games
class CoDAssets
{
public:
    // -- Global variables

    // A list of game process info
    const static std::vector<CoDGameProcess> GameProcessInfo;

    // The running game instance, if any
    static std::unique_ptr<ProcessReader> GameInstance;
    // The asset log, if any
    static std::unique_ptr<TextWriter> XAssetLogWriter;
    // The running game ID, if any
    static SupportedGames GameID;
    // The running game flags, if any
    static SupportedGameFlags GameFlags;
    // The load game flags, if any
    static bool VerifiedHashes;

    // The game's loaded assets
    static std::unique_ptr<AssetPool> GameAssets;
    // The game's package cache, if any
    static std::unique_ptr<CoDPackageCache> GamePackageCache;
    // The game's package cache, if any
    static std::unique_ptr<CoDPackageCache> OnDemandCache;
    // The game's cdn downloader, if any
    static std::unique_ptr<CoDCDNDownloader> CDNDownloader;
    // The game's ximage read handler
    static LoadXImageHandler GameXImageHandler;
    // The game's string read handler
    static LoadStringHandler GameStringHandler;
    // The game's asset name database.
    static WraithNameIndex AssetNameCache;
    // The game's string database.
    static WraithNameIndex StringCache;

    // The directory where the game is stored.
    static std::string GameDirectory;

    // -- Global game information

    // A list of game offsets, varies per-game
    static std::vector<uint64_t> GameOffsetInfos;
    // A list of game pool sizes, varies per-game
    static std::vector<uint32_t> GamePoolSizes;

    // A GDT processor for this game
    static std::unique_ptr<CoDGDTProcessor> GameGDTProcessor;

    // -- Loading and cleanup functions

    // Loads the game and finds assets if need be
    static FindGameResult BeginGameMode();
    // Loads the specified game file
    static LoadGameFileResult BeginGameFileMode(const std::string& FilePath);

    // Attempts to find one of the supported games (Process only, file support via LoadModtools())
    static FindGameResult FindGame();
    // Attempts to load assets from the loaded game
    static LoadGameResult LoadGame();
    // Attempts to load assets from the loaded game
    static LoadGameResult LoadGamePS();
    // Requests next XAsset
    static ps::XAsset64 ParasyteRequest(const uint64_t& AssetPointer);
    // Attempts to load assets from a file
    static LoadGameFileResult LoadFile(const std::string& FilePath);
    // Cleans up a game if attached
    static void CleanUpGame();

    // Logs XAsset on LoadGame
    static void LogXAsset(const std::string& Type, const std::string& Name);

    // -- Exporting functions

    // Occures to alert the dialog of progress
    static ExportProgressHandler OnExportProgress;
    // Occures to alert the dialog of an asset
    static ExportStatusHandler OnExportStatus;

    // A count of assets exported
    static std::atomic<uint32_t> ExportedAssetsCount;
    // A count of total assets to export
    static std::atomic<uint32_t> AssetsToExportCount;
    // Whether or not export can continue
    static std::atomic<bool> CanExportContinue;

    // Export all currently loaded game assets
    static void ExportAllAssets(void* Caller = NULL);
    // Export selected indicies only
    static void ExportSelection(const std::vector<uint32_t>& Indicies, void* Caller = NULL);

    // The latest export path
    static std::string LatestExportPath;

    // Exports the game asset
    static ExportGameResult ExportAsset(const CoDAsset_t* Asset);

    // Gets a model asset for previewing
    static std::unique_ptr<WraithModel> GetModelForPreview(const CoDModel_t* Model);

    // Gets an asset hash. If one is not found, a default value is used.
    static std::string GetHashedName(const std::string& type, const uint64_t hash);
    // Gets a string hash. If one is not found, a default value is used.
    static std::string GetHashedString(const std::string& type, const uint64_t hash);
private:
    // -- Game utility functions, internal

    // Attempt to locate the loaded game's offset info
    static bool LocateGameInfo();

    // Safely clean up the package cache if need be
    static void CleanupPackageCache();

    // Game generics, load generic assets of types

    // Loads a generic XAnim
    static std::unique_ptr<XAnim_t> LoadGenericAnimAsset(const CoDAnim_t* Anim);
    // Loads a generic XModel
    static std::unique_ptr<XModel_t> LoadGenericModelAsset(const CoDModel_t* Model);

    // -- Game export utility functions, internal

    // Builds the export path for the asset
    static std::string BuildExportPath(const CoDAsset_t* Asset);

    // Determines whether we should continue with exporting this anim based off if it exists
    static bool ShouldExportAnim(std::string ExportPath);
    // Determines whether we should continue with exporting this model based off if it exists
    static bool ShouldExportModel(std::string ExportPath);

    // Exports a game animation asset
    static ExportGameResult ExportAnimationAsset(const CoDAnim_t* Animation, const std::string& ExportPath);
    // Exports a game model asset
    static ExportGameResult ExportModelAsset(const CoDModel_t* Model, const std::string& ExportPath, const std::string& ImagesPath, const std::string& ImageRelativePath, const std::string& ImageExtension);
    // Exports a game image asset
    static ExportGameResult ExportImageAsset(const CoDImage_t* Image, const std::string& ExportPath, const std::string& ImageExtension);
    // Exports a game sound asset
    static ExportGameResult ExportSoundAsset(const CoDSound_t* Sound, const std::string& ExportPath, const std::string& SoundExtension);
    // Exports a game effect asset
    static ExportGameResult ExportEffectAsset(const CoDEffect_t* Effect, const std::string& ExportPath);
    // Exports a game rawfile asset
    static ExportGameResult ExportRawfileAsset(const CoDRawFile_t* Rawfile, const std::string& ExportPath);
    // Exports a game rawfile asset
    static ExportGameResult ExportMaterialAsset(const CoDMaterial_t* Material, const std::string& ExportPath, const std::string& ImagesPath, const std::string& ImageRelativePath, const std::string& ImageExtension);

    // Exports Material Image Names
    static void ExportMaterialImageNames(const XMaterial_t& Material, const std::string& ExportPath);

    // Exports images from a specific game material
    static void ExportMaterialImages(const XMaterial_t& Material, const std::string& ImagesPath, const std::string& ImageExtension, ImageFormat ImageFormatType);

    // Export a WraithModel to the various formats specified in settings
    static void ExportWraithModel(const std::unique_ptr<WraithModel>& Model, const std::string& ExportPath);

    // Exports the asset in the list provided, in async
    static void ExportSelectedAssets(void* Caller, const std::unique_ptr<std::vector<CoDAsset_t*>>& Assets);

    // Sorts the assets by various properties
    static bool SortAssets(const CoDAsset_t* lhs, const CoDAsset_t* rhs);

    // A locking mutex for proper async operations
    static std::mutex CodMutex;
};