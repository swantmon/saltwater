
#include "base/base_crc.h"

#include "editor_gui/edit_inspector_environment.h"

#include "editor_port/edit_message_manager.h"

#include <QFileDialog>

namespace Edit
{
    CInspectorEnvironment::CInspectorEnvironment(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID(-1)
        , m_TextureFileName()
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

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
        int Type = m_pTypeCB->currentIndex();

        QByteArray NewTextureBinary = m_TextureFileName.toLatin1();

        unsigned int TextureHash = m_pTextureEdit->text().toUInt();

        float Intensity = m_pIntensityEdit->text().toFloat();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutInt(Type);

        NewMessage.PutString(NewTextureBinary);

        NewMessage.PutInt(TextureHash);

        NewMessage.PutFloat(Intensity);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Light_Environment_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorEnvironment::loadTextureFromDialog()
    {
        QString TextureFile = QFileDialog::getOpenFileName(this, tr("Load environment file"), tr(""), tr("Environment files (*.dds *.hdr)"));

        // -----------------------------------------------------------------------------
        // Send message with new scene / map request
        // -----------------------------------------------------------------------------
        if (!TextureFile.isEmpty())
        {
            QDir dir("../assets/");

            m_TextureFileName = dir.relativeFilePath(TextureFile);

            // -----------------------------------------------------------------------------
            // Create hash
            // TODO: Should be done by a texture manager
            // -----------------------------------------------------------------------------
            QByteArray NewTextureBinary = m_TextureFileName.toLatin1();

            const char*  pHashIdentifier = NewTextureBinary.data();
            unsigned int NumberOfBytes;
            unsigned int Hash;

            const void* pData;

            NumberOfBytes = static_cast<unsigned int>(strlen(pHashIdentifier) * sizeof(char));
            pData = static_cast<const void*>(pHashIdentifier);

            Hash = Base::CRC32(pData, NumberOfBytes);

            m_pTextureEdit->setText(QString::number(Hash));

            valueChanged();
        }
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
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        int Type = _rMessage.GetInt();

        char pTemp[256];

        const char* pTexture = _rMessage.GetString(pTemp, 256);

        unsigned int TextureHash = _rMessage.GetInt();

        float Intensity = _rMessage.GetFloat();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pTypeCB->blockSignals(true);

        m_pTypeCB->setCurrentIndex(Type);

        m_pTypeCB->blockSignals(false);

        m_TextureFileName = pTexture;

        m_pTextureEdit->setText(QString::number(TextureHash));

        m_pIntensityEdit->setText(QString::number(Intensity));
    }
} // namespace Edit

