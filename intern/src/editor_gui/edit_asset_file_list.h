
#pragma once

#include <QFileSystemModel>
#include <QRegularExpression>
#include <QListView>

namespace Edit
{
    class CAssetFileList : public QListView
    {
        Q_OBJECT

    public:

        CAssetFileList(QWidget* _pParent = Q_NULLPTR);
        ~CAssetFileList();

    public:

        void SetDictionary(const QString& _rCurrentPath);

    private:

        QPoint m_MousePressPosition;

        QRegularExpression m_SupportedModelFiles;
        QRegularExpression m_SupportedMaterialFiles;
        QRegularExpression m_SupportedTextureFiles;

        QFileSystemModel* m_pFileModel;

        QString m_CurrentFile;

    private:

        void mousePressEvent(QMouseEvent* _pEvent);
        void mouseMoveEvent(QMouseEvent* _pEvent);
    };
} // namespace Edit