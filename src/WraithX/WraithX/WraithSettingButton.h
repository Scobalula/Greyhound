#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxcview.h>
#include <string>
#include <vector>
#include <memory>

// A class that handles a WraithSettingButton
class WraithSettingButton : public CStatic
{
public:
    // -- WraithSettingButton functions
    WraithSettingButton();

    // Changes the selected state
    void SetSelectedState(bool Value);

protected:

    // Whether or not we are selected
    bool IsSelected;
    // Whether or not the mouse is over
    bool IsHovering;
    // Whether or not we are tracking
    bool IsTracking;

    // The setting button icon, if any
    std::unique_ptr<Gdiplus::Bitmap> ButtonIcon;

    // Handle drawing the control
    afx_msg void OnPaint();
    // Handle mouse move
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    // Handle mouse leave
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);

    // Declare the message map instance
    DECLARE_MESSAGE_MAP()
};