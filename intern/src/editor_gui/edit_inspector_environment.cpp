
#include "editor_gui/edit_inspector_environment.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspectorEnvironment::CInspectorEnvironment(QWidget* _pParent)
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
        Edit::MessageManager::Register(Edit::SApplicationMessageType::LightInfoEnvironment, EDIT_RECEIVE_MESSAGE(&CInspectorEnvironment::OnEntityInfoEnvironment));
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
        int Type = m_pTypeCB->currentIndex();

        QString    NewEnvironmentTexture       = m_pTextureEdit->text();
        QByteArray NewEnvironmentTextureBinary = NewEnvironmentTexture.toLatin1();

        unsigned int CubemapHash = m_pCubemapHashEdit->text().toUInt();

        float Intensity = m_pIntensityEdit->text().toFloat();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutInt(Type);

        NewMessage.PutString(NewEnvironmentTextureBinary.data());

        if (CubemapHash != 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutInt(CubemapHash);
        }
        else
        {
            NewMessage.PutBool(false);
        }

        NewMessage.PutFloat(Intensity);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::LightInfoEnvironment, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEnvironment::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestLightInfoEnvironment, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEnvironment::OnEntityInfoEnvironment(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        int Type = _rMessage.GetInt();

        char pTemp[256];

        const char* pTexture = _rMessage.GetString(pTemp, 256);

        bool HasCubemap = _rMessage.GetBool();

        int CubemapHash = -1;

        if (HasCubemap)
        {
            CubemapHash = _rMessage.GetInt();
        }

        float Intensity = _rMessage.GetFloat();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pTypeCB->blockSignals(true);

        m_pTypeCB->setCurrentIndex(Type);

        m_pTypeCB->blockSignals(false);

        m_pTextureEdit->setText(pTexture);

        if (HasCubemap)
        {
            m_pCubemapHashEdit->setText(QString::number(CubemapHash));
        }
        else
        {
            m_pCubemapHashEdit->setText(QString::number(0));
        }

        m_pIntensityEdit->setText(QString::number(Intensity));
    }
} // namespace Edit

