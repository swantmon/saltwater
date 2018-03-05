
#include "base/base_crc.h"

#include "editor_gui/edit_texture_value.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMimeData>
#include <QUrl>

#include <assert.h>

namespace Edit
{
    CTextureValue::CTextureValue(QWidget* _pParent)
        : QWidget(_pParent)
        , m_File ("")
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Signal / slots
        // -----------------------------------------------------------------------------
        connect(m_pFileEdit, SIGNAL(textEdited(QString)), SLOT(fileValueChanged()));
    }

    // -----------------------------------------------------------------------------

    CTextureValue::~CTextureValue()
    {

    }

    // -----------------------------------------------------------------------------

    void CTextureValue::SetLayout(unsigned int _Layout)
    {
        // -----------------------------------------------------------------------------
        // Reset default style
        // -----------------------------------------------------------------------------
        m_pPreviewGV->setVisible(true);
        m_pFileLabel->setVisible(true);
        m_pFileEdit ->setVisible(true);

        // -----------------------------------------------------------------------------
        // Set style
        // -----------------------------------------------------------------------------
        if ((_Layout & ELayout::NoPreview) != 0)
        {
            m_pPreviewGV->setVisible(false);
        }

        if ((_Layout & ELayout::NoFile) != 0)
        {
            m_pFileLabel->setVisible(false);
            m_pFileEdit ->setVisible(false);
        }
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::SetTextureFile(const QString& _rTextureFile)
    {
        m_File = _rTextureFile;

        m_pFileEdit->setText(m_File);
    }

    // -----------------------------------------------------------------------------

    const QString& CTextureValue::GetTextureFile() const
    {
        return m_File;
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::fileValueChanged()
    {
        // -----------------------------------------------------------------------------
        // Get texture path from assets
        // -----------------------------------------------------------------------------
        QString NewTextureFile = m_pFileEdit->text();

        LoadTexture(NewTextureFile);
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::dragEnterEvent(QDragEnterEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        if (pMimeData->hasFormat("SW_TEXTURE_REL_PATH") == false) return;

        _pEvent->acceptProposedAction();
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::dropEvent(QDropEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        assert(pMimeData->hasFormat("SW_TEXTURE_REL_PATH"));

        QString RelativePathToFile = pMimeData->data("SW_TEXTURE_REL_PATH");

        LoadTexture(RelativePathToFile);
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::OnTextureInfo(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::LoadTexture(const QString& _rPathToTexture)
    {
        m_File = _rPathToTexture;

        m_pFileEdit->setText(m_File);

        // -----------------------------------------------------------------------------
        // Emit info
        // -----------------------------------------------------------------------------
        emit fileChanged(m_File);
    }
} // namespace Edit