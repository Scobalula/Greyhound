#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <string>

// A class that handles a WraithGroupBox
class WraithGroupBox
{
private:
	// -- Custom events

	// Handles painting the WraithGroupBox
	static void OnPaint(HWND hButton, HDC hDC, PAINTSTRUCT& pPaintStruct);

public:
	// Handles the WNDPROC messages for the WraithGroupBox
	static LRESULT CALLBACK WndProcWraithGroupBox(HWND hButton, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};