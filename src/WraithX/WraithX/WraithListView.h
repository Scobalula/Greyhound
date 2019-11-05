#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxcview.h>
#include <string>
#include <vector>

// We need the header
#include "WraithListViewHeader.h"

// A list of event delegates
typedef void(*ListViewInfoHandler)(LV_ITEM* ListItem, CWnd* Owner);

// A class that handles a WraithListView
class WraithListView : public CListCtrl
{
public:
    // -- ListView functions
    WraithListView();

    // Setup the list view, this must be called after the DDX has completed
    void InitializeListView();

    // Add a header to the control
    void AddHeader(const CString& HeaderName, uint32_t HeaderSize);
    // Remove a header
    void RemoveHeader(uint32_t Index);
    // Add an item to the control
    void AddItem();
    // Set an items text
    void SetItem(uint32_t Index, uint32_t Column, const CString& Value);
    // Remove an item
    void RemoveItem(uint32_t Index);
    // Remove all items
    void RemoveAllItems();

    // Search for an item, returns an index > -1 if found
    int32_t SearchForListItem(const CString& ItemName, int32_t Offset = -1);
    // Select an item and scroll it into view
    void SelectListItem(uint32_t ItemIndex);

    // Get the selected item indicies
    std::vector<uint32_t> GetSelectedItems() const;

    // Set the count if in virtual mode
    void SetVirtualCount(uint32_t ItemCount);

    // Occurs when we need to fetch list view item info
    ListViewInfoHandler OnGetListViewInfo;

private:
    // -- ListView helpers

    // Renders text, if a color hint is found it is used to render it
    void RenderColorhintText(CDC* pDC, COLORREF DefaultColor, bool ItemHighlighted, CString& sLabel, CRect& Region, UINT nFormat);

protected:

    // Our header instance
    WraithListViewHeader ListViewHeader;

    // Occures when an item needs rendering
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    // Handle NC paint override
    afx_msg void OnNcPaint();
    // Handle painting the background
    afx_msg BOOL OnEraseBkgnd(CDC* cDC);
    // Handle virtual mode requests
    afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);

    // Declare the message map instance
    DECLARE_MESSAGE_MAP()
};