#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <string>

// A class that handles rendering and subclassing the menu of a split button
class WraithSplitButton : public CSplitButton
{
public:
	// Constructors
	WraithSplitButton();
	virtual ~WraithSplitButton();

protected:

	// Events
	afx_msg void OnPaint();
	afx_msg void OnDropDown(NMHDR* pNMHDR, LRESULT* pResult);

	// Setup the message pump map
	DECLARE_MESSAGE_MAP();
};