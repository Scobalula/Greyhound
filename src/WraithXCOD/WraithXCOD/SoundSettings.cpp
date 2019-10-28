#include "stdafx.h"

// The class we are implementing
#include "SoundSettings.h"

// We need the Wraith theme and settings classes
#include "WraithTheme.h"
#include "SettingsManager.h"

// We need the following Wraith classes
#include "Strings.h"

BEGIN_MESSAGE_MAP(SoundSettings, WraithWindow)
    ON_COMMAND(IDC_KEEPPATHS, OnKeepPaths)
    ON_COMMAND(IDC_USESABINDEXBO4, OnUseSabIndexBo4)
    ON_COMMAND(IDC_SKIPBLANKAUDIO, OnSkipBlankAudio)
    ON_COMMAND(IDC_SKIPPREVSND, OnSkipPrevSound)
    ON_CBN_SELENDOK(IDC_SOUNDFORMAT, OnSoundFormat)
END_MESSAGE_MAP()

void SoundSettings::OnBeforeLoad()
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
    ((CButton*)GetDlgItem(IDC_KEEPPATHS))->SetCheck(SettingsManager::GetSetting("keepsndpath", "true") == "true");
    ((CButton*)GetDlgItem(IDC_SKIPPREVSND))->SetCheck(SettingsManager::GetSetting("skipprevsound", "true") == "true");
    ((CButton*)GetDlgItem(IDC_USESABINDEXBO4))->SetCheck(SettingsManager::GetSetting("usesabindexbo4", "false") == "true");
    ((CButton*)GetDlgItem(IDC_SKIPBLANKAUDIO))->SetCheck(SettingsManager::GetSetting("skipblankaudio", "false") == "true");

    // Add formats
    auto ComboControl = (CComboBox*)GetDlgItem(IDC_SOUNDFORMAT);
    // Add
    ComboControl->InsertString(0, L"WAV");
    ComboControl->InsertString(1, L"FLAC");

    // Image settings
    auto ImageFormat = SettingsManager::GetSetting("exportsnd", "WAV");
    // Apply
    if (ImageFormat == "WAV") { ComboControl->SetCurSel(0); }
    if (ImageFormat == "FLAC") { ComboControl->SetCurSel(1); }
}

void SoundSettings::OnKeepPaths()
{
    // Whether or not we are checked
    bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_KEEPPATHS))->GetState() & BST_CHECKED) == BST_CHECKED);
    // Set it
    SettingsManager::SetSetting("keepsndpath", (CheckboxChecked) ? "true" : "false");
}

void SoundSettings::OnSkipPrevSound()
{
    // Whether or not we are checked
    bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_SKIPPREVSND))->GetState() & BST_CHECKED) == BST_CHECKED);
    // Set it
    SettingsManager::SetSetting("skipprevsound", (CheckboxChecked) ? "true" : "false");
}

void SoundSettings::OnSkipBlankAudio()
{
    // Whether or not we are checked
    bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_SKIPBLANKAUDIO))->GetState() & BST_CHECKED) == BST_CHECKED);
    // Set it
    SettingsManager::SetSetting("skipblankaudio", (CheckboxChecked) ? "true" : "false");
}

void SoundSettings::OnUseSabIndexBo4()
{
    // Whether or not we are checked
    bool CheckboxChecked = ((((CButton*)GetDlgItem(IDC_USESABINDEXBO4))->GetState() & BST_CHECKED) == BST_CHECKED);
    // Set it
    SettingsManager::SetSetting("usesabindexbo4", (CheckboxChecked) ? "true" : "false");
}

void SoundSettings::OnSoundFormat()
{
    // Grab the format
    auto SelectedFormat = ((CComboBox*)GetDlgItem(IDC_SOUNDFORMAT))->GetCurSel();
    // Check and set
    switch (SelectedFormat)
    {
    case 0: SettingsManager::SetSetting("exportsnd", "WAV"); break;
    case 1: SettingsManager::SetSetting("exportsnd", "FLAC"); break;
    default: SettingsManager::SetSetting("exportsnd", "WAV"); break;
    }
}