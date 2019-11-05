#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxtoolbarimages.h>
#include <string>
#include <gdiplus.h>
#include <vector>

// A list of icon assets for Wraith controls
enum class WraithIconAssets
{
    // The icon that appears on the executable
    ApplicationIcon,
    // A larger version of the app icon for display
    ApplicationIconLarge,
    // The icon that appears when a checkbox has been checked off
    CheckboxCheckedIcon
};

// A list of image assets for Wraith controls
enum class WraithImageAssets
{
    // The normal state icon for settings
    SettingsNormalIcon,
    // The mouse over state icon for settings
    SettingsHoverIcon,
    // The selected state icon for settings
    SettingsSelectedIcon
};

// The callback function for loading a resource
typedef HICON(*IconResourceHandler)(WraithIconAssets AssetID);
typedef Gdiplus::Bitmap*(*ImageResourceHandler)(WraithImageAssets AssetID);

// A class that handles the WraithTheme
class WraithTheme
{
public:
    // -- Common theme colors

    static Gdiplus::Color DefaultForeground;
    static Gdiplus::Color DefaultControlBorder;
    static Gdiplus::Color DisabledControlBorder;
    static Gdiplus::Color DisabledControlFill;
    static Gdiplus::Color DefaultControlGradTop;
    static Gdiplus::Color DefaultControlGradBottom;
    static Gdiplus::Color DefaultFillGradTop;
    static Gdiplus::Color DefaultFillGradBottom;
    static Gdiplus::Color DefaultControlBackground;
    static Gdiplus::Color ListItemColorBase;
    static Gdiplus::Color ListItemColorAlt;
    static Gdiplus::Color DefaultHighlightForeground;
    static Gdiplus::Color DefaultHighlightBackground;
    static Gdiplus::Color HeaderFillGradTop;
    static Gdiplus::Color HeaderFillGradBottom;
    static Gdiplus::Color HeaderBorderColor;
    static Gdiplus::Color HeaderBackgroundColor;
    static Gdiplus::Color HeaderSeparatorColor;
    static Gdiplus::Color WindowFrameColor;

    // -- Theme color hints

    static std::vector<Gdiplus::Color> ColorHints;

    // -- Theme icons

    static HICON ApplicationIcon;
    static HICON ApplicationIconLarge;
    static HICON CheckboxCheckedIcon;

    // -- Theme images

    static Gdiplus::Bitmap* SettingsNormalImage;
    static Gdiplus::Bitmap* SettingsHoverImage;
    static Gdiplus::Bitmap* SettingsSelectedImage;

    // -- Functions

    // Load the default Wraith theme, a green / gray color scheme
    static void InitializeDefaultTheme();

    // Unload theme resources
    static void UnloadThemeResources();

    // -- Callbacks

    static IconResourceHandler OnLoadIconResource;
    static ImageResourceHandler OnLoadImageResource;
};