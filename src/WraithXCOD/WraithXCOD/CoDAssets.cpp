#include "stdafx.h"
#include "spdlog/spdlog.h"

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

// We need the CDN Downloaders
#include "CoDCDNDownloader.h"
#include "CoDCDNDownloaderV0.h"
#include "CoDCDNDownloaderV1.h"
#include "CoDCDNDownloaderV2.h"

// We need the main window for callbacks
#include "MainWindow.h"

// We need settings
#include "SettingsManager.h"

// We need the CoDTranslators
#include "CoDXAnimTranslator.h"
#include "CoDXModelTranslator.h"
#include "CoDRawfileTranslator.h"
#include "CoDXConverter.h"

// We need the game support functions
#include "GameWorldAtWar.h"
#include "GameBlackOps.h"
#include "GameBlackOps2.h"
#include "GameBlackOps3.h"
#include "GameBlackOps4.h"
#include "GameBlackOpsCW.h"
#include "GameModernWarfare.h"
#include "GameModernWarfare2.h"
#include "GameModernWarfare3.h"
#include "GameModernWarfare4.h"
#include "GameModernWarfare5.h"
#include "GameModernWarfare6.h"
#include "GameGhosts.h"
#include "GameAdvancedWarfare.h"
#include "GameModernWarfareRM.h"
#include "GameModernWarfare2RM.h"
#include "GameInfiniteWarfare.h"
#include "GameWorldWar2.h"
#include "GameQuantumSolace.h"
#include "GameVanguard.h"

// We need the game cache functions
// TODO: Reorganise how packages are handled, merge into a "reader" class that handles
// everything rather than duplicated code and seperate code for Load File/Caches
#include "IWDCache.h"
#include "IPAKCache.h"
#include "PAKCache.h"
#include "XPAKCache.h"
#include "SABCache.h"
#include "XPTOCCache.h"
#include "XSUBCache.h"
#include "XSUBCacheV2.h"
#include "XSUBCacheV3.h"
#include "VGXPAKCache.h"

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
#include "FBXExport.h"
#include "XMBExport.h"
#include "GLTFExport.h"
#include "CastExport.h"

// TODO: Use image usage/semantic hashes to determine image types instead when reading from XMaterials

// -- Setup global variables

WraithNameIndex CoDAssets::AssetNameCache;
WraithNameIndex CoDAssets::StringCache;


// Set the default game instant pointer
std::unique_ptr<ProcessReader> CoDAssets::GameInstance = nullptr;
// Set the default logger pointer
std::unique_ptr<TextWriter> CoDAssets::XAssetLogWriter = nullptr;
// The main runtime log
std::shared_ptr<spdlog::logger> CoDAssets::Log = nullptr;

// Set the default game id
SupportedGames CoDAssets::GameID = SupportedGames::None;
// Set the default flags
SupportedGameFlags CoDAssets::GameFlags = SupportedGameFlags::None;
// Set the default verification
bool CoDAssets::VerifiedHashes = false;

// Set game offsets
std::vector<uint64_t> CoDAssets::GameOffsetInfos = std::vector<uint64_t>();
// Set game sizes
std::vector<uint32_t> CoDAssets::GamePoolSizes = std::vector<uint32_t>();

// Set loaded assets
std::unique_ptr<AssetPool> CoDAssets::GameAssets = nullptr;
// Set cache
std::unique_ptr<CoDPackageCache> CoDAssets::GamePackageCache = nullptr;
// Set cache
std::unique_ptr<CoDPackageCache> CoDAssets::OnDemandCache = nullptr;
// Set downloader
std::unique_ptr<CoDCDNDownloader> CoDAssets::CDNDownloader = nullptr;

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

// Set game directory
std::string CoDAssets::GameDirectory = "";

