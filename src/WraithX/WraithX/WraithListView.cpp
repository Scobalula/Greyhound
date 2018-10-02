#include "stdafx.h"

// The class we are implementing
#include "WraithListView.h"

// We need the theme and strings
#include "WraithTheme.h"
#include "Strings.h"

// Our custom message map for WraithWindow
BEGIN_MESSAGE_MAP(WraithListView, CListCtrl)
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetdispinfo)
END_MESSAGE_MAP()

WraithListView::WraithListView() : CListCtrl()
{
	// Setup events
	this->OnGetListViewInfo = nullptr;
}

void WraithListView::InitializeListView()
{
	// Prepare to setup the listview

	// Enable owner draw
	this->ModifyStyle(0, LVS_OWNERDRAWFIXED);
	// Enable full row
	this->SetExtendedStyle(this->GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	// Subclass the CHeaderCtrl
	ListViewHeader.SubclassWindow(this->GetHeaderCtrl()->m_hWnd);
	// Set the background color, this applies for the header
	this->SetBkColor(RGB(WraithTheme::HeaderBackgroundColor.GetR(), WraithTheme::HeaderBackgroundColor.GetG(), WraithTheme::HeaderBackgroundColor.GetB()));
}

void WraithListView::AddHeader(const CString& HeaderName, uint32_t HeaderSize)
{
	// Add a header item
	auto HeaderCount = this->GetHeaderCtrl()->GetItemCount();

	// Add
	this->InsertColumn(HeaderCount + 1, HeaderName, LVCFMT_LEFT | HDF_OWNERDRAW, HeaderSize);
}

void WraithListView::RemoveHeader(uint32_t Index)
{
	// Remove
	this->DeleteColumn(Index);
}

void WraithListView::AddItem()
{
	// Get the count
	auto ItemCount = this->GetItemCount();

	// Add
	this->InsertItem(LVIF_TEXT | LVIF_STATE, ItemCount, L"", 0, LVIS_SELECTED, 0, 0);
}

void WraithListView::SetItem(uint32_t Index, uint32_t Column, const CString& Value)
{
	// Set the item text
	this->SetItemText(Index, Column, Value);
}

void WraithListView::RemoveItem(uint32_t Index)
{
	// Remove it
	this->DeleteItem(Index);
}

void WraithListView::RemoveAllItems()
{
	// Remove all
	this->DeleteAllItems();
}

int32_t WraithListView::SearchForListItem(const CString& ItemName, int32_t Offset)
{
	// Cache count
	auto ListCount = this->GetItemCount();

	// Copy our value
	CString SearchText = ItemName;
	// Trim it
	SearchText = SearchText.Trim();

	// Make sure we got something
	if (SearchText == "") { return -1; }

	// Iterate and search
	for (int i = Offset + 1; i < ListCount; i++)
	{
		// Search the first column
		CString szText = this->GetItemText(i, 0);
		// Compare name
		if (szText.Find(SearchText) > -1)
		{
			// Return it
			return i;
		}
	}
	
	// Not found
	return -1;
}

void WraithListView::SelectListItem(uint32_t ItemIndex)
{
	// Cache count
	auto ListCount = this->GetItemCount();

	// Clear current selection
	int SelectedIndex = -1;
	// Loop
	while ((SelectedIndex = GetNextItem(SelectedIndex, LVNI_SELECTED)) > -1)
	{
		// Set
		SetItemState(SelectedIndex, 0, LVIS_SELECTED);
	}
	
	// Ensure within bounds
	if (ItemIndex < (uint32_t)ListCount)
	{
		// Set focus to control
		SetFocus();
		// Select this item, and scroll into view
		SetItemState((int32_t)ItemIndex, LVIS_SELECTED, LVIS_SELECTED);
		// Scroll into view
		EnsureVisible((int32_t)ItemIndex, false);
	}
}

std::vector<uint32_t> WraithListView::GetSelectedItems() const
{
	// Build indicies
	std::vector<uint32_t> Result;

	POSITION SelectionStart = this->GetFirstSelectedItemPosition();
	// The selected index
	uint32_t SelectedIndex = 0;

	// Loop while positive
	while (SelectionStart)
	{
		// Set
		SelectedIndex = this->GetNextSelectedItem(SelectionStart);
		// Continue
		Result.push_back(SelectedIndex);
	}

	// Return it
	return Result;
}

void WraithListView::SetVirtualCount(uint32_t ItemCount)
{
	// Set it up
	this->SetItemCount(ItemCount);
}

BOOL WraithListView::OnEraseBkgnd(CDC* cDC)
{
	// The control rect, base rect, and header rect
	CRect ctrlRect, baseRect, headRect;

	// The height of an item
	int32_t itemHeight = 0x11;

	// Fetch the control and header rect
	this->GetClientRect(ctrlRect);
	this->GetHeaderCtrl()->GetClientRect(headRect);

	// Make the brushes for alternating rows
	CBrush brushDefault(RGB(WraithTheme::ListItemColorBase.GetR(), WraithTheme::ListItemColorBase.GetG(), WraithTheme::ListItemColorBase.GetB()));
	CBrush brushAlt(RGB(WraithTheme::ListItemColorAlt.GetR(), WraithTheme::ListItemColorAlt.GetG(), WraithTheme::ListItemColorAlt.GetB()));

	// The top level item index
	int32_t index = this->GetTopIndex();
	// The last visible index, + 1 for the overflow
	int32_t last_visible_index = index + this->GetCountPerPage() + 1;

	// Loop until we've rendered all the layers
	while (index <= last_visible_index)
	{
		// Fetch the current item
		this->GetItemRect(index, &baseRect, LVIR_BOUNDS);
		// Set the item height (minimum of 0x11 for no items)
		itemHeight = std::max<LONG>(0x11, baseRect.Height());
		// Set min rect size
		baseRect.top = std::max<LONG>(24, baseRect.top);
		baseRect.bottom = std::max<LONG>(41, baseRect.bottom);

		// Render the base items background
		cDC->FillRect(&baseRect, index % 2 ? &brushAlt : &brushDefault);

		// Calculate the part that has no column (right most side)
		ctrlRect.left = baseRect.right;
		ctrlRect.top = std::max<LONG>(baseRect.top, headRect.bottom);
		ctrlRect.bottom = baseRect.bottom;
		// Render the column part
		cDC->FillRect(&ctrlRect, index % 2 ? &brushAlt : &brushDefault);

		// Calculate the next item's position
		baseRect.top = baseRect.bottom;
		baseRect.bottom = baseRect.top + itemHeight;
		index++;
	}

	// We handled the event
	return TRUE;
}

void WraithListView::RenderColorhintText(CDC* pDC, COLORREF DefaultColor, bool ItemHighlighted, CString& sLabel, CRect& Region, UINT nFormat)
{
	// What we are drawing
	CString DrawString = sLabel;
	// Check for a color hint
	if (sLabel.GetLength() >= 2 && sLabel[0] == '^' && isdigit(sLabel[1]) != 0)
	{
		// We have a color
		DrawString = sLabel.Mid(2);
		// Set new color
		int ColorIndex = sLabel[1] - '0';
		// Only set if within bounds (And we aren't highlighted)
		if (!ItemHighlighted && ColorIndex > -1 && ColorIndex < 10)
		{
			// Set
			Gdiplus::Color& ColorUse = WraithTheme::ColorHints[ColorIndex];
			// Apply
			pDC->SetTextColor(RGB(ColorUse.GetR(), ColorUse.GetG(), ColorUse.GetB()));
		}
		else
		{
			// Default
			pDC->SetTextColor(DefaultColor);
		}
	}
	else
	{
		// Default
		pDC->SetTextColor(DefaultColor);
	}

	// Render it
	pDC->DrawText(DrawString, -1, Region, nFormat);
}

void WraithListView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: Call from onpaint

	// This is our item paint instance, used to render the control
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	// Save DC state for restoration later
	int nSavedDC = pDC->SaveDC();

	// Get item image and state info
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = lpDrawItemStruct->itemID;
	lvi.iSubItem = 0;
	lvi.stateMask = 0xFFFF;
	// Fetch item info
	this->GetItem(&lvi);

	// Should the item be highlighted
	bool ItemHighlighted = ((lvi.state & LVIS_DROPHILITED) || ((lvi.state & LVIS_SELECTED) && ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS))));

	// Get rectangles for drawing
	CRect rcBounds, rcLabel;
	// Fetch the bounds
	this->GetItemRect(lpDrawItemStruct->itemID, rcBounds, LVIR_BOUNDS);
	this->GetItemRect(lpDrawItemStruct->itemID, rcLabel, LVIR_LABEL);

	// Setup the column bounds
	CRect rcCol(rcBounds);

	// Fetch the text of the first item
	CString sLabel = this->GetItemText(lpDrawItemStruct->itemID, 0);

	// Labels are offset by a certain amount  
	// This offset is related to the width of a space character (Not used here)
	int offset = pDC->GetTextExtent(_T(" "), 1).cx + 1;

	// The rectangle of which to use for drawing the row
	CRect rcHighlight;
	// The client rectangle used for sizing
	CRect rcWnd;
	// Set it up
	rcHighlight = rcLabel;
	// Fetch client rectangle
	GetClientRect(&rcWnd);

	// Calculate new bounds
	rcHighlight = rcBounds;
	rcHighlight.left = 0;

	// Make the colors
	COLORREF TextHighlightColor = RGB(WraithTheme::DefaultHighlightForeground.GetR(), WraithTheme::DefaultHighlightForeground.GetG(), WraithTheme::DefaultHighlightForeground.GetB());
	COLORREF ItemHighlightColor = RGB(WraithTheme::DefaultHighlightBackground.GetR(), WraithTheme::DefaultHighlightBackground.GetG(), WraithTheme::DefaultHighlightBackground.GetB());
	COLORREF TextNormalColor = RGB(WraithTheme::DefaultForeground.GetR(), WraithTheme::DefaultForeground.GetG(), WraithTheme::DefaultForeground.GetB());
	COLORREF ItemDefaultColor = RGB(WraithTheme::ListItemColorBase.GetR(), WraithTheme::ListItemColorBase.GetG(), WraithTheme::ListItemColorBase.GetB());
	COLORREF ItemAltColor = RGB(WraithTheme::ListItemColorAlt.GetR(), WraithTheme::ListItemColorAlt.GetG(), WraithTheme::ListItemColorAlt.GetB());

	// Draw the background color
	if (ItemHighlighted)
	{
		// When we are highlighted, use white with black text
		pDC->SetTextColor(TextHighlightColor);
		pDC->SetBkColor(ItemHighlightColor);
		// White background
		pDC->FillRect(rcHighlight, &CBrush(ItemHighlightColor));
	}
	else
	{
		// Set the default text color
		pDC->SetTextColor(TextNormalColor);
		
		// Setup alternating row colors
		if (lpDrawItemStruct->itemID % 2 == 0)
		{
			// Use defaults
			pDC->FillRect(rcHighlight, &CBrush(ItemDefaultColor));
			pDC->SetBkColor(ItemDefaultColor);
		}
		else
		{
			// Use alts
			pDC->FillRect(rcHighlight, &CBrush(ItemAltColor));
			pDC->SetBkColor(ItemAltColor);
		}
	}

	// Set clip region for text
	rcCol.right = rcCol.left + this->GetColumnWidth(0);
	// The region buffer
	CRgn rgn;
	// Create it
	rgn.CreateRectRgnIndirect(&rcCol);
	// Select it
	pDC->SelectClipRgn(&rgn);
	// Clean it up
	rgn.DeleteObject();

	// Draw initial item label
	rcLabel.left += offset / 2;
	rcLabel.right -= offset;

	// Render out the text, parse color hint on the way
	RenderColorhintText(pDC, (ItemHighlighted) ? TextHighlightColor : TextNormalColor, ItemHighlighted, sLabel, rcLabel, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER | DT_END_ELLIPSIS);

	// Draw labels for remaining columns
	LV_COLUMN lvc;
	// Set new item mask
	lvc.mask = LVCF_FMT | LVCF_WIDTH;

	// Calculate the new item bounds
	rcBounds.right = rcHighlight.right > rcBounds.right ? rcHighlight.right : rcBounds.right;
	// Select it's clip
	rgn.CreateRectRgnIndirect(&rcBounds);
	// Select the new region
	pDC->SelectClipRgn(&rgn);

	for (int nColumn = 1; this->GetColumn(nColumn, &lvc); nColumn++)
	{
		// Adjust the clip
		rcCol.left = rcCol.right;
		rcCol.right += lvc.cx;

		// Get the next item text
		sLabel = this->GetItemText(lpDrawItemStruct->itemID, nColumn);

		// Skip over blank
		if (sLabel.GetLength() == 0) { continue; }

		// Get the text justification
		UINT nJustify = DT_LEFT;
		// Check it's justification
		switch (lvc.fmt & LVCFMT_JUSTIFYMASK)
		{
		case LVCFMT_RIGHT:
			nJustify = DT_RIGHT;
			break;
		case LVCFMT_CENTER:
			nJustify = DT_CENTER;
			break;
		default:
			break;
		}

		// Calculate text position
		rcLabel = rcCol;
		rcLabel.left += offset;
		rcLabel.right -= offset;

		// Render out the text, parse color hint on the way
		RenderColorhintText(pDC, (ItemHighlighted) ? TextHighlightColor : TextNormalColor, ItemHighlighted, sLabel, rcLabel, nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);
	}

	// Free it
	rgn.DeleteObject();

	// Restore the DC context
	pDC->RestoreDC(nSavedDC);
}

