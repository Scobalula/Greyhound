#include "stdafx.h"

// The class we are implementing
#include "WraithProgressDialog.h"

// We need the following Wraith classes
#include "Strings.h"

BEGIN_MESSAGE_MAP(WraithProgressDialog, WraithWindow)
    ON_WM_CLOSE()
    ON_WM_NCACTIVATE()
    ON_COMMAND((int32_t)WraithProgressDialogID::OkControl, OkClick)
    ON_COMMAND((int32_t)WraithProgressDialogID::CancelControl, CancelClick)
END_MESSAGE_MAP()

WraithProgressDialog::WraithProgressDialog(UINT nIDTemplate, CWnd* pParent) : WraithWindow(nIDTemplate, pParent)
{
    // Defaults
    IsSetup = false;
    CanClose = true;
    OnOkClick = nullptr;
    OnCancelClick = nullptr;
    // Set the parent
    ProgressOwner = pParent;
}

void WraithProgressDialog::OnBeforeLoad()
{
    // Adjust buttons
    ShiftControl((int32_t)WraithProgressDialogID::CancelControl, CRect(1, 0, 0, 0));
    // Set anchors
    SetControlAnchor((int32_t)WraithProgressDialogID::CancelControl, 0, 100, 0, 0);        // Cancel button
    SetControlAnchor((int32_t)WraithProgressDialogID::OkControl, 0, 100, 0, 0);            // Ok button
    SetControlAnchor((int32_t)WraithProgressDialogID::ProgressControl, 0, 0, 100, 0);    // Progress bar
}

BOOL WraithProgressDialog::OnInitDialog()
{
    // Handle base
    auto Result = WraithWindow::OnInitDialog();

    // Setup title
    this->SetWindowTextW(Strings::ToUnicodeString(Title).c_str());
    // Setup status
    this->SetDlgItemTextW((int32_t)WraithProgressDialogID::WorkingTextControl, Strings::ToUnicodeString(Status).c_str());

    // Reset button text
    this->SetDlgItemTextW((int32_t)WraithProgressDialogID::CancelControl, L"Cancel");
    this->SetDlgItemTextW((int32_t)WraithProgressDialogID::OkControl, L"Ok");

    // Setup buttons
    if (OkButton)
    {
        // Make ok visible
        this->GetDlgItem((int32_t)WraithProgressDialogID::OkControl)->ShowWindow(SW_SHOW);
    }
    else
    {
        // Make ok hidden
        this->GetDlgItem((int32_t)WraithProgressDialogID::OkControl)->ShowWindow(SW_HIDE);
    }

    if (CancelButton)
    {
        // Make cancel visible
        this->GetDlgItem((int32_t)WraithProgressDialogID::CancelControl)->ShowWindow(SW_SHOW);
    }
    else
    {
        // Make cancel hidden
        this->GetDlgItem((int32_t)WraithProgressDialogID::CancelControl)->ShowWindow(SW_HIDE);
    }

    // Set progress to 0
    ((CProgressCtrl*)GetDlgItem((int32_t)WraithProgressDialogID::ProgressControl))->SetPos(0);

    // Return it
    return Result;
}

void WraithProgressDialog::SetupDialog(const std::string& DialogTitle, const std::string& DialogStatus, bool IsOkEnabled, bool IsCancelEnabled)
{
    // Set
    Title = DialogTitle;
    Status = DialogStatus;
    OkButton = IsOkEnabled;
    CancelButton = IsCancelEnabled;
    IsSetup = false;
    CanClose = true;
}

void WraithProgressDialog::UpdateStatus(const std::string& DialogStatus)
{
    try
    {
        // Set
        Status = DialogStatus;
        // Set the text
        this->SetDlgItemTextW((int32_t)WraithProgressDialogID::WorkingTextControl, Strings::ToUnicodeString(Status).c_str());
    }
    catch (...)
    {
        // We aren't loaded, or unloaded before we could set it
    }
}

void WraithProgressDialog::UpdateProgress(uint32_t Progress)
{
    try
    {
        // Set the value
        ((CProgressCtrl*)GetDlgItem((int32_t)WraithProgressDialogID::ProgressControl))->SetPos(Progress);
    }
    catch (...)
    {
        // We aren't loaded, or unloaded before we could set it
    }
}

void WraithProgressDialog::UpdateButtons(bool CanOk, bool CanCancel)
{
    // Setup the controls
    try
    {
        // Set status
        this->GetDlgItem((int32_t)WraithProgressDialogID::OkControl)->EnableWindow(CanOk);
        this->GetDlgItem((int32_t)WraithProgressDialogID::CancelControl)->EnableWindow(CanCancel);

        // Set text back
        this->SetDlgItemTextW((int32_t)WraithProgressDialogID::CancelControl, L"Cancel");
        this->SetDlgItemTextW((int32_t)WraithProgressDialogID::OkControl, L"Ok");
    }
    catch (...)
    {
        // Nothing
    }
}

void WraithProgressDialog::WaitTillReady()
{
    // Block until ready
    while (!IsSetup){ Sleep(1); }
}

void WraithProgressDialog::UpdateWindowClose(bool Close)
{
    // Set it
    CanClose = Close;
}

void WraithProgressDialog::OnClose()
{
    // Check
    if (CanClose){ WraithWindow::OnClose(); }
}

BOOL WraithProgressDialog::OnNcActivate(BOOL bActive)
{
    // Set
    IsSetup = true;
    // Handle base
    return WraithWindow::OnNcActivate(bActive);
}

void WraithProgressDialog::CloseProgress()
{
    // Close it safe
    this->PostMessageW(WM_CLOSE, 0, 0);
}

void WraithProgressDialog::OkClick()
{
    // Disable the controls, since we're waiting on the ok
    this->GetDlgItem((int32_t)WraithProgressDialogID::CancelControl)->EnableWindow(0);
    this->GetDlgItem((int32_t)WraithProgressDialogID::OkControl)->EnableWindow(0);
    // Run event delegate if available
    if (OnOkClick != nullptr)
    {
        // Call it
        OnOkClick(ProgressOwner);
    }
}

void WraithProgressDialog::CancelClick()
{
    // When canceling, we must set the text to "Canceling..."
    this->SetDlgItemTextW((int32_t)WraithProgressDialogID::CancelControl, L"Canceling...");
    // Disable the control, since it was already clicked
    this->GetDlgItem((int32_t)WraithProgressDialogID::CancelControl)->EnableWindow(0);
    // Run event delegate if available
    if (OnCancelClick != nullptr)
    {
        // Call it
        OnCancelClick(ProgressOwner);
    }
}