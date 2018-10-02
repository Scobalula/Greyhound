#include "stdafx.h"

// The class we are implementing
#include "GDTSettings.h"

// We need the Wraith theme and settings classes
#include "WraithTheme.h"
#include "SettingsManager.h"
#include "CoDAssets.h"
#include "FileSystems.h"

// We need the following Wraith classes
#include "Strings.h"

BEGIN_MESSAGE_MAP(GDTSettings, WraithWindow)
	ON_COMMAND(IDC_BO3GDT, OnBO3GDT)
	ON_COMMAND(IDC_WAWGDT, OnWAWGDT)
	ON_COMMAND(IDC_CLEARCACHECLOSE, OnCacheClose)
	ON_COMMAND(IDC_OVERWRITEGDT, OnOverwriteGDT)
	ON_COMMAND(IDC_CLEARCACHE, OnClearCache)
END_MESSAGE_MAP()

void GDTSettings::OnBeforeLoad()
{
	// Shift
	ShiftControl(IDC_NOTICE, CRect(0, 1, 0, 0));

	// Make it
	TitleFont.CreateFont(20,           // Height
		0,                             // Width
		0,                             // Escapement
		0,                             // Orientation
		FW_NORMAL,                     // Weight
		FALSE,                         // Italic
		FALSE,                         // Underline
		0,                             // StrikeOut
		ANSI_CHARSET,                  // CharSet
		OUT_DEFAULT_PRECIS,            // OutPrecision
		CLIP_DEFAULT_PRECIS,           // ClipPrecision
		DEFAULT_QUALITY,               // Quality
		DEFAULT_PITCH | FF_SWISS,      // PitchAndFamily
		L"Microsoft Sans Serif");	   // Name

	// Set it
	GetDlgItem(IDC_TITLE)->SetFont(&TitleFont);

	// Set tip
	SetControlAnchor(IDC_TIP, 0, 100, 0, 0);
	SetControlAnchor(IDC_NOTICE, 0, 100, 0, 0);
	SetControlAnchor(IDC_CLEARCACHE, 100, 0, 0, 0);

	// Load up configuration
	((CButton*)GetDlgItem(IDC_BO3GDT))->SetCheck(SettingsManager::GetSetting("exportgdt_bo3", "true") == "true");
	((CButton*)GetDlgItem(IDC_WAWGDT))->SetCheck(SettingsManager::GetSetting("exportgdt_waw", "false") == "true");
	((CButton*)GetDlgItem(IDC_OVERWRITEGDT))->SetCheck(SettingsManager::GetSetting("overwrite_gdt", "true") == "true");
	((CButton*)GetDlgItem(IDC_CLEARCACHECLOSE))->SetCheck(SettingsManager::GetSetting("cleargdt_exit", "true") == "true");
}

void GDTSettings::OnBO3GDT()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_BO3GDT))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("exportgdt_bo3", (CheckboxChecked) ? "true" : "false");
}

void GDTSettings::OnWAWGDT()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_WAWGDT))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("exportgdt_waw", (CheckboxChecked) ? "true" : "false");
}

void GDTSettings::OnClearCache()
{
	// Ask to clear cache
	if (MessageBoxA(this->GetSafeHwnd(), "Are you sure you want to clear the GDT cache?", "Wraith", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
	{
		// Clear it, we must first close down the manager!
		if (CoDAssets::GameGDTProcessor != nullptr)
		{
			// Unload
			CoDAssets::GameGDTProcessor->CloseProcessor();
		}

		// Clear all cache files
		auto CacheFiles = FileSystems::GetFiles(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "gdt_cache"), "*.gdtc");
		// Clear all files
		for (auto& CacheFile : CacheFiles) { FileSystems::DeleteFile(CacheFile); }

		// Reload if possible
		if (CoDAssets::GameGDTProcessor != nullptr)
		{
			// Reload
			CoDAssets::GameGDTProcessor->ReloadProcessor();
		}
		// Alert the user
		MessageBoxA(this->GetSafeHwnd(), "The GDT cache has been cleared", "Wraith", MB_OK | MB_ICONINFORMATION);
	}
}

void GDTSettings::OnCacheClose()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_CLEARCACHECLOSE))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("cleargdt_exit", (CheckboxChecked) ? "true" : "false");
}

void GDTSettings::OnOverwriteGDT()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_OVERWRITEGDT))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("overwrite_gdt", (CheckboxChecked) ? "true" : "false");
}