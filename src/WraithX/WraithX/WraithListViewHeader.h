#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxcview.h>
#include <string>

// A class that handles a WraithListViewHeader
class WraithListViewHeader : public CHeaderCtrl
{
protected:

	// Occures when an item needs rendering
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	// Handle paint override
	afx_msg void OnPaint();
	// Handle NC paint override
	afx_msg void OnNcPaint();
	// Handle background paint override
	afx_msg BOOL OnEraseBkgnd(CDC* cDC);

	// Declare the message map instance
	DECLARE_MESSAGE_MAP()
};