#include "stdafx.h"

// The class we are implementing
#include "WraithCube.h"

// We need the theme and strings
#include "WraithTheme.h"
#include "BinaryWriter.h"
#include "Strings.h"

// Our custom message map for WraithWindow
BEGIN_MESSAGE_MAP(WraithCube, CWnd)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_PAINT()
    ON_WM_TIMER()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
END_MESSAGE_MAP()

WraithCube::WraithCube()
{
    // Defaults
    m_pDC = NULL;
    m_rTimer = NULL;
    m_iTexture = NULL;
}

BOOL WraithCube::PreCreateWindow(CREATESTRUCT& cs)
{
    // Set the proper styles
    cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    // Let base handle it
    return CWnd::PreCreateWindow(cs);
}

int WraithCube::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    // Initialize the base class first
    if (CWnd::OnCreate(lpCreateStruct) == -1)
    {
        // Failed
        return -1;
    }

    // Initialize OpenGL
    InitializeOpenGL();

    // Setup the timer (50ms seems to not effect CPU usage)
    m_rTimer = SetTimer(1, 50, NULL);
    
    // Success
    return 0;
}

void WraithCube::OnTimer(UINT_PTR nIDEvent)
{
    // If event is 1 change rotation
    if (nIDEvent == 1)
    {
        // Continue the rotation
        RotateX += 3.f;
        RotateZ += 3.f;

        // Redraw
        this->Invalidate();
    }

    if (RotateX >= 360.f)
    {
        // Reset
        RotateX = 0.f;
        RotateZ = 0.f;
    }

    // Handle base
    CWnd::OnTimer(nIDEvent);
}

void WraithCube::OnDestroy()
{
    // Close base first
    CWnd::OnDestroy();

    // Clean up icon
    ::glDeleteTextures(1, &m_iTexture);

    // Clean up context
    ::wglDeleteContext(m_hRC);

    // Clean up DC
    if (m_pDC)
    {
        // Delete it
        delete m_pDC;
    }

    // Kill the timer
    if (m_rTimer != NULL)
    {
        // Kill
        KillTimer(m_rTimer);
    }
}

BOOL WraithCube::OnEraseBkgnd(CDC* pDC)
{
    // Completely ignore this event
    return TRUE;
}

void WraithCube::RenderCube()
{
    // Clear the buffer
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup the matrix
    ::glPushMatrix();
    // Sets the cube to normal view distance
    ::glTranslated(0.0, 0.0, Distance);
    // This rotates the cube
    ::glRotated(RotateX, 1, 0, 0);
    ::glRotated(RotateZ, 0, 0, 1);

    // Bind the currently allocated texture...
    ::glBindTexture(GL_TEXTURE_2D, m_iTexture);

    // -- Begin all 6 faces of the cube

    ::glBegin(GL_POLYGON);
    ::glNormal3d(1.0, 0.0, 0.0);

    ::glTexCoord2d(0, 0); ::glVertex3d(1.0, 1.0, 1.0);
    ::glTexCoord2d(0, 1); ::glVertex3d(1.0, -1.0, 1.0);
    ::glTexCoord2d(1, 1); ::glVertex3d(1.0, -1.0, -1.0);
    ::glTexCoord2d(1, 0); ::glVertex3d(1.0, 1.0, -1.0);

    ::glEnd();

    ::glBegin(GL_POLYGON);
    ::glNormal3d(-1.0, 0.0, 0.0);

    ::glTexCoord2d(1, 1); ::glVertex3d(-1.0, -1.0, 1.0);
    ::glTexCoord2d(1, 0); ::glVertex3d(-1.0, 1.0, 1.0);
    ::glTexCoord2d(0, 0); ::glVertex3d(-1.0, 1.0, -1.0);
    ::glTexCoord2d(0, 1); ::glVertex3d(-1.0, -1.0, -1.0);

    ::glEnd();

    ::glBegin(GL_POLYGON);
    ::glNormal3d(0.0, 1.0, 0.0);

    ::glTexCoord2d(1, 1); ::glVertex3d(1.0, 1.0, 1.0);
    ::glTexCoord2d(1, 0); ::glVertex3d(-1.0, 1.0, 1.0);
    ::glTexCoord2d(0, 0); ::glVertex3d(-1.0, 1.0, -1.0);
    ::glTexCoord2d(0, 1); ::glVertex3d(1.0, 1.0, -1.0);

    ::glEnd();

    ::glBegin(GL_POLYGON);
    ::glNormal3d(0.0, -1.0, 0.0);

    ::glTexCoord2d(0, 0); ::glVertex3d(-1.0, -1.0, 1.0);
    ::glTexCoord2d(1, 0); ::glVertex3d(1.0, -1.0, 1.0);
    ::glTexCoord2d(1, 1); ::glVertex3d(1.0, -1.0, -1.0);
    ::glTexCoord2d(0, 1); ::glVertex3d(-1.0, -1.0, -1.0);

    ::glEnd();

    ::glBegin(GL_POLYGON);
    ::glNormal3d(0.0, 0.0, 1.0);

    ::glTexCoord2d(1, 0); ::glVertex3d(1.0, 1.0, 1.0);
    ::glTexCoord2d(0, 0); ::glVertex3d(-1.0, 1.0, 1.0);
    ::glTexCoord2d(0, 1); ::glVertex3d(-1.0, -1.0, 1.0);
    ::glTexCoord2d(1, 1); ::glVertex3d(1.0, -1.0, 1.0);

    ::glEnd();

    ::glBegin(GL_POLYGON);
    ::glNormal3d(0.0, 0.0, -1.0);

    ::glTexCoord2d(1, 0); ::glVertex3d(-1.0, 1.0, -1.0);
    ::glTexCoord2d(0, 0); ::glVertex3d(1.0, 1.0, -1.0);
    ::glTexCoord2d(0, 1); ::glVertex3d(1.0, -1.0, -1.0);
    ::glTexCoord2d(1, 1); ::glVertex3d(-1.0, -1.0, -1.0);

    ::glEnd();

    ::glPopMatrix();
}

