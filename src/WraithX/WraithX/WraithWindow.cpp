#include "stdafx.h"

// The class we are implementing
#include "WraithWindow.h"

// We need the custom control classes
#include "WraithButton.h"
#include "WraithCheckbox.h"
#include "WraithRadio.h"
#include "WraithTextbox.h"
#include "WraithCombobox.h"
#include "WraithProgressBar.h"
#include "WraithSplitButton.h"
#include "WraithGroupBox.h"
#include "WraithTheme.h"

// Our custom message map for WraithWindow
BEGIN_MESSAGE_MAP(WraithWindow, CResizableDialog)
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(IDOK, OnDenySubmit)
	ON_COMMAND(IDCANCEL, OnDenySubmit)
	ON_WM_SIZE()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

WraithWindow::WraithWindow() : CResizableDialog()
{
	// Set initial sizing
	MinimumWidth = -1;
	MinimumHeight = -1;
	MaximumWidth = -1;
	MaximumHeight = -1;
	// Setup colors
	BackgroundBrush.CreateSolidBrush(RGB(WraithTheme::DefaultControlBackground.GetR(), WraithTheme::DefaultControlBackground.GetG(), WraithTheme::DefaultControlBackground.GetB()));
}

WraithWindow::WraithWindow(UINT nIDTemplate, CWnd* pParent) : CResizableDialog(nIDTemplate, pParent)
{
	// Set initial sizing
	MinimumWidth = -1;
	MinimumHeight = -1;
	MaximumWidth = -1;
	MaximumHeight = -1;
	// Setup colors
	BackgroundBrush.CreateSolidBrush(RGB(WraithTheme::DefaultControlBackground.GetR(), WraithTheme::DefaultControlBackground.GetG(), WraithTheme::DefaultControlBackground.GetB()));
}

WraithWindow::~WraithWindow()
{
	// Clean up if need be
}

BOOL WraithWindow::OnInitDialog()
{
	// Call base
	CResizableDialog::OnInitDialog();
	// Set the icon from the wraith theme, this can be changed on load / before load
	this->SetIcon(WraithTheme::ApplicationIcon, false);
	// Call before load event
	OnBeforeLoad();

	// Enumerate all controls then apply WraithTheme to them
	CWnd* pChild = this->GetWindow(GW_CHILD);
	// Loop until end
	while (pChild)
	{ 
		// Check it's type
		WCHAR NameBuffer[400];
		// Fetch it
		GetClassName(pChild->GetSafeHwnd(), NameBuffer, 400);
		// Convert
		CString ControlType(NameBuffer);

		// Now, check type and continue
		if (ControlType == "Button")
		{
			// We have a CButton
			auto ButtonInstance = (CButton*)pChild;
			auto ButtonType = ButtonInstance->GetButtonStyle();
			// Check type and set it up
			if (ButtonType == BS_PUSHBUTTON || ButtonType == BS_DEFPUSHBUTTON)
			{
				// Normal button
				SetWraithButtonCtrl(GetWindowLong(pChild->GetSafeHwnd(), GWL_ID));
			}
			else if (ButtonType == BS_CHECKBOX || ButtonType == BS_AUTOCHECKBOX)
			{
				// Checkbox
				SetWraithCheckboxCtrl(GetWindowLong(pChild->GetSafeHwnd(), GWL_ID));
			}
			else if (ButtonType == BS_AUTORADIOBUTTON || ButtonType == BS_RADIOBUTTON)
			{
				// Radio
				SetWraithRadioCtrl(GetWindowLong(pChild->GetSafeHwnd(), GWL_ID));
			}
			else if (ButtonType == BS_GROUPBOX)
			{
				// Group box
				SetWraithGroupBoxCtrl(GetWindowLong(pChild->GetSafeHwnd(), GWL_ID));
			}
		}
		else if (ControlType == "Static")
		{
			// We have a CStatic just refresh it, DDX handles these
			((CWnd*)pChild)->Invalidate();
		}
		else if (ControlType == "Edit")
		{
			// We have a textbox just refresh it, DDX handles these
			((CWnd*)pChild)->Invalidate();
		}
		else if (ControlType == "ComboBox")
		{
			// We have a combobox
			SetWraithComboboxCtrl(GetWindowLong(pChild->GetSafeHwnd(), GWL_ID));
		}
		else if (ControlType == "SysListView32")
		{
			// We have a list view, just refresh it, DDX handles these
			((CWnd*)pChild)->Invalidate();
		}
		else if (ControlType == "msctls_progress32")
		{
			// We have a progress
			SetWraithProgressBarCtrl(GetWindowLong(pChild->GetSafeHwnd(), GWL_ID));
		}
		else if (ControlType == "RichEdit20W")
		{
			// Just invalidate it
			((CWnd*)pChild)->Invalidate();
		}
		else
		{
			// Debug log for new controls
#ifdef _DEBUG
			wprintf(L"Control type with no WraithTheme: %s\n", (LPCWSTR)ControlType);
#endif
		}

		// Get next one
		pChild = pChild->GetNextWindow(); 
	}

	// Call the Loaded event after applying theme
	OnLoad();
	// We were successful
	return TRUE;
}

