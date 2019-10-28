#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxcview.h>

// We need the resources
#include "resource.h"

// We need the WraithWindow and WraithAboutDialog classes
#include "WraithWindow.h"
#include "WraithAboutDialog.h"

// The Evil Within about window
class AboutWindow : public WraithAboutDialog
{
public:
    // Make a new about window
    AboutWindow(CWnd* pParent = NULL) : WraithAboutDialog(IDD_ABOUTDLG, pParent) { }

protected:

    // Handle adding the about text
    virtual void OnBeforeLoad()
    {
        // Call base
        WraithAboutDialog::OnBeforeLoad();
        // Setup about
        this->AddColorText("Greyhound - A continuation of Wraith Archon, The Call of Duty asset exporter\n", RGB(0, 102, 255), 14, true);
        this->AddColorText("Developed by ", RGB(255, 255, 255), 13);
        this->AddColorText("DTZxPorter", RGB(236, 52, 202), 13, true);
        this->AddColorText(" - Maintained by ", RGB(255, 255, 255), 13);
        this->AddColorText("Scobalula", RGB(52, 152, 219), 13, true);
        this->AddColorText(" - Some game research by ", RGB(255, 255, 255), 13);
        this->AddColorText("id-daemon", RGB(52, 152, 219), 13, true);
        this->AddColorText(".\n", RGB(255, 255, 255), 13);
        this->AddColorText("\n", RGB(255, 255, 255), 13);
        // Provided warning
        this->AddColorText("You must take note that EVERY asset exported using this tool is property of the game you extracted it from. Therefore you may not use them in any commercial environment and or profit off of them.\n\n", RGB(255, 255, 255), 13);
        // Rehost
        this->AddColorText("Greyhound, like Wraith Archon and its library, is licensed under the General Public License 3.0. You may distribute it and use it under the terms of the GPL 3.0, however these tools are provided as-is with no warranty provided. ", RGB(255, 255, 255), 13);
        this->AddColorText("You are responsible for any damages caused.\n\n", RGB(255, 255, 255), 13, true, false, true);
        // Changelog
        this->AddColorText("-- Changelog is available on the Github Repo --", RGB(0, 102, 255), 13);
    }
};