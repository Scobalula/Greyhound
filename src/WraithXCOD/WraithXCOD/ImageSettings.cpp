#include "stdafx.h"

// The class we are implementing
#include "ImageSettings.h"

// We need the Wraith theme and settings classes
#include "WraithTheme.h"
#include "SettingsManager.h"

// We need the following Wraith classes
#include "Strings.h"

BEGIN_MESSAGE_MAP(ImageSettings, WraithWindow)
    ON_COMMAND(IDC_REBUILDNORMAL, OnRebuildNormal)
    ON_COMMAND(IDC_REBUILDCOLOR, OnRebuildColor)
    ON_COMMAND(IDC_SKIPPREVIMG, OnSkipPrevImg)
    ON_CBN_SELENDOK(IDC_IMAGEFORMAT, OnImageFormat)
END_MESSAGE_MAP()

void ImageSettings::OnBeforeLoad()
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
        L"Microsoft Sans Serif");       // Name

    // Set it
    GetDlgItem(IDC_TITLE)->SetFont(&TitleFont);
    GetDlgItem(IDC_TITLE2)->SetFont(&TitleFont);

    // Set tip
    SetControlAnchor(IDC_TIP, 0, 100, 0, 0);
    SetControlAnchor(IDC_NOTICE, 0, 100, 0, 0);

    // Load up configuration
    ((CButton*)GetDlgItem(IDC_REBUILDNORMAL))->SetCheck(SettingsManager::GetSetting("patchnormals", "true") == "true");
    ((CButton*)GetDlgItem(IDC_REBUILDCOLOR))->SetCheck(SettingsManager::GetSetting("patchcolor", "true") == "true");
    ((CButton*)GetDlgItem(IDC_SKIPPREVIMG))->SetCheck(SettingsManager::GetSetting("skipprevimg", "true") == "true");

    // Add formats
    auto ComboControl = (CComboBox*)GetDlgItem(IDC_IMAGEFORMAT);
    // Add
    ComboControl->InsertString(0, L"DDS");
    ComboControl->InsertString(1, L"PNG");
    ComboControl->InsertString(2, L"TGA");
    ComboControl->InsertString(3, L"TIFF");

    // Image settings
    auto ImageFormat = SettingsManager::GetSetting("exportimg", "PNG");
    // Apply
    if (ImageFormat == "DDS") { ComboControl->SetCurSel(0); }
    if (ImageFormat == "PNG") { ComboControl->SetCurSel(1); }
    if (ImageFormat == "TGA") { ComboControl->SetCurSel(2); }
    if (ImageFormat == "TIFF") { ComboControl->SetCurSel(3); }
}

void ImageSettings::OnRebuildNormal()
{
    // Whether or not we are checked
    bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_REBUILDNORMAL))->GetState() & BST_CHECKED) == BST_CHECKED);
    // Set it
    SettingsManager::SetSetting("patchnormals", (CheckboxChecked) ? "true" : "false");
}

void ImageSettings::OnRebuildColor()
{
    // Whether or not we are checked
    bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_REBUILDCOLOR))->GetState() & BST_CHECKED) == BST_CHECKED);
    // Set it
    SettingsManager::SetSetting("patchcolor", (CheckboxChecked) ? "true" : "false");
}

void ImageSettings::OnSkipPrevImg()
{
    // Whether or not we are checked
    bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_SKIPPREVIMG))->GetState() & BST_CHECKED) == BST_CHECKED);
    // Set it
    SettingsManager::SetSetting("skipprevimg", (CheckboxChecked) ? "true" : "false");
}

void ImageSettings::OnImageFormat()
{
    // Grab the format
    auto SelectedFormat = ((CComboBox*)GetDlgItem(IDC_IMAGEFORMAT))->GetCurSel();
    // Check and set
    switch (SelectedFormat)
    {
    case 0: SettingsManager::SetSetting("exportimg", "DDS"); break;
    case 1: SettingsManager::SetSetting("exportimg", "PNG"); break;
    case 2: SettingsManager::SetSetting("exportimg", "TGA"); break;
    case 3: SettingsManager::SetSetting("exportimg", "TIFF"); break;
    default: SettingsManager::SetSetting("exportimg", "PNG"); break;
    }
}