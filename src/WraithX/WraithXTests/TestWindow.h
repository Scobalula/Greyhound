#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <afxcview.h>
#include <string>
#include <thread>

#include "resource.h"

// We need the WraithWindow and ListView classes
#include "WraithWindow.h"
#include "WraithListView.h"
#include "WraithAboutDialog.h"

// We need the string utilities
#include "Strings.h"

struct AssetTest
{
	std::wstring Name;
	std::wstring Size;
	std::wstring Bones;
};

// An about dialog
class TestAbout : public WraithAboutDialog
{
public:
	TestAbout(UINT nIDTemplate, CWnd* pParent = NULL) : WraithAboutDialog(nIDTemplate, pParent) { }

protected:

	virtual void OnBeforeLoad()
	{
		// Call base
		WraithAboutDialog::OnBeforeLoad();
		// Setup about
		this->AddColorText("Wraith Laura - The Evil Within extraction tool\n", RGB(50, 205, 50), 16, true);
		this->AddColorText("Developed by ", RGB(255, 255, 255), 13);
		this->AddColorText("DTZxPorter", RGB(236, 52, 202), 13, true);
		this->AddColorText(", some game research by ", RGB(255, 255, 255), 13);
		this->AddColorText("id-daemon", RGB(52, 152, 219), 13, true);
		this->AddColorText(".\n", RGB(255, 255, 255), 13);
		this->AddColorText("\n", RGB(255, 255, 255), 13);
		// Provided warning
		this->AddColorText("These tools are provided as-is with no warranty provided.\nYou must take note that EVERY asset exported using this tool is property of the game you extracted it from. Therefore you may not use them in any commercial environment and or profit off of them.\n\n", RGB(255, 255, 255), 13);
		// Rehost
		this->AddColorText("These tools may not be re-hosted at all. You must link directly to the Wraith extraction tools page. (", RGB(255, 255, 255), 13);
		this->AddColorText("http://aviacreations.com/wraith", RGB(255, 255, 255), 13, false, true, true);
		this->AddColorText(")\n\n", RGB(255, 255, 255), 13);
		// Changelog
		this->AddColorText("-- Changelog is available on the site --", RGB(50, 205, 50), 13);
	}
};

// A test window
class TestWindow : public WraithWindow
{
public:
	// Create a new test window (With our main window resource)
	TestWindow() : WraithWindow(IDD_MAINWIND) { lastSearch = ""; searchIndex = -1; }

	// Our list instance, in WraithListView form
	WraithListView AssetListView;
	// A list of wraith assets, for testing
	std::vector<AssetTest> Assets;

	int32_t searchIndex;
	std::string lastSearch;

private:
	static void GetListViewInfo(LV_ITEM* ListItem, CWnd* Owner)
	{
		// This is our testwindow instance
		auto WindowInstance = (TestWindow*)Owner;

		// Fetch
		if (ListItem->iItem < WindowInstance->Assets.size())
		{
			// Grab
			AssetTest& Asset = WindowInstance->Assets[ListItem->iItem];
			// Check it
			switch (ListItem->iSubItem)
			{
			case 0: _tcscpy_s(ListItem->pszText, ListItem->cchTextMax, Asset.Name.c_str()); break;
			case 1: _tcscpy_s(ListItem->pszText, ListItem->cchTextMax, Asset.Size.c_str()); break;
			case 2: _tcscpy_s(ListItem->pszText, ListItem->cchTextMax, Asset.Bones.c_str()); break;
			}
		}
	}

