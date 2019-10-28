#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxcview.h>
#include <vector>
#include <memory>

// We need the resources
#include "resource.h"

// We need the WraithWindow and ListView classes
#include "WraithWindow.h"
#include "WraithListView.h"
#include "WraithProgressDialog.h"
#include "WraithPopup.h"
#include "WraithMenu.h"
#include "WraithCube.h"
#include "WraithSplitButton.h"
#include "WraithTextbox.h"

// We need the cod assets
#include "CoDAssets.h"

// A main window for COD
class MainWindow : public WraithWindow
{
public:
    // Create a new MainWindow
    MainWindow() : WraithWindow(IDD_MAINDIALOG) { ProgressDialog = nullptr; SearchMode = false; }

    // Our list instance, in WraithListView form
    WraithListView AssetListView;
    // Our cube instance
    WraithCube GameCube;
    // Our dropdown button control
    WraithSplitButton ExtraSplitMenu;
    // Our search box
    WraithTextbox SearchTextBox;

    // -- Search information

    // A list of search results in the current asset pool
    std::vector<CoDAsset_t*> SearchResults;
    // Whether or not we are currently in search
    bool SearchMode;

    // Our progress window
    std::unique_ptr<WraithProgressDialog> ProgressDialog;
    // Our popup window
    std::unique_ptr<WraithPopup> PopupDialog;

    // Load the game in async
    void LoadGameAsync();
    // Clear the game in async
    void ClearAllAsync();
    // Load a game file in async
    void LoadGameFileAsync(const std::string& FilePath);

protected:

    // Occurs when the window is loading
    virtual void OnBeforeLoad();
    // Occurs when the window is loaded
    virtual void OnLoad();
    // Occurs when we need to swap control instances
    virtual void DoDataExchange(CDataExchange* pDX);
    // Occurs when dragging on a file
    virtual void OnFilesDrop(const std::vector<std::wstring> Files);
    // Occurs when the listctrl is double clicked
    afx_msg void OnAssetListDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
    // Occurs when the dialog is closing
    afx_msg void OnDestroy();
    // Occurs when a message needs translation
    afx_msg BOOL PreTranslateMessage(MSG* pMsg);
    // Occurs when we need to update the cube icon
    afx_msg LRESULT UpdateCubeIcon(WPARAM wParam, LPARAM lParam);

    // Make the map
    DECLARE_MESSAGE_MAP()

private:

    // For loading list info
    static void GetListViewInfo(LV_ITEM* ListItem, CWnd* Owner);

    // For the progress callback
    static void OnProgressCallback(void* Caller, uint32_t Progress);
    // For the status callback
    static void OnStatusCallback(void* Caller, uint32_t Index);

    // Clear the search results
    void ClearSearch(bool ResetSearch = false);

    // For the progress dialog
    static void CancelProgress(CWnd* Owner);
    static void FinishProgress(CWnd* Owner);

    // -- Control commands

    void OnLoadGame();
    void OnClearAll();
    void OnSettings();
    void OnExportAll();
    void OnSupport();
    void OnLoadFile();
    void OnExportSelected();
    void OnSearch();
    void OnClearSearch();
    void OnMore();
    void OnAbout();
};