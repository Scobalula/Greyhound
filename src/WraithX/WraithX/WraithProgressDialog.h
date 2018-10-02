#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxcview.h>
#include <vector>

// We need the WraithWindow class
#include "WraithWindow.h"

// A list of event delegates
typedef void(*ProgressDialogCallback)(CWnd* Owner);

// A list of control IDs by name, these must be specified in the resource for the dialog
enum class WraithProgressDialogID : int32_t
{
	CancelControl = 0x100,
	OkControl = 0x101,
	ProgressControl = 0x102,
	WorkingTextControl = 0x103,
	StatusTextControl = 0x104
};

// A class that handles progress updating for any task
class WraithProgressDialog : public WraithWindow
{
public:
	// Create a new WraithProgressDialog
	WraithProgressDialog(UINT nIDTemplate, CWnd* pParent = NULL);

	// -- Progress functions
	
	// Sets up the dialog, changing the title, status and button visibility (Call before DoModal)
	void SetupDialog(const std::string& DialogTitle, const std::string& DialogStatus, bool IsOkEnabled, bool IsCancelEnabled);
	// Updates the dialog status
	void UpdateStatus(const std::string& DialogStatus);
	// Updates the progress values
	void UpdateProgress(uint32_t Progress);
	// Updates the button enable state
	void UpdateButtons(bool CanOk, bool CanCancel);
	// Waits until the dialog is ready
	void WaitTillReady();
	// Updates the window close ready
	void UpdateWindowClose(bool Close);
	// Close it safely
	void CloseProgress();

	// Occurs when we clicked the cancel button
	ProgressDialogCallback OnCancelClick;
	// Occurs when we clicked the ok button
	ProgressDialogCallback OnOkClick;

private:
	// -- Properties

	// The title of the window
	std::string Title;
	// The base status for the control
	std::string Status;
	// Whether or not to show the ok button
	bool OkButton;
	// Whether or not to show the cancel button
	bool CancelButton;

	// Are we initialized
	bool IsSetup;

	// Are we allowed to close
	bool CanClose;

	// Event delegates
	void OkClick();
	void CancelClick();

	// A reference to the progress owner
	CWnd* ProgressOwner;

protected:

	// Occures when the dialog is loading
	virtual BOOL OnInitDialog();
	// Occures when the window is loading
	virtual void OnBeforeLoad();
	// Occures when the window is closing
	afx_msg void OnClose();
	// Occures when the window is being activated
	afx_msg BOOL OnNcActivate(BOOL bActive);

	// Make the map
	DECLARE_MESSAGE_MAP()
};