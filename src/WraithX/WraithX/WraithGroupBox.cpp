#include "stdafx.h"

// The class we are implementing
#include "WraithGroupBox.h"

// We need the theme
#include "WraithTheme.h"

LRESULT CALLBACK WraithGroupBox::WndProcWraithGroupBox(HWND hButton, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
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
    case WM_NCDESTROY:
        // Handle freeing the subclass instance
    {
        // Remove the subclass
        RemoveWindowSubclass(hButton, WndProcWraithGroupBox, uIdSubclass);
    }
    break;
    }

    // Handle default message, we don't need it
    return DefSubclassProc(hButton, message, wParam, lParam);
}

void WraithGroupBox::OnPaint(HWND hButton, HDC hDC, PAINTSTRUCT& pPaintStruct)
{
    // Prepare to paint our button, depending on it's current state
    auto ThisButton = (CButton*)CWnd::FromHandle(hButton);
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

    // Create the back brush
    Gdiplus::SolidBrush backBrush(WraithTheme::DefaultControlBackground);
    // Create the border brush (border changes on disabled)
    Gdiplus::SolidBrush borderBrush((ThisButton->IsWindowEnabled()) ? WraithTheme::DefaultForeground : WraithTheme::DisabledControlBorder);
    // Create the pen
    Gdiplus::Pen borderPen(&borderBrush, 1.0f);
    // Create the font
    Gdiplus::Font textFont(L"Microsoft Sans Serif", 12.0f);
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

    // Measure the text
    graphics.MeasureString(ButtonText, ButtonText.GetLength(), &textFont, textLayRect, &textSize);

    // Render the borders
    graphics.DrawLine(&borderPen, 0, (int)(textSize.Height / 2.0), 12, (int)(textSize.Height / 2.0));
    graphics.DrawLine(&borderPen, (int)(textSize.Width + 10.0f), (int)(textSize.Height / 2.0), clientRect.Width - 1, (int)(textSize.Height / 2.0));
    // Render borders left / right
    graphics.DrawLine(&borderPen, 0, (int)(textSize.Height / 2.0), 0, clientRect.Height - 1);
    graphics.DrawLine(&borderPen, clientRect.Width - 1, (int)(textSize.Height / 2.0), clientRect.Width - 1, clientRect.Height - 1);
    // Render border bottom
    graphics.DrawLine(&borderPen, 0, clientRect.Height - 1, clientRect.Width - 1, clientRect.Height - 1);

    // Draw the text
    graphics.DrawString(ButtonText, ButtonText.GetLength(), &textFont, Gdiplus::PointF(12, 0), &Gdiplus::StringFormat(), &textBrush);

    // -- End control render, output result

    // Render out the final buffer
    dbGraphic.DrawImage(&dbBuffer, clientRect);
}