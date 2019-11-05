#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxcview.h>
#include <vector>

// We need the WraithWindow class
#include "WraithWindow.h"

// A list of control IDs by name, these must be specified in the resource for the dialog
enum class WraithAboutDialogID : int32_t
{
    TextControl = 0x100,
    OkControl = 0x101,
    GithubControl = 0x102,
};

// A class that handles an about dialog for the tool
class WraithAboutDialog : public WraithWindow
{
public:
    // Create a new WraithAboutDialog
    WraithAboutDialog(UINT nIDTemplate, CWnd* pParent = NULL);

    // Adds a line of colored text to the dialog
    void AddColorText(const CString& Text, COLORREF Color = RGB(255, 255, 255), float FontSize = 12.0f, bool Bold = false, bool Italic = false, bool Underline = false);
    
private:

    // -- Event delegates

    // Occures when the user clicks ok
    void OnOkClick();
    // Occures when donate is clicked on
    void OnGithubClick();

protected:

    // Occures when the window is loading
    virtual void OnBeforeLoad();

    // Make the map
    DECLARE_MESSAGE_MAP()
};