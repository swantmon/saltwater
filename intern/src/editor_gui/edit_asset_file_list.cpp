
#include "edit_asset_file_list.h"

#include <QApplication>
#include <QDrag>
#include <QFileInfo>
#include <QListWidgetItem>
#include <QMimeData>
#include <QMouseEvent>

namespace Edit
{
    CAssetFileList::CAssetFileList(QWidget* _pParent)
        : QListView               (_pParent)
        , m_MousePressPosition    ()
        , m_SupportedTextureFiles ("(dds)|(hdr)|(jpg)|(jpeg)|(png)|(tga)", QRegularExpression::CaseInsensitiveOption)
        , m_SupportedMaterialFiles(QRegularExpression("(mat)", QRegularExpression::CaseInsensitiveOption))
        , m_SupportedModelFiles   (QRegularExpression("(dae)|(obj)", QRegularExpression::CaseInsensitiveOption))
        , m_pFileModel            (0)
        , m_CurrentFile           ()
    {
        m_pFileModel = new QFileSystemModel(this);

        setModel(m_pFileModel);
    }

    // -----------------------------------------------------------------------------

    CAssetFileList::~CAssetFileList() 
    {
        delete m_pFileModel;

        m_pFileModel = 0;
    }

    // -----------------------------------------------------------------------------

    void CAssetFileList::SetDictionary(const QString& _rCurrentPath)
    {
        m_pFileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);

        m_pFileModel->setRootPath(_rCurrentPath);

        setRootIndex(m_pFileModel->index(_rCurrentPath));
    }

    // -----------------------------------------------------------------------------

    void CAssetFileList::mousePressEvent(QMouseEvent* _pEvent)
    {
        if (_pEvent->button() == Qt::LeftButton)
        { 
            m_MousePressPosition = _pEvent->pos();

            QModelIndex Index = indexAt(m_MousePressPosition);

            m_CurrentFile = m_pFileModel->fileInfo(Index).absoluteFilePath();
        }
    }

    // -----------------------------------------------------------------------------

    void CAssetFileList::mouseMoveEvent(QMouseEvent* _pEvent)
    {
        if (!(_pEvent->buttons() & Qt::LeftButton)) return;

        if ((_pEvent->pos() - m_MousePressPosition).manhattanLength() < QApplication::startDragDistance()) return;

        QDrag* pDrag = new QDrag(this);
        QMimeData* pMimeData = new QMimeData();

        // -----------------------------------------------------------------------------
        // Set data
        // -----------------------------------------------------------------------------
        QFileInfo FileInfo(m_CurrentFile);
        QDir      Directory("../assets/");

        QString AbsPath = FileInfo.absoluteFilePath();

        QString RelPath = Directory.relativeFilePath(AbsPath);

        if (m_SupportedTextureFiles.match(FileInfo.completeSuffix()).hasMatch())
        {
            pMimeData->setData("SW_TEXTURE_ABS_PATH", AbsPath.toLatin1());

            pMimeData->setData("SW_TEXTURE_REL_PATH", RelPath.toLatin1());
        }
        else if (m_SupportedMaterialFiles.match(FileInfo.completeSuffix()).hasMatch())
        {
            pMimeData->setData("SW_MATERIAL_ABS_PATH", AbsPath.toLatin1());

            pMimeData->setData("SW_MATERIAL_REL_PATH", RelPath.toLatin1());
        }
        else if (m_SupportedModelFiles.match(FileInfo.completeSuffix()).hasMatch())
        {
            pMimeData->setData("SW_MODEL_ABS_PATH", AbsPath.toLatin1());

            pMimeData->setData("SW_MODEL_REL_PATH", RelPath.toLatin1());
        }

        // -----------------------------------------------------------------------------
        // Apply data
        // -----------------------------------------------------------------------------
        pDrag->setMimeData(pMimeData);

        // -----------------------------------------------------------------------------
        // Execute drag action
        // -----------------------------------------------------------------------------
        Qt::DropAction pDropAction = pDrag->exec(Qt::CopyAction | Qt::MoveAction);
    }
} // namespace Edit