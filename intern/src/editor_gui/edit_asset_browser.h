
#pragma once

#include "editor_gui/ui_edit_asset_browser.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <QFileSystemModel>
#include <QWidget>

namespace Edit
{
    class CAssetBrowser : public QWidget, public Ui::AssetBrowser
    {
        Q_OBJECT

    public:

        CAssetBrowser(QWidget * parent = Q_NULLPTR);
        ~CAssetBrowser();

    public:

        void Reset();

    public Q_SLOTS:

        void directoryClicked(const QModelIndex& _rModelIndex);

    private:

        QFileSystemModel* m_pDirectoryModel;

        QString m_RootPath;
        QString m_CurrentPath;
    };
} // namespace Edit