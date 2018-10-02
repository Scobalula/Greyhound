#include "stdafx.h"

// The class we are implementing
#include "WraithProgressBar.h"

// We need the theme
#include "WraithTheme.h"

LRESULT CALLBACK WraithProgressBar::WndProcWraithProgressBar(HWND hProgress, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	// Prepare to handle the custom events

	// Check what WM_ message we got
	switch (message)
	{
	case WM_ERASEBKGND:
		// Handle not painting the bg
	{
		// Do nothing
	}
	// We're done here
	return TRUE;
	case WM_PAINT:
		// Handle the rendering of our custom WraithProgressBar
	{
		// Fetch the info
		PAINTSTRUCT ps;
		// Start the paint instance
		HDC hDC = BeginPaint(hProgress, &ps);
		// Perform paint operation
		OnPaint(hProgress, hDC, ps);
		// Stop painting
		EndPaint(hProgress, &ps);
	}
	// We're done here
	return TRUE;
	case WM_ENABLE:
		// Handle painting before we disable
	{
		// Redraw
		CWnd::FromHandle(hProgress)->Invalidate();
	}
	// We're done here
	return FALSE;
	case WM_NCDESTROY:
		// Handle freeing the subclass instance
	{
		// Remove the subclass
		RemoveWindowSubclass(hProgress, WndProcWraithProgressBar, uIdSubclass);
	}
	break;
	}

	// Handle default message, we don't need it
	return DefSubclassProc(hProgress, message, wParam, lParam);
}

void WraithProgressBar::OnPaint(HWND hButton, HDC hDC, PAINTSTRUCT& pPaintStruct)
{
	// Prepare to paint our progress, depending on it's current state
	auto ThisProgress = (CProgressCtrl*)CWnd::FromHandle(hButton);
	// The progress fill
	uint32_t ProgressFill = ThisProgress->GetPos();
	// The client rect
	CRect ClientRect;
	// Get it
	ThisProgress->GetClientRect(&ClientRect);

	// -- Setup gdi utils

	// Setup double buffering image
	Gdiplus::Bitmap dbBuffer(ClientRect.right, ClientRect.bottom);
	// Setup double buffer graphics
	Gdiplus::Graphics dbGraphic(hDC);
	// Setup main graphics
	Gdiplus::Graphics graphics(&dbBuffer);
	// The full client area
	Gdiplus::Rect clientRect(0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);

	// -- Begin control render

	// Create the gradient brush
	Gdiplus::LinearGradientBrush backBrush(clientRect, WraithTheme::DefaultControlGradTop, WraithTheme::DefaultControlGradBottom, 90.0f);
	// Create the fill brush
	Gdiplus::LinearGradientBrush fillBrush(clientRect, (ThisProgress->IsWindowEnabled()) ? WraithTheme::DefaultFillGradTop : WraithTheme::DisabledControlBorder, (ThisProgress->IsWindowEnabled()) ? WraithTheme::DefaultFillGradBottom : WraithTheme::DisabledControlBorder, 90.0f);
	// Create the border brush (border changes on disabled)
	Gdiplus::SolidBrush borderBrush((ThisProgress->IsWindowEnabled()) ? WraithTheme::DefaultControlBorder : WraithTheme::DisabledControlBorder);
	// Create the pen
	Gdiplus::Pen borderPen(&borderBrush, 1.0f);

	// Render the background gradient
	graphics.FillRectangle(&backBrush, clientRect);
	// Render the border
	graphics.DrawRectangle(&borderPen, 0, 0, clientRect.Width - 1, clientRect.Height - 1);

	// Calculate the fill
	uint32_t WidthCalc = (uint32_t)((clientRect.Width - 4.0f) * (ProgressFill / 100.0f));
	// Render the progress fill
	graphics.FillRectangle(&fillBrush, 2, 2, WidthCalc, clientRect.Height - 4);

	// -- End control render, output result

	// Render out the final buffer
	dbGraphic.DrawImage(&dbBuffer, clientRect);
}