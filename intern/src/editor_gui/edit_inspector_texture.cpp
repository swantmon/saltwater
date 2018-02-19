
#include "base/base_crc.h"

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

        NewMessage.Put(m_TextureHash);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Texture_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorTexture::RequestInformation(const QString& _rRelPathToTexture)
    {
        // -----------------------------------------------------------------------------
        // Load texture
        // -----------------------------------------------------------------------------
        Edit::CMessage LoadMessage;

        LoadMessage.Put(std::string(_rRelPathToTexture.toLatin1().data()));

        LoadMessage.Reset();

        int Hash = Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Texture_Load, LoadMessage);

        if (Hash != -1)
        {
            m_TextureHash = static_cast<unsigned int>(Hash);

            // -----------------------------------------------------------------------------
            // Request info of texture
            // -----------------------------------------------------------------------------
            Edit::CMessage RequestMessage;

            RequestMessage.Put(m_TextureHash);

            RequestMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Texture_Info, RequestMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CInspectorTexture::OnTextureInfo(Edit::CMessage& _rMessage)
    {
        unsigned int Hash = _rMessage.Get<int>();

        if (Hash != m_TextureHash) return;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int Dimension = _rMessage.Get<int>();
        int Format    = _rMessage.Get<int>();
        int Semantic  = _rMessage.Get<int>();
        int Binding   = _rMessage.Get<int>();

        bool IsArray = _rMessage.Get<bool>();
        bool IsCube  = _rMessage.Get<bool>();
        bool IsDummy = _rMessage.Get<bool>();

        BASE_UNUSED(IsArray);
        BASE_UNUSED(IsDummy);

        std::string Filename;
        std::string Identifier;

        bool HasFilename = _rMessage.Get<bool>();
        
        if (HasFilename) Filename = _rMessage.Get<std::string>();

        bool HasIdentifier = _rMessage.Get<bool>();

        if (HasIdentifier) Identifier = _rMessage.Get<std::string>();

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
            m_pFilenameEdit->setText(QString(Filename.c_str()));
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