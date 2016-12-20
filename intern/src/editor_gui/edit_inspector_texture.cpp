
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

        m_TextureHash = Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Texture_Load, NewMessage);

        if (m_TextureHash > 0)
        {
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
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
    }
} // namespace Edit