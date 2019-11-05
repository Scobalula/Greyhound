#include "stdafx.h"

// The class we are implementing
#include "WraithAboutDialog.h"

// We need the Wraith theme
#include "WraithTheme.h"

// We need the following Wraith classes
#include "Strings.h"

BEGIN_MESSAGE_MAP(WraithAboutDialog, WraithWindow)
    ON_COMMAND((int32_t)WraithAboutDialogID::OkControl, OnOkClick)
    ON_COMMAND((int32_t)WraithAboutDialogID::GithubControl, OnGithubClick)
END_MESSAGE_MAP()

WraithAboutDialog::WraithAboutDialog(UINT nIDTemplate, CWnd* pParent) : WraithWindow(nIDTemplate, pParent)
{
    // Defaults
}

void WraithAboutDialog::OnBeforeLoad()
{
    // Setup the about UI
    auto RichEditor = (CRichEditCtrl*)this->GetDlgItem((int32_t)WraithAboutDialogID::TextControl);
    // Set the background (Lighter from the default)
    RichEditor->SetBackgroundColor(false, RGB(WraithTheme::DefaultControlBackground.GetR(), WraithTheme::DefaultControlBackground.GetG(), WraithTheme::DefaultControlBackground.GetB()));

    // Make the rich editor read only
    RichEditor->SetReadOnly(TRUE);

    // Adjust layout
    ShiftControl((int32_t)WraithAboutDialogID::OkControl, CRect(1, 0, 0, 0));
    // Setup alignment
    SetControlAnchor((int32_t)WraithAboutDialogID::OkControl, 100, 100, 0, 0);
    SetControlAnchor((int32_t)WraithAboutDialogID::GithubControl, 0, 100, 0, 0);
    SetControlAnchor((int32_t)WraithAboutDialogID::TextControl, 0, 0, 100, 100);
}

void WraithAboutDialog::AddColorText(const CString& Text, COLORREF Color, float FontSize, bool Bold, bool Italic, bool Underline)
{
    // Get editor instance
    auto RichEditor = (CRichEditCtrl*)this->GetDlgItem((int32_t)WraithAboutDialogID::TextControl);

    // Add a color line of text
    CHARFORMAT cf = { 0 };
    int txtLen = RichEditor->GetTextLength();

    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_COLOR | CFM_SIZE;
    cf.dwEffects = (Bold ? CFE_BOLD : 0) | (Italic ? CFE_ITALIC : 0) | (Underline ? CFM_UNDERLINE : 0);
    cf.crTextColor = Color;
    cf.yHeight = (LONG)(FontSize * 15);

    // Set it and insert
    RichEditor->SetSel(txtLen, -1);
    RichEditor->SetSelectionCharFormat(cf);
    RichEditor->ReplaceSel(Text);
}

void WraithAboutDialog::OnOkClick()
{
    // Close this dialog
    this->EndDialog(0);
}

void WraithAboutDialog::OnGithubClick()
{
    // Open the Github url
    ShellExecuteA(NULL, "open", "https://github.com/Scobalula/Greyhound", NULL, NULL, SW_SHOWNORMAL);
}