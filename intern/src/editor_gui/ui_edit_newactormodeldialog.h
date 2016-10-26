/********************************************************************************
** Form generated from reading UI file 'edit_newactormodeldialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_NEWACTORMODELDIALOG_H
#define UI_EDIT_NEWACTORMODELDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CNewActorModelDialog
{
public:
    QWidget *layoutWidget;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *CNewActorModelDialog)
    {
        if (CNewActorModelDialog->objectName().isEmpty())
            CNewActorModelDialog->setObjectName(QStringLiteral("CNewActorModelDialog"));
        CNewActorModelDialog->setWindowModality(Qt::ApplicationModal);
        CNewActorModelDialog->resize(400, 300);
        CNewActorModelDialog->setModal(true);
        layoutWidget = new QWidget(CNewActorModelDialog);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(20, 250, 351, 33));
        hboxLayout = new QHBoxLayout(layoutWidget);
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        okButton = new QPushButton(layoutWidget);
        okButton->setObjectName(QStringLiteral("okButton"));

        hboxLayout->addWidget(okButton);

        cancelButton = new QPushButton(layoutWidget);
        cancelButton->setObjectName(QStringLiteral("cancelButton"));

        hboxLayout->addWidget(cancelButton);


        retranslateUi(CNewActorModelDialog);
        QObject::connect(okButton, SIGNAL(clicked()), CNewActorModelDialog, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), CNewActorModelDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(CNewActorModelDialog);
    } // setupUi

    void retranslateUi(QDialog *CNewActorModelDialog)
    {
        CNewActorModelDialog->setWindowTitle(QApplication::translate("CNewActorModelDialog", "Add new model to scene.", 0));
        okButton->setText(QApplication::translate("CNewActorModelDialog", "OK", 0));
        cancelButton->setText(QApplication::translate("CNewActorModelDialog", "Cancel", 0));
    } // retranslateUi

};

namespace Ui {
    class CNewActorModelDialog: public Ui_CNewActorModelDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_NEWACTORMODELDIALOG_H
