#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxshellmanager.h>

// We need the window classes
#include "WraithWindow.h"

// A class that handles an app instance for Wraith
class WraithApp : public CWinApp
{
private:
	// The handle to the window to show
	WraithWindow* MainAppWindow;

public:
	// Create a new WraithApp instance
	WraithApp();

	// Initialize the application
	virtual BOOL InitInstance();

	// Run the application, initializing if need be, with the specified WraithWindow
	void RunApplication(WraithWindow& WraithWnd);

	DECLARE_MESSAGE_MAP()
};