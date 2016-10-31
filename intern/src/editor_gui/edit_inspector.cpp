

#include "editor_gui/edit_inspector.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspector::CInspector(QWidget* _pParent)
        : QWidget             (_pParent)
        , m_pInspectorLayout  ()
        , m_pPointlightWidget ()
        , m_pTransformWidget  ()
        , m_pEnvironmentWidget()
        , m_pGlobalProbeWidget()
        , m_pBloomWidget      ()
        , m_pDOFWidget        ()
        , m_pFXAAWidget       ()
        , m_pSSRWidget        ()
        , m_pVolumeFogWidget  ()


    {
        // -----------------------------------------------------------------------------
        // Create layout
        // -----------------------------------------------------------------------------
        m_pInspectorLayout = new QVBoxLayout();

        // -----------------------------------------------------------------------------
        // Create transformation widget and add to layout
        // -----------------------------------------------------------------------------
        m_pPointlightWidget  = new CInspectorPointlight();
        m_pTransformWidget   = new CInspectorTransformation();
        m_pEnvironmentWidget = new CInspectorEnvironment();
        m_pGlobalProbeWidget = new CInspectorGlobalProbe();
        m_pBloomWidget       = new CInspectorBloom();
        m_pDOFWidget         = new CInspectorDOF();
        m_pFXAAWidget        = new CInspectorFXAA();
        m_pSSRWidget         = new CInspectorSSR();
        m_pVolumeFogWidget   = new CInspectorVolumeFog();

        m_pInspectorLayout->addWidget(m_pTransformWidget);
        m_pInspectorLayout->addWidget(m_pPointlightWidget);
        m_pInspectorLayout->addWidget(m_pEnvironmentWidget);
        m_pInspectorLayout->addWidget(m_pGlobalProbeWidget);
        m_pInspectorLayout->addWidget(m_pBloomWidget);
        m_pInspectorLayout->addWidget(m_pDOFWidget);
        m_pInspectorLayout->addWidget(m_pFXAAWidget);
        m_pInspectorLayout->addWidget(m_pSSRWidget);
        m_pInspectorLayout->addWidget(m_pVolumeFogWidget);

        m_pPointlightWidget ->setVisible(false);
        m_pTransformWidget  ->setVisible(false);
        m_pEnvironmentWidget->setVisible(false);
        m_pGlobalProbeWidget->setVisible(false);
        m_pBloomWidget      ->setVisible(false);
        m_pDOFWidget        ->setVisible(false);
        m_pFXAAWidget       ->setVisible(false);
        m_pSSRWidget        ->setVisible(false);
        m_pVolumeFogWidget  ->setVisible(false);

        // -----------------------------------------------------------------------------
        // Set layout
        // -----------------------------------------------------------------------------
        setLayout(m_pInspectorLayout);

        // -----------------------------------------------------------------------------
        // Register messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EntityInfoFacets, EDIT_RECEIVE_MESSAGE(&CInspector::OnEntityInfoFacets));
    }

    // -----------------------------------------------------------------------------

    CInspector::~CInspector() 
    {

    }

    // -----------------------------------------------------------------------------

    void CInspector::updateContentForEntity(int _ID)
    {
        CMessage NewMessage;

        NewMessage.PutInt(_ID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoFacets, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspector::OnEntityInfoFacets(Edit::CMessage& _rMessage)
    {
        int Category = _rMessage.GetInt();
        int Type     = _rMessage.GetInt();

        bool HasTransformation = _rMessage.GetBool();
        bool HasHierarchy      = _rMessage.GetBool();

        bool HasDetailData    = _rMessage.GetBool();
        bool HasDetailGraphic = _rMessage.GetBool();
        bool HasDetailScript  = _rMessage.GetBool();

        if (HasTransformation)
        {
            m_pTransformWidget->RequestInformation();

            m_pTransformWidget->setVisible(true);
        }
        else
        {
            m_pTransformWidget->setVisible(false);
        }

        if (HasDetailData)
        {
            m_pPointlightWidget ->setVisible(false);
            m_pEnvironmentWidget->setVisible(false);
            m_pGlobalProbeWidget->setVisible(false);
            m_pBloomWidget      ->setVisible(false);
            m_pDOFWidget        ->setVisible(false);
            m_pFXAAWidget       ->setVisible(false);
            m_pSSRWidget        ->setVisible(false);
            m_pVolumeFogWidget  ->setVisible(false);

            if (Category == 0) // Actors
            {
                if (Type == 0) // Node
                {
                }
                else if (Type == 0) // Model
                {
                }
                else if (Type == 1) // AR
                {
                }
                else if (Type == 2) // Camera
                {
                }
            }
            else if (Category == 1) // Light
            {
                if (Type == 0) // Point
                {
                    m_pPointlightWidget->RequestInformation();

                    m_pPointlightWidget->setVisible(true);
                }
                else if (Type == 1) // Sun
                {
                }
                else if (Type == 2) // GlobalProbe
                {
                    m_pGlobalProbeWidget->RequestInformation();

                    m_pGlobalProbeWidget->setVisible(true);
                }
                else if (Type == 3) // Environment
                {
                    m_pEnvironmentWidget->RequestInformation();

                    m_pEnvironmentWidget->setVisible(true);
                }
            }
            else if (Category == 2) // FX
            {
                if (Type == 0) // Bloom
                {
                    m_pBloomWidget->RequestInformation();

                    m_pBloomWidget->setVisible(true);
                }
                else if (Type == 1) // SSR
                {
                    m_pSSRWidget->RequestInformation();

                    m_pSSRWidget->setVisible(true);
                }
                else if (Type == 2) // DOF
                {
                    m_pDOFWidget->RequestInformation();

                    m_pDOFWidget->setVisible(true);
                }
                else if (Type == 3) // FXAA
                {
                    m_pFXAAWidget->RequestInformation();

                    m_pFXAAWidget->setVisible(true);
                }
                else if (Type == 4) // SSAO
                {
                }
                else if (Type == 5) // VolumeFog
                {
                    m_pVolumeFogWidget->RequestInformation();

                    m_pVolumeFogWidget->setVisible(true);
                }
            }
            else if (Category == 3) // Plugin
            {
                if (Type == 0) // ARControlManager
                {
                }
                else if (Type == 1) // ARTrackedObject
                {
                }
            }
        }
    }
} // namespace Edit