

#include "editor_gui/edit_inspector.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspector::CInspector(QWidget* _pParent)
        : QWidget              (_pParent)
        , m_pInspectorLayout   ()
        , m_pEntityWidget      ()
        , m_pPointlightWidget  ()
        , m_pSunWidget         ()
        , m_pTransformWidget   ()
        , m_pEnvironmentWidget ()
        , m_pGlobalProbeWidget ()
        , m_pBloomWidget       ()
        , m_pDOFWidget         ()
        , m_pFXAAWidget        ()
        , m_pSSRWidget         ()
        , m_pVolumeFogWidget   ()
        , m_pMaterialWidget    ()
        , m_pCameraWidget      ()
        , m_pARControllerWidget()
        , m_pTextureWidget     ()
        , m_ActiveEntityID     (-1)
    {
        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        setupUi(this);

        m_pInspectorContent->setAlignment(Qt::AlignTop);

        // -----------------------------------------------------------------------------
        // Create transformation widget and add to layout
        // -----------------------------------------------------------------------------
        m_pEntityWidget       = new CInspectorEntity();
        m_pPointlightWidget   = new CInspectorPointlight();
        m_pSunWidget          = new CInspectorSun();
        m_pTransformWidget    = new CInspectorTransformation();
        m_pEnvironmentWidget  = new CInspectorEnvironment();
        m_pGlobalProbeWidget  = new CInspectorGlobalProbe();
        m_pBloomWidget        = new CInspectorBloom();
        m_pDOFWidget          = new CInspectorDOF();
        m_pFXAAWidget         = new CInspectorPostAA();
        m_pSSRWidget          = new CInspectorSSR();
        m_pVolumeFogWidget    = new CInspectorVolumeFog();
        m_pMaterialWidget     = new CInspectorMaterial();
        m_pCameraWidget       = new CInspectorCamera();
        m_pARControllerWidget = new CInspectorARController();
        m_pTextureWidget      = new CInspectorTexture();

        m_pInspectorContent->addWidget(m_pEntityWidget);
        m_pInspectorContent->addWidget(m_pTransformWidget);
        m_pInspectorContent->addWidget(m_pPointlightWidget);
        m_pInspectorContent->addWidget(m_pSunWidget);
        m_pInspectorContent->addWidget(m_pEnvironmentWidget);
        m_pInspectorContent->addWidget(m_pGlobalProbeWidget);
        m_pInspectorContent->addWidget(m_pBloomWidget);
        m_pInspectorContent->addWidget(m_pDOFWidget);
        m_pInspectorContent->addWidget(m_pFXAAWidget);
        m_pInspectorContent->addWidget(m_pSSRWidget);
        m_pInspectorContent->addWidget(m_pVolumeFogWidget);
        m_pInspectorContent->addWidget(m_pMaterialWidget);
        m_pInspectorContent->addWidget(m_pCameraWidget);
        m_pInspectorContent->addWidget(m_pARControllerWidget);
        m_pInspectorContent->addWidget(m_pTextureWidget);

        ResetLayout();

        // -----------------------------------------------------------------------------
        // Register messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Entity_Facets_Info, EDIT_RECEIVE_MESSAGE(&CInspector::OnEntityInfoFacets));
    }

    // -----------------------------------------------------------------------------

    CInspector::~CInspector() 
    {
        delete m_pEntityWidget;
        delete m_pPointlightWidget;
        delete m_pSunWidget;
        delete m_pTransformWidget;
        delete m_pEnvironmentWidget;
        delete m_pGlobalProbeWidget;
        delete m_pBloomWidget;
        delete m_pDOFWidget;
        delete m_pFXAAWidget;
        delete m_pSSRWidget;
        delete m_pVolumeFogWidget;
        delete m_pMaterialWidget;
        delete m_pCameraWidget;

        m_pEntityWidget      = 0;
        m_pPointlightWidget  = 0;
        m_pSunWidget         = 0;
        m_pTransformWidget   = 0;
        m_pEnvironmentWidget = 0;
        m_pGlobalProbeWidget = 0;
        m_pBloomWidget       = 0;
        m_pDOFWidget         = 0;
        m_pFXAAWidget        = 0;
        m_pSSRWidget         = 0;
        m_pVolumeFogWidget   = 0;
        m_pMaterialWidget    = 0;
        m_pCameraWidget      = 0;
    }

    // -----------------------------------------------------------------------------

    void CInspector::updateContentForEntity(int _ID)
    {
        m_ActiveEntityID = _ID;

        // -----------------------------------------------------------------------------

        HighlightEntity(m_ActiveEntityID);

        // -----------------------------------------------------------------------------

        CMessage FacetMessage;

        FacetMessage.PutInt(m_ActiveEntityID);

        FacetMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Facets_Info, FacetMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspector::updateContentForTexture(const QString& _rRelPath)
    {
        ResetLayout();

        ResetHighlight();

        m_pTextureWidget->RequestInformation(_rRelPath);

        m_pTextureWidget->setVisible(true);
    }

    // -----------------------------------------------------------------------------

    void CInspector::updateContentForMaterial(const QString& _rRelPath)
    {
        ResetLayout();

        ResetHighlight();

        m_pMaterialWidget->RequestInformation(_rRelPath);

        m_pMaterialWidget->setVisible(true);
    }

    // -----------------------------------------------------------------------------

    void CInspector::updateContentForModel(const QString& _rRelPath)
    {
        ResetLayout();

        ResetHighlight();

//         m_pModelWidget->RequestInformation(_rRelPath);
// 
//         m_pModelWidget->setVisible(true);
    }

    // -----------------------------------------------------------------------------

    void CInspector::mousePressEvent(QMouseEvent* _pEvent)
    {
        ResetHighlight();
    }

    // -----------------------------------------------------------------------------

    void CInspector::OnEntityInfoFacets(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ResetLayout();

        // -----------------------------------------------------------------------------
        // Read data
        // -----------------------------------------------------------------------------
        unsigned int EntityID = static_cast<unsigned int>(_rMessage.GetInt());

        int Category = _rMessage.GetInt();
        int Type     = _rMessage.GetInt();

        bool HasTransformation = _rMessage.GetBool();
        bool HasHierarchy      = _rMessage.GetBool();

        bool HasDetailData    = _rMessage.GetBool();
        bool HasDetailGraphic = _rMessage.GetBool();
        bool HasDetailScript  = _rMessage.GetBool();

        // -----------------------------------------------------------------------------
        // General informations
        // -----------------------------------------------------------------------------
        m_pEntityWidget->RequestInformation(EntityID);

        m_pEntityWidget->setVisible(true);

        // -----------------------------------------------------------------------------
        // Transformation
        // -----------------------------------------------------------------------------
        if (HasTransformation)
        {
            m_pTransformWidget->RequestInformation(EntityID);

            m_pTransformWidget->setVisible(true);
        }

        // -----------------------------------------------------------------------------
        // Details
        // -----------------------------------------------------------------------------
        if (HasDetailData)
        {
            if (Category == 0) // Actors
            {
                if (Type == 0) // Node
                {
                }
                else if (Type == 1) // Model
                {
                    m_pMaterialWidget->RequestInformation(EntityID);

                    m_pMaterialWidget->setVisible(true);
                }
                else if (Type == 2) // Camera
                {
                    m_pCameraWidget->RequestInformation(EntityID);

                    m_pCameraWidget->setVisible(true);
                }
            }
            else if (Category == 1) // Light
            {
                if (Type == 0) // Point
                {
                    m_pPointlightWidget->RequestInformation(EntityID);

                    m_pPointlightWidget->setVisible(true);
                }
                else if (Type == 1) // Sun
                {
                    m_pSunWidget->RequestInformation(EntityID);

                    m_pSunWidget->setVisible(true);
                }
                else if (Type == 2) // Probe
                {
                    m_pGlobalProbeWidget->RequestInformation(EntityID);

                    m_pGlobalProbeWidget->setVisible(true);
                }
                else if (Type == 3) // Environment
                {
                    m_pEnvironmentWidget->RequestInformation(EntityID);

                    m_pEnvironmentWidget->setVisible(true);
                }
            }
            else if (Category == 2) // FX
            {
                if (Type == 0) // Bloom
                {
                    m_pBloomWidget->RequestInformation(EntityID);

                    m_pBloomWidget->setVisible(true);
                }
                else if (Type == 1) // SSR
                {
                    m_pSSRWidget->RequestInformation(EntityID);

                    m_pSSRWidget->setVisible(true);
                }
                else if (Type == 2) // DOF
                {
                    m_pDOFWidget->RequestInformation(EntityID);

                    m_pDOFWidget->setVisible(true);
                }
                else if (Type == 3) // FXAA
                {
                    m_pFXAAWidget->RequestInformation(EntityID);

                    m_pFXAAWidget->setVisible(true);
                }
                else if (Type == 4) // SSAO
                {
                }
                else if (Type == 5) // VolumeFog
                {
                    m_pVolumeFogWidget->RequestInformation(EntityID);

                    m_pVolumeFogWidget->setVisible(true);
                }
            }
            else if (Category == 3) // Plugin
            {
                if (Type == 0) // ARControlManager
                {
                    m_pARControllerWidget->RequestInformation(EntityID);

                    m_pARControllerWidget->setVisible(true);
                }
                else if (Type == 1) // ARTrackedObject
                {
                }
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CInspector::ResetLayout()
    {
        m_pEntityWidget      ->setVisible(false);
        m_pTransformWidget   ->setVisible(false);
        m_pPointlightWidget  ->setVisible(false);
        m_pSunWidget         ->setVisible(false);
        m_pEnvironmentWidget ->setVisible(false);
        m_pGlobalProbeWidget ->setVisible(false);
        m_pBloomWidget       ->setVisible(false);
        m_pDOFWidget         ->setVisible(false);
        m_pFXAAWidget        ->setVisible(false);
        m_pSSRWidget         ->setVisible(false);
        m_pVolumeFogWidget   ->setVisible(false);
        m_pMaterialWidget    ->setVisible(false);
        m_pCameraWidget      ->setVisible(false);
        m_pARControllerWidget->setVisible(false);
        m_pTextureWidget     ->setVisible(false);
    }

    // -----------------------------------------------------------------------------

    void CInspector::HighlightEntity(int _ID)
    {
        // -----------------------------------------------------------------------------
        // Send messages: Selection and facet infos
        // -----------------------------------------------------------------------------
        Edit::CMessage SelectionMessage;

        SelectionMessage.PutInt(_ID);

        SelectionMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Graphic_HighlightEntity, SelectionMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspector::ResetHighlight()
    {
        Edit::CMessage NewMessage;

        NewMessage.PutInt(-1);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Graphic_HighlightEntity, NewMessage);
    }
} // namespace Edit