#include "stdafx.h"

// The class we are implementing
#include "WraithMenu.h"

// We need the theme
#include "WraithTheme.h"

WraithMenu::WraithMenu() : CMenu()
{
	// Prepare the control
}

WraithMenu::~WraithMenu()
{
	// Defaults
}

void WraithMenu::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// Setup the renderer
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	

	// If the menu item is selected
	if ((lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		// Draw a highlighted background
		pDC->FillSolidRect(&lpDrawItemStruct->rcItem, RGB(44, 44, 44));
	}
	else
	{
		// Draw the normal background
		pDC->FillSolidRect(&lpDrawItemStruct->rcItem, RGB(36, 36, 36));
	}

	// Prepare to render text
	CString TextBuffer;
	this->GetMenuStringW(lpDrawItemStruct->itemID, TextBuffer, MF_BYCOMMAND);

	// Set the color and render
	pDC->SetTextColor(RGB(255, 255, 255));
	pDC->SetTextAlign(TA_TOP | TA_NOUPDATECP);
	
	// Make bounds
	CRect Base;
	Base.left = 8;
	Base.right = lpDrawItemStruct->rcItem.right - 8;
	Base.bottom = lpDrawItemStruct->rcItem.bottom;
	Base.top = lpDrawItemStruct->rcItem.top;
	// Render
	pDC->DrawText(TextBuffer, &Base, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

void WraithMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// Setup sizes
	lpMeasureItemStruct->itemHeight = 20;
	lpMeasureItemStruct->itemWidth = 100;
}

std::unique_ptr<WraithMenu> WraithMenu::GetWraithSubMenu(int nPos) const
{
	// Fetch the menu
	auto SubHandle = ::GetSubMenu(*((HMENU *)this + 1), nPos);
	auto Result = std::make_unique<WraithMenu>();
	Result->Attach(SubHandle);

	// Return it
	return Result;
}

void WraithMenu::EnableOwnerDraw()
{
	// Get the number of the menu items of the parent menu
	int iMenuCount = this->GetMenuItemCount();
	UINT nID; // Use to hold the identifier of the menu items
	for (int i = 0; i < iMenuCount; i++)
	{
		// If the parent menu has sub menu
		if (IsMenu(this->GetSubMenu(i)->GetSafeHmenu()))
		{
			((WraithMenu*)this->GetSubMenu(i))->EnableOwnerDraw();
		}
		else
		{
			nID = this->GetMenuItemID(i);
			this->ModifyMenu(i, MF_BYPOSITION | MF_OWNERDRAW, (UINT_PTR)nID, (LPWSTR)NULL);
		}
	}
}