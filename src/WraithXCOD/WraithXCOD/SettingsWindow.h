#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxcview.h>
#include <afxext.h>

// We need the resources
#include "resource.h"

// We need the WraithWindow class
#include "WraithWindow.h"
#include "WraithSettingButton.h"

// The COD settings window
class SettingsWindow : public WraithWindow
{
public:
    // Make a new settings window
    SettingsWindow(CWnd* pParent = NULL) : WraithWindow(IDD_SETTINGSDIALOG, pParent) { }

private:
    // -- Event delegates

    void OnGeneralPage();
    void OnModelsPage();
    void OnAnimsPage();
    void OnImagesPage();
    void OnSoundsPage();

    // Helper for disabling all controls
    void SetUnselected();

protected:

    // -- Control instances

    WraithSettingButton GeneralButton;
    WraithSettingButton ModelButton;
    WraithSettingButton AnimButton;
    WraithSettingButton ImageButton;
    WraithSettingButton SoundButton;

    // The current settings panel
    std::unique_ptr<WraithWindow> SettingsPanel;

    // Occures when we need to swap control instances
    virtual void DoDataExchange(CDataExchange* pDX);
    // Occures when the window is loading
    virtual void OnBeforeLoad();
    // Occures when the window was loaded
    virtual void OnLoad();
    // Handle painting
    afx_msg void OnPaint();

    // Make the map
    DECLARE_MESSAGE_MAP()
};