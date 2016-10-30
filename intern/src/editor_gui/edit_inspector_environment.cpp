
#include "editor_gui/edit_inspector_environment.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspectorEnvironment::CInspectorEnvironment(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EntityInfoEnvironment, EDIT_RECEIVE_MESSAGE(&CInspectorEnvironment::OnEntityInfoEnvironment));
    }

    // -----------------------------------------------------------------------------

    CInspectorEnvironment::~CInspectorEnvironment() 
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorEnvironment::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        int Type = m_pTypeCB->currentIndex();

        SaveCopy = m_pTextureEdit->text().toStdString();

        const char* pNewTexture = SaveCopy.c_str();

        float Intensity = m_pIntensityEdit->text().toFloat();

        NewMessage.PutInt(Type);

        NewMessage.PutString(pNewTexture);

        NewMessage.PutFloat(Intensity);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::EntityInfoEnvironment, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEnvironment::RequestInformation()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoEnvironment, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEnvironment::OnEntityInfoEnvironment(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int Type = _rMessage.GetInt();

        char pTemp[256];

        const char* pTexture = _rMessage.GetString(pTemp, 256);

        float Intensity = _rMessage.GetFloat();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pTypeCB->blockSignals(true);

        m_pTypeCB->setCurrentIndex(Type);

        m_pTypeCB->blockSignals(false);

        m_pTextureEdit->setText(pTexture);

        m_pIntensityEdit->setText(QString::number(Intensity));
    }
} // namespace Edit

