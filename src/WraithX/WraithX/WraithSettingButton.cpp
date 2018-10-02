#include "stdafx.h"

// The class we are implementing
#include "WraithSettingButton.h"

// We need the theme and strings
#include "WraithTheme.h"
#include "Strings.h"

// Our custom message map for WraithWindow
BEGIN_MESSAGE_MAP(WraithSettingButton, CStatic)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

WraithSettingButton::WraithSettingButton() : CStatic()
{
	// Defaults
	this->IsSelected = false;
	this->IsHovering = false;
	this->IsTracking = false;

	ButtonIcon = nullptr;
}

void WraithSettingButton::OnPaint()
{
	// Prepare to paint
	CPaintDC dc(this);

	// The button text
	CString ButtonText;
	// Fetch button text
	this->GetWindowText(ButtonText);
	// The client rect
	CRect ClientRect;
	// Get it
	this->GetClientRect(&ClientRect);

	// -- Setup gdi utils

	// Setup double buffering image
	Gdiplus::Bitmap dbBuffer(ClientRect.right, ClientRect.bottom);
	// Setup double buffer graphics
	Gdiplus::Graphics dbGraphic(dc.GetSafeHdc());
	// Setup main graphics
	Gdiplus::Graphics graphics(&dbBuffer);
	// The full client area
	Gdiplus::Rect clientRect(0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);

	// -- Begin control render

	// Create the font
	Gdiplus::Font textFont(L"Microsoft Sans Serif", 9.25f);
	// Create the text brush
	Gdiplus::SolidBrush textBrush(WraithTheme::DefaultForeground);
	// Create the text layout rect
	Gdiplus::RectF textLayRect(0.0f, 0.0f, (float)clientRect.Width, (float)clientRect.Height);
	// The text size
	Gdiplus::RectF textSize;

	// Render the background image
	if (this->IsSelected)
	{
		// Draw the selected only
		if (WraithTheme::SettingsSelectedImage != nullptr)
		{
			graphics.DrawImage(WraithTheme::SettingsSelectedImage, clientRect);
		}
	}
	else
	{
		if (this->IsHovering)
		{
			// Draw hover
			if (WraithTheme::SettingsHoverImage != nullptr)
			{
				graphics.DrawImage(WraithTheme::SettingsHoverImage, clientRect);
			}
		}
		else
		{
			// Draw normal
			if (WraithTheme::SettingsNormalImage != nullptr)
			{
				graphics.DrawImage(WraithTheme::SettingsNormalImage, clientRect);
			}
		}
	}	

	// Measure the text
	graphics.MeasureString(ButtonText, ButtonText.GetLength(), &textFont, textLayRect, &textSize);
	// Draw the text
	graphics.DrawString(ButtonText, ButtonText.GetLength(), &textFont, Gdiplus::PointF(((clientRect.Width - textSize.Width) - 1) / 2.0f, ((clientRect.Height - textSize.Height) - 1) / 2.0f), &Gdiplus::StringFormat(), &textBrush);

	// -- End control render, output result

	// Render out the final buffer
	dbGraphic.DrawImage(&dbBuffer, clientRect);
}

void WraithSettingButton::SetSelectedState(bool Value)
{
	// Set it
	this->IsSelected = Value;

	// Refresh
	this->Invalidate();
}

void WraithSettingButton::OnMouseMove(UINT nFlags, CPoint point)
{
	// Check to track
	if (!IsTracking)
	{
		// Set it
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = this->m_hWnd;

		// Track it
		if (TrackMouseEvent(&tme))
		{
			// Set
			IsTracking = true;
		}
	}

	// Check
	if (!this->IsHovering)
	{
		// Set
		this->IsHovering = true;
		// Redraw
		this->Invalidate();
	}

	// Handle base
	CStatic::OnMouseMove(nFlags, point);
}

LRESULT WraithSettingButton::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	// Reset
	this->IsHovering = false;
	this->Invalidate();

	// Stop tracker
	this->IsTracking = false;
	// Success
	return TRUE;
}