void WraithWindow::OnLoad()
{
	// Default logic is to do nothing
}

void WraithWindow::OnBeforeLoad()
{
	// Default logic is to do nothing
}

void WraithWindow::OnFilesDrop(std::vector<std::wstring> Files)
{
	// Default logic is to do nothing
}

void WraithWindow::SetWraithButtonCtrl(UINT cID)
{
	// Prepare to setup the control using the WraithButton WNDPROC
	auto ButtonInstance = GetDlgItem(cID);
	auto ButtonHandle = ButtonInstance->GetSafeHwnd();

	// Disable built-in theme for the button, helps with rendering
	SetWindowTheme(ButtonHandle, L" ", L" ");

	// The resulting ID
	UINT_PTR uIdSubclass = 0;

	// Apply the subclass
	SetWindowSubclass(ButtonHandle, WraithButton::WndProcWraithButton, uIdSubclass, 0);

	// Lets tell the button to redraw it's area
	ButtonInstance->Invalidate();
}

void WraithWindow::SetWraithGroupBoxCtrl(UINT cID)
{
	// Prepare to setup the control using the WraithGroupBox WNDPROC
	auto ButtonInstance = GetDlgItem(cID);
	auto ButtonHandle = ButtonInstance->GetSafeHwnd();

	// The resulting ID
	UINT_PTR uIdSubclass = 0;

	// Apply the subclass
	SetWindowSubclass(ButtonHandle, WraithGroupBox::WndProcWraithGroupBox, uIdSubclass, 0);

	// Lets tell the groupbox to redraw it's area
	ButtonInstance->Invalidate();
}

void WraithWindow::SetWraithCheckboxCtrl(UINT cID)
{
	// Prepare to setup the control using the WraithCheckbox WNDPROC
	auto CheckboxInstance = GetDlgItem(cID);
	auto CheckboxHandle = CheckboxInstance->GetSafeHwnd();

	// The resulting ID
	UINT_PTR uIdSubclass = 0;

	// Apply the subclass
	SetWindowSubclass(CheckboxHandle, WraithCheckbox::WndProcWraithCheckbox, uIdSubclass, 0);

	// Lets tell the checkbox to redraw it's area
	CheckboxInstance->Invalidate();
}

void WraithWindow::SetWraithRadioCtrl(UINT cID)
{
	// Prepare to setup the control using the WraitRadio WNDPROC
	auto RadioInstance = GetDlgItem(cID);
	auto RadioHandle = RadioInstance->GetSafeHwnd();

	// The resulting ID
	UINT_PTR uIdSubclass = 0;

	// Apply the subclass
	SetWindowSubclass(RadioHandle, WraithRadio::WndProcWraithRadio, uIdSubclass, 0);

	// Lets tell the radio to redraw it's area
	RadioInstance->Invalidate();
}

void WraithWindow::SetWraithComboboxCtrl(UINT cID)
{
	// Prepare to setup the control using the WraithCheckbox WNDPROC
	auto CheckboxInstance = GetDlgItem(cID);
	auto CheckboxHandle = CheckboxInstance->GetSafeHwnd();

	// The resulting ID
	UINT_PTR uIdSubclass = 0;

	// Apply the subclass
	SetWindowSubclass(CheckboxHandle, WraithCombobox::WndProcWraithCombobox, uIdSubclass, 0);

	// Lets tell the checkbox to redraw it's area
	CheckboxInstance->Invalidate();
}

void WraithWindow::SetWraithProgressBarCtrl(UINT cID)
{
	// Prepare to setup the control using the WraithProgressBar WNDPROC
	auto ProgressInstance = GetDlgItem(cID);
	auto ProgressHandle = ProgressInstance->GetSafeHwnd();

	// The resulting ID
	UINT_PTR uIdSubclass = 0;

	// Apply the subclass
	SetWindowSubclass(ProgressHandle, WraithProgressBar::WndProcWraithProgressBar, uIdSubclass, 0);

	// Lets tell the checkbox to redraw it's area
	ProgressInstance->Invalidate();
}

