#pragma once

#include <afxwin.h>
#include <string>
#include <vector>

// A class that handles a resizable dialog window
class CResizableDialog : public CDialog
{
public:
    // Create a new CResizableDialog
    CResizableDialog();
    // Create a new CResizableDialog
    CResizableDialog(UINT nIDTemplate, CWnd* pParent = NULL);
    // Occures when we destroy the dialog instance, for cleanup
    virtual ~CResizableDialog();
    // Setup a control anchor, this will allow it to resize properly
    void SetControlAnchor(int iID, double dXMovePct, double dYMovePct, double dXSizePct, double dYSizePct);

protected:
    // -- Built-in events

    // Occures when the dialog is setup
    virtual BOOL OnInitDialog();
    // Occures when we fetch the min/max size info
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
    // Occurs on the window size update
    afx_msg void OnSize(UINT nType, int cx, int cy);

    // Declare the message map instance
    DECLARE_MESSAGE_MAP()

private:
    // -- Structure info
    struct SMovingChild
    {
        HWND        m_hWnd;
        double      m_dXMoveFrac;
        double      m_dYMoveFrac;
        double      m_dXSizeFrac;
        double      m_dYSizeFrac;
        CRect       m_rcInitial;
    };

    // A type of moving children for this control
    typedef std::vector<SMovingChild>   MovingChildren;

    // A list of all moving children for this control
    MovingChildren  m_MovingChildren;
    // Additional information
    CSize           m_szInitial;
    CSize           m_szMinimum;
};