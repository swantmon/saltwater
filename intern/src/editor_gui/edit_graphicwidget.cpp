#include "edit_graphicwidget.h"

COpenGLWidget::COpenGLWidget(QWidget* _pParent) 
    : QWidget   (_pParent)
    , m_pContext(0)
{
    m_NativeWindowHandle = (HWND)this->winId();

    m_NativeDeviceContextHandle = ::GetDC(m_NativeWindowHandle);

    const PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
        32,                        //Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                        //Number of bits for the depthbuffer
        8,                        //Number of bits for the stencilbuffer
        0,                        //Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int iFormat = ChoosePixelFormat(m_NativeDeviceContextHandle, &pfd);

    SetPixelFormat(m_NativeDeviceContextHandle, iFormat, &pfd);

    m_NativeCreateContectHandle = ::wglCreateContext(m_NativeDeviceContextHandle);

    wglMakeCurrent(m_NativeDeviceContextHandle, m_NativeCreateContectHandle);
}

// -----------------------------------------------------------------------------

COpenGLWidget::~COpenGLWidget() 
{
    wglDeleteContext(m_NativeCreateContectHandle);
}

// -----------------------------------------------------------------------------

void COpenGLWidget::initializeGL()
{
    wglMakeCurrent(m_NativeDeviceContextHandle, m_NativeCreateContectHandle);
}

// -----------------------------------------------------------------------------

void COpenGLWidget::resizeGL(int w, int h)
{
}

// -----------------------------------------------------------------------------

void COpenGLWidget::paintGL()
{
    SwapBuffers(m_NativeDeviceContextHandle);
    wglMakeCurrent(NULL, NULL);
}