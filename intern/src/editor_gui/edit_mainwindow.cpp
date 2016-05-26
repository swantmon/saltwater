
#include "editor_gui/edit_mainwindow.h"

#include <QString>

namespace Edit
{
    CMainWindow::CMainWindow(QWidget* _pParent) 
        : QMainWindow   (_pParent)
        , m_pStatusLabel(nullptr)
        , m_IsPlaying   (false)
    {
        // -----------------------------------------------------------------------------
        // Form setup
        // -----------------------------------------------------------------------------
        m_UserInterface.setupUi(this);

        // -----------------------------------------------------------------------------
        // User specific setup
        // -----------------------------------------------------------------------------
        m_pStatusLabel = new QLabel();
        m_pStatusLabel->setText("FPS:");

        m_UserInterface.m_pStatusBar->addPermanentWidget(m_pStatusLabel, 1);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::FramesPerSecond, EDIT_RECEIVE_MESSAGE(&CMainWindow::OnFramesPerSecond));
    }

    // -----------------------------------------------------------------------------

    CMainWindow::~CMainWindow() 
    {
        // -----------------------------------------------------------------------------
        // Remove user specific UI
        // -----------------------------------------------------------------------------
        delete m_pStatusLabel;
    }

    // -----------------------------------------------------------------------------

    void* CMainWindow::GetEditorWindowHandle()
    {
        return (HWND)m_UserInterface.m_pEditorRenderContext->winId();
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::switchPlayingCurrentScene()
    {
        CMessage NewMessage;

        NewMessage.Reset();

        if (m_IsPlaying == false)
        {
            MessageManager::SendMessage(SGUIMessageType::Play, NewMessage);

            m_IsPlaying = true;
        }
        else
        {
            MessageManager::SendMessage(SGUIMessageType::Edit, NewMessage);
            
            m_IsPlaying = false;
        }
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::takeScreenshot()
    {
        CMessage NewMessage;

        NewMessage.PutString("save.png");

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::TakeScreenshot, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::closeEvent(QCloseEvent* _pEvent)
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Exit, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::OnFramesPerSecond(Edit::CMessage& _rMessage)
    {
        float FPS = _rMessage.GetDouble();

        m_pStatusLabel->setText("FPS: " + QString::number(1.0f / FPS));
    }
} // namespace Edit