// Image Hashes from Black Ops 3's Techsets
std::map<uint32_t, std::string> SemanticHashes = 
{
    { 0xA0AB1041, "colorMap" },
    { 0xB34B914B, "minimapMask" },
    { 0x369DEAC4, "colorMapSampler1" },
    { 0x12B6046F, "aberrationMask" },
    { 0xD90F4DC7, "gridTexture" },
    { 0x120E7A31, "uvOffsetTexture" },
    { 0x738502A8, "warpMap" },
    { 0x9E7BA008, "maskMap" },
    { 0x961141F1, "offsetMap" },
    { 0x4C094D3E, "irisTexture" },
    { 0x8FAE02B , "warpTexture" },
    { 0x1055275D, "lookupTexture" },
    { 0x7EF460A8, "blockNoiseTexture" },
    { 0x34D849D5, "revealMap" },
    { 0x264F0732, "diffuseMap" },
    { 0xE3C58D6C, "highlightMap" },
    { 0x56FF7ACA, "vignetteMap" },
    { 0x59D30D0F, "normalMap" },
    { 0xD78EC38E, "hexagonMap" },
    { 0x996BD23D, "irisMap" },
    { 0x2177C7B4, "lensFlareMap" },
    { 0x5A9A5201, "blurMaskMap" },
    { 0xDE799F3C, "warpMaskMap" },
    { 0xA6F70E30, "durdenMap" },
    { 0xF12B5F40, "pulseTexture" },
    { 0xE8515E3F, "distortionMask" },
    { 0xB1B4E3D7, "distortionMap" },
    { 0x665A1B6D, "scanlineMap" },
    { 0x29B553E1, "widgetMap1" },
    { 0x29B553E2, "widgetMap2" },
    { 0x29B553E3, "widgetMap3" },
    { 0x29B553E4, "widgetMap4" },
    { 0x29B553E5, "widgetMap5" },
    { 0x973D1967, "staticTexture" },
    { 0xA6DBAABD, "blurMask" },
    { 0x892E3B8B, "maskTexture" },
    { 0x50DA1DE9, "specHighlightMap" },
    { 0xCB13F18C, "normalTexture" },
    { 0xD5058185, "octogonMask" },
    { 0x6663034A, "noDamage" },
    { 0x4B87DB55, "lightDamage" },
    { 0xA9714557, "blockNoise" },
    { 0x7EAF2270, "lookup2" },
    { 0x69D4E61B, "spaceTexture" },
    { 0x75CD4B2 , "spaceAnusTexture" },
    { 0x9294363B, "pantherVagTexture" },
    { 0x6FB3CFCF, "lightningTexture" },
    { 0x7C86392D, "sparkleTexture" },
    { 0x3E013F6F, "faceTexture1" },
    { 0x3E013F6C, "faceTexture2" },
    { 0x369DEAC7, "colorMapSampler2" },
    { 0x369DEAC6, "colorMapSampler3" },
    { 0x9CD45BE1, "extraCamSampler" },
    { 0xFE195021, "noiseTexture" },
    { 0x3C19872 , "washTexture" },
    { 0x439044D6, "revealTexture" },
    { 0xEC84E459, "scubaTexture" },
    { 0xCDF5594E, "rivuletWarpTexture" },
    { 0x4E22BBB3, "rivuletRevealTexture" },
    { 0xF92FE655, "uvRevealTexture" },
    { 0x3FA0721A, "twinkleMap" },
    { 0x817A829B, "wormMap" },
    { 0x5BDF2D54, "distortionTexture" },
    { 0xB60D18E9, "colorMask" },
    { 0x77B50536, "bloodMotesMap" },
    { 0x389DD40F, "thermalHeatmap" },
    { 0x7176BF2 , "aoMap" },
    { 0x6D0A6C98, "glossMap" },
    { 0xE9817F0D, "glossMap" },
    { 0xEC443804, "specColorMap" },
    { 0x9A97E4B1, "customizeMask" },
    { 0xEB529B4D, "detailMap" },
    { 0x34614347, "emissiveMap" },
    { 0x95DF2A73, "detailNormal1" },
    { 0x95DF2A70, "detailNormal2" },
    { 0x95DF2A71, "detailNormal3" },
    { 0x95DF2A76, "detailNormal4" },
    { 0xD4A24996, "detailNormalMask" },
    { 0x53A3FCE2, "flickerLookupMap" },
    { 0xC089ABAF, "emissiveMask" },
    { 0x199A03D3, "tintMask" },
    { 0x2335512A, "thicknessMap" },
    { 0x55A604DC, "detailMap1" },
    { 0x55A604DF, "detailMap2" },
    { 0x80951342, "colorMapDetail2" },
    { 0x4429A80 , "mixMap" },
    { 0x4CDA7E01, "tintMask2" },
    { 0x56B697BB, "glossMapDetail2" },
    { 0x4ED66430, "specularMapDetail2" },
    { 0x338272BC, "alphaMaskMap" },
    { 0x6592A6E , "flowMap" },
    { 0x73502222, "noiseMap" },
    { 0xF79C1C0E, "rippleMap" },
    { 0x511B2AE1, "transitionDiffuse" },
    { 0x864EA29C, "transitionNormal" },
    { 0xF5276B0B, "transitionGloss" },
    { 0x5265433E, "sparkleDataMap" },
    { 0xA301628 , "alphaMap" },
    { 0xFB28D077, "image0" },
    { 0xFB28D076, "image1" },
    { 0xFB28D075, "image2" },
    { 0xFB28D074, "image3" },
    { 0xB598DFF4, "revealTextureB" },
    { 0x1CD5B065, "randomTextureA" },
    { 0x1CD5B066, "randomTextureB" },
    { 0x1CD5B067, "randomTextureC" },
    { 0x49F0DA7F, "distortMap" },
    { 0xFAF906C6, "gridTextureA" },
    { 0xFAF906C5, "gridTextureB" },
    { 0xFAF906C4, "gridTextureC" },
    { 0xFAF906C3, "gridTextureD" },
    { 0x1436343A, "heatLookup" },
    { 0xBE8B269C, "sceneLookup" },
    { 0xA60FE460, "uvOffsetDetailTexture" },
    { 0xB7A07713, "colorDetailTexture" },
    { 0xE625304 , "fireMap" },
    { 0xED590D65, "hudMap" },
    { 0x1EFD70AA, "colorRemapMap" },
    { 0xC67D666E, "shadowMaskMap" },
    { 0x81054C3A, "ditherMap" },
    { 0x5C800242, "smaaMap" },
    { 0x6FB66F22, "velveteenMask" },
    { 0xF7C2DB12, "tintBlendMask" },
    { 0x77151208, "camoMaskMap" },
    { 0xC0F0BF5F, "normalBodyMap" },
    { 0x1B768248, "glossBodyMap" },
    { 0xE86F76D9, "specColorMapThick" },
    { 0x6C246337, "underFfuseMap" },
    { 0xE57FFAA , "glossMap2" },
    { 0x1DE8CDAC, "cavityMap" },
    { 0xA41C9B9F, "styleMaskMap" },
    { 0xC80B7D0A, "colorSwatch1Map" },
    { 0xC807C8E9, "colorSwatch2Map" },
    { 0xBAEAD204, "normalSwatch1Map" },
    { 0xBAE92E67, "normalSwatch2Map" },
    { 0x656DD3F2, "specSwatch1Map" },
    { 0x656E3ED1, "specSwatch2Map" },
    { 0xE55690F3, "glossSwatch1Map" },
    { 0xE5583490, "glossSwatch2Map" },
    { 0xC089AC16, "emissiveMap1" },
    { 0xC089AC15, "emissiveMap2" },
    { 0xC089AC14, "emissiveMap3" },
    { 0x77B02241, "colorMap00" },
    { 0x4C596633, "flagRippleDetailMap" },
    { 0xD8BD9CD3, "decalMap" },
    { 0xA96FDB44, "crackMap" },
    { 0x39EC02D7, "crackNormalMap" },
    { 0x2BE9AF4 , "baseColorMap" },
    { 0x4CDA7E02, "tintMask1" },
    { 0x46FE0E6E, "baseTintMap" },
    { 0x55A604DE, "detailMap3" },
    { 0x80951343, "colorMapDetail3" },
    { 0x4E82BD61, "rColorRamp" },
    { 0xCCF4C850, "foamBase" },
    { 0xAE5C4C5 , "treadHeightMap" },
    { 0x50108D4D, "camoDetailMap" },
    { 0x8C22E816, "coneLUTMap" },
    { 0xFC80F332, "bentNormalMap" },
    { 0xA52C26B4, "specOcclusionMap" },
    { 0xC408965C, "stretchNormal" },
    { 0x6DFCC8D5, "compressNormal" },
    { 0x8F9CAD22, "cloudLayer0" },
    { 0xE566B555, "cloudMask0" },
    { 0x8F9CAD23, "cloudLayer1" },
    { 0xE566B554, "cloudMask1" },
    { 0x56B697BA, "glossMapDetail3" },
    { 0x4ED66431, "specularMapDetail3" },
    { 0x8C95EAB1, "mixMap1" },
    { 0xEE97A158, "clearcoatGlossMap" },
    { 0x8AD0A39B, "metalFlakeNormalMap" },
    { 0x2563EA9C, "metalFlakeMaskMap" },
    { 0x9783E4CD, "wobbleMap" },
    { 0xD6B59E75, "emissiveFlowMap" },
    { 0xCBF21CFB, "extracamTexture1" },
    { 0xCBF21CF8, "extracamTexture2" },
    { 0xCBF21CF9, "extracamTexture3" },
    { 0xCBF21CFE, "extracamTexture4" },
    { 0x8875EE6 , "breakUpMap" },
    { 0x74589F70, "diamondMask" },
    { 0x797F4F5C, "outlineMap" },
    { 0x5032DBE0, "alphaMask" },
    { 0x273482B9, "edgeFadeMap" },
    { 0xD28662DB, "specularMask" },
    { 0x64343DB8, "characterColorMap" },
    { 0xA7527356, "characterNormalMap" },
    { 0xE007460C, "characterRevealMap" },
    { 0x70DE0F82, "edgeColorMap" },
    { 0xDE36D64 , "edgeEmissiveMap" },
    { 0xC1EF31A8, "veinMap" },
    { 0x5E5425B9, "FutureMap" },
    { 0xA1712D0D, "ui3dSampler_C1_P0" },
    { 0xD2EEEEB8, "FontTextutre" },
    { 0x2A6C8C86, "OverlayMap" },
    { 0x85511979, "resolvedScene_C4_P0" },
    { 0x6F691B32, "codeTexture0_C3_P0" },
    { 0x6F645F19, "codeTexture0_C8_P0" },
    { 0x6F666E95, "codeTexture0_C4_P0" },
    { 0xCFED92EA, "Reveal_Map" },
    { 0xF039EC2D, "Diffuse_Map" },
    { 0xE98A255D, "AddMap" },
    { 0xB17777AC, "CompassMap" },
    { 0x3D4F14  , "Mask" },
    { 0x48FF11AE, "Diffuse" },
    { 0xF4E39943, "resolvedPostSun_C11_P0" },
    { 0x32F87F8 , "floatZSampler_C63_P0" },
    { 0x740FE37F, "fontCache" },
    { 0x1BB49   , "EKG" },
    { 0x8B4B7DF2, "resolvedPostSun_C1_P0" },
    { 0xBC83639B, "LightDamange" },
    { 0xF12FF2CE, "DpadTexture" },
    { 0x5F77D83E, "Noise_Texture" },
    { 0xF452E202, "Lookup" },
    { 0xE2AA34C0, "DayMap" },
    { 0x3D3E13  , "Mesh" },
    { 0x6519A44A, "heatmapSampler_C9_P0" },
    { 0x77F67259, "Overlay_Map" },
    { 0xAD1D0190, "Grain_Map" },
    { 0xFABDFA1A, "FadeMap" },
    { 0x6520EB80, "heatmapSampler_C3_P0" },
    { 0x9E2B786E, "C004_File" },
    { 0xAE7C5C1A, "HexagonPattern" },
    { 0xD657AB6E, "PlusTile" },
    { 0x44341ED2, "ScrollTexture" },
    { 0x3CEA479A, "sonarColorSampler_C1_P0" },
    { 0x589C508E, "ScrollTextureMap" },
    { 0x9D55EDBB, "GridTextureMap" },
    { 0x83F6B06F, "floatZSampler_C2_P0" },
    { 0x4969081A, "Static_Noise_Map" },
    { 0x8C128212, "LineMap" },
    { 0x855022BF, "resolvedScene_C2_P0" },
    { 0x7CF86BE , "Noise" },
    { 0xAC16CC94, "Wireframe" },
    { 0x79C6367 , "Image" },
    { 0xDF1738FD, "BackgroundGlow" },
    { 0x1E0097C9, "postEffect1_C4_P0" },
    { 0x1D23D48C, "postEffect0_C0_P0" },
    { 0x813B3FF , "Smoke" },
    { 0x942CBFF0, "Normal_Map" },
    { 0x55A04772, "Detail_Map" },
    { 0x8389EAF0, "PieChart" },
    { 0xE700765E, "LookupMap" },
    { 0xF5095EA4, "resolvedPostSun_C34_P0" },
    { 0x1D21496F, "postEffect0_C3_P0" },
    { 0xDE89DA4B, "TickMarkMaterial" },
    { 0x3E2C14  , "Tile" },
    { 0xA172C0E8, "ui3dSampler_C4_P0" },
    { 0x721D7116, "Vignette" },
    { 0x51CF2F85, "BlurredTexture" },
    { 0xC6CAE542, "ColorTexture" },
    { 0x82A61F22, "YUV_Image" },
    { 0x6F634657, "codeTexture0_C6_P0" },
    { 0x4E07583B, "transColorMap" },
    { 0xAB07D475, "transNormalMap" },
    { 0x9A8739AF, "transRevealMap" },
    { 0x1782DE2 , "transGlossMap" },
    { 0x95B48AE5, "meltRevealMap" },
    { 0x1DCB003F, "meltNormalMap" },
    { 0xB607C0FE, "Color_Map" },
    { 0x34ECCCB3, "specularMap" },
    { 0x6001F931, "occlusionMap" }
};

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
    // Black Ops CW
    { "blackopscoldwar.exe", SupportedGames::BlackOpsCW, SupportedGameFlags::SP },
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
    { "s2_mp64_ship.exe", SupportedGames::WorldWar2, SupportedGameFlags::MP },
    // Cordycep
    { "Cordycep.CLI.exe", SupportedGames::Parasyte, SupportedGameFlags::SP },
    // Modern Warfare 2 Remastered
    { "mw2cr.exe", SupportedGames::ModernWarfare2Remastered, SupportedGameFlags::SP },
    // 007 Quantum Solace
    { "jb_liveengine_s.exe", SupportedGames::QuantumSolace, SupportedGameFlags::SP },
};

