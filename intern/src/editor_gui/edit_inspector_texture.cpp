
#include "base/base_crc.h"
#include "base/base_vector3.h"
#include "base/base_vector4.h"

#include "editor_gui/edit_inspector_Texture.h"
#include "editor_gui/edit_texture_value.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMimeData>
#include <QResizeEvent>
#include <QUrl>

namespace Edit
{
    CInspectorTexture::CInspectorTexture(QWidget* _pParent)
        : QWidget      (_pParent)
        , m_TextureHash(0)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Texture_Info, EDIT_RECEIVE_MESSAGE(&CInspectorTexture::OnTextureInfo));
    }

    // -----------------------------------------------------------------------------

    CInspectorTexture::~CInspectorTexture() 
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorTexture::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Texture_Update, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorTexture::RequestInformation(int _Hash)
    {
        m_TextureHash = _Hash;

        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_TextureHash);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Texture_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorTexture::RequestInformation(const QString& _rRelPathToTexture)
    {
        // -----------------------------------------------------------------------------
        // Load texture
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutString(_rRelPathToTexture.toLatin1().data());

        NewMessage.Reset();

        int Hash = Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Texture_Load, NewMessage);

        if (Hash != -1)
        {
            m_TextureHash = static_cast<unsigned int>(Hash);

            // -----------------------------------------------------------------------------
            // Request info of texture
            // -----------------------------------------------------------------------------
            Edit::CMessage NewMessage;

            NewMessage.PutInt(m_TextureHash);

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Texture_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CInspectorTexture::OnTextureInfo(Edit::CMessage& _rMessage)
    {
        unsigned int Hash = _rMessage.GetInt();

        if (Hash != m_TextureHash) return;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int Dimension = _rMessage.GetInt();
        int Format    = _rMessage.GetInt();
        int Semantic  = _rMessage.GetInt();
        int Binding   = _rMessage.GetInt();

        bool IsArray = _rMessage.GetBool();
        bool IsCube  = _rMessage.GetBool();
        bool IsDummy = _rMessage.GetBool();

        char Filename[256];
        char Identifier[256];

        bool HasFilename = _rMessage.GetBool();
        
        if (HasFilename) _rMessage.GetString(Filename, 256);

        bool HasIdentifier = _rMessage.GetBool();

        if (HasIdentifier) _rMessage.GetString(Identifier, 256);

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pDimensionCB->blockSignals(true);
        m_pFormatCB   ->blockSignals(true);
        m_pSemanticCB ->blockSignals(true);
        m_pBindingCB  ->blockSignals(true);

        if (IsCube)
        {
            m_pDimensionCB->setCurrentIndex(3);
        }
        else
        {
            m_pDimensionCB->setCurrentIndex(Dimension);
        }

        m_pFormatCB   ->setCurrentIndex(Format);
        m_pSemanticCB ->setCurrentIndex(Semantic);
        m_pBindingCB  ->setCurrentIndex(Binding);

        if (HasFilename)
        {
            m_pFilenameEdit->setText(Filename);
        }
        else
        {
            m_pFilenameEdit->setText("");
        }

        m_pHashEdit->setText(QString::number(Hash));

        m_pDimensionCB->blockSignals(false);
        m_pFormatCB   ->blockSignals(false);
        m_pSemanticCB ->blockSignals(false);
        m_pBindingCB  ->blockSignals(false);
    }
} // namespace Edit