#include "stdafx.h"

// The class we are implementing
#include "WraithPopup.h"

// We need the following Wraith classes
#include "Strings.h"
#include "FileSystems.h"
#include "WraithTheme.h"

BEGIN_MESSAGE_MAP(WraithPopup, WraithWindow)
    ON_WM_TIMER()
    ON_WM_PAINT()
    ON_WM_SETCURSOR()
    ON_WM_MOUSELEAVE()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

WraithPopup::WraithPopup(UINT nIDTemplate, CWnd* pParent) : WraithWindow(nIDTemplate, pParent)
{
    // Defaults
    CloseTimer = NULL;
    PopupTemplateID = nIDTemplate;
    PopupParent = pParent;
    WasCreated = false;
    MouseTrackingMode = false;
    WasLeftButtonDown = false;
    // Load hand
    HandCursor = LoadCursor(NULL, IDC_HAND);
}

void WraithPopup::AlignWindow()
{
    // Align the window with the bottom right corner
    CRect WindowRect;
    // Fetch it
    this->GetWindowRect(WindowRect);

    // Get the monitor from the rect
    HMONITOR WindowMonitor = MonitorFromWindow((this->PopupParent != NULL) ? this->PopupParent->GetSafeHwnd() : this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);

    // Allocate a buffer for info
    MONITORINFO MonitorInfo;
    // Set size
    MonitorInfo.cbSize = sizeof(MonitorInfo);
    // Fetch the info
    GetMonitorInfo(WindowMonitor, &MonitorInfo);

    // Get the region
    RECT WindowWorkArea;
    // Grab
    WindowWorkArea = MonitorInfo.rcWork;

    // Position the window in the bottom left
    this->SetWindowPos(NULL, WindowWorkArea.right - (WindowRect.Width()), WindowWorkArea.bottom - (WindowRect.Height()), 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void WraithPopup::ShowPopup(const CString& Message, const CString& Directory, uint32_t Milliseconds)
{
    // Set the message, copy the directory and prepare the timer
    DirectoryOpen = Directory;
    // Show the dialog
    if (!WasCreated)
    {
        // Create
        this->Create(PopupTemplateID, PopupParent);
        // Set
        WasCreated = true;
    }
    // Show us
    this->ShowWindow(SW_SHOW);
    // Set the position
    this->AlignWindow();
    // Set message
    this->DisplayMessage = Message;
    // Force redraw
    this->Invalidate();
    // Setup timer
    if (CloseTimer != NULL)
    {
        // Kill it first
        KillTimer(CloseTimer);
    }
    // Setup a new one
    CloseTimer = SetTimer(1, Milliseconds, NULL);
}

void WraithPopup::PreparePopup()
{
    // Create if not already
    if (!WasCreated)
    {
        // Create
        this->Create(PopupTemplateID, PopupParent);
        // Set
        WasCreated = true;
    }
}

void WraithPopup::OnTimer(UINT_PTR nIDEvent)
{
    // Call base handler
    WraithWindow::OnTimer(nIDEvent);

    // Kill the timer, one time event
    KillTimer(CloseTimer);
    // Reset it
    CloseTimer = NULL;

    // Hide us
    this->ShowWindow(SW_HIDE);
}

void WraithPopup::OnPaint()
{
    // Allow the base to render first
    WraithWindow::OnPaint();

    // Grab a paint context
    CWindowDC dc(this);
    // Get the size
    CRect WindowRect;
    // Fetch it
    this->GetWindowRect(WindowRect);

    // -- Setup gdi utils

    // Setup double buffering image
    Gdiplus::Bitmap dbBuffer(WindowRect.Width(), WindowRect.Height());
    // Setup double buffer graphics
    Gdiplus::Graphics dbGraphic(dc.GetSafeHdc());
    // Setup main graphics
    Gdiplus::Graphics graphics(&dbBuffer);
    // The full client area
    Gdiplus::Rect clientRect(0, 0, WindowRect.Width(), WindowRect.Height());

    // -- Begin control render

    // Create the gradient brush
    Gdiplus::LinearGradientBrush backBrush(clientRect, WraithTheme::DefaultControlGradTop, WraithTheme::DefaultControlGradBottom, 90.0f);
    // Create the border brush (border changes on disabled)
    Gdiplus::SolidBrush borderBrush(WraithTheme::DefaultControlBorder);
    // Create the pen
    Gdiplus::Pen borderPen(&borderBrush, 1.0f);
    // Create the text brush
    Gdiplus::SolidBrush textBrush(WraithTheme::DefaultForeground);
    // Create the text layout rect
    Gdiplus::RectF textLayRect(0.0f, 0.0f, (float)clientRect.Width, (float)clientRect.Height);
    // The text size
    Gdiplus::RectF textSize;

    // Render the background gradient
    graphics.FillRectangle(&backBrush, clientRect);
    // Render the border
    graphics.DrawRectangle(&borderPen, 0, 0, clientRect.Width - 1, clientRect.Height - 1);

    // Render the title
    {
        // Create the font
        Gdiplus::Font textFont(L"Arial", 9.0f, Gdiplus::FontStyle::FontStyleBold);

        // Render the title
        graphics.DrawString(L"Greyhound - Export complete", 27, &textFont, Gdiplus::PointF(9.0f, 9.0f), &textBrush);
    }

    // Render the message
    {
        // Create the font
        Gdiplus::Font textFont(L"Microsoft Sans Serif", 8.25f);
        // Create the text format
        Gdiplus::StringFormat textFormat;
        // Setup format
        textFormat.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
        // Create the layout position (With enough padding at the bottom for the click to open)
        Gdiplus::RectF textMargins(93.0f, 37.0f, (WindowRect.Width() - 93.0f) - 15.0f, (WindowRect.Height() - 37.0f) - 46.0f);

        // Render the message
        graphics.DrawString(DisplayMessage, DisplayMessage.GetLength(), &textFont, textMargins, &textFormat, &textBrush);
    }

    // Render the click to open
    {
        // Create the font
        Gdiplus::Font textFont(L"Microsoft Sans Serif", 8.25f);

        // Render the title
        graphics.DrawString(L"Click to open export folder", 27, &textFont, Gdiplus::PointF(93.0f, 110.0f), &textBrush);
    }

    // Render the icon
    {
        // Check if loaded
        if (WraithTheme::ApplicationIconLarge != NULL)
        {
            // Load the application icon
            Gdiplus::Bitmap IconResource(WraithTheme::ApplicationIconLarge);
            // Smoothing
            graphics.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeHighQuality);

            // Render the icon
            graphics.DrawImage(&IconResource, Gdiplus::Rect(16, 40, 62, 62));
        }
    }

    // -- End control render, output result

    // Render out the final buffer
    dbGraphic.DrawImage(&dbBuffer, clientRect);
}

BOOL WraithPopup::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    // Set it to the hand cursor
    SetCursor(HandCursor);
    // We're done
    return TRUE;
}

