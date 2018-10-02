#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxcmn.h>
#include <string>

// A class that handles a WraithProgressBar
class WraithProgressBar
{
private:
	// -- Custom events

	// Handles painting the WraithProgressBar
	static void OnPaint(HWND hProgress, HDC hDC, PAINTSTRUCT& pPaintStruct);

public:
	// Handles the WNDPROC messages for the WraithProgressBar
	static LRESULT CALLBACK WndProcWraithProgressBar(HWND hProgress, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};