void WraithCube::OnPaint()
{
    // Validate the region
    CWnd::OnPaint();

    // Clear the buffer
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup scene
    ::glDrawBuffer(GL_BACK);
    ::glEnable(GL_TEXTURE_2D);

    
    ::glPushMatrix();

    // Render scene
    RenderCube();

    ::glPopMatrix();

    // Finish scene
    ::glFinish();

    // Swap out buffer
    ::SwapBuffers(m_pDC->GetSafeHdc());
}

void WraithCube::OnSize(UINT nType, int cx, int cy)
{
    // Handle base sizing event first
    CWnd::OnSize(nType, cx, cy);

    // Only continue when we have a size
    if (0 >= cx || 0 >= cy)
    {
        return;
    }

    // Setup the viewport
    ::glViewport(0, 0, cx, cy);

    // Select the projection matrix and clear it
    ::glMatrixMode(GL_PROJECTION);
    ::glLoadIdentity();
    ::glScalef(1.0f, -1.0f, 1.0f);

    // Compute the aspect ratio
    GLdouble aspect_ratio = (GLdouble)cx / (GLdouble)cy;

    // Select the viewing volumn
    ::gluPerspective(40.0f, aspect_ratio, .1f, 20.0f);

    // Switch back to the modelview matrix
    ::glMatrixMode(GL_MODELVIEW);
    ::glLoadIdentity();
}

void WraithCube::InitializeOpenGL()
{
    // Setup the context
    m_pDC = new CClientDC(this);

    // Check if we worked
    if (m_pDC == NULL)
    {
        // Failed
        return;
    }

    // Setup the pixel format
    PIXELFORMATDESCRIPTOR PixelFormat = 
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        24,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    // Choose the format
    auto PixelResult = ::ChoosePixelFormat(m_pDC->GetSafeHdc(), &PixelFormat);

    // Make sure it isn't 0
    if (PixelResult == 0) { return; }

    // Set the format
    auto Result = ::SetPixelFormat(m_pDC->GetSafeHdc(), PixelResult, &PixelFormat);

    // Make sure we did it
    if (!Result){ return; }

    // Setup the context
    m_hRC = ::wglCreateContext(m_pDC->GetSafeHdc());

    // Make sure we got it
    if (m_hRC == 0) { return; }

    // Set the context as active
    ::wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

    // Set the background
    ::glClearColor(0.14509f, 0.14509f, 0.14509f, 1.0f);
    ::glClearDepth(1.0f);
    ::glEnable(GL_DEPTH_TEST);

    // Clean up initial OpenGl resource allocation
    EmptyWorkingSet(GetCurrentProcess());
}

void WraithCube::LoadCubeIcon(HICON IconHandle)
{
    // We need to prepare the icon
    ICONINFO iconInfo;
    GetIconInfo(IconHandle, &iconInfo);
    
    // Get info
    BITMAP bmp;
    GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp);

    // Header for getting the bits
    BITMAPINFOHEADER bmi = { 0 };
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = bmp.bmBitsPixel;
    bmi.biWidth = bmp.bmWidth;
    bmi.biHeight = bmp.bmHeight;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;

    // Get the DC context
    auto dcCTX = GetDC();

    // Get the data
    auto DataBuffer = std::make_unique<uint8_t[]>(4 * bmp.bmWidth * bmp.bmHeight);

    // Get it
    GetDIBits(dcCTX->GetSafeHdc(), iconInfo.hbmColor, 0, bmp.bmHeight, DataBuffer.get(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

    // Clean up
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    ReleaseDC(dcCTX);

    // Clear and recreate it
    ::glDeleteTextures(1, &m_iTexture);
    ::glGenTextures(1, &m_iTexture);
    
    // Bind it
    ::glBindTexture(GL_TEXTURE_2D, m_iTexture);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load the image data from the buffer
    ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmp.bmWidth, bmp.bmHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, DataBuffer.get());
}