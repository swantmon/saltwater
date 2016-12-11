
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

namespace Edit
{
    CTextureValue::CTextureValue(QWidget* _pParent)
        : QWidget         (_pParent)
        , m_SupportedFiles("(dds)|(hdr)|(jpg)|(jpeg)|(png)|(tga)", QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption)
        , m_File          ()
        , m_Hash          ()
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Signal / slots
        // -----------------------------------------------------------------------------
        connect(m_pHashEdit, SIGNAL(textEdited(QString)), SLOT(hashValueChanged()));
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
        m_pHashLabel->setVisible(true);
        m_pHashEdit ->setVisible(true);
        m_pFileLabel->setVisible(true);
        m_pFileEdit ->setVisible(true);

        // -----------------------------------------------------------------------------
        // Set style
        // -----------------------------------------------------------------------------
        if ((_Layout & ELayout::NoPreview) != 0)
        {
            m_pPreviewGV->setVisible(false);
        }

        if ((_Layout & ELayout::NoHash) != 0)
        {
            m_pHashLabel->setVisible(false);
            m_pHashEdit ->setVisible(false);
        }

        if ((_Layout & ELayout::NoFile) != 0)
        {
            m_pFileLabel->setVisible(false);
            m_pFileEdit ->setVisible(false);
        }
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::SetSupportedFiles(const QString& _rSupportedFiles)
    {
        m_SupportedFiles.setPattern(_rSupportedFiles);
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

    void CTextureValue::SetTextureHash(unsigned int _Hash)
    {
        m_Hash = _Hash;

        m_pHashEdit->setText(QString::number(m_Hash));
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureValue::GetTextureHash() const
    {
        return m_Hash;
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::hashValueChanged()
    {
        m_Hash = m_pHashEdit->text().toInt();

        m_File = "";

        m_pFileEdit->setText(m_File);

        emit hashChanged(m_Hash);
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::fileValueChanged()
    {
        // -----------------------------------------------------------------------------
        // Get texture path from assets
        // -----------------------------------------------------------------------------
        QString NewTextureFile = m_pFileEdit->text();

        QDir Directory("../assets/");

        QString AbsolutePathToFile = Directory.absoluteFilePath(NewTextureFile);

        // -----------------------------------------------------------------------------
        // Check if new input is a file and an image
        // -----------------------------------------------------------------------------
        QFileInfo FileInfo(AbsolutePathToFile);

        if (FileInfo.exists())
        {
            if (m_SupportedFiles.match(FileInfo.completeSuffix()).hasMatch())
            {
                m_File = NewTextureFile;

                // -----------------------------------------------------------------------------
                // Create hash
                // TODO: Hash should be requested by a message to editor
                // -----------------------------------------------------------------------------
                QByteArray NewTextureBinary = m_File.toLatin1();

                const char*  pHashIdentifier = NewTextureBinary.data();
                unsigned int NumberOfBytes;
                unsigned int Hash;

                const void* pData;

                NumberOfBytes = static_cast<unsigned int>(strlen(pHashIdentifier) * sizeof(char));
                pData = static_cast<const void*>(pHashIdentifier);

                m_Hash = Base::CRC32(pData, NumberOfBytes);

                // -----------------------------------------------------------------------------
                // Set UI
                // -----------------------------------------------------------------------------
                m_pHashEdit->setText(QString::number(m_Hash));

                // -----------------------------------------------------------------------------
                // Emit info
                // -----------------------------------------------------------------------------
                emit fileChanged(m_File);
            }
        }

        if (NewTextureFile == "")
        {
            m_File = "";
            m_Hash = 0;

            m_pFileEdit->setText(m_File);

            m_pHashEdit->setText(QString::number(m_Hash));

            // -----------------------------------------------------------------------------
            // Emit info
            // -----------------------------------------------------------------------------
            emit fileChanged(m_File);
        }
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::dragEnterEvent(QDragEnterEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        if (pMimeData->hasText())
        {
            QString Text = pMimeData->text();

            QFileInfo FileInfo(Text);

            if (m_SupportedFiles.match(FileInfo.completeSuffix()).hasMatch())
            {
                _pEvent->acceptProposedAction();
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::dropEvent(QDropEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        if (pMimeData->hasUrls())
        {
            QUrl Url = pMimeData->urls()[0];

            QFileInfo FileInfo(Url.toLocalFile());

            if (m_SupportedFiles.match(FileInfo.completeSuffix()).hasMatch())
            {
                // -----------------------------------------------------------------------------
                // Create path
                // -----------------------------------------------------------------------------
                QDir Directory("../assets/");

                QString AbsPath = FileInfo.absoluteFilePath();

                m_File = Directory.relativeFilePath(AbsPath);

                // -----------------------------------------------------------------------------
                // Create hash
                // TODO: Hash should be requested by a message to editor
                // -----------------------------------------------------------------------------
                QByteArray NewTextureBinary = m_File.toLatin1();

                const char*  pHashIdentifier = NewTextureBinary.data();
                unsigned int NumberOfBytes;
                unsigned int Hash;

                const void* pData;

                NumberOfBytes = static_cast<unsigned int>(strlen(pHashIdentifier) * sizeof(char));
                pData = static_cast<const void*>(pHashIdentifier);

                m_Hash = Base::CRC32(pData, NumberOfBytes);

                // -----------------------------------------------------------------------------
                // Set UI
                // -----------------------------------------------------------------------------
                m_pFileEdit->setText(m_File);

                m_pHashEdit->setText(QString::number(m_Hash));

                // -----------------------------------------------------------------------------
                // Emit signals
                // -----------------------------------------------------------------------------
                emit fileChanged(m_File);

                emit hashChanged(m_Hash);
            }
        }
    }
} // namespace Edit