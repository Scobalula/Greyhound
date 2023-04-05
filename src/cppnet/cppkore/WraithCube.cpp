#include "stdafx.h"

// The class we are implementing
#include "WraithCube.h"

// We need the theme and strings
#include "Mangler.h"
#include "Matrix.h"
#include "File.h"
#include "Environment.h"

namespace Forms
{
    WraithCube::WraithCube()
        : Control(), _DCHandle(nullptr), _GLHandle(nullptr)
    {
        m_rTimer = 0;
        m_iTexture = 0;
    }

    WraithCube::~WraithCube()
    {
        glDeleteTextures(1, &m_iTexture);
        if (_DCHandle)
            ReleaseDC(this->_Handle, this->_DCHandle);
        if (_GLHandle)
            wglDeleteContext(this->_GLHandle);

        // Kill the timer
        if (m_rTimer != NULL)
        {
            // Kill
            KillTimer(this->GetHandle(), m_rTimer);
        }
    }

    void WraithCube::OnTimer(Message& Msg)
    {
        // If event is 1 change rotation
        if (Msg.WParam == 1)
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
    }

    void WraithCube::Redraw()
    {
        this->OnRender();
    }

    void WraithCube::RenderCube()
    {
        // Clear the buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup the matrix
        glPushMatrix();
        // Sets the cube to normal view distance
        glTranslated(0.0, 0.0, Distance);
        // This rotates the cube
        glRotated(RotateX, 1, 0, 0);
        glRotated(RotateZ, 0, 0, 1);

        // Bind the currently allocated texture...
        glBindTexture(GL_TEXTURE_2D, m_iTexture);

        // -- Begin all 6 faces of the cube

        //switch to Modern OpenGL VBO? etc

        glBegin(GL_POLYGON);
        glNormal3d(1.0, 0.0, 0.0);

        glTexCoord2d(0, 0); glVertex3d(1.0, 1.0, 1.0);
        glTexCoord2d(0, 1); glVertex3d(1.0, -1.0, 1.0);
        glTexCoord2d(1, 1); glVertex3d(1.0, -1.0, -1.0);
        glTexCoord2d(1, 0); glVertex3d(1.0, 1.0, -1.0);

        glEnd();

        glBegin(GL_POLYGON);
        glNormal3d(-1.0, 0.0, 0.0);

        glTexCoord2d(1, 1); glVertex3d(-1.0, -1.0, 1.0);
        glTexCoord2d(1, 0); glVertex3d(-1.0, 1.0, 1.0);
        glTexCoord2d(0, 0); glVertex3d(-1.0, 1.0, -1.0);
        glTexCoord2d(0, 1); glVertex3d(-1.0, -1.0, -1.0);

        glEnd();

        glBegin(GL_POLYGON);
        glNormal3d(0.0, 1.0, 0.0);

        glTexCoord2d(1, 1); glVertex3d(1.0, 1.0, 1.0);
        glTexCoord2d(1, 0); glVertex3d(-1.0, 1.0, 1.0);
        glTexCoord2d(0, 0); glVertex3d(-1.0, 1.0, -1.0);
        glTexCoord2d(0, 1); glVertex3d(1.0, 1.0, -1.0);

        glEnd();

        glBegin(GL_POLYGON);
        glNormal3d(0.0, -1.0, 0.0);

        glTexCoord2d(0, 0); glVertex3d(-1.0, -1.0, 1.0);
        glTexCoord2d(1, 0); glVertex3d(1.0, -1.0, 1.0);
        glTexCoord2d(1, 1); glVertex3d(1.0, -1.0, -1.0);
        glTexCoord2d(0, 1); glVertex3d(-1.0, -1.0, -1.0);

        glEnd();

        glBegin(GL_POLYGON);
        glNormal3d(0.0, 0.0, 1.0);

        glTexCoord2d(1, 0); glVertex3d(1.0, 1.0, 1.0);
        glTexCoord2d(0, 0); glVertex3d(-1.0, 1.0, 1.0);
        glTexCoord2d(0, 1); glVertex3d(-1.0, -1.0, 1.0);
        glTexCoord2d(1, 1); glVertex3d(1.0, -1.0, 1.0);

        glEnd();
        
        glBegin(GL_POLYGON);
        glNormal3d(0.0, 0.0, -1.0);

        glTexCoord2d(1, 0); glVertex3d(-1.0, 1.0, -1.0);
        glTexCoord2d(0, 0); glVertex3d(1.0, 1.0, -1.0);
        glTexCoord2d(0, 1); glVertex3d(1.0, -1.0, -1.0);
        glTexCoord2d(1, 1); glVertex3d(-1.0, -1.0, -1.0);

        glEnd();

        glPopMatrix();
    }

