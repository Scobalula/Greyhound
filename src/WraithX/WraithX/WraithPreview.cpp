#include "stdafx.h"

// The class we are implementing
#include "WraithPreview.h"

// Wraith classes
#include "WraithTheme.h"

// Our custom message map for WraithWindow
BEGIN_MESSAGE_MAP(WraithPreview, WraithWindow)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
END_MESSAGE_MAP()

WraithPreview::WraithPreview(UINT nIDTemplate) : WraithWindow(nIDTemplate)
{
    // Defaults
    m_pDC = NULL;
    m_iTexture = NULL;
    m_Created = FALSE;
    m_TemplateID = nIDTemplate;
}

BOOL WraithPreview::PreCreateWindow(CREATESTRUCT& cs)
{
    // Set the proper styles
    cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    // Let base handle it
    return WraithWindow::PreCreateWindow(cs);
}

int WraithPreview::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    // Initialize the base class first
    if (WraithWindow::OnCreate(lpCreateStruct) == -1)
    {
        // Failed
        return -1;
    }

    // Initialize OpenGL
    InitializeOpenGL();

    // Success
    return 0;
}

WraithPreview::~WraithPreview()
{
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
}

BOOL WraithPreview::OnEraseBkgnd(CDC* pDC)
{
    // Completely ignore this event
    return TRUE;
}

void WraithPreview::RenderScene()
{
    // Render scene if loaded
    if (this->PreviewScene != nullptr)
        this->PreviewScene->RenderScene();
}

void WraithPreview::OnPaint()
{
    // Validate the region
    WraithWindow::OnPaint();
    ::wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);
    // Clear the buffer
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ::glPushMatrix();

    // Setup camera
    glTranslatef(0, 0, -10.2f);
    glRotatef(45, 1, 0, 0);   // Pitch
    glRotatef(45, 0, 1, 0);   // Heading

    // Render scene
    RenderScene();

    ::glPopMatrix();

    // Finish scene
    ::glFinish();

    // Swap out buffer
    ::SwapBuffers(m_pDC->GetSafeHdc());
}

void WraithPreview::OnSize(UINT nType, int cx, int cy)
{
    ::wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);
    // Handle base sizing event first
    WraithWindow::OnSize(nType, cx, cy);

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

    // Compute the aspect ratio
    GLdouble aspect_ratio = (GLdouble)cx / (GLdouble)cy;

    // Select the viewing volumn
    ::gluPerspective(60.0f, aspect_ratio, 1.0f, 1000.0f);

    // Switch back to the modelview matrix
    ::glMatrixMode(GL_MODELVIEW);
}

void WraithPreview::InitializeOpenGL()
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

    GLfloat light_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };  /* Red diffuse light. */
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };  /* Infinite light location. */
    ::glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    ::glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    ::glEnable(GL_LIGHT0);
    ::glEnable(GL_LIGHTING);
    ::glEnable(GL_DEPTH_TEST);

    // Clean up initial OpenGl resource allocation
    EmptyWorkingSet(GetCurrentProcess());
}

void WraithPreview::LoadWraithModel(const std::unique_ptr<WraithModel>& Model)
{
    // If the current scene is loaded, reset
    this->PreviewScene = std::make_unique<WraithPreviewScene>();
    ::wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);
    // Prepare to load scene data
    for (auto& Submesh : Model->Submeshes)
    {
        GLuint SubmeshList = ::glGenLists(1);

        // Start the list
        ::glNewList(SubmeshList, GL_COMPILE);
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
        ::glEndList();

        // Add it
        this->PreviewScene->AddSubmesh(SubmeshList);
    }
}

void WraithPreview::ShowPreview()
{
    // Show the dialog
    if (!m_Created)
    {
        // Create
        this->Create(m_TemplateID, GetDesktopWindow());
        // Set
        m_Created = TRUE;
    }

    // Show us
    this->ShowWindow(SW_SHOW);
}

WraithPreviewScene::WraithPreviewScene()
{
    // Setup data
}