// -- End find game database

FindGameResult CoDAssets::BeginGameMode()
{
    // Aquire a lock
    std::lock_guard<std::mutex> Lock(CodMutex);

    // Result
    auto Result = FindGameResult::Success;

    // Check if we have a game
    if (ps::state != nullptr || GameInstance == nullptr || !GameInstance->IsRunning())
    {
        // Load the game
        Result = FindGame();
    }

    // If success, load assets
    if (Result == FindGameResult::Success)
    {
        // Load assets, check for Parasyte
        if (ps::state != nullptr)
            LoadGamePS();
        else
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
    // Clear Parasyte
    ps::state = nullptr;
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

        // Create log, if desired
        if (SettingsManager::GetSetting("createxassetlog", "false") == "true")
        {
            XAssetLogWriter = std::make_unique<TextWriter>();
            XAssetLogWriter->Open(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "AssetLog.txt"));
        }

        // Load assets from the game
        switch (GameID)
        {
        case SupportedGames::QuantumSolace: Success = GameQuantumSolace::LoadAssets(); break;
        case SupportedGames::WorldAtWar: Success = GameWorldAtWar::LoadAssets(); break;
        case SupportedGames::BlackOps: Success = GameBlackOps::LoadAssets(); break;
        case SupportedGames::BlackOps2: Success = GameBlackOps2::LoadAssets(); break;
        case SupportedGames::BlackOps3: Success = GameBlackOps3::LoadAssets(); break;
        case SupportedGames::BlackOps4:
            // Allocate a new XPAK Mega Cache (Must reload CASC as the game can affect it if rerunning, etc. and result in corrupt exports)
            // TODO: Find a better solution to this, a good trigger for it to occur is relaunching the game, moving to different parts or Blizzard editing the CASC while
            // we have a handle, then try export an image, it'll probably come out black
            CleanupPackageCache();
            GamePackageCache = std::make_unique<XPAKCache>();
            // Set the XPAK path
            GamePackageCache->LoadPackageCacheAsync(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()));
            // Load as normally
            Success = GameBlackOps4::LoadAssets(); break;
        case SupportedGames::BlackOpsCW:
            // Allocate a new XPAK Mega Cache (Must reload CASC as the game can affect it if rerunning, etc. and result in corrupt exports)
            // TODO: Find a better solution to this, a good trigger for it to occur is relaunching the game, moving to different parts or Blizzard editing the CASC while
            // we have a handle, then try export an image, it'll probably come out black
            CleanupPackageCache();
            GamePackageCache = std::make_unique<XSUBCache>();
            // Set the XPAK path
            GamePackageCache->LoadPackageCacheAsync(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()));
            // Load as normally
            Success = GameBlackOpsCW::LoadAssets(); break;
        case SupportedGames::ModernWarfare: Success = GameModernWarfare::LoadAssets(); break;
        case SupportedGames::ModernWarfare2: Success = GameModernWarfare2::LoadAssets(); break;
        case SupportedGames::ModernWarfare3: Success = GameModernWarfare3::LoadAssets(); break;
        case SupportedGames::Ghosts: Success = GameGhosts::LoadAssets(); break;
        case SupportedGames::AdvancedWarfare: Success = GameAdvancedWarfare::LoadAssets(); break;
        case SupportedGames::ModernWarfareRemastered: Success = GameModernWarfareRM::LoadAssets(); break;
        case SupportedGames::ModernWarfare2Remastered: Success = GameModernWarfare2RM::LoadAssets(); break;
        case SupportedGames::InfiniteWarfare: Success = GameInfiniteWarfare::LoadAssets(); break;
        case SupportedGames::WorldWar2: Success = GameWorldWar2::LoadAssets(); break;
        }

        // Done with logger
        XAssetLogWriter = nullptr;

        // Result check
        if (Success)
        {
            // Sort the assets, for now we only support 2 modes, but this can be extended in the future.
            auto sortMethod = AssetCompareMethodHelper::CalculateCompareMethod(SettingsManager::GetSetting("assetsortmethod", "Name"));

            if (sortMethod != AssetCompareMethod::None)
            {
                std::stable_sort(GameAssets->LoadedAssets.begin(), GameAssets->LoadedAssets.end(), [sortMethod](const CoDAsset_t* lhs, const CoDAsset_t* rhs)
                {
                    return lhs->Compare(rhs, sortMethod);
                });
            }

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

LoadGameResult CoDAssets::LoadGamePS()
{
    // Make sure the process is running
    if (GameInstance->IsRunning())
    {
        // Setup the assets
        GameAssets.reset(new AssetPool());
        // Whether or not we loaded assets
        bool Success = false;

        // Create log, if desired
        if (SettingsManager::GetSetting("createxassetlog", "false") == "true")
        {
            XAssetLogWriter = std::make_unique<TextWriter>();
            XAssetLogWriter->Open(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "AssetLog.txt"));
        }

        // Check for CDN support.
        const bool CDNSupport = SettingsManager::GetSetting("cdn_downloader", "false") == "true";

        // Cleanup
        CleanupPackageCache();

        // Load assets from the game
        switch (ps::state->GameID)
        {
        // Modern Warfare 2019
        case 0x3931524157444F4D:
            GameModernWarfare4::PerformInitialSetup();
            GameID            = SupportedGames::ModernWarfare4;
            GameFlags         = SupportedGameFlags::None;
            GameXImageHandler = GameModernWarfare4::LoadXImage;
            GameStringHandler = GameModernWarfare4::LoadStringEntry;
            GamePackageCache  = std::make_unique<XPAKCache>();
            OnDemandCache     = std::make_unique<XPAKCache>();
            CDNDownloader     = CDNSupport ? std::make_unique<CoDCDNDownloaderV0>() : nullptr;
            GamePackageCache->LoadPackageCacheAsync(ps::state->GameDirectory);
            OnDemandCache->LoadPackageCacheAsync(FileSystems::CombinePath(ps::state->GameDirectory, "xpak_cache"));
            Success = GameModernWarfare4::LoadAssets();
            break;
        // Vanguard
        case 0x44524155474E4156:
            GameVanguard::PerformInitialSetup();
            GameID            = SupportedGames::Vanguard;
            GameFlags         = SupportedGameFlags::None;
            GameXImageHandler = GameVanguard::LoadXImage;
            GameStringHandler = GameVanguard::LoadStringEntry;
            GamePackageCache  = std::make_unique<XSUBCacheV2>();
            OnDemandCache     = std::make_unique<VGXPAKCache>();
            CDNDownloader     = CDNSupport ? std::make_unique<CoDCDNDownloaderV1>() : nullptr;
            GamePackageCache->LoadPackageCacheAsync(ps::state->GameDirectory);
            OnDemandCache->LoadPackageCacheAsync(FileSystems::CombinePath(ps::state->GameDirectory, "xpak_cache"));
            Success = GameVanguard::LoadAssets();
            break;
        // Modern Warfare Remastered
        case 0x30305453414D4552:
            GameID            = SupportedGames::ModernWarfareRemastered;
            GameFlags         = SupportedGameFlags::None;
            GameXImageHandler = GameModernWarfareRM::LoadXImagePS;
            GameStringHandler = GameModernWarfareRM::LoadStringEntry;
            GamePackageCache  = std::make_unique<PAKCache>();
            GamePackageCache->LoadPackageCacheAsync(ps::state->GameDirectory);
            Success = GameModernWarfareRM::LoadAssetsPS();
            break;
        // Advanced Warfare
        case 0x5241574E41564441:
            GameID            = SupportedGames::AdvancedWarfare;
            GameFlags         = SupportedGameFlags::None;
            GameXImageHandler = GameAdvancedWarfare::LoadXImagePS;
            GameStringHandler = GameAdvancedWarfare::LoadStringEntry;
            GamePackageCache  = std::make_unique<PAKCache>();
            GamePackageCache->LoadPackageCacheAsync(ps::state->GameDirectory);
            Success = GameAdvancedWarfare::LoadAssetsPS();
            break;
        // Infinite Warfare
        case 0x4652415749464E49:
            GameID = SupportedGames::InfiniteWarfare;
            GameFlags = SupportedGameFlags::None;
            GameXImageHandler = GameInfiniteWarfare::LoadXImagePS;
            GameStringHandler = GameInfiniteWarfare::LoadStringEntry;
            GamePackageCache = std::make_unique<PAKCache>();
            GamePackageCache->LoadPackageCacheAsync(ps::state->GameDirectory);
            Success = GameInfiniteWarfare::LoadAssetsPS();
            break;
        // Modern Warfare 2 Remastered
        case 0x32305453414D4552:
            GameID = SupportedGames::ModernWarfare2Remastered;
            GameFlags = SupportedGameFlags::None;
            GameXImageHandler = GameModernWarfare2RM::LoadXImagePS;
            GameStringHandler = GameModernWarfare2RM::LoadStringEntry;
            GamePackageCache = std::make_unique<PAKCache>();
            GamePackageCache->LoadPackageCacheAsync(ps::state->GameDirectory);
            Success = GameModernWarfare2RM::LoadAssetsPS();
            break;
        // Modern Warfare 2 (2022)
        case 0x3232524157444F4D:
            GameModernWarfare5::PerformInitialSetup();
            GameID            = SupportedGames::ModernWarfare5;
            GameFlags         = ps::state->HasFlag("sp") ? SupportedGameFlags::SP : SupportedGameFlags::MP;
            GameXImageHandler = GameModernWarfare5::LoadXImage;
            GameStringHandler = GameModernWarfare5::LoadStringEntry;
            GamePackageCache  = std::make_unique<XSUBCacheV3>();
            CDNDownloader     = CDNSupport ? std::make_unique<CoDCDNDownloaderV2>() : nullptr;
            GamePackageCache->LoadPackageCacheAsync(ps::state->GameDirectory);
            Success = GameModernWarfare5::LoadAssets();
            break;
        // Modern Warfare 3 (2023)
        case 0x4B4F4D41594D4159:
            GameModernWarfare6::PerformInitialSetup();
            GameID = SupportedGames::ModernWarfare6;
            GameFlags = ps::state->HasFlag("sp") ? SupportedGameFlags::SP : SupportedGameFlags::MP;
            GameXImageHandler = GameModernWarfare6::LoadXImage;
            GameStringHandler = GameModernWarfare6::LoadStringEntry;
            GamePackageCache = std::make_unique<XSUBCacheV3>();
            CDNDownloader = CDNSupport ? std::make_unique<CoDCDNDownloaderV2>() : nullptr;
            GamePackageCache->LoadPackageCacheAsync(ps::state->GameDirectory);
            Success = GameModernWarfare6::LoadAssets();
            break;
        }

        // Check for CDN
        if (CDNDownloader != nullptr)
            CDNDownloader->Initialize(ps::state->GameDirectory);

        // Done with logger
        XAssetLogWriter = nullptr;

        // Result check
        if (Success)
        {
            // Sort the assets, for now we only support 2 modes, but this can be extended in the future.
            auto sortMethod = AssetCompareMethodHelper::CalculateCompareMethod(SettingsManager::GetSetting("assetsortmethod", "Name"));

            if (sortMethod != AssetCompareMethod::None)
            {
                std::stable_sort(GameAssets->LoadedAssets.begin(), GameAssets->LoadedAssets.end(), [sortMethod](const CoDAsset_t* lhs, const CoDAsset_t* rhs)
                {
                    return lhs->Compare(rhs, sortMethod);
                });
            }


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

ps::XAsset64 CoDAssets::ParasyteRequest(const uint64_t& AssetPointer)
{
    return CoDAssets::GameInstance->Read<ps::XAsset64>(AssetPointer);
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
        // Sort the assets, for now we only support 2 modes, but this can be extended in the future.
        auto sortMethod = AssetCompareMethodHelper::CalculateCompareMethod(SettingsManager::GetSetting("assetsortmethod", "Name"));

        if (sortMethod != AssetCompareMethod::None)
        {
            std::stable_sort(GameAssets->LoadedAssets.begin(), GameAssets->LoadedAssets.end(), [sortMethod](const CoDAsset_t* lhs, const CoDAsset_t* rhs)
            {
                return lhs->Compare(rhs, sortMethod);
            });
        }

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

//#ifndef _DEBUG
    try
    {
//#endif
        // Send to specific export handler
        switch (Asset->AssetType)
        {
            // Export an animation
            case WraithAssetType::Animation: {Result = ExportAnimationAsset((CoDAnim_t*)Asset, ExportPath); break;}
            // Export a model, combine the name of the model with the export path!
            case WraithAssetType::Model: {Result = ExportModelAsset((CoDModel_t*)Asset, ExportPath, ImagesPath, ImageRelativePath, ImageExtension); break;}
            // Export an image
            case WraithAssetType::Image: {Result = ExportImageAsset((CoDImage_t*)Asset, ExportPath, ImageExtension); break;}
            // Export a sound
            case WraithAssetType::Sound: {Result = ExportSoundAsset((CoDSound_t*)Asset, ExportPath, CoDAssets::GameID == SupportedGames::WorldAtWar ? ".wav" : SoundExtension); break;}
            // Export a rawfile
            case WraithAssetType::RawFile: {Result = ExportRawfileAsset((CoDRawFile_t*)Asset, ExportPath); break;}
            // Export a material
            case WraithAssetType::Material: {Result = ExportMaterialAsset((CoDMaterial_t*)Asset, ExportPath, ImagesPath, ImageRelativePath, ImageExtension); break;}
        }
//#ifndef _DEBUG
    }
    catch (...)
    {
        Result = ExportGameResult::UnknownError;
        CoDAssets::Log->error("An exception occurred while exporting asset: {0}", Asset->AssetName);
#if _DEBUG
		std::cout << "Error: An exception occurred while exporting asset:" << Asset->AssetName.c_str() << "\n";
#endif // _DEBUG
    }
//#endif

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

std::string CoDAssets::GetHashedName(const std::string& type, const uint64_t hash)
{
    auto found = AssetNameCache.NameDatabase.find(hash & 0xFFFFFFFFFFFFFFF);

    if (found != AssetNameCache.NameDatabase.end())
    {
        return found->second;
    }

    return Strings::Format("%s_%llx", type.c_str(), hash & 0xFFFFFFFFFFFFFFF);
}

std::string CoDAssets::GetHashedString(const std::string& type, const uint64_t hash)
{
    auto found = StringCache.NameDatabase.find(hash);

    if (found != StringCache.NameDatabase.end())
    {
        return found->second;
    }

    return Strings::Format("%s_%llx", type.c_str(), hash);
}

bool CoDAssets::LocateGameInfo()
{
    // Whether or not we found what we need
    bool Success = false;

    // Attempt to find the loaded game's offsets, either via DB or heuristics
    // Also, apply proper handlers for various game read functions (Non-inlinable functions only)
    switch (GameID)
    {
    case SupportedGames::QuantumSolace:
        // Load game offset info
        Success = GameQuantumSolace::LoadOffsets();
        // Set game ximage handler
        GameXImageHandler = GameQuantumSolace::LoadXImage;
        // Set game string handler
        GameStringHandler = GameQuantumSolace::LoadStringEntry;
        break;
    case SupportedGames::WorldAtWar:
        // Load game offset info
        Success = GameWorldAtWar::LoadOffsets();
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
        // Initial setup required for BO4
        GameBlackOps4::PerformInitialSetup();
        // Load game offset info
        Success = GameBlackOps4::LoadOffsets();
        // Set game ximage handler
        GameXImageHandler = GameBlackOps4::LoadXImage;
        // Set game string handler
        GameStringHandler = GameBlackOps4::LoadStringEntry;
        break;
    }
    case SupportedGames::BlackOpsCW:
    {
        // Initial setup required for BO4
        GameBlackOpsCW::PerformInitialSetup();
        // Load game offset info
        Success = GameBlackOpsCW::LoadOffsets();
        // Set game ximage handler
        GameXImageHandler = GameBlackOpsCW::LoadXImage;
        // Set game string handler
        GameStringHandler = GameBlackOpsCW::LoadStringEntry;
        break;
    }
    case SupportedGames::ModernWarfare:
        // Load game offset info
        Success = GameModernWarfare::LoadOffsets();
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
        // Set game ximage handler
        GameXImageHandler = GameModernWarfareRM::LoadXImage;
        // Set game string handler
        GameStringHandler = GameModernWarfareRM::LoadStringEntry;
        // Allocate a new PAK Mega Cache
        GamePackageCache = std::make_unique<PAKCache>();
        // Set the PAK path
        GamePackageCache->LoadPackageCacheAsync(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()));
        break;
    case SupportedGames::ModernWarfare2Remastered:
        // Load game offset info
        Success = GameModernWarfare2RM::LoadOffsets();
        // Set game ximage handler
        GameXImageHandler = GameModernWarfare2RM::LoadXImage;
        // Set game string handler
        GameStringHandler = GameModernWarfare2RM::LoadStringEntry;
        // Allocate a new PAK Mega Cache
        GamePackageCache = std::make_unique<PAKCache>();
        // Set the PAK path
        GamePackageCache->LoadPackageCacheAsync(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()));
        break;
    case SupportedGames::InfiniteWarfare:
        // Load game offset info
        Success = GameInfiniteWarfare::LoadOffsets();
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
        // Set game ximage handler
        GameXImageHandler = GameWorldWar2::LoadXImage;
        // Set game string handler
        GameStringHandler = GameWorldWar2::LoadStringEntry;
        // Allocate a new PAK Mega Cache
        GamePackageCache = std::make_unique<XPTOCCache>();
        // Set the PAK path
        GamePackageCache->LoadPackageCacheAsync(FileSystems::GetDirectoryName(GameInstance->GetProcessPath()));
        break;
    case SupportedGames::Parasyte:
        // Locate Parasyte Current Handler Database
        const auto DBFile = FileSystems::CombinePath(FileSystems::GetDirectoryName(CoDAssets::GameInstance->GetProcessPath()), "Data\\CurrentHandler.csi");
        Success = FileSystems::FileExists(DBFile);
        // Validate
        if (Success)
        {
            ps::state = std::make_unique<ps::State>();
            Success = ps::state->Load(DBFile);
        }
        // Don't Check Offsets or Set up GDT until below
        return Success;
    }

    // Validate the results, every game should have at least 1 offset and 1 size, and success must be true
    if (Success && !GameOffsetInfos.empty() && !GamePoolSizes.empty())
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
    case SupportedGames::QuantumSolace: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "quantum_solace"); break;
    case SupportedGames::WorldAtWar: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "world_at_war"); break;
    case SupportedGames::BlackOps: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "black_ops_1"); break;
    case SupportedGames::BlackOps2: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "black_ops_2"); break;
    case SupportedGames::BlackOps3: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "black_ops_3"); break;
    case SupportedGames::BlackOps4: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "black_ops_4"); break;
    case SupportedGames::BlackOpsCW: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "black_ops_cw"); break;
    case SupportedGames::ModernWarfare: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare"); break;
    case SupportedGames::ModernWarfare2: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare_2"); break;
    case SupportedGames::ModernWarfare3: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare_3"); break;
    case SupportedGames::ModernWarfare4: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare_4"); break;
    case SupportedGames::ModernWarfare5: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare_5"); break;
    case SupportedGames::ModernWarfare6: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare_6"); break;
    case SupportedGames::Ghosts: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "ghosts"); break;
    case SupportedGames::AdvancedWarfare: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "advanced_warfare"); break;
    case SupportedGames::ModernWarfareRemastered: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare_rm"); break;
    case SupportedGames::ModernWarfare2Remastered: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "modern_warfare_2_rm"); break;
    case SupportedGames::InfiniteWarfare: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "infinite_warfare"); break;
    case SupportedGames::WorldWar2: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "world_war_2"); break;
    case SupportedGames::Vanguard: ApplicationPath = FileSystems::CombinePath(ApplicationPath, "vanguard"); break;
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
    case WraithAssetType::RawFile:
        // Default directory
        ApplicationPath = FileSystems::CombinePath(ApplicationPath, "xrawfiles");
        break;
    case WraithAssetType::Material:
        // Directory with asset name
        ApplicationPath = FileSystems::CombinePath(FileSystems::CombinePath(ApplicationPath, "xmaterials"), Asset->AssetName);
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
    case SupportedGames::QuantumSolace: return GameQuantumSolace::ReadXAnim(Animation); break;
    case SupportedGames::WorldAtWar: return GameWorldAtWar::ReadXAnim(Animation); break;
    case SupportedGames::BlackOps: return GameBlackOps::ReadXAnim(Animation); break;
    case SupportedGames::BlackOps2: return GameBlackOps2::ReadXAnim(Animation); break;
    case SupportedGames::BlackOps3: return GameBlackOps3::ReadXAnim(Animation); break;
    case SupportedGames::BlackOps4: return GameBlackOps4::ReadXAnim(Animation); break;
    case SupportedGames::BlackOpsCW: return GameBlackOpsCW::ReadXAnim(Animation); break;
    case SupportedGames::ModernWarfare: return GameModernWarfare::ReadXAnim(Animation); break;
    case SupportedGames::ModernWarfare2: return GameModernWarfare2::ReadXAnim(Animation); break;
    case SupportedGames::ModernWarfare3: return GameModernWarfare3::ReadXAnim(Animation); break;
    case SupportedGames::ModernWarfare4: return GameModernWarfare4::ReadXAnim(Animation); break;
    case SupportedGames::ModernWarfare5: return GameModernWarfare5::ReadXAnim(Animation); break;
    case SupportedGames::ModernWarfare6: return GameModernWarfare6::ReadXAnim(Animation); break;
    case SupportedGames::Ghosts: return GameGhosts::ReadXAnim(Animation); break;
    case SupportedGames::AdvancedWarfare: return GameAdvancedWarfare::ReadXAnim(Animation); break;
    case SupportedGames::ModernWarfareRemastered: return GameModernWarfareRM::ReadXAnim(Animation); break;
    case SupportedGames::ModernWarfare2Remastered: return GameModernWarfare2RM::ReadXAnim(Animation); break;
    case SupportedGames::InfiniteWarfare: return GameInfiniteWarfare::ReadXAnim(Animation); break;
    case SupportedGames::WorldWar2: return GameWorldWar2::ReadXAnim(Animation); break;
    case SupportedGames::Vanguard: return GameVanguard::ReadXAnim(Animation); break;
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
            // Check for Cast format
            if (SettingsManager::GetSetting("export_castanim") == "true")
            {
                // Export a Cast
                Cast::ExportCastAnim(*Result.get(), FileSystems::CombinePath(ExportPath, Result->AssetName + ".cast"));
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
    case SupportedGames::QuantumSolace: return GameQuantumSolace::ReadXModel(Model); break;
    case SupportedGames::WorldAtWar: return GameWorldAtWar::ReadXModel(Model); break;
    case SupportedGames::BlackOps: return GameBlackOps::ReadXModel(Model); break;
    case SupportedGames::BlackOps2: return GameBlackOps2::ReadXModel(Model); break;
    case SupportedGames::BlackOps3: return GameBlackOps3::ReadXModel(Model); break;
    case SupportedGames::BlackOps4: return GameBlackOps4::ReadXModel(Model); break;
    case SupportedGames::BlackOpsCW: return GameBlackOpsCW::ReadXModel(Model); break;
    case SupportedGames::ModernWarfare: return GameModernWarfare::ReadXModel(Model); break;
    case SupportedGames::ModernWarfare2: return GameModernWarfare2::ReadXModel(Model); break;
    case SupportedGames::ModernWarfare3: return GameModernWarfare3::ReadXModel(Model); break;
    case SupportedGames::ModernWarfare4: return GameModernWarfare4::ReadXModel(Model); break;
    case SupportedGames::ModernWarfare5: return GameModernWarfare5::ReadXModel(Model); break;
    case SupportedGames::ModernWarfare6: return GameModernWarfare6::ReadXModel(Model); break;
    case SupportedGames::Ghosts: return GameGhosts::ReadXModel(Model); break;
    case SupportedGames::AdvancedWarfare: return GameAdvancedWarfare::ReadXModel(Model); break;
    case SupportedGames::ModernWarfareRemastered: return GameModernWarfareRM::ReadXModel(Model); break;
    case SupportedGames::ModernWarfare2Remastered: return GameModernWarfare2RM::ReadXModel(Model); break;
    case SupportedGames::InfiniteWarfare: return GameInfiniteWarfare::ReadXModel(Model); break;
    case SupportedGames::WorldWar2: return GameWorldWar2::ReadXModel(Model); break;
    case SupportedGames::Vanguard: return GameVanguard::ReadXModel(Model); break;
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
    // Check it
    if (SettingsManager::GetSetting("export_castanim") == "true" && !FileSystems::FileExists(ExportPath + ".cast"))
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
    // Check it
    if (SettingsManager::GetSetting("export_gltf") == "true" && !FileSystems::FileExists(ExportPath + ".gltf"))
        Result = true;
    // Check it
    if (SettingsManager::GetSetting("export_castmdl") == "true" && !FileSystems::FileExists(ExportPath + ".cast"))
        Result = true;
    //// Check it
    //if (SettingsManager::GetSetting("export_fbx") == "true" && !FileSystems::FileExists(ExportPath + ".fbx"))
    //    Result = true;

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
    // Check if we want material folders
    auto ExportMaterialFolders = SettingsManager::GetSetting("mdlmtlfolders") == "true";


    // Only create if Model Images are enabled
    if (ExportImages)
    {
        // Create if not exists
        FileSystems::CreateDirectory(ImagesPath);
    }

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
                auto CompleteImagesPath = ImagesPath;
                auto CompleteImageRelativePath = ImageRelativePath;

                // Check if we want material folders
                if (ExportMaterialFolders && ExportImages)
                {
                    // Create a new Folder
                    CompleteImagesPath = FileSystems::CombinePath(ImagesPath, Material.MaterialName);
                    CompleteImageRelativePath = FileSystems::CombinePath(ImageRelativePath, Material.MaterialName) + "\\\\";
                    // Create if not exists
                    FileSystems::CreateDirectory(CompleteImagesPath);
                }

                // Export image names if needed
                if (ExportImageNames)
                {
                    // Process Image Names
                    ExportMaterialImageNames(Material, ExportPath);
                }
                if (ExportImages)
                {
                    // Process the material
                    ExportMaterialImages(Material, CompleteImagesPath, ImageExtension, ImageFormatType);
                }

                // Apply image paths
                for (auto& Image : Material.Images)
                {
                    // Append the relative path and image extension here, since we are done with these images
                    Image.ImageName = CompleteImageRelativePath + Image.ImageName + ImageExtension;
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
            const auto BiggestLodIndex = CoDXModelTranslator::CalculateBiggestLodIndex(GenericModel);

            // If the biggest > -1 translate
            if (BiggestLodIndex > -1)
            {
                // Default lod index suffix using 0
                std::string LodIndexSuffix = "_LOD0";

                // If we are using the "match game lod index" setting
                if (SettingsManager::GetSetting("match_game_lod_index", "false") == "true")
                {
                    LodIndexSuffix = Strings::Format("_LOD%d", BiggestLodIndex);
                }

                // Check if we should not export this model (files already exist)
                if (ShouldExportModel(FileSystems::CombinePath(ExportPath, Model->AssetName + LodIndexSuffix)))
                {
                    // Translate generic model to a WraithModel, then export
                    const auto Result = CoDXModelTranslator::TranslateXModel(GenericModel, BiggestLodIndex);

                    // Apply lod name (_LODx)
                    Result->AssetName += LodIndexSuffix;

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
            case SupportedGames::BlackOps3:
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
            case SupportedGames::BlackOpsCW:
            case SupportedGames::ModernWarfare4:
            case SupportedGames::ModernWarfare5:
                ImageData = XPAKSupport::ReadImageFile(Image);
                break;
            }
        }
        else
        {
            // Read from game
            switch (CoDAssets::GameID)
            {
            case SupportedGames::BlackOps3: ImageData                = GameBlackOps3::ReadXImage(Image); break;
            case SupportedGames::BlackOps4: ImageData                = GameBlackOps4::ReadXImage(Image); break;
            case SupportedGames::BlackOpsCW: ImageData               = GameBlackOpsCW::ReadXImage(Image); break;
            case SupportedGames::Ghosts: ImageData                   = GameGhosts::ReadXImage(Image); break;
            case SupportedGames::AdvancedWarfare: ImageData          = GameAdvancedWarfare::ReadXImage(Image); break;
            case SupportedGames::ModernWarfareRemastered: ImageData  = GameModernWarfareRM::ReadXImage(Image); break;
            case SupportedGames::ModernWarfare2Remastered: ImageData = GameModernWarfare2RM::ReadXImage(Image); break;
            case SupportedGames::InfiniteWarfare: ImageData          = GameInfiniteWarfare::ReadXImage(Image); break;
            case SupportedGames::ModernWarfare4: ImageData           = GameModernWarfare4::ReadXImage(Image); break;
            case SupportedGames::ModernWarfare5: ImageData           = GameModernWarfare5::ReadXImage(Image); break;
            case SupportedGames::ModernWarfare6: ImageData           = GameModernWarfare6::ReadXImage(Image); break;
            case SupportedGames::WorldWar2: ImageData                = GameWorldWar2::ReadXImage(Image); break;
            case SupportedGames::Vanguard: ImageData                 = GameVanguard::ReadXImage(Image); break;
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
        case SupportedGames::BlackOpsCW:
            SoundData = GameBlackOpsCW::ReadXSound(Sound);
            break;
        case SupportedGames::Ghosts:
        case SupportedGames::AdvancedWarfare:
        case SupportedGames::ModernWarfareRemastered:
        case SupportedGames::ModernWarfare2Remastered:
        case SupportedGames::WorldWar2:
        case SupportedGames::WorldAtWar:
        case SupportedGames::ModernWarfare:
        case SupportedGames::ModernWarfare2:
        case SupportedGames::ModernWarfare3:
            SoundData = GameWorldWar2::ReadXSound(Sound);
            break;
        case SupportedGames::ModernWarfare4:
            if(Sound->IsFileEntry)
                SoundData = SABSupport::LoadOpusSound(Sound);
            else
                SoundData = GameVanguard::ReadXSound(Sound);
            break;
        case SupportedGames::ModernWarfare5:
            if (Sound->IsFileEntry)
                SoundData = SABSupport::LoadOpusSound(Sound);
            else
                SoundData = GameModernWarfare5::ReadXSound(Sound);
            break;
        case SupportedGames::ModernWarfare6:
            SoundData = GameModernWarfare6::ReadXSound(Sound);
            break;
        case SupportedGames::Vanguard:
            if (Sound->IsFileEntry)
                SoundData = SABSupport::LoadOpusSound(Sound);
            else
                SoundData = GameVanguard::ReadXSound(Sound);
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
            if (((SoundData->DataType == SoundDataTypes::FLAC_WithHeader && SoundFormatType == SoundFormat::Standard_FLAC) || SoundData->DataType == SoundDataTypes::WAV_WithHeader && SoundFormatType == SoundFormat::Standard_WAV) || CoDAssets::GameID == SupportedGames::WorldAtWar)
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
    case SupportedGames::ModernWarfare4:
        // Send to MW Raw File Extractor (SAB Files)
        GameModernWarfare4::TranslateRawfile(Rawfile, ExportPath);
        break;
    case SupportedGames::ModernWarfare5:
        // Send to MW Raw File Extractor (SAB Files)
        GameModernWarfare5::TranslateRawfile(Rawfile, ExportPath);
        break;
    case SupportedGames::Vanguard:
        // Send to VG Raw File Extractor (SAB Files)
        GameVanguard::TranslateRawfile(Rawfile, ExportPath);
        break;
    }

    // Success, unless specific error
    return ExportGameResult::Success;
}

ExportGameResult CoDAssets::ExportMaterialAsset(const CoDMaterial_t* Material, const std::string& ExportPath, const std::string& ImagesPath, const std::string& ImageRelativePath, const std::string& ImageExtension)
{
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

    XMaterial_t XMaterial(0);

    // Attempt to load it based on game
    switch (CoDAssets::GameID)
    {
    case SupportedGames::ModernWarfareRemastered:
        XMaterial = GameModernWarfareRM::ReadXMaterial(Material->AssetPointer);
        break;
    case SupportedGames::BlackOps4:
        XMaterial = GameBlackOps4::ReadXMaterial(Material->AssetPointer);
        break;
    case SupportedGames::ModernWarfare4:
        XMaterial = GameModernWarfare4::ReadXMaterial(Material->AssetPointer);
        break;
    case SupportedGames::ModernWarfare5:
        XMaterial = GameModernWarfare5::ReadXMaterial(Material->AssetPointer);
        break;
    case SupportedGames::ModernWarfare6:
        XMaterial = GameModernWarfare6::ReadXMaterial(Material->AssetPointer);
        break;
    case SupportedGames::BlackOpsCW:
        XMaterial = GameBlackOpsCW::ReadXMaterial(Material->AssetPointer);
        break;
    case SupportedGames::Vanguard:
        XMaterial = GameVanguard::ReadXMaterial(Material->AssetPointer);
        break;
    case SupportedGames::WorldWar2:
        XMaterial = GameWorldWar2::ReadXMaterial(Material->AssetPointer);
        break;
    }

    // Process Image Names
    ExportMaterialImageNames(XMaterial, ExportPath);
    // Process the material
    ExportMaterialImages(XMaterial, ExportPath, ImageExtension, ImageFormatType);

    // Success, unless specific error
    return ExportGameResult::Success;
}

void CoDAssets::ExportWraithModel(const std::unique_ptr<WraithModel>& Model, const std::string& ExportPath)
{
    // Write Cosmetic List
    TextWriter Cosmetics;
    Cosmetics.Create(FileSystems::CombinePath(ExportPath, Model->AssetName + "_cosmetics.mel"));

    for (auto& Bone : Model->Bones)
    {
        if (Bone.IsCosmetic)
        {
            Cosmetics.WriteLineFmt("select -add %s;", Bone.TagName.c_str());
        }
    }

    // Prepare to export to the formats specified in settings

    // Check for XME format
    if (SettingsManager::GetSetting("export_xmexport") == "true")
    {
        // Export a XME file
        CodXME::ExportXME(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".XMODEL_EXPORT"));
    }
    // Check for XMB format
    if (SettingsManager::GetSetting("export_xmbin") == "true")
    {
        // Export a XMB file
        CodXMB::ExportXMB(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".XMODEL_BIN"));
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
    // Check for GLTF format
    if (SettingsManager::GetSetting("export_gltf") == "true")
    {
        // Export a SEModel file
        GLTF::ExportGLTF(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".gltf"));
    }
    // Check for SEModel format
    if (SettingsManager::GetSetting("export_semodel") == "true")
    {
        // Export a SEModel file
        SEModel::ExportSEModel(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".semodel"));
    }
    // Check for Cast format
    if (SettingsManager::GetSetting("export_castmdl") == "true")
    {
        // Export a Cast file
        Cast::ExportCastModel(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".cast"));
    }
    // Check for FBX format
    if (SettingsManager::GetSetting("export_fbx") == "true")
    {
        // Export an FBX file
        // FBX::ExportFBX(*Model.get(), FileSystems::CombinePath(ExportPath, Model->AssetName + ".fbx"));
    }
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

    // Check if even loaded
    if (OnDemandCache != nullptr)
    {
        // Wait until load completes
        OnDemandCache->WaitForPackageCacheLoad();

        // Clean up
        OnDemandCache.reset();
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

    // Clear global lists
    AssetNameCache.NameDatabase.clear();
    StringCache.NameDatabase.clear();

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

    // Clean Up log
    if (XAssetLogWriter != nullptr)
    {
        XAssetLogWriter.reset();
    }

    // Clean up Parasyte
    ps::state = nullptr;
}

