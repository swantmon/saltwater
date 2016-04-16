
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

        MessageManager::SendMessage(SGUIMessageType::Undefined, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::closeEvent(QCloseEvent* _pEvent)
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Exit, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::keyPressEvent(QKeyEvent* _pKeyEvent)
    {
        CMessage NewMessage;

        NewMessage.PutInt(_pKeyEvent->key());
        NewMessage.PutInt(_pKeyEvent->modifiers());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::KeyPressed, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::keyReleaseEvent(QKeyEvent* _pKeyEvent)
    {
        CMessage NewMessage;

        NewMessage.PutInt(_pKeyEvent->key());
        NewMessage.PutInt(_pKeyEvent->modifiers());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::KeyReleased, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::mouseMoveEvent(QMouseEvent* _pMouseEvent)
    {
        CMessage NewMessage;

        NewMessage.PutInt(_pMouseEvent->globalX());
        NewMessage.PutInt(_pMouseEvent->globalY());
        NewMessage.PutInt(_pMouseEvent->x());
        NewMessage.PutInt(_pMouseEvent->y());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::MouseMove, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::mousePressEvent(QMouseEvent* _pMouseEvent)
    {
        CMessage NewMessage;

        NewMessage.PutInt(_pMouseEvent->globalX());
        NewMessage.PutInt(_pMouseEvent->globalY());
        NewMessage.PutInt(_pMouseEvent->x());
        NewMessage.PutInt(_pMouseEvent->y());

        NewMessage.Reset();

        switch (_pMouseEvent->button())
        {
        case Qt::LeftButton:
            MessageManager::SendMessage(SGUIMessageType::MouseLeftPressed, NewMessage);
            break;
        case Qt::MiddleButton:
            MessageManager::SendMessage(SGUIMessageType::MouseMiddlePressed, NewMessage);
            break;
        case Qt::RightButton:
            MessageManager::SendMessage(SGUIMessageType::MouseRightPressed, NewMessage);
            break;
        }
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::mouseReleaseEvent(QMouseEvent* _pMouseEvent)
    {
        CMessage NewMessage;

        NewMessage.PutInt(_pMouseEvent->globalX());
        NewMessage.PutInt(_pMouseEvent->globalY());
        NewMessage.PutInt(_pMouseEvent->x());
        NewMessage.PutInt(_pMouseEvent->y());

        NewMessage.Reset();

        switch (_pMouseEvent->button())
        {
        case Qt::LeftButton:
            MessageManager::SendMessage(SGUIMessageType::MouseLeftReleased, NewMessage);
            break;
        case Qt::MiddleButton:
            MessageManager::SendMessage(SGUIMessageType::MouseMiddleReleased, NewMessage);
            break;
        case Qt::RightButton:
            MessageManager::SendMessage(SGUIMessageType::MouseRightReleased, NewMessage);
            break;
        }
    }
} // namespace Edit


