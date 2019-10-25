#include <conio.h>
#include <stdio.h>
#include <memory>
#include <chrono>

// Wraith application and api (Must be included before additional includes)
#include "WraithApp.h"
#include "WraithWindow.h"
#include "WraithTheme.h"
#include "WraithX.h"
#include "Instance.h"
#include "WraithUpdate.h"

#include "MainWindow.h"

// The resource file
#include "resource.h"

// Settings manager
#include "SettingsManager.h"
#include "CoDAssets.h"
#include "FileSystems.h"

// Allow modern GUI controls
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
// The WraithX App Instance
WraithApp WraithAppInstance;

// Handler for loading theme icons from resources
HICON LoadIconResource(WraithIconAssets AssetID)
{
    // Check the ID, if we have it, we can load it
    switch (AssetID)
    {
    case WraithIconAssets::ApplicationIcon:
        // Load the application icon
        return LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAINICON));
    case WraithIconAssets::ApplicationIconLarge:
        // Load the application icon large
        return (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 64, 64, LR_SHARED);
    case WraithIconAssets::CheckboxCheckedIcon:
        // Load the checkbox checked icon
        return LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CHECKMARK));
    }
    // Failed
    return NULL;
}

// Handler for loading theme images from resources
Gdiplus::Bitmap* LoadImageResource(WraithImageAssets AssetID)
{
    // Check the ID, if we have it, we can load it
    CPngImage Image;
    // Temporary buffer for result
    CBitmap Result;

    // Check ID
    switch (AssetID)
    {
    case WraithImageAssets::SettingsNormalIcon:
        // Parse and load a PNG
        Image.Load(IDB_SETTINGSNORMAL, AfxGetInstanceHandle());
        // Converts the PNG object to a BITMAP
        Result.Attach(Image.Detach());
        // Return a GDI+ object
        return Gdiplus::Bitmap::FromHBITMAP((HBITMAP)Result, NULL);
    case WraithImageAssets::SettingsSelectedIcon:
        // Parse and load a PNG
        Image.Load(IDB_SETTINGSSELECT, AfxGetInstanceHandle());
        // Converts the PNG object to a BITMAP
        Result.Attach(Image.Detach());
        // Return a GDI+ object
        return Gdiplus::Bitmap::FromHBITMAP((HBITMAP)Result, NULL);
    case WraithImageAssets::SettingsHoverIcon:
        // Parse and load a PNG
        Image.Load(IDB_SETTINGSHOVER, AfxGetInstanceHandle());
        // Converts the PNG object to a BITMAP
        Result.Attach(Image.Detach());
        // Return a GDI+ object
        return Gdiplus::Bitmap::FromHBITMAP((HBITMAP)Result, NULL);
    }
    // Failed
    return nullptr;
}

// Cleanup old patch files before launch...
static void CleanupFilesystem()
{
    // Attempt to clean up the existing old files and folders
    auto CurrentPath = FileSystems::GetApplicationPath();

    // Attempt to delete the file
    FileSystems::DeleteFile(FileSystems::CombinePath(CurrentPath, "exhalelib.dll"));
    FileSystems::DeleteFile(FileSystems::CombinePath(CurrentPath, "latest_log.txt"));
    FileSystems::DeleteFile(FileSystems::CombinePath(CurrentPath, "settings.dat"));
    FileSystems::DeleteFile(FileSystems::CombinePath(CurrentPath, "Wraith.exe"));

    // Attempt to delete old cache
    FileSystems::DeleteDirectory(FileSystems::CombinePath(CurrentPath, "data"));
    // Attempt to delete old update cache
    FileSystems::DeleteDirectory(FileSystems::CombinePath(CurrentPath, "Temp"));
}

// Main entry point of app
#ifdef _DEBUG
int main(int argc, char** argv)
#else
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    // Hook theme callbacks
    WraithTheme::OnLoadIconResource = LoadIconResource;
    WraithTheme::OnLoadImageResource = LoadImageResource;

    // Start the instance (We must provide the main window title, never include versions from now on)
    auto CanContinue = Instance::BeginSingleInstance("Greyhound");

    // Only resume if we can
    if (CanContinue)
    {
        // Load settings, specify default
        SettingsManager::LoadSettings("greyhound",
        {
            { "exportimg", "PNG" },
            { "exportsnd", "WAV" },
            { "keepsndpath", "true" },
            { "keepsndpath", "true" },
            { "skipblankaudio", "false" },
            { "usesabindexbo4", "false" },
            { "sortbydetails", "false" },
            { "exportmodelimg", "true" },
            { "exportalllods", "false" },
            { "exporthitbox", "false" },
            { "exportvtxcolor", "false" },
            { "exportimgnames", "false"},
            { "skipprevmodel", "true"},
            { "skipprevimg", "true"},
            { "skipprevsound", "true"},
            { "skipprevanim", "true"},
            { "export_ma", "true" },
            { "export_obj", "false" },
            { "export_xna", "false" },
            { "export_smd", "false" },
            { "export_xmexport", "false" },
            { "export_xmbin", "false" },
            { "export_seanim", "true" },
            { "export_semodel", "false" },
            { "export_fbx", "false" },
            { "export_directxanim", "false" },
            { "directxanim_ver", "17" },
            { "global_images", "true" },
            { "patchnormals", "true" },
            { "patchcolor", "true" },
            { "showxanim", "true" },
            { "showxmodel", "true" },
            { "showximage", "false" },
            { "showefx", "false" },
            { "showxrawfiles", "false" },
            { "showxsounds", "false" },
            { "exportgdt_bo3", "true" },
            { "exportgdt_waw", "false" },
            { "cleargdt_exit", "true" },
            { "overwrite_gdt", "true" }
        });

        // Clean up files
        CleanupFilesystem();
        // Check for updates
        WraithUpdate::CheckForUpdates("Scobalula", "Greyhound", "Greyhound", "greyhound.exe");

        // Initialize the API (This must be done BEFORE running a WraithApp)
        if (!WraithX::InitializeAPI(true))
        {
            // Failed to initialize
            MessageBoxA(NULL, "A fatal error occured while initializing Greyhound", "Greyhound", MB_OK | MB_ICONEXCLAMATION);
            // Failed
            return -1;
        }

        // Show the main window instance
        WraithAppInstance.RunApplication(MainWindow());

        // Tell the asset cache to clean up (Prevents crash on async cache loading)
        CoDAssets::CleanUpGame();

        // Shutdown the API, we're done
        WraithX::ShutdownAPI(true);

        // Stop the instance
        Instance::EndSingleInstance();
    }

    // We're done here
    return 0;
}