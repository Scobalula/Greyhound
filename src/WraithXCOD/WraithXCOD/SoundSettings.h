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

// The COD sound settings
class SoundSettings : public WraithWindow
{
public:
    // Make a new panel
    SoundSettings(CWnd* pParent = NULL) : WraithWindow(IDD_SOUNDSETTINGS, pParent) { }

private:
    // -- Event delegates

    void OnKeepPaths();
    void OnSkipPrevSound();
    void OnSkipBlankAudio();
    void OnUseSabIndexBo4();
    void OnSoundFormat();

protected:

    // Occures when the window is loading
    virtual void OnBeforeLoad();

    // The title font
    CFont TitleFont;

    // Make the map
    DECLARE_MESSAGE_MAP()
};