    void WraithCube::OnPaint(const std::unique_ptr<PaintEventArgs>& EventArgs)
    {
        // Proxy this off, they don't need paint event args...
        OnRender();
    }

    void WraithCube::WndProc(Message& Msg)
    {
        switch (Msg.Msg)
        {
        case WM_TIMER:
            OnTimer(Msg);
            break;
        default:
            Control::WndProc(Msg);
            break;
        }
    }

    void WraithCube::OnRender()
    {
        // Clear the buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Setup scene
        glDrawBuffer(GL_BACK);
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        // Render scene
        RenderCube();
        glPopMatrix();
        // Finish scene
        glFinish();
        // Swap out buffer
        SwapBuffers(this->_DCHandle);
    }

    void WraithCube::OnResize()
    {
        // Only continue when we have a size
        if (0 >= this->_ClientWidth || 0 >= this->_ClientHeight)
        {
            return;
        }
        // Setup the viewport
        glViewport(0, 0, this->_ClientWidth, this->_ClientHeight);
        // Select the projection matrix and clear it
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glScalef(1.0f, -1.0f, 1.0f);
        // Compute the aspect ratio
        GLdouble aspect_ratio = (GLdouble)this->_ClientWidth / (GLdouble)this->_ClientHeight;
        // Select the viewing volumn
        glLoadMatrixf(Math::Matrix::CreatePerspectiveFov(40.0f, aspect_ratio, .1f, 20.0f).GetMatrix());
        // Switch back to the modelview matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void WraithCube::OnHandleCreated()
    {
        // We initialize OpenGL here, this can only be setup once...
        this->_DCHandle = GetDC(this->_Handle);

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
        auto PixelFormatIndex = ChoosePixelFormat(this->_DCHandle, &PixelFormat);
        // Make sure it isn't 0
        if (PixelFormatIndex == 0) { return; }
        // Set the format
        auto Result = SetPixelFormat(this->_DCHandle, PixelFormatIndex, &PixelFormat);

        // Make sure we did it
        if (!Result) { return; }

        int gl_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
        };

        // Setup the context
        this->_GLHandle = wglCreateContext(this->_DCHandle);
        //this->_GLHandle = wglCreateContextAttribsARB(this->_DCHandle, 0, gl_attribs);
        wglMakeCurrent(this->_DCHandle, this->_GLHandle);

        // Make sure we got it
        if (this->_GLHandle == 0) { return; }

        wglMakeCurrent(this->_DCHandle, this->_GLHandle);

        if (!MGL::Mangler::Initalize())
            throw std::exception("Error initializing OpenGL!");

        // Set the background
        glClearColor(0.14509f, 0.14509f, 0.14509f, 1.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);

        // Setup the timer (50ms seems to not effect CPU usage)
        m_rTimer = SetTimer(this->GetHandle(), 1, 50, NULL);

        auto GameIcon = IO::File::ExtractFileIcon(std::string(System::Environment::GetApplication()));
        if (GameIcon != NULL)
        {
            // Load into cube, then clean up
            LoadCubeIcon(GameIcon);
            DestroyIcon(GameIcon);
        }

        // We must call base event last
        Control::OnHandleCreated();
    }

    void WraithCube::OnBackColorChanged()
    {
        auto Background = this->BackColor();
        glClearColor(Background.GetR() / 255.f, Background.GetG() / 255.f, Background.GetB() / 255.f, Background.GetA() / 255.f);
        // We must call base event last
        Control::OnBackColorChanged();
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
        auto dcCTX = GetDC(this->_Handle);

        // Get the data
        auto DataBuffer = std::make_unique<uint8_t[]>(4 * bmp.bmWidth * bmp.bmHeight);

        // Get it
        GetDIBits(dcCTX, iconInfo.hbmColor, 0, bmp.bmHeight, DataBuffer.get(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

        // Clean up
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        ReleaseDC(this->_Handle,dcCTX);

        // Clear and recreate it
        glDeleteTextures(1, &m_iTexture);
        glGenTextures(1, &m_iTexture);
        auto t = glGetError();

        // Bind it
        glBindTexture(GL_TEXTURE_2D, m_iTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load the image data from the buffer
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmp.bmWidth, bmp.bmHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, DataBuffer.get());
    }

    CreateParams WraithCube::GetCreateParams()
    {
        auto Cp = Control::GetCreateParams();
        Cp.ClassName = "WraithCube";

        Cp.Style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        Cp.ClassStyle |= CS_VREDRAW | CS_HREDRAW | CS_OWNDC;

        return Cp;
    }
}