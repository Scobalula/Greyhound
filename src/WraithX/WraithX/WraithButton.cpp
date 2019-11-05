#include "stdafx.h"

// The class we are implementing
#include "WraithButton.h"

// We need the theme
#include "WraithTheme.h"

LRESULT CALLBACK WraithButton::WndProcWraithButton(HWND hButton, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Prepare to handle the custom events

    // Check what WM_ message we got
    switch (message)
    {
    case WM_PAINT:
    // Handle the rendering of our custom WraithButton
    {
        // Fetch the info
        PAINTSTRUCT ps;
        // Start the paint instance
        HDC hDC = BeginPaint(hButton, &ps);
        // Perform paint operation
        OnPaint(hButton, hDC, ps);
        // Stop painting
        EndPaint(hButton, &ps);
    }
    // We're done here
    return TRUE;
    case WM_ENABLE:
    // Handle painting before we disable
    {
        // Redraw
        CWnd::FromHandle(hButton)->Invalidate();
    }
    // We're done here
    return FALSE;
    case WM_SETTEXT:
    {
        // Call base
        DefSubclassProc(hButton, message, wParam, lParam);
        // Redraw
        CWnd::FromHandle(hButton)->Invalidate();
    }
    // We're done here
    return TRUE;
    case WM_NCDESTROY:
    // Handle freeing the subclass instance
    {
        // Remove the subclass
        RemoveWindowSubclass(hButton, WndProcWraithButton, uIdSubclass);
    }
    break;
    }

    // Handle default message, we don't need it
    return DefSubclassProc(hButton, message, wParam, lParam);
}

void WraithButton::OnPaint(HWND hButton, HDC hDC, PAINTSTRUCT& pPaintStruct)
{
    // Prepare to paint our button, depending on it's current state
    auto ThisButton = (CButton*)CWnd::FromHandle(hButton);
    // The button state
    bool ButtonPressed = ((ThisButton->GetState() & 0x0004) == 0x0004);
    // The button text
    CString ButtonText;
    // Fetch button text
    ThisButton->GetWindowText(ButtonText);
    // The client rect
    CRect ClientRect;
    // Get it
    ThisButton->GetClientRect(&ClientRect);

    // -- Setup gdi utils

    // Setup double buffering image
    Gdiplus::Bitmap dbBuffer(ClientRect.right, ClientRect.bottom);
    // Setup double buffer graphics
    Gdiplus::Graphics dbGraphic(hDC);
    // Setup main graphics
    Gdiplus::Graphics graphics(&dbBuffer);
    // The full client area
    Gdiplus::Rect clientRect(0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);

    // -- Begin control render

    // Create the gradient brush
    Gdiplus::LinearGradientBrush backBrush(clientRect, WraithTheme::DefaultControlGradTop, WraithTheme::DefaultControlGradBottom, (ButtonPressed) ? 270.0f : 90.0f);
    // Create the border brush (border changes on disabled)
    Gdiplus::SolidBrush borderBrush((ThisButton->IsWindowEnabled()) ? WraithTheme::DefaultControlBorder : WraithTheme::DisabledControlBorder);
    // Create the pen
    Gdiplus::Pen borderPen(&borderBrush, 1.0f);
    // Create the font
    Gdiplus::Font textFont(L"Microsoft Sans Serif", 8.25f);
    // Create the text brush
    Gdiplus::SolidBrush textBrush(WraithTheme::DefaultForeground);
    // Create the text format
    Gdiplus::StringFormat textFormat;
    // Create the text layout rect
    Gdiplus::RectF textLayRect(0.0f, 0.0f, (float)clientRect.Width, (float)clientRect.Height);
    // The text size
    Gdiplus::RectF textSize;

    // Render the background gradient
    graphics.FillRectangle(&backBrush, clientRect);
    // Render the border
    graphics.DrawRectangle(&borderPen, 0, 0, clientRect.Width - 1, clientRect.Height - 1);
    
    // Measure the text
    graphics.MeasureString(ButtonText, ButtonText.GetLength(), &textFont, textLayRect, &textSize);
    // Draw the text
    graphics.DrawString(ButtonText, ButtonText.GetLength(), &textFont, Gdiplus::PointF((clientRect.Width - textSize.Width) / 2.0f, (clientRect.Height - textSize.Height) / 2.0f), &Gdiplus::StringFormat(), &textBrush);

    // -- End control render, output result

    // Render out the final buffer
    dbGraphic.DrawImage(&dbBuffer, clientRect);
}