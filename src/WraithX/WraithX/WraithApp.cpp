#include "stdafx.h"

// The class we are implementing
#include "WraithApp.h"

// We need the WraithWindow class
#include "WraithWindow.h"

BEGIN_MESSAGE_MAP(WraithApp, CWinApp)
    // Nothing
END_MESSAGE_MAP()

WraithApp::WraithApp()
{
    // Defaults
}

BOOL WraithApp::InitInstance()
{
    // We must setup the common controls first
    INITCOMMONCONTROLSEX InitCtrls;
    // Set the size
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    // Setup the controls
    InitCommonControlsEx(&InitCtrls);

    // Initialize the base instance
    CWinApp::InitInstance();

    // Setup control container
    AfxEnableControlContainer();

    // Set
    m_pMainWnd = MainAppWindow;
    // Start the dialog
    MainAppWindow->DoModal();

    // Set
    MainAppWindow = NULL;

    // Success, don't start another wndproc
    return FALSE;
}

void WraithApp::RunApplication(WraithWindow& WraithWnd)
{
    // Set the application reference, this is stack allocated for easier use
    MainAppWindow = &WraithWnd;
    // Initialize and run
    InitInstance();
}