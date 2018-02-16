
#include "editor_gui/edit_inspector_entity.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspectorEntity::CInspectorEntity(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID(static_cast<unsigned int>(-1))
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Entity_Info, EDIT_RECEIVE_MESSAGE(&CInspectorEntity::OnEntityInfoEntity));
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

        int Layer = m_pLayerCB->currentIndex();

        int Category = m_pCategoryCB->currentIndex();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Put(Layer);

        NewMessage.Put(Category);

        if (NewEntityName.length() > 0)
        {
            NewMessage.Put(true);

            NewMessage.PutString(NewEntityNameBinary.data());
        }
        else
        {
            NewMessage.Put(false);
        }

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Entity_Info_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEntity::enableEntity(bool _Flag)
    {
        Edit::CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        if (_Flag)
        {
            Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Entity_Add, NewMessage);
        }
        else
        {
            Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Entity_Remove, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CInspectorEntity::RequestInformation(Base::ID _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEntity::OnEntityInfoEntity(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        char EntityName[256];

        Base::ID EntityID = _rMessage.Get<Base::ID>();

        if (EntityID != m_CurrentEntityID) return;

        bool IsEnabled = _rMessage.Get<bool>();

        int Layer    = _rMessage.Get<int>();
        int Category = _rMessage.Get<int>();

        bool HasName = _rMessage.Get<bool>();

        if (HasName)
        {
            _rMessage.GetString(EntityName, 256);
        }

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pEnabledCB ->blockSignals(true);
        m_pLayerCB   ->blockSignals(true);
        m_pCategoryCB->blockSignals(true);

        m_pEntityIDEdit->setText(QString::number(EntityID));

        if (HasName)
        {
            m_pEntityNameEdit->setText(EntityName);
        }
        else
        {
            m_pEntityNameEdit->setText("Unnamed entity");
        }

        m_pEnabledCB->setChecked(IsEnabled);

        m_pLayerCB->setCurrentIndex(Layer);

        m_pCategoryCB->setCurrentIndex(Category);

        m_pEnabledCB ->blockSignals(false);
        m_pLayerCB   ->blockSignals(false);
        m_pCategoryCB->blockSignals(false);
    }
} // namespace Edit


