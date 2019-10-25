#include "stdafx.h"

// The class we are implementing
#include "ModelSettings.h"

// We need the Wraith theme and settings classes
#include "WraithTheme.h"
#include "SettingsManager.h"

// We need the following Wraith classes
#include "Strings.h"

BEGIN_MESSAGE_MAP(ModelSettings, WraithWindow)
	ON_COMMAND(IDC_GLOBALIMG, OnGlobalImages)
	ON_COMMAND(IDC_EXPORTIMG, OnExportImages)
	ON_COMMAND(IDC_EXPORTALLLOD, OnExportAllLods)
	ON_COMMAND(IDC_EXPORTHITBOX, OnExportHitbox)
	ON_COMMAND(IDC_EXPORTVTXCOLOR, OnExportVTXColor)
	ON_COMMAND(IDC_EXPORTIMGNAMES, OnExportIMGNames)
	ON_COMMAND(IDC_SKIPPREVMODEL, OnSkipPrevModels)
	ON_COMMAND(IDC_EXPORTMA, OnExportMA)
	ON_COMMAND(IDC_EXPORTOBJ, OnExportOBJ)
	ON_COMMAND(IDC_EXPORTXNA, OnExportXNA)
	ON_COMMAND(IDC_EXPORTSMD, OnExportSMD)
	ON_COMMAND(IDC_EXPORTXME, OnExportXME)
	ON_COMMAND(IDC_EXPORTSEMODEL, OnExportSEModel)
	ON_COMMAND(IDC_EXPORTFBX, OnExportFBX)
END_MESSAGE_MAP()

void ModelSettings::OnBeforeLoad()
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
	GetDlgItem(IDC_TITLE2)->SetFont(&TitleFont);

	// Set tip
	SetControlAnchor(IDC_TIP, 0, 100, 0, 0);
	SetControlAnchor(IDC_NOTICE, 0, 100, 0, 0);

	// Load up configuration
	((CButton*)GetDlgItem(IDC_GLOBALIMG))->SetCheck(SettingsManager::GetSetting("global_images", "true") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTIMG))->SetCheck(SettingsManager::GetSetting("exportmodelimg", "true") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTALLLOD))->SetCheck(SettingsManager::GetSetting("exportalllods", "false") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTHITBOX))->SetCheck(SettingsManager::GetSetting("exporthitbox", "false") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTVTXCOLOR))->SetCheck(SettingsManager::GetSetting("exportvtxcolor", "false") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTIMGNAMES))->SetCheck(SettingsManager::GetSetting("exportimgnames", "false") == "true");
	((CButton*)GetDlgItem(IDC_SKIPPREVMODEL))->SetCheck(SettingsManager::GetSetting("skipprevmodel", "true") == "true");

	((CButton*)GetDlgItem(IDC_EXPORTMA))->SetCheck(SettingsManager::GetSetting("export_ma", "true") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTOBJ))->SetCheck(SettingsManager::GetSetting("export_obj", "false") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTXNA))->SetCheck(SettingsManager::GetSetting("export_xna", "false") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTSMD))->SetCheck(SettingsManager::GetSetting("export_smd", "false") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTXME))->SetCheck(SettingsManager::GetSetting("export_xmexport", "false") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTSEMODEL))->SetCheck(SettingsManager::GetSetting("export_semodel", "false") == "true");
	((CButton*)GetDlgItem(IDC_EXPORTFBX))->SetCheck(SettingsManager::GetSetting("export_fbx", "false") == "true");
}

void ModelSettings::OnGlobalImages()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_GLOBALIMG))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("global_images", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportImages()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTIMG))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("exportmodelimg", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportAllLods()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTALLLOD))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("exportalllods", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportHitbox()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTHITBOX))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("exporthitbox", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportMA()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTMA))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("export_ma", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportOBJ()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTOBJ))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("export_obj", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportXNA()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTXNA))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("export_xna", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportSMD()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTSMD))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("export_smd", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportXME()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTXME))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("export_xmexport", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportSEModel()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTSEMODEL))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("export_semodel", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportFBX()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTFBX))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("export_fbx", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportVTXColor()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTVTXCOLOR))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("exportvtxcolor", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnExportIMGNames()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_EXPORTIMGNAMES))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("exportimgnames", (CheckboxChecked) ? "true" : "false");
}

void ModelSettings::OnSkipPrevModels()
{
	// Whether or not we are checked
	bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_SKIPPREVMODEL))->GetState() & BST_CHECKED) == BST_CHECKED);
	// Set it
	SettingsManager::SetSetting("skipprevmodel", (CheckboxChecked) ? "true" : "false");
}
