#pragma once
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QWindow>

class COpenGLWidget : public QWidget 
{
public:

	COpenGLWidget(QWidget* _pParent = Q_NULLPTR);
	~COpenGLWidget();

public:

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:

    HWND  m_NativeWindowHandle;
    HDC   m_NativeDeviceContextHandle;
    HGLRC m_NativeCreateContectHandle;

    QWindow*        m_pWindow;
    QOpenGLContext* m_pContext;

};
