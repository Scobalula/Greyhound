#include "stdafx.h"

// The class we are implementing
#include "WraithListViewHeader.h"

// We need the theme
#include "WraithTheme.h"

// Our custom message map for WraithListViewHeader
BEGIN_MESSAGE_MAP(WraithListViewHeader, CHeaderCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_NCPAINT()
END_MESSAGE_MAP()

BOOL WraithListViewHeader::OnEraseBkgnd(CDC* cDC)
{
	// We can skip this
	return TRUE;
}

void WraithListViewHeader::OnNcPaint()
{
	// We don't need this
}

void WraithListViewHeader::OnPaint()
{
	// Setup the paint instance
	CPaintDC dc(this);
	// Fetch the clip box
	CRect rcClipBox;
	// Get the value
	dc.GetClipBox(rcClipBox);

	// Handle base event, we need to invoke drawitem on the required items
	auto ItemCount = this->GetItemCount();

	// Setup a draw item struct
	DRAWITEMSTRUCT dd;
	dd.hwndItem = m_hWnd;
	dd.hDC = dc.GetSafeHdc();

	// Buffer for an intersection of two points
	CRect IntersectPoint;

	// Loop through, render header items which intersect with the clip box
	for (auto i = 0; i < ItemCount; i++)
	{
		// Set the item ID
		dd.itemID = i;

		// The items rect
		CRect rcItem;
		// Fetch it
		this->GetItemRect(i, rcItem);

		// Check for an intersection
		if (IntersectPoint.IntersectRect(rcItem, rcClipBox))
		{
			// We had an intersection, set the clip rect
			dd.rcItem = rcItem;
			// Tell the control to be rendered
			DrawItem(&dd);
		}
	}

	// Check
	if (ItemCount > 0)
	{
		// Get last item bounds
		CRect rcBounds;
		// Fetch
		GetItemRect(ItemCount - 1, rcBounds);
		// Get client bounds
		CRect rcClient;
		// Fetch
		GetClientRect(&rcClient);
		// Compare
		if (rcBounds.right < rcClient.right)
		{
			// We need to render the remainder
			CWindowDC dc(this);
			// Make graphics
			Gdiplus::Graphics graphics(dc.GetSafeHdc());
			// Build area
			Gdiplus::Rect clientRect(rcBounds.right, 0, rcClient.right - rcBounds.right, rcBounds.bottom);
			// Make the gradient brush
			Gdiplus::LinearGradientBrush backBrush(clientRect, WraithTheme::HeaderFillGradTop, WraithTheme::HeaderFillGradBottom, 90.0f);
			// Make the border brush
			Gdiplus::SolidBrush borderBrush(WraithTheme::HeaderBorderColor);
			// The border pen
			Gdiplus::Pen borderPen(&borderBrush);

			// Fill the background
			graphics.FillRectangle(&backBrush, clientRect);
			// Draw the border
			graphics.DrawLine(&borderPen, Gdiplus::Point(rcBounds.right, clientRect.Height - 1), Gdiplus::Point(rcClient.right, clientRect.Height - 1));
		}
	}
}

void WraithListViewHeader::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// Build the client rect, this is only used when rendering the final cached bitmap
	Gdiplus::Rect clientRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top);
	// Build the draw rect from the client rect
	Gdiplus::Rect drawRect(0, 0, clientRect.Width, clientRect.Height);

	
	// -- Setup gdi utils

	// Setup double buffering image
	Gdiplus::Bitmap dbBuffer(clientRect.Width, clientRect.Height);
	// Setup double buffer graphics
	Gdiplus::Graphics dbGraphic(lpDrawItemStruct->hDC);
	// Setup main graphics
	Gdiplus::Graphics graphics(&dbBuffer);

	// Header count
	auto HeaderCount = this->GetItemCount();

	// Make the gradient brush
	Gdiplus::LinearGradientBrush backBrush(clientRect, WraithTheme::HeaderFillGradTop, WraithTheme::HeaderFillGradBottom, 90.0f);
	// Make the border brush
	Gdiplus::SolidBrush borderBrush(WraithTheme::HeaderBorderColor);
	// The border pen
	Gdiplus::Pen borderPen(&borderBrush);
	// Make the separator brush
	Gdiplus::SolidBrush sepBrush(WraithTheme::HeaderSeparatorColor);
	// The separator pen
	Gdiplus::Pen sepPen(&sepBrush);
	// Create the font
	Gdiplus::Font textFont(L"Microsoft Sans Serif", 8.25f);
	// Create the text brush
	Gdiplus::SolidBrush textBrush(WraithTheme::DefaultForeground);
	// Create the text format
	Gdiplus::StringFormat textFormat;
	// Set format
	textFormat.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
	textFormat.SetFormatFlags(textFormat.GetFormatFlags() | Gdiplus::StringFormatFlagsNoWrap | Gdiplus::StringFormatFlags::StringFormatFlagsNoFitBlackBox);

	// Get the item
	HDITEM hdi;
	// Buffer for text
	TCHAR  lpBuffer[256];
	// Set options
	hdi.mask = HDI_TEXT;
	hdi.pszText = lpBuffer;
	hdi.cchTextMax = 256;

	// Fetch info
	GetItem(lpDrawItemStruct->itemID, &hdi);
	// Make
	CString ItemText = CString(hdi.pszText);

	// Fill the background
	graphics.FillRectangle(&backBrush, drawRect);
	// Size
	Gdiplus::RectF TextSize;
	// Measure it
	graphics.MeasureString(ItemText, ItemText.GetLength(), &textFont, Gdiplus::PointF(0, 0), &TextSize);
	// Draw the border
	graphics.DrawLine(&borderPen, Gdiplus::Point(0, clientRect.Height - 1), Gdiplus::Point(clientRect.Width, clientRect.Height - 1));
	// Create the text layout rect
	Gdiplus::RectF textLayRect(1.0f, (lpDrawItemStruct->rcItem.bottom - TextSize.Height) / 2.0f, (float)clientRect.Width, (float)clientRect.Height);
	// Render it
	graphics.DrawString(ItemText, ItemText.GetLength(), &textFont, textLayRect, &textFormat, &textBrush);
	// Adjust
	auto AdjustWidth = (lpDrawItemStruct->itemID == (HeaderCount - 1)) ? 1 : 2;
	// Render the divider
	graphics.DrawLine(&sepPen, Gdiplus::Point(clientRect.Width - AdjustWidth, 0), Gdiplus::Point(clientRect.Width - AdjustWidth, clientRect.Height - 2));

	// -- End control render, output result

	// Render out the final buffer
	dbGraphic.DrawImage(&dbBuffer, clientRect);
}