	// Before load event, used for control setup + adding
	virtual void OnBeforeLoad()
	{
		// Add test items
		for (auto i = 0; i < 300; i++)
		{
			// Add an item slot
			AssetTest asset;
			

			if (i % 2 == 0)
			{
				asset.Size = L"^0300";
				asset.Name = L"raygun";
			}
			else
			{
				asset.Size = L"^2300";
				asset.Name = L"thundergun";
			}

			asset.Bones = L"^41";

			Assets.emplace_back(asset);
		}
		// Hook listview
		AssetListView.OnGetListViewInfo = GetListViewInfo;
		// Set the control anchor points for this window
		SetControlAnchor(IDC_LIST1, 0, 0, 100, 100);
		SetControlAnchor(IDTEST1, 0, 100, 0, 0);
		SetControlAnchor(IDTEST2, 0, 100, 0, 0);
		SetControlAnchor(IDC_SPLIT1, 0, 100, 0, 0);
		SetControlAnchor(IDC_SPLIT2, 0, 100, 0, 0);
		SetControlAnchor(IDC_SETTINGTEST, 0, 100, 0, 0);
		SetControlAnchor(IDC_SETTINGTEST2, 0, 100, 0, 0);
		SetControlAnchor(IDC_RADIO1, 100, 0, 0, 0);
		SetControlAnchor(IDC_RADIO2, 100, 0, 0, 0);
		SetControlAnchor(IDC_RADIO3, 100, 0, 0, 0);
		SetControlAnchor(IDC_EDIT1, 100, 0, 0, 0);
		SetControlAnchor(IDC_EDIT2, 100, 0, 0, 0);
		SetControlAnchor(IDC_EDIT3, 100, 0, 0, 0);
		SetControlAnchor(IDC_EDIT4, 100, 0, 0, 0);
		SetControlAnchor(IDC_COMBO1, 100, 0, 0, 0);
		SetControlAnchor(IDC_COMBO2, 100, 0, 0, 0);
		SetControlAnchor(IDC_PROGRESS1, 100, 0, 0, 0);
		SetControlAnchor(IDC_PROGRESS2, 100, 0, 0, 0);
		SetControlAnchor(IDTEST3, 0, 100, 0, 0);
		SetControlAnchor(IDTEST4, 0, 100, 0, 0);
		// Add test columns

		// Insert a column This override is the most convenient
		AssetListView.AddHeader("Asset name", 220);
		AssetListView.AddHeader("Size", 80);
		AssetListView.AddHeader("Bones", 80);

		// Setup item count items
		AssetListView.SetVirtualCount(300);

		// Set assets loaded
		CString NewText;
		NewText.Format(L"Assets loaded: %d", 300);
		SetDlgItemText(IDC_ASSETSCOUNT, NewText);

		// Add formats
		auto ComboControl = (CComboBox*)GetDlgItem(IDC_COMBO1);

		// Add
		ComboControl->InsertString(0, L"DDS");
		ComboControl->InsertString(1, L"BMP");
		ComboControl->InsertString(2, L"PNG");
		ComboControl->InsertString(3, L"TGA");
		ComboControl->InsertString(4, L"TIFF");
		ComboControl->InsertString(5, L"JPEG");

		// Disabled
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS2))->SetPos(100);

		// Select DDS
		ComboControl->SetCurSel(0);
	}

	virtual void OnLoad()
	{
		// The window was loaded, and themed
	}

	virtual void OnFilesDrop(const std::vector<std::wstring> Files)
	{
		// Loop and print entries
		for (auto& File : Files)
		{
			// Print name
			printf("File: %s\n", Strings::ToNormalString(File).c_str());
		}
	}

	virtual void DoDataExchange(CDataExchange* pDX)
	{
		// Handle base
		WraithWindow::DoDataExchange(pDX);
		// Map our list control to a WraithListView
		DDX_Control(pDX, IDC_LIST1, AssetListView);
		// Initialize the view
		AssetListView.InitializeListView();
	}

	void TestSplitSearch()
	{
		if (lastSearch == "raygun")
		{
			searchIndex = AssetListView.SearchForListItem("raygun", searchIndex);
			printf("found: %d\n", searchIndex);
		}
		else
		{
			searchIndex = AssetListView.SearchForListItem("raygun");
			printf("found: %d\n", searchIndex);
		}
		lastSearch = "raygun";

		if (searchIndex > -1)
		{
			// Select it
			AssetListView.SelectListItem(searchIndex);
		}
		else
		{
			// Notify
			MessageBoxA(this->GetSafeHwnd(), "You have reached the end of the loaded assets.", "Wraith", MB_OK | MB_ICONINFORMATION);
		}
	}

	void AboutClick()
	{
		TestAbout About(IDD_ABOUTDLG, this);

		// Setup about text


		About.DoModal();
	}

protected:

	// Simulate loading a game
	afx_msg void LoadGamePress();
	// Simulate settings
	afx_msg void SettingsTest();

	// Make the map
	DECLARE_MESSAGE_MAP()
};