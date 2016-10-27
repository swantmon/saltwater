

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
        m_pTransformWidget = new CInspectorTransformation();

        m_pInspectorLayout->addWidget(m_pTransformWidget);

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
            m_pInspectorLayout->removeWidget(m_pTransformWidget);

            m_pTransformWidget->setVisible(false);
        }
    }
} // namespace Edit