#include "stdafx.h"

// The class we are implementing
#include "WraithSplitButton.h"

// We need the theme
#include "WraithTheme.h"
#include "WraithMenu.h"

// Our custom message map for WraithSplitButton
BEGIN_MESSAGE_MAP(WraithSplitButton, CSplitButton)
    ON_WM_PAINT()
    ON_NOTIFY_REFLECT(BCN_DROPDOWN, &OnDropDown)
END_MESSAGE_MAP()

WraithSplitButton::WraithSplitButton() : CSplitButton()
{
    // Defaults
}

WraithSplitButton::~WraithSplitButton()
{
    // Defaults
}

void WraithSplitButton::OnDropDown(NMHDR* pNMHDR, LRESULT* pResult)
{
    // Prepare to dropdown the button
    ASSERT(pResult != NULL);
    WraithMenu menu; // Used only for loading menu from resource

    // Use the pointer first
    std::unique_ptr<WraithMenu> pMenu = nullptr;
    // Use the Menu IDs if pointer is NULL
    if (pMenu == nullptr && m_nMenuId != (UINT)-1 && m_nSubMenuId != (UINT)-1)
    {
        // Load the root menu
        menu.LoadMenu(m_nMenuId);
        // Get the sub instance
        pMenu = menu.GetWraithSubMenu(m_nSubMenuId);
        pMenu->EnableOwnerDraw();

        // Apply background styles
        MENUINFO MenuInfo = { 0 };
        MenuInfo.cbSize = sizeof(MenuInfo);
        MenuInfo.hbrBack = ::CreateSolidBrush(RGB(54, 54, 54));
        MenuInfo.fMask = MIM_BACKGROUND;
        SetMenuInfo(pMenu->GetSafeHmenu(), &MenuInfo);
    }

    if (pMenu != nullptr)
    {
        CRect rectButton;
        this->GetWindowRect(&rectButton);

        TPMPARAMS tpmParams;
        tpmParams.cbSize = sizeof(TPMPARAMS);
        tpmParams.rcExclude = rectButton;

        pMenu->TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, rectButton.left, rectButton.bottom, this->GetParent(), &tpmParams);
    }
    *pResult = 0;
}

void WraithSplitButton::OnPaint()
{
    // Begin paint instance
    CPaintDC dc(this);

    // The button state
    bool ButtonPressed = ((this->GetState() & 0x0004) == 0x0004);
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
    Gdiplus::Graphics dbGraphic(dc);
    // Setup main graphics
    Gdiplus::Graphics graphics(&dbBuffer);
    // The full client area
    Gdiplus::Rect clientRect(0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);
    // The text client area
    Gdiplus::Rect textRect(0, 0, ClientRect.Width() - 18, clientRect.Height);

    // -- Begin control render

    // Create the gradient brush
    Gdiplus::LinearGradientBrush backBrush(clientRect, WraithTheme::DefaultControlGradTop, WraithTheme::DefaultControlGradBottom, (ButtonPressed) ? 270.0f : 90.0f);
    // Create the border brush (border changes on disabled)
    Gdiplus::SolidBrush borderBrush((this->IsWindowEnabled()) ? WraithTheme::DefaultControlBorder : WraithTheme::DisabledControlBorder);
    // Create the pen
    Gdiplus::Pen borderPen(&borderBrush, 1.0f);
    // Create the font
    Gdiplus::Font textFont(L"Microsoft Sans Serif", 8.25f);
    // Create the text brush
    Gdiplus::SolidBrush textBrush(WraithTheme::DefaultForeground);
    // Create the text format
    Gdiplus::StringFormat textFormat;
    // Create the text layout rect
    Gdiplus::RectF textLayRect(0.0f, 0.0f, (float)textRect.Width, (float)textRect.Height);
    // The text size
    Gdiplus::RectF textSize;

    // Render the background gradient
    graphics.FillRectangle(&backBrush, clientRect);
    // Render the border
    graphics.DrawRectangle(&borderPen, 0, 0, clientRect.Width - 1, clientRect.Height - 1);

    // Measure the text
    graphics.MeasureString(ButtonText, ButtonText.GetLength(), &textFont, textLayRect, &textSize);
    // Draw the text (slightly off center)
    graphics.DrawString(ButtonText, ButtonText.GetLength(), &textFont, Gdiplus::PointF((textRect.Width - textSize.Width) / 2.0f, (textRect.Height - textSize.Height) / 2.0f), &Gdiplus::StringFormat(), &textBrush);

    // Render the separator
    graphics.DrawLine(&borderPen, Gdiplus::Point(ClientRect.Width() - 18, 7), Gdiplus::Point(ClientRect.Width() - 18, ClientRect.Height() - 8));

    // Render the glyph
    Gdiplus::GraphicsPath Path;
    // Render
    graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);

    // Set the glyph transform
    Gdiplus::Rect Rectbutton(clientRect.Width - 18, 0, 18, clientRect.Height - 1);

    // Setup transform
    graphics.TranslateTransform(Rectbutton.X + Rectbutton.Width / 2.0f, Rectbutton.Y + Rectbutton.Height / 2.0f);

    // Make it
    Path.AddLine(Gdiplus::PointF(-6 / 2.0f, -3 / 2.0f), Gdiplus::PointF(6 / 2.0f, -3 / 2.0f));
    Path.AddLine(Gdiplus::PointF(6 / 2.0f, -3 / 2.0f), Gdiplus::PointF(0, 6 / 2.0f));

    // Close
    Path.CloseFigure();
    // Set the rotate
    graphics.RotateTransform(0);

    // Fill
    graphics.FillPath(&borderBrush, &Path);
    // Reset
    graphics.ResetTransform();

    // -- End control render, output result

    // Render out the final buffer
    dbGraphic.DrawImage(&dbBuffer, clientRect);
}