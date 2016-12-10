
#include "base/base_crc.h"

#include "editor_gui/edit_texture_value.h"

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
        setupUi(this);
    }

    // -----------------------------------------------------------------------------

    CTextureValue::~CTextureValue()
    {

    }

    // -----------------------------------------------------------------------------

    void CTextureValue::dragEnterEvent(QDragEnterEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        if (pMimeData->hasText())
        {
            QString Text = pMimeData->text();

            QFileInfo FileInfo(Text);

            if (FileInfo.completeSuffix() == "JPG" || FileInfo.completeSuffix() == "png")
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

            if (FileInfo.completeSuffix() == "JPG" || FileInfo.completeSuffix() == "png")
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
                m_pPathEdit->setText(m_CurrentTextureFile);

                m_pHashEdit->setText(QString::number(m_CurrentTextureHash));
            }
        }
    }
} // namespace Edit