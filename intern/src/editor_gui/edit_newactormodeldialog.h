#pragma once

#include "editor_gui/ui_edit_newactormodeldialog.h"

#include <QDialog>

namespace Edit
{
    class CNewActorModelDialog : public QDialog, public Ui::CNewActorModelDialog
    {
        Q_OBJECT

    public:
        CNewActorModelDialog(QObject* _pParent = Q_NULLPTR);
        ~CNewActorModelDialog();

    private:

    };
} // namespace Edit
