#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <string>

// We need the OpenGL classes
#include <gl\GL.h>
#include <gl\GLU.h>

// A class that handles a WraithCube
class WraithCube : public CWnd
{
public:
    // -- WraithCube functions
    WraithCube();

    // Loads a new icon from an HICON resource handle
    void LoadCubeIcon(HICON IconHandle);

protected:

    // Handle proper control styles for OpenGL
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    // Handle drawing the control
    afx_msg void OnPaint();
    // Occures when the control is made
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    // Occures when the control is closing
    afx_msg void OnDestroy();
    // Handle painting the background
    afx_msg BOOL OnEraseBkgnd(CDC* cDC);
    // Handle resizing the buffer
    afx_msg void OnSize(UINT nType, int cx, int cy);
    // Handles rotating the cube
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    // Declare the message map instance
    DECLARE_MESSAGE_MAP()

private:

    // -- WraithCube helpers

    // Sets up OpenGL properly
    void InitializeOpenGL();
    // Renders the cube
    void RenderCube();
    

    // -- Variables

    HGLRC m_hRC;
    CDC* m_pDC;
    UINT_PTR m_rTimer;

    // -- Current texture
    GLuint m_iTexture;

    // -- Values
    GLdouble RotateX = 0.0f;
    GLdouble RotateZ = 0.0f;
    GLdouble Distance = -5.2f;
};