void WraithPopup::OnMouseLeave()
{
    // Reset the tracker
    MouseTrackingMode = false;
    // Resetup timer
    if (CloseTimer != NULL)
    {
        // Kill it first
        KillTimer(CloseTimer);
    }
    // Setup a new one with 2s to close
    CloseTimer = SetTimer(1, 2000, NULL);
}

void WraithPopup::OnMouseMove(UINT nFlags, CPoint point)
{
    // Check
    if (!MouseTrackingMode)
    {
        // Set it
        MouseTrackingMode = true;
        // Stop the timer
        if (CloseTimer != NULL)
        {
            // Kill it first
            KillTimer(CloseTimer);
            // Reset
            CloseTimer = NULL;
        }
        // Begin mouse tracking
        TRACKMOUSEEVENT Tracker;
        Tracker.cbSize = sizeof(Tracker);
        Tracker.hwndTrack = this->GetSafeHwnd();
        Tracker.dwFlags = TME_LEAVE;
        Tracker.dwHoverTime = HOVER_DEFAULT;
        // Track it
        TrackMouseEvent(&Tracker);
    }
}

void WraithPopup::OnLButtonDown(UINT nFlags, CPoint point)
{
    // Handle base
    WraithWindow::OnLButtonDown(nFlags, point);

    // Set that the button was down
    WasLeftButtonDown = true;
}

void WraithPopup::OnLButtonUp(UINT nFlags, CPoint point)
{
    // Handle base
    WraithWindow::OnLButtonUp(nFlags, point);

    // Check
    if (WasLeftButtonDown)
    {
        // We had a click event, handle it
        this->OnWindowClick();
    }

    // Set that the button was up
    WasLeftButtonDown = false;
}

void WraithPopup::OnWindowClick()
{
    // Stop the timer
    if (CloseTimer != NULL)
    {
        // Kill it first
        KillTimer(CloseTimer);
        // Reset
        CloseTimer = NULL;
    }
    // Prepare to close the dialog
    this->ShowWindow(SW_HIDE);

    // Get the path
    auto PathOpen = Strings::ToNormalString(std::wstring(DirectoryOpen));

    // Now verify directory information and launch if possible
    if (DirectoryOpen != "" && FileSystems::DirectoryExists(PathOpen))
    {
        // Launch the path with explorer
        ShellExecuteA(NULL, "open", PathOpen.c_str(), NULL, NULL, SW_SHOWDEFAULT);
    }
}