void CoDAssets::LogXAsset(const std::string& Type, const std::string& Name)
{
    if (XAssetLogWriter != nullptr && XAssetLogWriter->IsOpen())
    {
        XAssetLogWriter->WriteLineFmt("%s,%s", Type.c_str(), Name.c_str());
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
        // Write header
        //ImageNames.WriteLineFmt("# Material: %s", Material.MaterialName.c_str());
        //ImageNames.WriteLineFmt("# Techset/Type: %s", Material.TechsetName.c_str());
        ImageNames.WriteLine("semantic,image_name");
        // Write each name
        for (auto& Image : Material.Images)
        {
            if (SemanticHashes.find(Image.SemanticHash) != SemanticHashes.end())
            {
                ImageNames.WriteLineFmt("%s,%s", SemanticHashes[Image.SemanticHash].c_str(), Image.ImageName.c_str());
            }
            else
            {
                ImageNames.WriteLineFmt("unk_semantic_0x%X,%s", Image.SemanticHash, Image.ImageName.c_str());
            }
        }

        // Check if we have settings
        if (Material.Settings.size() > 0)
        {
            // Image Names Output
            TextWriter Settings;

            // Get File Name
            auto SettingsPath = FileSystems::CombinePath(ExportPath, Material.MaterialName + "_settings.txt");

            // Create File
            Settings.Create(SettingsPath);

            // Write header
            Settings.WriteLineFmt("# Material: %s", Material.MaterialName.c_str());
            Settings.WriteLineFmt("# Techset/Type: %s", Material.TechsetName.c_str());
            Settings.WriteLine("name,type,x,y,z,w");

            // Write each name
            for (auto& Setting : Material.Settings)
            {
                Settings.WriteLineFmt("%s,%s,%f,%f,%f,%f",
                    Setting.Name.c_str(),
                    Setting.Type.c_str(),
                    Setting.Data[0],
                    Setting.Data[1],
                    Setting.Data[2],
                    Setting.Data[3]);
            }
        }
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
    
#if _DEBUG
    // Clamp it, no less than 1, no more than 3
    auto DegreeOfConverter = 1;
#else
    // Clamp it, no less than 1, no more than 3
    auto DegreeOfConverter = VectorMath::Clamp<uint32_t>(NumberOfCores, 1, 3);
#endif
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
#ifndef DEBUG
            if (Asset->AssetStatus != WraithAssetStatus::Placeholder)
#endif
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
                    CoDAssets::Log->info("Exporting: {0}...", Asset->AssetName);
                    Result = CoDAssets::ExportAsset(Asset);
                    CoDAssets::Log->info("Successfully exported: {0}", Asset->AssetName);
                }
                catch (std::exception& ex)
                {
                    CoDAssets::Log->error(ex.what());
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
