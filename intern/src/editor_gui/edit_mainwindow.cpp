
#include "editor_gui/edit_mainwindow.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

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

    void* CMainWindow::GetEditorWindowHandle()
    {
        return (HWND)m_UserInterface.m_pEditorRenderContext->winId();
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::slot1()
    {
        CMessage NewMessage;

        NewMessage.PutInt(1337);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::KeyPressed, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::closeEvent(QCloseEvent* _pEvent)
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Exit, NewMessage);
    }
} // namespace Edit


