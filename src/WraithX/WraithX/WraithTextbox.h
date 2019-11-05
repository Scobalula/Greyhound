#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <string>

// A class that handles a WraithTextbox
class WraithTextbox : public CEdit
{
public:
    // Constructors
    WraithTextbox();
    virtual ~WraithTextbox();

protected:

    // Non-client rect borders
    CRect m_rectNCBottom;
    CRect m_rectNCTop;

    // Events
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
    afx_msg void OnNcPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg UINT OnGetDlgCode();
    
    // Setup message map
    DECLARE_MESSAGE_MAP();
};