void WraithListView::OnNcPaint()
{
	// Handle base event
	CListCtrl::OnNcPaint();
	// Prepare to paint our listview border, depending on it's current state

	// -- Setup gdi utils

	// The actual area
	CRect AreaRegion;
	// Fetch
	this->GetWindowRect(&AreaRegion);
	// Setup main graphics
	Gdiplus::Graphics graphics(this->GetWindowDC()->GetSafeHdc());
	// The full client area
	Gdiplus::Rect clientRect(0, 0, AreaRegion.right - AreaRegion.left, AreaRegion.bottom - AreaRegion.top);

	// -- Begin control render

	// Create the border brush (border changes on disabled)
	Gdiplus::SolidBrush borderBrush(WraithTheme::ListItemColorBase);
	// Create the next border brush
	Gdiplus::SolidBrush nextBorderBrush(WraithTheme::ListItemColorAlt);
	// Create the pen
	Gdiplus::Pen borderPen(&borderBrush, 1.0f);
	// Create the pen
	Gdiplus::Pen nextBorderPen(&nextBorderBrush, 1.0f);

	// Render the background

	// Render the borders
	graphics.DrawRectangle(&borderPen, 0, 0, clientRect.Width - 1, clientRect.Height - 1);
	graphics.DrawRectangle(&nextBorderPen, 1, 1, clientRect.Width - 3, clientRect.Height - 3);

	// -- End control render
}

void WraithListView::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Lets see what we can do, first, fetch the display info
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// Fetch the specific item we need data for
	LV_ITEM* pItem = &(pDispInfo)->item;

	// We only support text based items in WraithListView (With color hints)
	if (pItem->mask & LVIF_TEXT)
	{
		// Ship off to event delegate if hooked
		if (OnGetListViewInfo != nullptr)
		{
			// Call it, with the owner
			OnGetListViewInfo(pItem, GetParentOwner());
		}
	}
}