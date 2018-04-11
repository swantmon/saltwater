
#include "base/base_typedef.h"
#include "base/base_type_info.h"

#include "engine/data/data_ar_controller_component.h"
#include "engine/data/data_area_light_component.h"
#include "engine/data/data_bloom_component.h"
#include "engine/data/data_camera_component.h"
#include "engine/data/data_dof_component.h"
#include "engine/data/data_light_probe_component.h"
#include "engine/data/data_mesh_component.h"
#include "engine/data/data_point_light_component.h"
#include "engine/data/data_post_aa_component.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_ssr_component.h"
#include "engine/data/data_sun_component.h"
#include "engine/data/data_volume_fog_component.h"

#include "editor_gui/edit_inspector.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspector::CInspector(QWidget* _pParent)
        : QWidget              (_pParent)
        , m_pInspectorLayout   (0)
        , m_pEntityWidget      (0)
        , m_pPointlightWidget  (0)
        , m_pArealightWidget   (0)
        , m_pSunWidget         (0)
        , m_pTransformWidget   (0)
        , m_pEnvironmentWidget (0)
        , m_pLightProbeWidget  (0)
        , m_pBloomWidget       (0)
        , m_pDOFWidget         (0)
        , m_pFXAAWidget        (0)
        , m_pSSRWidget         (0)
        , m_pVolumeFogWidget   (0)
        , m_pMaterialWidget    (0)
        , m_pCameraWidget      (0)
        , m_pARControllerWidget(0)
        , m_pTextureWidget     (0)
        , m_ActiveEntityID     (static_cast<Base::ID>(-1))
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
        m_pArealightWidget    = new CInspectorArealight();
        m_pSunWidget          = new CInspectorSun();
        m_pTransformWidget    = new CInspectorTransformation();
        m_pEnvironmentWidget  = new CInspectorEnvironment();
        m_pLightProbeWidget   = new CInspectorLightProbe();
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
        m_pInspectorContent->addWidget(m_pArealightWidget);
        m_pInspectorContent->addWidget(m_pSunWidget);
        m_pInspectorContent->addWidget(m_pEnvironmentWidget);
        m_pInspectorContent->addWidget(m_pLightProbeWidget);
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
        delete m_pArealightWidget;
        delete m_pSunWidget;
        delete m_pTransformWidget;
        delete m_pEnvironmentWidget;
        delete m_pLightProbeWidget;
        delete m_pBloomWidget;
        delete m_pDOFWidget;
        delete m_pFXAAWidget;
        delete m_pSSRWidget;
        delete m_pVolumeFogWidget;
        delete m_pMaterialWidget;
        delete m_pCameraWidget;

        m_pEntityWidget      = 0;
        m_pPointlightWidget  = 0;
        m_pArealightWidget   = 0;
        m_pSunWidget         = 0;
        m_pTransformWidget   = 0;
        m_pEnvironmentWidget = 0;
        m_pLightProbeWidget  = 0;
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
        m_ActiveEntityID = static_cast<Base::ID>(_ID);

        // -----------------------------------------------------------------------------

        HighlightEntity(m_ActiveEntityID);

        // -----------------------------------------------------------------------------

        CMessage FacetMessage;

        FacetMessage.Put(m_ActiveEntityID);

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
        BASE_UNUSED(_rRelPath);

        ResetLayout();

        ResetHighlight();

//         m_pModelWidget->RequestInformation(_rRelPath);
// 
//         m_pModelWidget->setVisible(true);
    }

    // -----------------------------------------------------------------------------

    void CInspector::mousePressEvent(QMouseEvent* _pEvent)
    {
        BASE_UNUSED(_pEvent);

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
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        bool HasTransformation = _rMessage.Get<bool>();

        size_t NumberOfComponents = _rMessage.Get<size_t>();

        std::vector<Base::ID> ComponentIDs;

        for (int IndexOfComponent = 0; IndexOfComponent < NumberOfComponents; ++IndexOfComponent)
        {
            ComponentIDs.push_back(_rMessage.Get<Base::ID>());
        }

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
        for (auto TypeID : ComponentIDs)
        {
            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CMeshComponent>())
            {
                m_pMaterialWidget->RequestInformation(EntityID);

                m_pMaterialWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CCameraComponent>())
            {
                m_pCameraWidget->RequestInformation(EntityID);

                m_pCameraWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CPointLightComponent>())
            {
                m_pPointlightWidget->RequestInformation(EntityID);

                m_pPointlightWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CAreaLightComponent>())
            {
                m_pArealightWidget->RequestInformation(EntityID);

                m_pArealightWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CSunComponent>())
            {
                m_pSunWidget->RequestInformation(EntityID);

                m_pSunWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CLightProbeComponent>())
            {
                m_pLightProbeWidget->RequestInformation(EntityID);

                m_pLightProbeWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CSkyComponent>())
            {
                m_pEnvironmentWidget->RequestInformation(EntityID);

                m_pEnvironmentWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CBloomComponent>())
            {
                m_pBloomWidget->RequestInformation(EntityID);

                m_pBloomWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CSSRComponent>())
            {
                m_pSSRWidget->RequestInformation(EntityID);

                m_pSSRWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CDOFComponent>())
            {
                m_pDOFWidget->RequestInformation(EntityID);

                m_pDOFWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CPostAAComponent>())
            {
                m_pFXAAWidget->RequestInformation(EntityID);

                m_pFXAAWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CVolumeFogComponent>())
            {
                m_pVolumeFogWidget->RequestInformation(EntityID);

                m_pVolumeFogWidget->setVisible(true);
            }

            if (TypeID == Base::CTypeInfo::GetTypeID<Dt::CARControllerPluginComponent>())
            {
                m_pARControllerWidget->RequestInformation(EntityID);

                m_pARControllerWidget->setVisible(true);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CInspector::ResetLayout()
    {
        m_pEntityWidget      ->setVisible(false);
        m_pTransformWidget   ->setVisible(false);
        m_pPointlightWidget  ->setVisible(false);
        m_pArealightWidget   ->setVisible(false);
        m_pSunWidget         ->setVisible(false);
        m_pEnvironmentWidget ->setVisible(false);
        m_pLightProbeWidget  ->setVisible(false);
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

    void CInspector::HighlightEntity(Base::ID _ID)
    {
        // -----------------------------------------------------------------------------
        // Send messages: Selection and facet infos
        // -----------------------------------------------------------------------------
        Edit::CMessage SelectionMessage;

        SelectionMessage.Put(_ID);

        SelectionMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Graphic_HighlightEntity, SelectionMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspector::ResetHighlight()
    {
        Edit::CMessage NewMessage;

        NewMessage.Put<Base::ID>(static_cast<Base::ID>(-1));

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Graphic_HighlightEntity, NewMessage);
    }
} // namespace Edit