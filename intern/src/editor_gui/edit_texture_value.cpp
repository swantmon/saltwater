
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
#include <QResizeEvent>
#include <QUrl>

namespace Edit
{
    CTextureValue::CTextureValue(QWidget* _pParent)
        : QWidget             (_pParent)
        , m_CurrentTextureFile()
        , m_CurrentTextureHash()
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

    void CTextureValue::SetTextureFile(const QString& _rTextureFile)
    {
        m_CurrentTextureFile = _rTextureFile;

        m_pFileEdit->setText(m_CurrentTextureFile);
    }

    // -----------------------------------------------------------------------------

    const QString& CTextureValue::GetTextureFile()
    {
        return m_CurrentTextureFile;
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::SetTextureHash(unsigned int _Hash)
    {
        m_CurrentTextureHash = _Hash;

        m_pHashEdit->setText(QString::number(m_CurrentTextureHash));
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureValue::GetTextureHash()
    {
        return m_CurrentTextureHash;
    }

    // -----------------------------------------------------------------------------

    void CTextureValue::hashValueChanged()
    {
        m_CurrentTextureHash = m_pHashEdit->text().toInt();

        m_CurrentTextureFile = "";

        m_pFileEdit->setText(m_CurrentTextureFile);

        emit hashChanged(m_CurrentTextureHash);
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

        if (FileInfo.exists() && (FileInfo.completeSuffix() == "dds" || FileInfo.completeSuffix() == "hdr"))
        {
            m_CurrentTextureFile = NewTextureFile;

            // -----------------------------------------------------------------------------
            // Create hash
            // TODO: Hash should be requested by a message to editor
            // -----------------------------------------------------------------------------
            QByteArray NewTextureBinary = m_CurrentTextureFile.toLatin1();

            const char*  pHashIdentifier = NewTextureBinary.data();
            unsigned int NumberOfBytes;
            unsigned int Hash;

            const void* pData;

            NumberOfBytes = static_cast<unsigned int>(strlen(pHashIdentifier) * sizeof(char));
            pData         = static_cast<const void*>(pHashIdentifier);

            m_CurrentTextureHash = Base::CRC32(pData, NumberOfBytes);

            // -----------------------------------------------------------------------------
            // Set UI
            // -----------------------------------------------------------------------------
            m_pHashEdit->setText(QString::number(m_CurrentTextureHash));

            // -----------------------------------------------------------------------------
            // Emit info
            // -----------------------------------------------------------------------------
            emit fileChanged(m_CurrentTextureFile);
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

            if (FileInfo.completeSuffix() == "dds" || FileInfo.completeSuffix() == "hdr")
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

            if (FileInfo.completeSuffix() == "dds" || FileInfo.completeSuffix() == "hdr")
            {
                // -----------------------------------------------------------------------------
                // Create path
                // -----------------------------------------------------------------------------
                QDir Directory("../assets/");

                QString AbsPath = FileInfo.absoluteFilePath();

                m_CurrentTextureFile = Directory.relativeFilePath(AbsPath);

                // -----------------------------------------------------------------------------
                // Create hash
                // TODO: Hash should be requested by a message to editor
                // -----------------------------------------------------------------------------
                QByteArray NewTextureBinary = m_CurrentTextureFile.toLatin1();

                const char*  pHashIdentifier = NewTextureBinary.data();
                unsigned int NumberOfBytes;
                unsigned int Hash;

                const void* pData;

                NumberOfBytes = static_cast<unsigned int>(strlen(pHashIdentifier) * sizeof(char));
                pData = static_cast<const void*>(pHashIdentifier);

                m_CurrentTextureHash = Base::CRC32(pData, NumberOfBytes);

                // -----------------------------------------------------------------------------
                // Set UI
                // -----------------------------------------------------------------------------
                m_pFileEdit->setText(m_CurrentTextureFile);

                m_pHashEdit->setText(QString::number(m_CurrentTextureHash));

                // -----------------------------------------------------------------------------
                // Emit signals
                // -----------------------------------------------------------------------------
                emit fileChanged(m_CurrentTextureFile);

                emit hashChanged(m_CurrentTextureHash);
            }
        }
    }
} // namespace Edit