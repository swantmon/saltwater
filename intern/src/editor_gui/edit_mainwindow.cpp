
#include "editor_gui/edit_mainwindow.h"
#include "editor_gui/edit_newscenedialog.h"

#include <QString>

namespace Edit
{
    CMainWindow::CMainWindow(QWidget* _pParent) 
        : QMainWindow           (_pParent)
        , m_pStatusLabel        (nullptr)
        , m_pNewActorModelDialog(nullptr)
        , m_pNewSceneDialog     (nullptr)
        , m_IsPlaying           (false)
        , m_IsSceneLoaded       (false)
    {
        // -----------------------------------------------------------------------------
        // Dialogs
        // -----------------------------------------------------------------------------
        m_pNewActorModelDialog = new CNewActorModelDialog();
        m_pNewSceneDialog      = new CNewSceneDialog();

        // -----------------------------------------------------------------------------
        // Form setup
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // User specific setup
        // -----------------------------------------------------------------------------
        m_pStatusLabel = new QLabel();
        m_pStatusLabel->setText("FPS:");

        m_pStatusBar->addPermanentWidget(m_pStatusLabel, 1);

        // -----------------------------------------------------------------------------
        // Dock widgets setup
        // -----------------------------------------------------------------------------
        m_pHistogramDockWidget->setVisible(false);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::FramesPerSecond, EDIT_RECEIVE_MESSAGE(&CMainWindow::OnFramesPerSecond));
        Edit::MessageManager::Register(Edit::SApplicationMessageType::HistogramInfo, EDIT_RECEIVE_MESSAGE(&CMainWindow::OnHistogramInfo));
    }

    // -----------------------------------------------------------------------------

    CMainWindow::~CMainWindow() 
    {
        // -----------------------------------------------------------------------------
        // Delete dialogs
        // -----------------------------------------------------------------------------
        delete m_pNewActorModelDialog;
        delete m_pNewSceneDialog;

        // -----------------------------------------------------------------------------
        // Remove user specific UI
        // -----------------------------------------------------------------------------
        delete m_pStatusLabel;
    }

    // -----------------------------------------------------------------------------

    void* CMainWindow::GetEditorWindowHandle()
    {
        return (HWND)m_pEditorRenderContext->winId();
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Check startup behavior
        // -----------------------------------------------------------------------------
        if (m_IsSceneLoaded == false)
        {
            m_pNewSceneDialog->show();

            m_IsSceneLoaded = true;
        }
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::OnExit()
    {

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

        // -----------------------------------------------------------------------------
        // Prepare image path
        // -----------------------------------------------------------------------------
        QString    PathToImage       = m_pMailAdressEdit->text() + ".png";
        QByteArray PathToImageBinary = PathToImage.toLatin1();

        // -----------------------------------------------------------------------------
        // Compare message
        // -----------------------------------------------------------------------------
        NewMessage.PutString(PathToImageBinary.data());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::TakeScreenshot, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::openNewSceneDialog()
    {
        m_pNewSceneDialog->show();
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::openNewActorModelDialog()
    {
        m_pNewActorModelDialog->show();
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewLightDirectional()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::NewLightDirectional, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewLightPoint()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::NewLightPoint, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewLightEnvironment()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::NewLightEnvironment, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewLightGlobalProbe()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::NewLightGlobalProbe, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::changeHistogramSettings()
    {
        float LowerBound = m_pHistogramLowerBoundEdit->text().toFloat();
        float UpperBound = m_pHistogramUpperBoundEdit->text().toFloat();

        float LogMin = m_pHistogramLogMinEdit->text().toFloat();
        float LogMax = m_pHistogramLogMaxEdit->text().toFloat();

        float EyeUp   = m_pHistogramEyeUpEdit->text().toFloat();
        float EyeDown = m_pHistogramEyeDownEdit->text().toFloat();

        Edit::CMessage NewMessage;

        NewMessage.PutFloat(LowerBound);
        NewMessage.PutFloat(UpperBound);
        NewMessage.PutFloat(LogMin);
        NewMessage.PutFloat(LogMax);
        NewMessage.PutFloat(EyeUp);
        NewMessage.PutFloat(EyeDown);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::HistogramInfo, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::toggleHistogramDock()
    {
        m_pHistogramDockWidget->setVisible(!m_pHistogramDockWidget->isVisible());

        if (m_pHistogramDockWidget->isVisible())
        {
            CMessage NewMessage(true);

            NewMessage.Reset();

            MessageManager::SendMessage(SGUIMessageType::RequestHistogramInfo, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::closeEvent(QCloseEvent* _pEvent)
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Exit, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::mousePressEvent(QMouseEvent* _pMouseEvent)
    {
        setFocus();
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::OnFramesPerSecond(Edit::CMessage& _rMessage)
    {
        float FPS = _rMessage.GetDouble();

        m_pStatusLabel->setText("FPS: " + QString::number(1.0f / FPS));
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::OnHistogramInfo(Edit::CMessage& _rMessage)
    {
        float LowerBound = _rMessage.GetFloat();
        float UpperBound = _rMessage.GetFloat();

        float LogMin = _rMessage.GetFloat();
        float LogMax = _rMessage.GetFloat();

        float EyeUp   = _rMessage.GetFloat();
        float EyeDown = _rMessage.GetFloat();

        m_pHistogramLowerBoundEdit->setText(QString::number(LowerBound));
        m_pHistogramUpperBoundEdit->setText(QString::number(UpperBound));

        m_pHistogramLogMinEdit->setText(QString::number(LogMin));
        m_pHistogramLogMaxEdit->setText(QString::number(LogMax));

        m_pHistogramEyeUpEdit->setText(QString::number(EyeUp));
        m_pHistogramEyeDownEdit->setText(QString::number(EyeDown));
    }
} // namespace Edit


