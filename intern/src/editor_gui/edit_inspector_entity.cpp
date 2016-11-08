
#include "editor_gui/edit_inspector_entity.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspectorEntity::CInspectorEntity(QWidget* _pParent)
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
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EntityInfoEntity, EDIT_RECEIVE_MESSAGE(&CInspectorEntity::OnEntityInfoEntity));
    }

    // -----------------------------------------------------------------------------

    CInspectorEntity::~CInspectorEntity()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorEntity::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        QString    NewEntityName       = m_pEntityNameEdit->text();
        QByteArray NewEntityNameBinary = NewEntityName.toLatin1();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        if (NewEntityName.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(NewEntityNameBinary.data());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::EntityInfoEntity, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEntity::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoEntity, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEntity::OnEntityInfoEntity(Edit::CMessage& _rMessage)
    {
        auto GetCategoryName = [&](unsigned int _Category)->const char*
        {
            const char* pCategoryStrings[]
            {
                "Actor" ,
                "Light" ,
                "FX"    ,
                "Plugin",
            };

            return pCategoryStrings[_Category];
        };

        char EntityName[256];

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        int Category = _rMessage.GetInt();
        int Type     = _rMessage.GetInt();

        bool HasName = _rMessage.GetBool();

        if (HasName)
        {
            _rMessage.GetString(EntityName, 256);
        }

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pEntityIDEdit->setText(QString::number(EntityID));

        if (HasName)
        {
            m_pEntityNameEdit->setText(EntityName);
        }
        else
        {
            m_pEntityNameEdit->setText("Unnamed entity");
        }
    }
} // namespace Edit


