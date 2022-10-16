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

// The COD general settings
class GeneralSettings : public WraithWindow
{
public:
    // Make a new panel
    GeneralSettings(CWnd* pParent = NULL) : WraithWindow(IDD_GENERALSETTINGS, pParent) { }

private:
    // -- Event delegates

    void OnXModels();
    void OnXAnims();
    void OnXImages();
    void OnXEffects();
    void OnXRawFiles();
    void OnXSounds();
    void OnXMTL();
    void OnAssetSortMethod();


protected:

    // Occures when the window is loading
    virtual void OnBeforeLoad();

    // The title font
    CFont TitleFont;

    // Make the map
    DECLARE_MESSAGE_MAP()
};