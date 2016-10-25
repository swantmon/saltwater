#pragma once

#include "editor_gui/ui_edit_newscenedialog.h"

#include <QDialog>

namespace Edit
{
    class CNewSceneDialog : public QDialog, public Ui::CNewSceneDialog
    {
        Q_OBJECT

    public:

        CNewSceneDialog(QObject* _pParent = Q_NULLPTR);
        ~CNewSceneDialog();

    public Q_SLOTS:

        void pressOkay();
        void pressCancel();
    };
} // namespace Edit