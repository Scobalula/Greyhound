#include "stdafx.h"

// The class we are implementing
#include "WraithTextbox.h"

// We need the theme
#include "WraithTheme.h"

// Our custom message map for WraithTextbox
BEGIN_MESSAGE_MAP(WraithTextbox, CEdit)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_SETFOCUS()
	//ON_WM_ENABLE()
	ON_WM_ERASEBKGND()
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()

WraithTextbox::WraithTextbox()
	: m_rectNCBottom(0, 0, 0, 0), m_rectNCTop(0, 0, 0, 0)
{
	// Defaults
}

WraithTextbox::~WraithTextbox()
{
	// Defaults
}

void WraithTextbox::OnSetFocus(CWnd* pOldWnd)
{
	// Make sure to generate a nc rect
	if (m_rectNCTop.IsRectEmpty())
	{
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
	}

#if _DEBUG
	printf("Focus\n");
#endif

	// Default logic
	CEdit::OnSetFocus(pOldWnd);
}

void WraithTextbox::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	// Base rects
	CRect rectWnd, rectClient;

	// Calculate client area height needed for a font
	CFont *pFont = GetFont();
	CRect rectText;
	rectText.SetRectEmpty();

	CDC *pDC = GetDC();

	CFont *pOld = pDC->SelectObject(pFont);
	pDC->DrawText("Ky", rectText, DT_CALCRECT | DT_LEFT);
	UINT uiVClientHeight = rectText.Height();

	pDC->SelectObject(pOld);
	ReleaseDC(pDC);

	// Calculate NC area to center text.
	GetClientRect(rectClient);
	GetWindowRect(rectWnd);

	ClientToScreen(rectClient);

	UINT uiCenterOffset = (rectClient.Height() - uiVClientHeight) / 2;
	UINT uiCY = (rectWnd.Height() - rectClient.Height()) / 2;
	UINT uiCX = (rectWnd.Width() - rectClient.Width()) / 2;

	rectWnd.OffsetRect(-rectWnd.left, -rectWnd.top);
	m_rectNCTop = rectWnd;

	m_rectNCTop.DeflateRect(uiCX, uiCY, uiCX, uiCenterOffset + uiVClientHeight + uiCY);

	m_rectNCBottom = rectWnd;

	m_rectNCBottom.DeflateRect(uiCX, uiCenterOffset + uiVClientHeight + uiCY, uiCX, uiCY);

	lpncsp->rgrc[0].top += uiCenterOffset;
	lpncsp->rgrc[0].bottom -= uiCenterOffset;

	// Shift, to not throw off our border control
	lpncsp->rgrc[0].left += uiCX + 1;
	lpncsp->rgrc[0].right -= uiCY + 1;

	// Invalidate the entire control
	this->Invalidate();

#if _DEBUG
	printf("CalcNc: %d\n", bCalcValidRects);
#endif
}

void WraithTextbox::OnNcPaint()
{
	// Handle default logic first
	Default();
	// Allocate a paint context
	CWindowDC dc(this);
	// Allocate a flush context
	CPaintDC dp(this);

#if _DEBUG
	printf("RenderNC\n");
#endif

	// -- Setup gdi utils

	// The actual area
	CRect AreaRegion;
	// Fetch
	this->GetWindowRect(&AreaRegion);
	// Setup main graphics
	Gdiplus::Graphics graphics(dc);
	// The full client area
	Gdiplus::Rect clientRect(0, 0, AreaRegion.right - AreaRegion.left, AreaRegion.bottom - AreaRegion.top);

	// -- Begin control render

	// Create the border brush (border changes on disabled)
	Gdiplus::SolidBrush borderBrush((this->IsWindowEnabled()) ? WraithTheme::DefaultControlBorder : WraithTheme::DisabledControlBorder);
	// Create the pen
	Gdiplus::Pen borderPen(&borderBrush, 1.0f);

	// Render the background

	// Render the border
	graphics.DrawRectangle(&borderPen, 0, 0, clientRect.Width - 1, clientRect.Height - 1);

	// -- End control render
}

UINT WraithTextbox::OnGetDlgCode()
{
	// Make sure to generate a nc rect
	if (m_rectNCTop.IsRectEmpty())
	{
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
	}

	// Default logic
	return CEdit::OnGetDlgCode();
}

BOOL WraithTextbox::OnEraseBkgnd(CDC* pDC)
{
	// Default render logic
	auto Result = CEdit::OnEraseBkgnd(pDC);

#if _DEBUG
	printf("OnEraseBkgnd\n");
#endif

	// Ensure we have a rect here
	if (m_rectNCTop.IsRectEmpty())
	{
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
	}

	// Default result
	return Result;
}