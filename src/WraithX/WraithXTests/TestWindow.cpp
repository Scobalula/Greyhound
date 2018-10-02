#include "stdafx.h"

// The class we are implementing
#include "TestWindow.h"

// We need the theme
#include "WraithTheme.h"
#include "WraithFileDialogs.h"
#include "WraithProgressDialog.h"
#include "WraithPopup.h"
#include "WraithPreview.h"

#include <thread>

BEGIN_MESSAGE_MAP(TestWindow, WraithWindow)
	ON_COMMAND(IDTEST2, LoadGamePress)
	ON_COMMAND(IDTEST3, SettingsTest)
	ON_COMMAND(IDC_SPLIT1, TestSplitSearch)
	ON_COMMAND(IDTEST4, AboutClick)
END_MESSAGE_MAP()

std::unique_ptr<WraithPreview> previewWindow;

class CAboutDlg : public WraithWindow
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_DIALOG1 };

	std::unique_ptr<WraithProgressDialog> dlg;
	std::unique_ptr<WraithPopup> popup;

protected:

	
	void Test();
	void Test2();
	virtual void OnBeforeLoad()
	{
		dlg = std::make_unique<WraithProgressDialog>(IDD_PROGRESSDLG, this);
		popup = std::make_unique<WraithPopup>(IDD_POPUPDLG, this);

		// Anchor buttons
		SetControlAnchor(IDOK, 100, 100, 0, 0);
		SetControlAnchor(IDCANCEL, 100, 100, 0, 0);
		// TODO: Set min / max
		
	}

	// Implementation
protected:

	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : WraithWindow(CAboutDlg::IDD)
{
}


void CAboutDlg::Test()
{
	this->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	//auto Result = WraithFileDialogs::OpenMultiFileDialog("Hello world!", "", "All Files (*.*)|*.*", this->GetSafeHwnd());
	//auto Result2 = WraithFileDialogs::SaveFileDialog("Save me!", "", "All Files (*.*)|*.*", this->GetSafeHwnd());
	
	dlg->SetupDialog("Wraith | Loading...", "Preparing for awesome...", true, true);
	dlg->DoModal();
	
	
	printf("Cause redraw\n");
}

void CAboutDlg::Test2()
{
	popup->ShowPopup("i_mtl_zombie_raygunmk3_c was exported", "D:\\HD Documents\\Visual Studio Projects\\WraithX\\WraithXTests", 6000);
	//popup->ShowPopup("i_mtl_zombie_raygunmk3_c i_mtl_zombie_raygunmk3_c i_mtl_zombie_raygunmk3_c i_mtl_zombie_raygunmk3_c i_mtl_zombie_raygunmk3_c i_mtl_zombie_raygunmk3_c i_mtl_zombie_raygunmk3_c has been exported", "", 6000);
}

BEGIN_MESSAGE_MAP(CAboutDlg, WraithWindow)
	ON_COMMAND(IDOK, Test)
	ON_COMMAND(IDCANCEL, Test2)
END_MESSAGE_MAP()



void TestWindow::SettingsTest()
{
	auto SelectedItems = AssetListView.GetSelectedItems();

	for (auto& Item : SelectedItems)
	{
		printf("Selected Item Index: %d\n", Item);
	}

	if (previewWindow == nullptr)
	{
		previewWindow = std::make_unique<WraithPreview>(IDD_PREVIEW);
	}
	previewWindow->ShowPreview();
	
	/*CAboutDlg test;
	test.DoModal();*/
}

void TestWindow::LoadGamePress()
{
	// The thread instance
	std::thread t([this]
	{
		for (auto i = 0; i < 300; i++)
		{
			
			try
			{
				if (this->AssetListView && this->AssetListView.m_hWnd)
				{
					// Set
					Assets[i].Bones = L"^3Exported";
					// Value
					this->AssetListView.Update(i);
					// Progress
					auto ProgressBar = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS1);
					ProgressBar->SetPos((int)(((float)(i + 1) / 300.0) * 100.0));
					Sleep(10);
				}
			}
			catch (...)
			{
				// Nothing
			}
		}
	});
	// Run freely
	t.detach();
}