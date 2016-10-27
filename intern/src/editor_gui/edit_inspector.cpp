

#include "editor_gui/edit_inspector.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspector::CInspector(QWidget* _pParent)
        : QWidget           (_pParent)
        , m_pInspectorLayout()
    {
        // -----------------------------------------------------------------------------
        // Create layout
        // -----------------------------------------------------------------------------
        m_pInspectorLayout = new QVBoxLayout();

        // -----------------------------------------------------------------------------
        // Create transformation widget and add to layout
        // -----------------------------------------------------------------------------
        m_pPointlightWidget = new CInspectorPointlight();
        m_pTransformWidget  = new CInspectorTransformation();

        m_pInspectorLayout->addWidget(m_pTransformWidget);
        m_pInspectorLayout->addWidget(m_pPointlightWidget);

        m_pPointlightWidget->setVisible(false);
        m_pTransformWidget ->setVisible(false);

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
            m_pPointlightWidget->setVisible(false);

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
                    m_pPointlightWidget->setVisible(true);
                }
                else if (Type == 1) // Sun
                {
                }
                else if (Type == 2) // GlobalProbe
                {
                }
                else if (Type == 3) // Skybox
                {
                }
            }
            else if (Category == 2) // FX
            {
                if (Type == 0) // Bloom
                {
                }
                else if (Type == 1) // SSR
                {
                }
                else if (Type == 2) // DOF
                {
                }
                else if (Type == 3) // FXAA
                {
                }
                else if (Type == 4) // SSAO
                {
                }
                else if (Type == 5) // VolumeFog
                {
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