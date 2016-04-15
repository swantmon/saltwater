
#include "editor_gui/edit_mainwindow.h"

namespace Edit
{
    CMainWindow::CMainWindow(QWidget* _pParent) 
        : QMainWindow(_pParent)
    {
        m_UserInterface.setupUi(this);
    }

    // -----------------------------------------------------------------------------

    CMainWindow::~CMainWindow() 
    {

    }

    // -----------------------------------------------------------------------------

    void CMainWindow::CreateContext()
    {
        m_UserInterface.m_pRenderContext->initializeGL();
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::SwapWindow()
    {
        m_UserInterface.m_pRenderContext->paintGL();
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::slot1()
    {
        int a = 4;
    }
} // namespace Edit


