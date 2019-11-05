#include "stdafx.h"

// The class we are implementing
#include "CResizableDialog.h"

// Our custom message map for CResizableDialog
BEGIN_MESSAGE_MAP(CResizableDialog, CDialog)
    ON_WM_GETMINMAXINFO()
    ON_WM_SIZE()
END_MESSAGE_MAP()

CResizableDialog::CResizableDialog() : m_szMinimum(0, 0), CDialog()
{
    // Defaults
}

CResizableDialog::CResizableDialog(UINT nIDTemplate, CWnd* pParent) : m_szMinimum(0, 0), CDialog(nIDTemplate, pParent)
{
    // Defaults
}

CResizableDialog::~CResizableDialog()
{
    // Clean up if need be
    m_MovingChildren.clear();
}

void CResizableDialog::SetControlAnchor(int iID, double dXMovePct, double dYMovePct, double dXSizePct, double dYSizePct)
{
    // Get the child
    SMovingChild s;
    GetDlgItem(iID, &s.m_hWnd);
    // Ensure success
    if (s.m_hWnd != NULL)
    {
        // Resize
        s.m_dXMoveFrac = dXMovePct / 100.0;
        s.m_dYMoveFrac = dYMovePct / 100.0;
        s.m_dXSizeFrac = dXSizePct / 100.0;
        s.m_dYSizeFrac = dYSizePct / 100.0;
        // Apply it
        ::GetWindowRect(s.m_hWnd, &s.m_rcInitial);
        ScreenToClient(s.m_rcInitial);
        // Add it
        m_MovingChildren.push_back(s);
    }
}

BOOL CResizableDialog::OnInitDialog()
{
    // Setup the base
    CDialog::OnInitDialog();

    // Use the initial dialog size as the default minimum
    if ((m_szMinimum.cx == 0) && (m_szMinimum.cy == 0))
    {
        CRect rcWindow;
        GetWindowRect(rcWindow);
        m_szMinimum = rcWindow.Size();
    }

    // Keep the initial size of the client area as a baseline for moving/sizing controls
    CRect rcClient;
    GetClientRect(rcClient);
    m_szInitial = rcClient.Size();

    // Success
    return TRUE;
}

void CResizableDialog::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
    // Handle base event
    CDialog::OnGetMinMaxInfo(lpMMI);

    // Calculate the tracking size
    if (lpMMI->ptMinTrackSize.x < m_szMinimum.cx)
        lpMMI->ptMinTrackSize.x = m_szMinimum.cx;
    if (lpMMI->ptMinTrackSize.y < m_szMinimum.cy)
        lpMMI->ptMinTrackSize.y = m_szMinimum.cy;
}

void CResizableDialog::OnSize(UINT nType, int cx, int cy)
{
    // Handle base event
    CDialog::OnSize(nType, cx, cy);

    // Calculate delta
    int iXDelta = cx - m_szInitial.cx;
    int iYDelta = cy - m_szInitial.cy;
    HDWP hDefer = NULL;

    // Iterate over all children
    for (auto& MoveChild : m_MovingChildren)
    {
        // Only if the handle is valid
        if (MoveChild.m_hWnd != NULL)
        {
            // Setup new rect
            CRect rcNew(MoveChild.m_rcInitial);
            // Calculate
            rcNew.OffsetRect(LONG(iXDelta * MoveChild.m_dXMoveFrac), LONG(iYDelta * MoveChild.m_dYMoveFrac));
            rcNew.right += LONG(iXDelta * MoveChild.m_dXSizeFrac);
            rcNew.bottom += LONG(iYDelta * MoveChild.m_dYSizeFrac);
            // Check if we need to defer
            if (hDefer == NULL) { hDefer = BeginDeferWindowPos((int32_t)m_MovingChildren.size()); }

            // Get flags
            UINT uFlags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER;
            // If we don't have it, set no copy bits
            if ((MoveChild.m_dXSizeFrac != 0.0) || (MoveChild.m_dYSizeFrac != 0.0)) { uFlags |= SWP_NOCOPYBITS; }

            // Defer the render
            DeferWindowPos(hDefer, MoveChild.m_hWnd, NULL, rcNew.left, rcNew.top, rcNew.Width(), rcNew.Height(), uFlags);
        }
    }

    // If we are defering, stop and setup
    if (hDefer != NULL) { EndDeferWindowPos(hDefer); }
}