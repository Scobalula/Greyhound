#include "stdafx.h"

// The class we are implementing
#include "WraithCombobox.h"

// We need the theme
#include "WraithTheme.h"

LRESULT CALLBACK WraithCombobox::WndProcWraithCombobox(HWND hControl, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	// Prepare to handle the custom events

	// Check what WM_ message we got
	switch (message)
	{
	case WM_ERASEBKGND:
		// Don't paint here!
	{
		// Nothing
	}
	// We're done here
	return FALSE;
	case WM_PAINT:
		// Handle the rendering of our custom WraithButton
	{
		// Fetch the info
		PAINTSTRUCT ps;
		// Start the paint instance
		HDC hDC = BeginPaint(hControl, &ps);
		// Perform paint operation
		OnPaint(hControl, hDC, ps);
		// Stop painting
		EndPaint(hControl, &ps);
	}
	// We're done here
	return TRUE;
	case WM_ENABLE:
		// Handle painting before we disable
	{
		// Redraw
		CWnd::FromHandle(hControl)->Invalidate();
	}
	// We're done here
	return FALSE;
	case WM_NCDESTROY:
		// Handle freeing the subclass instance
	{
		// Remove the subclass
		RemoveWindowSubclass(hControl, WndProcWraithCombobox, uIdSubclass);
	}
	break;
	}

	// Handle default message, we don't need it
	return DefSubclassProc(hControl, message, wParam, lParam);
}

void WraithCombobox::OnPaint(HWND hControl, HDC hDC, PAINTSTRUCT& pPaintStruct)
{
	// Prepare to paint our button, depending on it's current state
	auto ThisControl = (CButton*)CWnd::FromHandle(hControl);
	// The control text
	CString ControlText;
	// Fetch control text
	ThisControl->GetWindowText(ControlText);
	// The client rect
	CRect ClientRect;
	// Get it
	ThisControl->GetClientRect(&ClientRect);

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

	// Create the background brush
	Gdiplus::LinearGradientBrush backBrush(clientRect, WraithTheme::DefaultControlGradTop, WraithTheme::DefaultControlGradBottom, 90.0f);
	// Create the border brush (border changes on disabled)
	Gdiplus::SolidBrush borderBrush((ThisControl->IsWindowEnabled()) ? WraithTheme::DefaultControlBorder : WraithTheme::DisabledControlBorder);
	// Create the gradient fill default brush
	
	// Create the pen
	Gdiplus::Pen borderPen(&borderBrush, 1.0f);
	// Create the font
	Gdiplus::Font textFont(L"Microsoft Sans Serif", 8.25f);
	// Create the text brush
	Gdiplus::SolidBrush textBrush(WraithTheme::DefaultForeground);
	// Create the text format
	Gdiplus::StringFormat textFormat;
	// Create the text layout rect
	Gdiplus::RectF textLayRect(0.0f, 0.0f, (float)clientRect.Width, (float)clientRect.Height);
	// The text size
	Gdiplus::RectF textSize;

	// Render the background color
	graphics.FillRectangle(&backBrush, 0, 0, clientRect.Width, clientRect.Height);
	// Render the border
	graphics.DrawRectangle(&borderPen, 0, 0, clientRect.Width - 1, clientRect.Height - 1);

	// Measure the text
	graphics.MeasureString(ControlText, ControlText.GetLength(), &textFont, textLayRect, &textSize);
	// Draw the text
	graphics.DrawString(ControlText, ControlText.GetLength(), &textFont, Gdiplus::PointF(3, (clientRect.Height - textSize.Height) / 2.0f), &Gdiplus::StringFormat(), &textBrush);

	// Render the glyph
	Gdiplus::GraphicsPath Path;
	// Render
	graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);

	// Set the glyph transform
	Gdiplus::Rect Rectbutton(clientRect.Width - 18, 0, 18, clientRect.Height - 1);

	// Setup transform
	graphics.TranslateTransform(Rectbutton.X + Rectbutton.Width / 2.0f, Rectbutton.Y + Rectbutton.Height / 2.0f);

	// Make it
	Path.AddLine(Gdiplus::PointF(-6 / 2.0f, -3 / 2.0f), Gdiplus::PointF(6 / 2.0f, -3 / 2.0f));
	Path.AddLine(Gdiplus::PointF(6 / 2.0f, -3 / 2.0f), Gdiplus::PointF(0, 6 / 2.0f));

	// Close
	Path.CloseFigure();
	// Set the rotate
	graphics.RotateTransform(0);

	// Fill
	graphics.FillPath(&borderBrush, &Path);
	// Reset
	graphics.ResetTransform();

	// -- End control render, output result

	// Render out the final buffer
	dbGraphic.DrawImage(&dbBuffer, clientRect);
}