void WraithWindow::MoveControl(UINT cID, CRect cPosition)
{
	// Get the control, if possible
	auto ControlHandle = GetDlgItem(cID);
	// Ensure we got it
	if (ControlHandle != NULL)
	{
		// Move it
		ControlHandle->MoveWindow(cPosition, true);
	}
}

void WraithWindow::ShiftControl(UINT cID, CRect cShift)
{
	// Get the control, if possible
	auto ControlHandle = GetDlgItem(cID);
	// Ensure we got it
	if (ControlHandle != NULL)
	{
		// Grab the current position
		CRect CurrentPos;
		// Fetch it
		ControlHandle->GetWindowRect(CurrentPos);
		// Set the new position
		ScreenToClient(&CurrentPos);
		// Adjust
		CurrentPos.top += cShift.top;
		CurrentPos.left += cShift.left;
		CurrentPos.right += cShift.right + cShift.left;
		CurrentPos.bottom += cShift.bottom + cShift.top;
		// Set
		ControlHandle->MoveWindow(CurrentPos, true);
	}
}

void WraithWindow::DoDataExchange(CDataExchange* pDX)
{
	// Handle base event
	CResizableDialog::DoDataExchange(pDX);
}

void WraithWindow::OnClose()
{
	// Base event
	CResizableDialog::OnClose();
	// Close this dialog
	EndDialog(0);
}

void WraithWindow::OnDropFiles(HDROP hDROP)
{
	// Parse files, then pass to event handler
	uint32_t FileCount = DragQueryFile(hDROP, 0xFFFFFFFF, NULL, NULL);

	// The char instance
	WCHAR FileBuffer[MAX_PATH] = { 0 };

	// Buffer for result
	std::vector<std::wstring> Files;
	
	// Loop if we have some
	if (FileCount > 0)
	{
		// Loop
		for (uint32_t i = 0; i < FileCount; i++)
		{
			// Clear
			std::memset(&FileBuffer, 0, (sizeof(WCHAR) * MAX_PATH));
			// Load it
			if (DragQueryFile(hDROP, i, FileBuffer, MAX_PATH))
			{
				// We loaded it, add it to list
				Files.push_back(std::wstring(FileBuffer));
			}
		}
	}

	// We're done
	DragFinish(hDROP);

	// Call window event
	OnFilesDrop(Files);
}

BOOL WraithWindow::OnEraseBkgnd(CDC* pDC)
{
	// Prepare to render it
	Gdiplus::Graphics graphics(pDC->GetSafeHdc());
	// Make the brush
	Gdiplus::SolidBrush backBrush(WraithTheme::DefaultControlBackground);
	// Get size
	CRect SizeRect;
	// Fetch
	GetClientRect(&SizeRect);
	// Render it
	graphics.FillRectangle(&backBrush, 0, 0, SizeRect.right, SizeRect.bottom);
	// Worked
	return TRUE;
}

HBRUSH WraithWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Set the text color to theme color
	pDC->SetTextColor(RGB(WraithTheme::DefaultForeground.GetR(), WraithTheme::DefaultForeground.GetG(), WraithTheme::DefaultForeground.GetB()));

	// Set the background color the same as default control background
	pDC->SetBkColor(RGB(WraithTheme::DefaultControlBackground.GetR(), WraithTheme::DefaultControlBackground.GetG(), WraithTheme::DefaultControlBackground.GetB()));

	// Return the brush
	return BackgroundBrush;
}

void WraithWindow::OnDenySubmit()
{
	// Handle IDOK / IDCANCEL here.
}

void WraithWindow::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	// Handle base
	CResizableDialog::OnGetMinMaxInfo(lpMMI);
	// Adjust to taste
	if (MinimumWidth > -1)
	{
		lpMMI->ptMinTrackSize.x = MinimumWidth;
	}
	if (MinimumHeight > -1)
	{
		lpMMI->ptMinTrackSize.y = MinimumHeight;
	}
	if (MinimumWidth > -1)
	{
		lpMMI->ptMaxTrackSize.x = MaximumWidth;
	}
	if (MaximumHeight > -1)
	{
		lpMMI->ptMaxTrackSize.y = MaximumHeight;
	}
}