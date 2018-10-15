#include "stdafx.h"

// The class we are implementing
#include "GeneralSettings.h"

// We need the Wraith theme and settings classes
#include "WraithTheme.h"
#include "SettingsManager.h"

// We need the following Wraith classes
#include "Strings.h"

BEGIN_MESSAGE_MAP(GeneralSettings, WraithWindow)
	ON_COMMAND(IDC_SHOWXMODEL, OnXModels)
	ON_COMMAND(IDC_SHOWXANIM, OnXAnims)
	ON_COMMAND(IDC_SHOWXIMAGE, OnXImages)
	ON_COMMAND(IDC_SHOWEFFECTS, OnXEffects)
	ON_COMMAND(IDC_SHOWXRAW, OnXRawFiles)
	ON_COMMAND(IDC_SHOWXSOUNDS, OnXSounds)
	ON_COMMAND(IDC_SORTBYDATA, OnSortByData)
END_MESSAGE_MAP()

void GeneralSettings::OnBeforeLoad()
{
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

	// Load up configuration
	((CButton*)GetDlgItem(IDC_SHOWXMODEL))->SetCheck(SettingsManager::GetSetting("showxmodel", "true") == "true");
	((CButton*)GetDlgItem(IDC_SHOWXANIM))->SetCheck(SettingsManager::GetSetting("showxanim", "true") == "true");
	((CButton*)GetDlgItem(IDC_SHOWXIMAGE))->SetCheck(SettingsManager::GetSetting("showximage", "false") == "true");
	((CButton*)GetDlgItem(IDC_SHOWEFFECTS))->SetCheck(SettingsManager::GetSetting("showefx", "false") == "true");
	((CButton*)GetDlgItem(IDC_SHOWXRAW))->SetCheck(SettingsManager::GetSetting("showxrawfiles", "false") == "true");
	((CButton*)GetDlgItem(IDC_SHOWXSOUNDS))->SetCheck(SettingsManager::GetSetting("showxsounds", "false") == "true");
	((CButton*)GetDlgItem(IDC_SORTBYDATA))->SetCheck(SettingsManager::GetSetting("sortbydetails", "false") == "true");
}

void GeneralSettings::OnXModels()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_SHOWXMODEL))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("showxmodel", (CheckboxChecked) ? "true" : "false");
}

void GeneralSettings::OnXAnims()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_SHOWXANIM))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("showxanim", (CheckboxChecked) ? "true" : "false");
}

void GeneralSettings::OnXImages()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_SHOWXIMAGE))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("showximage", (CheckboxChecked) ? "true" : "false");
}

void GeneralSettings::OnXEffects()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_SHOWEFFECTS))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("showefx", (CheckboxChecked) ? "true" : "false");
}

void GeneralSettings::OnXRawFiles()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_SHOWXRAW))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("showxrawfiles", (CheckboxChecked) ? "true" : "false");
}

void GeneralSettings::OnXSounds()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_SHOWXSOUNDS))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("showxsounds", (CheckboxChecked) ? "true" : "false");
}

void GeneralSettings::OnSortByData()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_SORTBYDATA))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("sortbydetails", (CheckboxChecked) ? "true" : "false");
}