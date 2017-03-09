
#include "base/base_vector3.h"

#include "editor_gui/edit_inspector_post_aa.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorPostAA::CInspectorPostAA(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID(-1)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Effect_PostAA_Info, EDIT_RECEIVE_MESSAGE(&CInspectorPostAA::OnEntityInfoPostAA));
    }

    // -----------------------------------------------------------------------------

    CInspectorPostAA::~CInspectorPostAA()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorPostAA::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        const int Index = m_pTypeCB->currentIndex();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutInt(Index);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Effect_PostAA_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorPostAA::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Effect_PostAA_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorPostAA::OnEntityInfoPostAA(Edit::CMessage& _rMessage)
    {
        int Type;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        Type = _rMessage.GetInt();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------

        m_pTypeCB->setCurrentIndex(Type);
    }
} // namespace Edit
