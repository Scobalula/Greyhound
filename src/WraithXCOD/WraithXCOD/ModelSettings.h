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

// The COD model settings
class ModelSettings : public WraithWindow
{
public:
    // Make a new panel
    ModelSettings(CWnd* pParent = NULL) : WraithWindow(IDD_MODELSETTINGS, pParent) { }

private:
    // -- Event delegates

    void OnGlobalImages();
    void OnExportImages();
    void OnExportAllLods();
    void OnExportHitbox();
    void OnExportVTXColor();
    void OnExportIMGNames();
    void OnMDLMTLFolders();
    void OnSkipPrevModels();

    void OnExportMA();
    void OnExportOBJ();
    void OnExportXNA();
    void OnExportSMD();
    void OnExportXMB();
    void OnExportXME();
    void OnExportGLTF();
    void OnExportCast();
    void OnExportSEModel();
    void OnExportFBX();

protected:

    // Occures when the window is loading
    virtual void OnBeforeLoad();

    // The title font
    CFont TitleFont;

    // Make the map
    DECLARE_MESSAGE_MAP()
};