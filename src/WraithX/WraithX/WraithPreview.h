#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <string>
#include <vector>
#include <thread>

// We need the OpenGL classes
#include <gl\GL.h>
#include <gl\GLU.h>

// We need the following Wraith classes
#include "WraithWindow.h"
#include "WraithModel.h"

// A class that represents a preview object
class WraithPreviewScene
{
public:
	// -- WraithPreviewScene
	WraithPreviewScene();
	~WraithPreviewScene();

	// Renders the preview scene
	void RenderScene();

	// Adds a submesh
	void AddSubmesh(GLuint SubmeshList);

private:

	// A list of opengl displays
	std::vector<GLuint> SubmeshDrawCalls;
};

// A class that handles previewing assets
class WraithPreview : public WraithWindow
{
public:
	// -- WraithPreview functions
	WraithPreview(UINT nIDTemplate);
	virtual ~WraithPreview();

	// Loads a WraithModel to preview
	void LoadWraithModel(const std::unique_ptr<WraithModel>& Model);

	// Shows the preview window
	void ShowPreview();

protected:

	// Handle proper control styles for OpenGL
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	// Handle drawing the control
	afx_msg void OnPaint();
	// Occures when the control is made
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	// Handle painting the background
	afx_msg BOOL OnEraseBkgnd(CDC* cDC);
	// Handle resizing the buffer
	afx_msg void OnSize(UINT nType, int cx, int cy);

	// Declare the message map instance
	DECLARE_MESSAGE_MAP()

private:

	// -- WraithPreview helpers

	// Sets up OpenGL properly
	void InitializeOpenGL();
	// Renders the scene
	void RenderScene();

	// The loaded scene object
	std::unique_ptr<WraithPreviewScene> PreviewScene;

	// -- Variables

	HGLRC m_hRC;
	CDC* m_pDC;
	BOOL m_Created;
	UINT m_TemplateID;

	// -- Current texture
	GLuint m_iTexture;
};