#pragma once

#define _AFXDLL // AFX Shared DLL

#include <afxwin.h>
#include <string>
#include <vector>
#include <thread>

// We need the resizable dialog
#include "CResizableDialog.h"

// Remove WinAPI Includes
#undef CreateWindow

// A class that handles a window
class WraithWindow : public CResizableDialog
{
protected:
	// The background of this dialog (and any child controls that use this)
	CBrush BackgroundBrush;
	// The actual client area of this dialog (Relative to the window)
	CRect ClientClip;

	// The minimum size of this window
	int32_t MinimumWidth;
	int32_t MinimumHeight;
	// The maximum size of this window
	int32_t MaximumWidth;
	int32_t MaximumHeight;

public:
	// Create a new WraithWindow
	WraithWindow();
	// Create a new WraithWindow with the specified resource and parent (If specified)
	WraithWindow(UINT nIDTemplate, CWnd* pParent = NULL);
	// Occures when we destroy the window instance, for cleanup
	virtual ~WraithWindow();

	// -- WraithWindow custom ui functions

	// Setup a button control using a WraithButton
	void SetWraithButtonCtrl(UINT cID);
	// Setup a checkbox control using a WraithButton
	void SetWraithCheckboxCtrl(UINT cID);
	// Setup a radio control using a WraithRadio
	void SetWraithRadioCtrl(UINT cID);
	// Setup a combobox control using a WraithCombobox
	void SetWraithComboboxCtrl(UINT cID);
	// Setup a combobox control using a WraithProgressBar
	void SetWraithProgressBarCtrl(UINT cID);
	// Setup a groupbox control using a WraithGroupBox
	void SetWraithGroupBoxCtrl(UINT cID);

	// Allows adjusting the position of a control that is a child of this window
	void MoveControl(UINT cID, CRect cPosition);
	// Allows shifting the position of a control
	void ShiftControl(UINT cID, CRect cShift);

protected:
	// -- Build-in events

	// Occures when the dialog was ok'd
	void OnDenySubmit();
	// Occurs when the dialog is being setup
	virtual BOOL OnInitDialog();
	// Occures when we need to setup custom controls
	virtual void DoDataExchange(CDataExchange* pDX);
	// Handle the dialog closing
	afx_msg void OnClose();
	// Handle the dialog background
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	// Handle text color
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	// Handle minmax calc
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);

	// -- Drag events

	// Occures when files are dropped on
	afx_msg void OnDropFiles(HDROP hDROP);

	// -- WraithWindow events

	// Occurs when the window has been loaded
	virtual void OnLoad();
	// Occurs when the window is being loaded, used for adding controls before theming
	virtual void OnBeforeLoad();
	// Occures when files have been dragged onto the window
	virtual void OnFilesDrop(const std::vector<std::wstring> Files);

	// Declare the message map instance
	DECLARE_MESSAGE_MAP()
};