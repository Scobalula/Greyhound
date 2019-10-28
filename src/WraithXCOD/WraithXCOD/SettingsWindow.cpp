#include "stdafx.h"

// The class we are implementing
#include "SettingsWindow.h"

// We need the Wraith theme and settings classes
#include "WraithTheme.h"
#include "SettingsManager.h"

// We need the panels
#include "GeneralSettings.h"
#include "ModelSettings.h"
#include "AnimSettings.h"
#include "ImageSettings.h"
#include "SoundSettings.h"
#include "GDTSettings.h"

// We need the following Wraith classes
#include "Strings.h"

BEGIN_MESSAGE_MAP(SettingsWindow, WraithWindow)
    ON_WM_PAINT()
    ON_COMMAND(IDC_GENERALPANEL, OnGeneralPage)
    ON_COMMAND(IDC_MODELPANEL, OnModelsPage)
    ON_COMMAND(IDC_ANIMPANEL, OnAnimsPage)
    ON_COMMAND(IDC_IMAGEPANEL, OnImagesPage)
    ON_COMMAND(IDC_SOUNDPANEL, OnSoundsPage)
    ON_COMMAND(IDC_GDTPANEL, OnGDTPage)
END_MESSAGE_MAP()

void SettingsWindow::DoDataExchange(CDataExchange* pDX)
{
    // Handle base
    WraithWindow::DoDataExchange(pDX);
    // Map our buttons properly
    DDX_Control(pDX, IDC_GENERALPANEL, GeneralButton);
    DDX_Control(pDX, IDC_MODELPANEL, ModelButton);
    DDX_Control(pDX, IDC_ANIMPANEL, AnimButton);
    DDX_Control(pDX, IDC_IMAGEPANEL, ImageButton);
    DDX_Control(pDX, IDC_SOUNDPANEL, SoundButton);
    DDX_Control(pDX, IDC_GDTPANEL, GDTButton);
}

void SettingsWindow::OnBeforeLoad()
{
    // Size
    this->MinimumWidth = 668;
    this->MinimumHeight = 398;

    // Adjust controls
    ShiftControl(IDC_SOUNDPANEL, CRect(0, -1, 0, 0));
    ShiftControl(IDC_GDTPANEL, CRect(0, -1, 0, 0));
}

void SettingsWindow::OnLoad()
{
    // Setup default panel
    CRect Size;
    // Fetch
    this->GetClientRect(&Size);

    SettingsPanel = std::make_unique<GeneralSettings>();
    SettingsPanel->Create(IDD_GENERALSETTINGS, this);
    SettingsPanel->MoveWindow(176, 0, Size.right - 177, Size.bottom);
    SettingsPanel->ShowWindow(SW_SHOW);
    // Set button
    this->GeneralButton.SetSelectedState(true);
}

void SettingsWindow::OnPaint()
{
    // Make a paint context
    CPaintDC dc(this);

    // Get client size
    CRect Size;
    // Fetch
    this->GetClientRect(&Size);
    // Fill the color
    dc.FillRect(CRect(0, 0, 176, Size.bottom), &CBrush(RGB(42, 42, 42)));

    // Draw rest
    WraithWindow::OnPaint();
}

void SettingsWindow::SetUnselected()
{
    // Set them
    this->GeneralButton.SetSelectedState(false);
    this->ModelButton.SetSelectedState(false);
    this->AnimButton.SetSelectedState(false);
    this->ImageButton.SetSelectedState(false);
    this->SoundButton.SetSelectedState(false);
    this->GDTButton.SetSelectedState(false);
}

void SettingsWindow::OnGeneralPage()
{
    // Clean up current
    if (SettingsPanel != nullptr)
    {
        SettingsPanel->DestroyWindow();
        SettingsPanel.reset();
    }

    // -- General

    CRect Size;
    // Fetch
    this->GetClientRect(&Size);

    SettingsPanel = std::make_unique<GeneralSettings>();
    SettingsPanel->Create(IDD_GENERALSETTINGS, this);
    SettingsPanel->MoveWindow(176, 0, Size.right - 177, Size.bottom);
    SettingsPanel->ShowWindow(SW_SHOW);

    this->SetUnselected();
    this->GeneralButton.SetSelectedState(true);
}

void SettingsWindow::OnModelsPage()
{
    // Clean up current
    if (SettingsPanel != nullptr)
    {
        SettingsPanel->DestroyWindow();
        SettingsPanel.reset();
    }

    // -- Models

    CRect Size;
    // Fetch
    this->GetClientRect(&Size);

    SettingsPanel = std::make_unique<ModelSettings>();
    SettingsPanel->Create(IDD_MODELSETTINGS, this);
    SettingsPanel->MoveWindow(176, 0, Size.right - 177, Size.bottom);
    SettingsPanel->ShowWindow(SW_SHOW);

    this->SetUnselected();
    this->ModelButton.SetSelectedState(true);
}

void SettingsWindow::OnAnimsPage()
{
    // Clean up current
    if (SettingsPanel != nullptr)
    {
        SettingsPanel->DestroyWindow();
        SettingsPanel.reset();
    }

    // -- Models

    CRect Size;
    // Fetch
    this->GetClientRect(&Size);

    SettingsPanel = std::make_unique<AnimSettings>();
    SettingsPanel->Create(IDD_ANIMSETTINGS, this);
    SettingsPanel->MoveWindow(176, 0, Size.right - 177, Size.bottom);
    SettingsPanel->ShowWindow(SW_SHOW);

    this->SetUnselected();
    this->AnimButton.SetSelectedState(true);
}

void SettingsWindow::OnImagesPage()
{
    // Clean up current
    if (SettingsPanel != nullptr)
    {
        SettingsPanel->DestroyWindow();
        SettingsPanel.reset();
    }

    // -- Models

    CRect Size;
    // Fetch
    this->GetClientRect(&Size);

    SettingsPanel = std::make_unique<ImageSettings>();
    SettingsPanel->Create(IDD_IMAGESETTINGS, this);
    SettingsPanel->MoveWindow(176, 0, Size.right - 177, Size.bottom);
    SettingsPanel->ShowWindow(SW_SHOW);

    this->SetUnselected();
    this->ImageButton.SetSelectedState(true);
}

void SettingsWindow::OnSoundsPage()
{
    // Clean up current
    if (SettingsPanel != nullptr)
    {
        SettingsPanel->DestroyWindow();
        SettingsPanel.reset();
    }

    // -- Models

    CRect Size;
    // Fetch
    this->GetClientRect(&Size);

    SettingsPanel = std::make_unique<SoundSettings>();
    SettingsPanel->Create(IDD_SOUNDSETTINGS, this);
    SettingsPanel->MoveWindow(176, 0, Size.right - 177, Size.bottom);
    SettingsPanel->ShowWindow(SW_SHOW);

    this->SetUnselected();
    this->SoundButton.SetSelectedState(true);
}

void SettingsWindow::OnGDTPage()
{
    // Clean up current
    if (SettingsPanel != nullptr)
    {
        SettingsPanel->DestroyWindow();
        SettingsPanel.reset();
    }

    // -- Models

    CRect Size;
    // Fetch
    this->GetClientRect(&Size);

    SettingsPanel = std::make_unique<GDTSettings>();
    SettingsPanel->Create(IDD_GDTSETTINGS, this);
    SettingsPanel->MoveWindow(176, 0, Size.right - 177, Size.bottom);
    SettingsPanel->ShowWindow(SW_SHOW);

    this->SetUnselected();
    this->GDTButton.SetSelectedState(true);
}