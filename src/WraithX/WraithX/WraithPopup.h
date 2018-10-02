#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxcview.h>
#include <vector>

// We need the WraithWindow class
#include "WraithWindow.h"

// A class that handles a popup message
class WraithPopup : public WraithWindow
{
public:
	// Create a new WraithPopup
	WraithPopup(UINT nIDTemplate, CWnd* pParent = NULL);

	// Shows a popup notification
	void ShowPopup(const CString& Message, const CString& Directory, uint32_t Milliseconds);
	// Prepares the notification (Use on multi-threaded workflow)
	void PreparePopup();

private:

	// The directory to open when clicked on
	CString DirectoryOpen;
	// The message to present
	CString DisplayMessage;

	// The currently active wait timer
	UINT_PTR CloseTimer;

	// The dialog resource ID
	UINT PopupTemplateID;
	// The dialog parent handle
	CWnd* PopupParent;
	// Whether or not we created already
	bool WasCreated;

	// Aligns the window with the screen
	void AlignWindow();

	// The cursor for the window
	HCURSOR HandCursor;

	// Whether or not we are tracking
	bool MouseTrackingMode;
	// Whether or not the mouse left button was down
	bool WasLeftButtonDown;

	// Handler for when the window is clicked
	void OnWindowClick();

protected:

	// Occurs when a timer fires
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	// Occurs when it's time to paint
	afx_msg void OnPaint();
	// Occurs when we need to set a cursor
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	// Occurs when the mouse leaves the window
	afx_msg void OnMouseLeave();
	// Occurs when the mouse moves
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	// Occurs when the mouse left button goes down
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	// Occurs when the mouse left button goes up
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	// Make the map
	DECLARE_MESSAGE_MAP()
};