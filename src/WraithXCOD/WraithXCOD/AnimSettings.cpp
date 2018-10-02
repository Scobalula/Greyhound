#include "stdafx.h"

// The class we are implementing
#include "AnimSettings.h"

// We need the Wraith theme and settings classes
#include "WraithTheme.h"
#include "SettingsManager.h"

// We need the following Wraith classes
#include "Strings.h"

BEGIN_MESSAGE_MAP(AnimSettings, WraithWindow)
	ON_COMMAND(IDC_EXPORTSEANIM, OnExportSEAnim)
	ON_COMMAND(IDC_EXPORTDIRECTX, OnExportDirect)
	ON_COMMAND(IDC_WAWCOMPAT, OnWAWCompat)
	ON_COMMAND(IDC_BO1COMPAT, OnBOCompat)
END_MESSAGE_MAP()

void AnimSettings::OnBeforeLoad()
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

	// Load up configuration
	((CButton*)GetDlgItem(IDC_EXPORTSEANIM))->SetCheck(SettingsManager::GetSetting("export_seanim", "true") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTDIRECTX))->SetCheck(SettingsManager::GetSetting("export_directxanim", "true") == "true");

	// Get directxanim version
	auto DirectXAnimVersion = SettingsManager::GetSetting("directxanim_ver", "17");

	// Check and set
	if (DirectXAnimVersion == "17")
	{
		((CButton*)GetDlgItem(IDC_WAWCOMPAT))->SetCheck(true);
		((CButton*)GetDlgItem(IDC_BO1COMPAT))->SetCheck(false);
	}
	else if (DirectXAnimVersion == "19")
	{
		((CButton*)GetDlgItem(IDC_WAWCOMPAT))->SetCheck(false);
		((CButton*)GetDlgItem(IDC_BO1COMPAT))->SetCheck(true);
	}
}

void AnimSettings::OnExportSEAnim()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTSEANIM))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("export_seanim", (CheckboxChecked) ? "true" : "false");
}

void AnimSettings::OnExportDirect()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTDIRECTX))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("export_directxanim", (CheckboxChecked) ? "true" : "false");
}

void AnimSettings::OnWAWCompat()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_WAWCOMPAT))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("directxanim_ver", (CheckboxChecked) ? "17" : "19");
}

void AnimSettings::OnBOCompat()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_BO1COMPAT))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("directxanim_ver", (CheckboxChecked) ? "19" : "17");
}