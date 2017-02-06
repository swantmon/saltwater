
#include "editor_gui/edit_mainwindow.h"
#include "editor_gui/edit_newscenedialog.h"

#include <QFileDialog>
#include <QString>

namespace Edit
{
    CMainWindow::CMainWindow(QWidget* _pParent) 
        : QMainWindow           (_pParent)
        , m_pStatusLabel        (nullptr)
        , m_pNewSceneDialog     (nullptr)
        , m_IsPlaying           (false)
        , m_IsSceneLoaded       (true)           //< TODO: Check if scene is loaded or not; should be done in load map state!
    {
        // -----------------------------------------------------------------------------
        // Dialogs
        // -----------------------------------------------------------------------------
        m_pNewSceneDialog = new CNewSceneDialog();

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
        m_pConsoleDockWidget  ->setVisible(false);

        tabifyDockWidget(m_pAssetsDockWidget, m_pConsoleDockWidget);

        // -----------------------------------------------------------------------------
        // Signal / slots
        // -----------------------------------------------------------------------------
        connect(m_pAssetBrowserWidget->m_pFileTreeView, SIGNAL(textureClicked(const QString&)), m_pInspector, SLOT(updateContentForTexture(const QString&)));
        connect(m_pAssetBrowserWidget->m_pFileTreeView, SIGNAL(materialClicked(const QString&)), m_pInspector, SLOT(updateContentForMaterial(const QString&)));

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::App_State_Change      , EDIT_RECEIVE_MESSAGE(&CMainWindow::OnStateChange));
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Graphic_FPS_Info      , EDIT_RECEIVE_MESSAGE(&CMainWindow::OnFramesPerSecond));
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Graphic_Histogram_Info, EDIT_RECEIVE_MESSAGE(&CMainWindow::OnHistogramInfo));
    }

    // -----------------------------------------------------------------------------

    CMainWindow::~CMainWindow() 
    {
        // -----------------------------------------------------------------------------
        // Delete dialogs
        // -----------------------------------------------------------------------------
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
        
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CMainWindow::SetRenderSize(int _Width, int _Height)
    {
        m_pEditorRenderContext->setFixedSize(QSize(_Width, _Height));
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::switchPlayingCurrentScene()
    {
        CMessage NewMessage;

        NewMessage.Reset();

        if (m_IsPlaying == false)
        {
            MessageManager::SendMessage(SGUIMessageType::App_Play, NewMessage);

            m_IsPlaying = true;
        }
        else
        {
            MessageManager::SendMessage(SGUIMessageType::App_Edit, NewMessage);
            
            m_IsPlaying = false;
        }
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::openNewSceneDialog()
    {
        m_pNewSceneDialog->show();
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewLightSun()
    {
        CMessage     NewMessage;
        unsigned int EntityID = 0;

        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        NewMessage.Reset();

        EntityID = MessageManager::SendMessage(SGUIMessageType::Entity_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create facet on entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Sun_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create and add entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Create, NewMessage);

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Add, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewEntityBloom()
    {
        CMessage     NewMessage;
        unsigned int EntityID = 0;

        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        NewMessage.Reset();

        EntityID = MessageManager::SendMessage(SGUIMessageType::Entity_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create facet on entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Effect_Bloom_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create and add entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Create, NewMessage);

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Add, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewEntityDOF()
    {
        CMessage     NewMessage;
        unsigned int EntityID = 0;

        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        NewMessage.Reset();

        EntityID = MessageManager::SendMessage(SGUIMessageType::Entity_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create facet on entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Effect_DOF_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create and add entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Create, NewMessage);

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Add, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewEntityPostAA()
    {
        CMessage     NewMessage;
        unsigned int EntityID = 0;

        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        NewMessage.Reset();

        EntityID = MessageManager::SendMessage(SGUIMessageType::Entity_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create facet on entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Effect_PostAA_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create and add entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Create, NewMessage);

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Add, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewEntitySSR()
    {
        CMessage     NewMessage;
        unsigned int EntityID = 0;

        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        NewMessage.Reset();

        EntityID = MessageManager::SendMessage(SGUIMessageType::Entity_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create facet on entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Effect_SSR_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create and add entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Create, NewMessage);

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Add, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewEntityVolumeFog()
    {
        CMessage     NewMessage;
        unsigned int EntityID = 0;

        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        NewMessage.Reset();

        EntityID = MessageManager::SendMessage(SGUIMessageType::Entity_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create facet on entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Effect_VolumeFog_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create and add entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Create, NewMessage);

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Add, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewLightPoint()
    {
        CMessage     NewMessage;
        unsigned int EntityID = 0;

        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        NewMessage.Reset();

        EntityID = MessageManager::SendMessage(SGUIMessageType::Entity_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create facet on entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Pointlight_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create and add entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Create, NewMessage);

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Add, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewLightEnvironment()
    {
        CMessage     NewMessage;
        unsigned int EntityID = 0;

        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        NewMessage.Reset();

        EntityID = MessageManager::SendMessage(SGUIMessageType::Entity_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create facet on entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Environment_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create and add entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Create, NewMessage);

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Add, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewLightGlobalProbe()
    {
        CMessage     NewMessage;
        unsigned int EntityID = 0;

        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        NewMessage.Reset();

        EntityID = MessageManager::SendMessage(SGUIMessageType::Entity_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create facet on entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Probe_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create and add entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Create, NewMessage);

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Add, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewLightArea()
    {
        CMessage     NewMessage;
        unsigned int EntityID = 0;

        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        NewMessage.Reset();

        EntityID = MessageManager::SendMessage(SGUIMessageType::Entity_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create facet on entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Arealight_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create and add entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Create, NewMessage);

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Add, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::createNewPluginARController()
    {
        CMessage     NewMessage;
        unsigned int EntityID = 0;

        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        NewMessage.Reset();

        EntityID = MessageManager::SendMessage(SGUIMessageType::Entity_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create facet on entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Plugin_ARConroller_New, NewMessage);

        // -----------------------------------------------------------------------------
        // Create and add entity
        // -----------------------------------------------------------------------------
        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Create, NewMessage);

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Add, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::reloadRenderer()
    {
        Edit::CMessage NewMessage(true);

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Graphic_ReloadRenderer, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::reloadAllShader()
    {
        Edit::CMessage NewMessage(true);

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Graphic_ReloadAllShader, NewMessage);
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

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Graphic_Histogram_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::toggleHistogramDock()
    {
        m_pHistogramDockWidget->setVisible(!m_pHistogramDockWidget->isVisible());

        if (m_pHistogramDockWidget->isVisible())
        {
            CMessage NewMessage(true);

            NewMessage.Reset();

            MessageManager::SendMessage(SGUIMessageType::Graphic_Histogram_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::toggleSceneGraphDock()
    {
        m_pSceneGraphDockWidget->setVisible(!m_pSceneGraphDockWidget->isVisible());
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::toggleInspectorDock()
    {
        m_pInspectorDockWidget->setVisible(!m_pInspectorDockWidget->isVisible());
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::toggleConsoleDock()
    {
        m_pConsoleDockWidget->setVisible(!m_pConsoleDockWidget->isVisible());
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::toggleAssetsDock()
    {
        m_pAssetsDockWidget->setVisible(!m_pAssetsDockWidget->isVisible());
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::closeEvent(QCloseEvent* _pEvent)
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::App_Exit, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::mousePressEvent(QMouseEvent* _pMouseEvent)
    {
        setFocus();
    }

    // -----------------------------------------------------------------------------

    void CMainWindow::OnStateChange(Edit::CMessage& _rMessage)
    {
        int OldState = _rMessage.GetInt();
        int NewState = _rMessage.GetInt();

        // TODO by tschwandt
        // Find a better solution to handle different states
        if (NewState == 1) // Intro
        {
            // -----------------------------------------------------------------------------
            // Check startup behavior
            // -----------------------------------------------------------------------------
            if (m_IsSceneLoaded == false)
            {
                m_pNewSceneDialog->show();

                m_IsSceneLoaded = true;
            }
            else
            {
                CMessage NewMessage(true);

                NewMessage.Reset();

                MessageManager::SendMessage(SGUIMessageType::App_LoadMap, NewMessage);
            }
        }
        else if (NewState == 3) // Edit
        {
            // -----------------------------------------------------------------------------
            // Reset all content widgets
            // -----------------------------------------------------------------------------
            m_pAssetBrowserWidget->Reset();

            // -----------------------------------------------------------------------------
            // Set focus of edit window
            // -----------------------------------------------------------------------------
            m_pEditorRenderContext->setFocus();
        }
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


