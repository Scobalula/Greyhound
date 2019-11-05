#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <string>

// A class that handles a WraithCheckbox
class WraithCheckbox
{
private:
    // -- Custom events

    // Handles painting the WraithCheckbox
    static void OnPaint(HWND hCheckbox, HDC hDC, PAINTSTRUCT& pPaintStruct);

public:
    // Handles the WNDPROC messages for the WraithCheckbox
    static LRESULT CALLBACK WndProcWraithCheckbox(HWND hCheckbox, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};