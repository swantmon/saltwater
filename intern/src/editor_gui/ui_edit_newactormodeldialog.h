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
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CNewActorModelDialog
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLineEdit *m_pEditModel;
    QPushButton *m_pLoadModel;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *m_pEditMaterial;
    QPushButton *m_pLoadMaterial;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *m_pOkButton;
    QPushButton *m_pCancelButton;

    void setupUi(QDialog *CNewActorModelDialog)
    {
        if (CNewActorModelDialog->objectName().isEmpty())
            CNewActorModelDialog->setObjectName(QStringLiteral("CNewActorModelDialog"));
        CNewActorModelDialog->setWindowModality(Qt::ApplicationModal);
        CNewActorModelDialog->resize(353, 171);
        CNewActorModelDialog->setModal(true);
        verticalLayoutWidget = new QWidget(CNewActorModelDialog);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 10, 331, 151));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        m_pEditModel = new QLineEdit(verticalLayoutWidget);
        m_pEditModel->setObjectName(QStringLiteral("m_pEditModel"));
        m_pEditModel->setReadOnly(true);

        horizontalLayout->addWidget(m_pEditModel);

        m_pLoadModel = new QPushButton(verticalLayoutWidget);
        m_pLoadModel->setObjectName(QStringLiteral("m_pLoadModel"));

        horizontalLayout->addWidget(m_pLoadModel);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        m_pEditMaterial = new QLineEdit(verticalLayoutWidget);
        m_pEditMaterial->setObjectName(QStringLiteral("m_pEditMaterial"));
        m_pEditMaterial->setReadOnly(true);

        horizontalLayout_2->addWidget(m_pEditMaterial);

        m_pLoadMaterial = new QPushButton(verticalLayoutWidget);
        m_pLoadMaterial->setObjectName(QStringLiteral("m_pLoadMaterial"));

        horizontalLayout_2->addWidget(m_pLoadMaterial);


        verticalLayout->addLayout(horizontalLayout_2);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        m_pOkButton = new QPushButton(verticalLayoutWidget);
        m_pOkButton->setObjectName(QStringLiteral("m_pOkButton"));

        hboxLayout->addWidget(m_pOkButton);

        m_pCancelButton = new QPushButton(verticalLayoutWidget);
        m_pCancelButton->setObjectName(QStringLiteral("m_pCancelButton"));

        hboxLayout->addWidget(m_pCancelButton);


        verticalLayout->addLayout(hboxLayout);


        retranslateUi(CNewActorModelDialog);
        QObject::connect(m_pOkButton, SIGNAL(clicked()), CNewActorModelDialog, SLOT(accept()));
        QObject::connect(m_pCancelButton, SIGNAL(clicked()), CNewActorModelDialog, SLOT(reject()));
        QObject::connect(m_pOkButton, SIGNAL(clicked()), CNewActorModelDialog, SLOT(pressOkay()));
        QObject::connect(m_pCancelButton, SIGNAL(clicked()), CNewActorModelDialog, SLOT(pressCancel()));
        QObject::connect(m_pLoadModel, SIGNAL(clicked()), CNewActorModelDialog, SLOT(openFileDialogModel()));
        QObject::connect(m_pLoadMaterial, SIGNAL(clicked()), CNewActorModelDialog, SLOT(openFileDialogMaterial()));

        QMetaObject::connectSlotsByName(CNewActorModelDialog);
    } // setupUi

    void retranslateUi(QDialog *CNewActorModelDialog)
    {
        CNewActorModelDialog->setWindowTitle(QApplication::translate("CNewActorModelDialog", "Add new model to scene.", 0));
        m_pEditModel->setPlaceholderText(QApplication::translate("CNewActorModelDialog", "Model", 0));
        m_pLoadModel->setText(QApplication::translate("CNewActorModelDialog", "Load", 0));
        m_pEditMaterial->setPlaceholderText(QApplication::translate("CNewActorModelDialog", "Material", 0));
        m_pLoadMaterial->setText(QApplication::translate("CNewActorModelDialog", "Load", 0));
        m_pOkButton->setText(QApplication::translate("CNewActorModelDialog", "OK", 0));
        m_pCancelButton->setText(QApplication::translate("CNewActorModelDialog", "Cancel", 0));
    } // retranslateUi

};

namespace Ui {
    class CNewActorModelDialog: public Ui_CNewActorModelDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_NEWACTORMODELDIALOG_H
