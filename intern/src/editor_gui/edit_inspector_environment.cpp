
#include "base/base_crc.h"

#include "editor_gui/edit_inspector_environment.h"

#include "editor_port/edit_message_manager.h"

#include <QFileDialog>

namespace Edit
{
    CInspectorEnvironment::CInspectorEnvironment(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID(static_cast<unsigned int>(-1))
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Setup user UI
        // -----------------------------------------------------------------------------
        m_pTextureValue->SetLayout(CTextureValue::NoPreview);

        // -----------------------------------------------------------------------------
        // Signal / slots
        // -----------------------------------------------------------------------------
        connect(m_pTextureValue, SIGNAL(fileChanged(QString)), SLOT(valueChanged()));

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Light_Environment_Info, EDIT_RECEIVE_MESSAGE(&CInspectorEnvironment::OnEntityInfoEnvironment));
    }

    // -----------------------------------------------------------------------------

    CInspectorEnvironment::~CInspectorEnvironment() 
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorEnvironment::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int RefreshMode = m_pRefreshModeCB->currentIndex();
        
        int Type = m_pTypeCB->currentIndex();

        std::string Texture = std::string(m_pTextureValue->GetTextureFile().toLatin1());

        float Intensity = m_pIntensityEdit->text().toFloat();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Put(RefreshMode);

        NewMessage.Put(Type);

        NewMessage.Put(Texture);

        NewMessage.Put(Intensity);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Light_Environment_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEnvironment::RequestInformation(Base::ID _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Environment_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEnvironment::OnEntityInfoEnvironment(Edit::CMessage& _rMessage)
    {
        std::string TextureName;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        if (EntityID != m_CurrentEntityID) return;

        int RefreshMode = _rMessage.Get<int>();

        int Type = _rMessage.Get<int>();

        bool HasTexture = _rMessage.Get<bool>();

        if (HasTexture)
        {
            bool HasName = _rMessage.Get<bool>();

            if (HasName)
            {
                TextureName = _rMessage.Get<std::string>();
            }
        }

        float Intensity = _rMessage.Get<float>();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pRefreshModeCB->blockSignals(true);
        m_pTypeCB->blockSignals(true);

        m_pRefreshModeCB->setCurrentIndex(RefreshMode);

        m_pTypeCB->setCurrentIndex(Type);

        if (HasTexture)
        {
            m_pTextureValue->SetTextureFile(QString(TextureName.c_str()));
        }

        m_pIntensityEdit->setText(QString::number(Intensity));

        m_pTypeCB->blockSignals(false);
        m_pRefreshModeCB->blockSignals(false);
    }
} // namespace Edit

