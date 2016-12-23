
#pragma once

#include <QFileSystemModel>
#include <QModelIndex>
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

    Q_SIGNALS:

        void modelClicked(const QString& _rRelPath);
        void materialClicked(const QString& _rRelPath);
        void textureClicked(const QString& _rRelPath);

    public Q_SLOTS:

        void setDictionary(const QString& _rCurrentPath);

        void fileClicked(const QModelIndex& _rModelIndex);

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