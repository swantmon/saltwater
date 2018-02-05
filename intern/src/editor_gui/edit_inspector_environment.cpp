
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
        connect(m_pTextureValue, SIGNAL(hashChanged(unsigned int)), SLOT(valueChanged()));
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

        unsigned int TextureHash = m_pTextureValue->GetTextureHash();

        float Intensity = m_pIntensityEdit->text().toFloat();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutInt(RefreshMode);

        NewMessage.PutInt(Type);

        NewMessage.PutInt(TextureHash);

        NewMessage.PutFloat(Intensity);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Light_Environment_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEnvironment::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Environment_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEnvironment::OnEntityInfoEnvironment(Edit::CMessage& _rMessage)
    {
        char pTemp[256];
        const char* pTexture = nullptr;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        int RefreshMode = _rMessage.GetInt();

        int Type = _rMessage.GetInt();

        bool HasTexture = _rMessage.GetBool();

        if (HasTexture)
        {
            bool HasName = _rMessage.GetBool();

            if (HasName)
            {
                pTexture = _rMessage.GetString(pTemp, 256);
            }
        }

        unsigned int TextureHash = _rMessage.GetInt();

        float Intensity = _rMessage.GetFloat();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pRefreshModeCB->blockSignals(true);
        m_pTypeCB->blockSignals(true);

        m_pRefreshModeCB->setCurrentIndex(RefreshMode);

        m_pTypeCB->setCurrentIndex(Type);

        if (HasTexture)
        {
            m_pTextureValue->SetTextureFile(pTexture);
        }

        m_pTextureValue->SetTextureHash(TextureHash);

        m_pIntensityEdit->setText(QString::number(Intensity));

        m_pTypeCB->blockSignals(false);
        m_pRefreshModeCB->blockSignals(false);
    }
} // namespace Edit

