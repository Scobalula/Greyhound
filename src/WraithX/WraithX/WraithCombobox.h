#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <string>

// A class that handles a WraithCombobox
class WraithCombobox
{
private:
	// -- Custom events

	// Handles painting the WraithCombobox
	static void OnPaint(HWND hRadio, HDC hDC, PAINTSTRUCT& pPaintStruct);

public:
	// Handles the WNDPROC messages for the WraithCombobox
	static LRESULT CALLBACK WndProcWraithCombobox(HWND hControl, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};