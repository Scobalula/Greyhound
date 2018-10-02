#include "stdafx.h"

// The class we are implementing
#include "WraithTheme.h"

// Set default colors
Gdiplus::Color WraithTheme::DefaultForeground = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::DefaultControlBorder = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::DisabledControlBorder = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::DisabledControlFill = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::DefaultControlGradTop = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::DefaultControlGradBottom = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::DefaultFillGradTop = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::DefaultFillGradBottom = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::DefaultControlBackground = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::ListItemColorBase = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::ListItemColorAlt = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::DefaultHighlightForeground = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::DefaultHighlightBackground = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::HeaderFillGradTop = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::HeaderFillGradBottom = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::HeaderBorderColor = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::HeaderBackgroundColor = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::HeaderSeparatorColor = Gdiplus::Color(0, 0, 0);
Gdiplus::Color WraithTheme::WindowFrameColor = Gdiplus::Color(0, 0, 0);

// Set default icons
HICON WraithTheme::ApplicationIcon = NULL;
HICON WraithTheme::ApplicationIconLarge = NULL;
HICON WraithTheme::CheckboxCheckedIcon = NULL;

// Set default images
Gdiplus::Bitmap* WraithTheme::SettingsNormalImage = nullptr;
Gdiplus::Bitmap* WraithTheme::SettingsHoverImage = nullptr;
Gdiplus::Bitmap* WraithTheme::SettingsSelectedImage = nullptr;

// Setup hints
std::vector<Gdiplus::Color> WraithTheme::ColorHints = std::vector<Gdiplus::Color>();

// Setup callbacks
IconResourceHandler WraithTheme::OnLoadIconResource = nullptr;
ImageResourceHandler WraithTheme::OnLoadImageResource = nullptr;

void WraithTheme::InitializeDefaultTheme()
{
	// Set the colors
	DefaultForeground          = Gdiplus::Color(255, 255, 255, 255);	// WHITE
	DefaultControlBorder       = Gdiplus::Color(255, 30, 144, 255);		// DODGERBLUE
	DisabledControlBorder      = Gdiplus::Color(255, 169, 169, 169);	// DARKGRAY
	DisabledControlFill        = Gdiplus::Color(255, 169, 169, 169);	// DARKGRAY
	DefaultControlGradTop      = Gdiplus::Color(255, 50, 50, 50);		// GRADIENT COLOR TOP
	DefaultControlGradBottom   = Gdiplus::Color(255, 42, 42, 42);		// GRADIENT COLOR BOTTOM
	DefaultFillGradTop         = Gdiplus::Color(255, 0, 102, 255);		// GRADIENT COLOR TOP
	DefaultFillGradBottom      = Gdiplus::Color(255, 0, 51, 204);		// GRADIENT COLOR BOTTOM
	DefaultControlBackground   = Gdiplus::Color(255, 37, 37, 37);		// A DARK BLACK-GRAY COLOR
	ListItemColorBase          = Gdiplus::Color(255, 54, 54, 54);		// A DARK BLACK-GRAY COLOR
	ListItemColorAlt           = Gdiplus::Color(255, 36, 36, 36);		// A DARK BLACK-GRAY COLOR
	DefaultHighlightForeground = Gdiplus::Color(0, 0, 0);				// BLACK
	DefaultHighlightBackground = Gdiplus::Color(255, 255, 255);			// WHITE
	HeaderFillGradTop          = Gdiplus::Color(50, 50, 50);			// A DARK GRAY COLOR
	HeaderFillGradBottom       = Gdiplus::Color(42, 42, 42);			// A DARK GRAY COLOR
	HeaderBorderColor          = Gdiplus::Color(32, 32, 32);			// A DARK GRAY BORDER
	HeaderBackgroundColor      = Gdiplus::Color(48, 48, 48);			// A DARK GRAY BACK
	HeaderSeparatorColor       = Gdiplus::Color(64, 64, 64);			// A LIGHT GRAY DIVIDER
	WindowFrameColor           = Gdiplus::Color(54, 54, 54);			// A GRAY BORDER

	// Clear
	ColorHints.clear();

	// Add hints
	ColorHints.push_back(Gdiplus::Color(255, 30, 144, 255));	// DODGERBLUE (0)
	ColorHints.push_back(Gdiplus::Color(255, 255, 0, 0));		// RED (1)
	ColorHints.push_back(Gdiplus::Color(255, 50, 205, 50));		// LIMEGREEN (2)
	ColorHints.push_back(Gdiplus::Color(255, 255, 255, 50));	// YELLOW (3)
	ColorHints.push_back(Gdiplus::Color(255, 255, 165, 0));		// ORANGE (4)
	ColorHints.push_back(Gdiplus::Color(255, 182, 57, 255));	// PURPLEISH COLOR (5)

	// Run callback if available
	if (OnLoadIconResource != nullptr)
	{
		// Call and load
		ApplicationIcon = OnLoadIconResource(WraithIconAssets::ApplicationIcon);
		ApplicationIconLarge = OnLoadIconResource(WraithIconAssets::ApplicationIconLarge);
		CheckboxCheckedIcon = OnLoadIconResource(WraithIconAssets::CheckboxCheckedIcon);
	}

	// Run callback if available
	if (OnLoadImageResource != nullptr)
	{
		// Call and load
		SettingsNormalImage = OnLoadImageResource(WraithImageAssets::SettingsNormalIcon);
		SettingsHoverImage = OnLoadImageResource(WraithImageAssets::SettingsHoverIcon);
		SettingsSelectedImage = OnLoadImageResource(WraithImageAssets::SettingsSelectedIcon);
	}
}

void WraithTheme::UnloadThemeResources()
{
	// Delete icons if loaded
	if (ApplicationIcon != NULL)
	{
		// Destroy and mark
		DestroyIcon(ApplicationIcon);
		ApplicationIcon = NULL;
	}
	if (CheckboxCheckedIcon != NULL)
	{
		// Destroy and mark
		DestroyIcon(CheckboxCheckedIcon);
		CheckboxCheckedIcon = NULL;
	}
	if (ApplicationIconLarge != NULL)
	{
		// Destroy and mark
		DestroyIcon(ApplicationIconLarge);
		ApplicationIconLarge = NULL;
	}

	// Delete images if loaded
	if (SettingsNormalImage != nullptr)
	{
		// Delete
		delete SettingsNormalImage;
		SettingsNormalImage = nullptr;
	}
	if (SettingsHoverImage != nullptr)
	{
		// Delete
		delete SettingsHoverImage;
		SettingsHoverImage = nullptr;
	}
	if (SettingsSelectedImage != nullptr)
	{
		// Delete
		delete SettingsSelectedImage;
		SettingsHoverImage = nullptr;
	}
}