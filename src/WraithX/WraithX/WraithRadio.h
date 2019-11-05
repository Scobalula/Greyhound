#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <string>

// A class that handles a WraithRadio
class WraithRadio
{
private:
    // -- Custom events

    // Handles painting the WraithRadio
    static void OnPaint(HWND hRadio, HDC hDC, PAINTSTRUCT& pPaintStruct);

public:
    // Handles the WNDPROC messages for the WraithRadio
    static LRESULT CALLBACK WndProcWraithRadio(HWND hRadio, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};