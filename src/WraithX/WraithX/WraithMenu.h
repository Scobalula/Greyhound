#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <string>

// A class that handles a WraithMenu
class WraithMenu : public CMenu
{
public:
	WraithMenu();
	virtual ~WraithMenu();

	// Enables the menu to draw on it's own
	void EnableOwnerDraw();

	// Gets a reference to the submenu item
	std::unique_ptr<WraithMenu> GetWraithSubMenu(int nPos) const;

protected:

	// Overridden events
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
};