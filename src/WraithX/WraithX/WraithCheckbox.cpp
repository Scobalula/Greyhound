#include "stdafx.h"

// The class we are implementing
#include "WraithCheckbox.h"

// We need the theme
#include "WraithTheme.h"

LRESULT CALLBACK WraithCheckbox::WndProcWraithCheckbox(HWND hCheckbox, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	// Prepare to handle the custom events

	// Check what WM_ message we got
	switch (message)
	{
	case WM_PAINT:
		// Handle the rendering of our custom WraithCheckbox
	{
		// Fetch the info
		PAINTSTRUCT ps;
		// Start the paint instance
		HDC hDC = BeginPaint(hCheckbox, &ps);
		// Perform paint operation
		OnPaint(hCheckbox, hDC, ps);
		// Stop painting
		EndPaint(hCheckbox, &ps);
	}
	// We're done here
	return TRUE;
	case WM_ENABLE:
		// Handle painting before we disable
	{
		// Redraw
		CWnd::FromHandle(hCheckbox)->Invalidate();
	}
	// We're done here
	return FALSE;
	case WM_NCDESTROY:
		// Handle freeing the subclass instance
	{
		// Remove the subclass
		RemoveWindowSubclass(hCheckbox, WndProcWraithCheckbox, uIdSubclass);
	}
	break;
	}

	// Handle default message, we don't need it
	return DefSubclassProc(hCheckbox, message, wParam, lParam);
}

void WraithCheckbox::OnPaint(HWND hCheckbox, HDC hDC, PAINTSTRUCT& pPaintStruct)
{
	// Prepare to paint our button, depending on it's current state
	auto ThisCheckbox = (CButton*)CWnd::FromHandle(hCheckbox);
	// The checkbox state
	bool CheckboxChecked = ((ThisCheckbox->GetState() & BST_CHECKED) == BST_CHECKED);
	// The checkbox text
	CString CheckboxText;
	// Fetch checkbox text
	ThisCheckbox->GetWindowText(CheckboxText);
	// The client rect
	CRect ClientRect;
	// Get it
	ThisCheckbox->GetClientRect(&ClientRect);

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
	Gdiplus::SolidBrush backBrush(WraithTheme::DefaultControlBackground);
	// Create the border brush (border changes on disabled)
	Gdiplus::SolidBrush borderBrush((ThisCheckbox->IsWindowEnabled()) ? WraithTheme::DefaultControlBorder : WraithTheme::DisabledControlBorder);
	// Create the gradient fill brush
	Gdiplus::LinearGradientBrush fillBrush(clientRect, (ThisCheckbox->IsWindowEnabled()) ? WraithTheme::DefaultFillGradTop : WraithTheme::DisabledControlBorder, (ThisCheckbox->IsWindowEnabled()) ? WraithTheme::DefaultFillGradBottom : WraithTheme::DisabledControlBorder, 90.0f);
	// Create the gradient fill default brush
	Gdiplus::LinearGradientBrush fillDefaultBrush(clientRect, WraithTheme::DefaultControlGradTop, WraithTheme::DefaultControlGradBottom, 90.0f);
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
	graphics.FillRectangle(&backBrush, clientRect);
	// Render the border
	graphics.DrawRectangle(&borderPen, 0, 0, clientRect.Height - 1, clientRect.Height - 1);
	// Render the fill (If checked)
	if (CheckboxChecked)
	{
		// Render fill
		graphics.FillRectangle(&fillBrush, 1, 1, clientRect.Height - 2, clientRect.Height - 2);
		// Render icon, if loaded
		if (WraithTheme::CheckboxCheckedIcon != NULL)
		{
			// Load the icon
			Gdiplus::Bitmap CheckIcon(WraithTheme::CheckboxCheckedIcon);
			// Set interpolation mode
			graphics.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeHighQuality);
			// Render it
			graphics.DrawImage(&CheckIcon, ((clientRect.Height - 12) / 2), (clientRect.Height - 12) / 2, 12, 12);
		}
		else
		{
			// Only in debug
#ifdef _DEBUG
			printf("Warning: Missing icon from theme \"CheckboxCheckedIcon\"\n");
#endif
		}
	}
	else
	{
		// Render default
		graphics.FillRectangle(&fillDefaultBrush, 1, 1, clientRect.Height - 2, clientRect.Height - 2);
	}


	// Measure the text
	graphics.MeasureString(CheckboxText, CheckboxText.GetLength(), &textFont, textLayRect, &textSize);
	// Draw the text
	graphics.DrawString(CheckboxText, CheckboxText.GetLength(), &textFont, Gdiplus::PointF((clientRect.Height - 1.0f) + 4.0f, (clientRect.Height - textSize.Height) / 2.0f), &Gdiplus::StringFormat(), &textBrush);

	// -- End control render, output result

	// Render out the final buffer
	dbGraphic.DrawImage(&dbBuffer, clientRect);
}