WraithPreviewScene::~WraithPreviewScene()
{
    // Cleanup
    for (auto& Submesh : this->SubmeshDrawCalls)
        ::glDeleteLists(Submesh, 1);
}

void WraithPreviewScene::AddSubmesh(GLuint SubmeshList)
{
    this->SubmeshDrawCalls.emplace_back(SubmeshList);
}

void WraithPreviewScene::RenderScene()
{
    /*for (auto& SubmeshDraw : this->SubmeshDrawCalls)
        ::glCallList(SubmeshDraw);*/
    ::glCallList(this->SubmeshDrawCalls[0]);

    ///*::glBegin(GL_POLYGON);
    //::glNormal3d(1.0, 0.0, 0.0);

    //::glTexCoord2d(0, 0); ::glVertex3d(1.0, 1.0, 1.0);
    //::glTexCoord2d(0, 1); ::glVertex3d(1.0, -1.0, 1.0);
    //::glTexCoord2d(1, 1); ::glVertex3d(1.0, -1.0, -1.0);
    //::glTexCoord2d(1, 0); ::glVertex3d(1.0, 1.0, -1.0);

    //::glEnd();

    //::glBegin(GL_POLYGON);
    //::glNormal3d(-1.0, 0.0, 0.0);

    //::glTexCoord2d(1, 1); ::glVertex3d(-1.0, -1.0, 1.0);
    //::glTexCoord2d(1, 0); ::glVertex3d(-1.0, 1.0, 1.0);
    //::glTexCoord2d(0, 0); ::glVertex3d(-1.0, 1.0, -1.0);
    //::glTexCoord2d(0, 1); ::glVertex3d(-1.0, -1.0, -1.0);

    //::glEnd();

    //::glBegin(GL_POLYGON);
    //::glNormal3d(0.0, 1.0, 0.0);

    //::glTexCoord2d(1, 1); ::glVertex3d(1.0, 1.0, 1.0);
    //::glTexCoord2d(1, 0); ::glVertex3d(-1.0, 1.0, 1.0);
    //::glTexCoord2d(0, 0); ::glVertex3d(-1.0, 1.0, -1.0);
    //::glTexCoord2d(0, 1); ::glVertex3d(1.0, 1.0, -1.0);

    //::glEnd();

    //::glBegin(GL_POLYGON);
    //::glNormal3d(0.0, -1.0, 0.0);

    //::glTexCoord2d(0, 0); ::glVertex3d(-1.0, -1.0, 1.0);
    //::glTexCoord2d(1, 0); ::glVertex3d(1.0, -1.0, 1.0);
    //::glTexCoord2d(1, 1); ::glVertex3d(1.0, -1.0, -1.0);
    //::glTexCoord2d(0, 1); ::glVertex3d(-1.0, -1.0, -1.0);

    //::glEnd();

    //::glBegin(GL_POLYGON);
    //::glNormal3d(0.0, 0.0, 1.0);

    //::glTexCoord2d(1, 0); ::glVertex3d(1.0, 1.0, 1.0);
    //::glTexCoord2d(0, 0); ::glVertex3d(-1.0, 1.0, 1.0);
    //::glTexCoord2d(0, 1); ::glVertex3d(-1.0, -1.0, 1.0);
    //::glTexCoord2d(1, 1); ::glVertex3d(1.0, -1.0, 1.0);

    //::glEnd();

    //::glBegin(GL_POLYGON);
    //::glNormal3d(0.0, 0.0, -1.0);

    //::glTexCoord2d(1, 0); ::glVertex3d(-1.0, 1.0, -1.0);
    //::glTexCoord2d(0, 0); ::glVertex3d(1.0, 1.0, -1.0);
    //::glTexCoord2d(0, 1); ::glVertex3d(1.0, -1.0, -1.0);
    //::glTexCoord2d(1, 1); ::glVertex3d(-1.0, -1.0, -1.0);

    //::glEnd();*/
}