/********************************************************************************
** Form generated from reading UI file 'edit_newscenedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_NEWSCENEDIALOG_H
#define UI_EDIT_NEWSCENEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_CNewSceneDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *m_pEditY;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *m_pEditX;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *m_pOkButton;
    QPushButton *m_pCancelButton;

    void setupUi(QDialog *CNewSceneDialog)
    {
        if (CNewSceneDialog->objectName().isEmpty())
            CNewSceneDialog->setObjectName(QStringLiteral("CNewSceneDialog"));
        CNewSceneDialog->setWindowModality(Qt::ApplicationModal);
        CNewSceneDialog->resize(232, 107);
        CNewSceneDialog->setContextMenuPolicy(Qt::PreventContextMenu);
        CNewSceneDialog->setModal(true);
        verticalLayout_2 = new QVBoxLayout(CNewSceneDialog);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label_2 = new QLabel(CNewSceneDialog);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_2->addWidget(label_2);

        m_pEditY = new QLineEdit(CNewSceneDialog);
        m_pEditY->setObjectName(QStringLiteral("m_pEditY"));

        horizontalLayout_2->addWidget(m_pEditY);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(CNewSceneDialog);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        m_pEditX = new QLineEdit(CNewSceneDialog);
        m_pEditX->setObjectName(QStringLiteral("m_pEditX"));

        horizontalLayout->addWidget(m_pEditX);


        verticalLayout_2->addLayout(horizontalLayout);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        m_pOkButton = new QPushButton(CNewSceneDialog);
        m_pOkButton->setObjectName(QStringLiteral("m_pOkButton"));

        hboxLayout->addWidget(m_pOkButton);

        m_pCancelButton = new QPushButton(CNewSceneDialog);
        m_pCancelButton->setObjectName(QStringLiteral("m_pCancelButton"));

        hboxLayout->addWidget(m_pCancelButton);


        verticalLayout_2->addLayout(hboxLayout);


        retranslateUi(CNewSceneDialog);
        QObject::connect(m_pOkButton, SIGNAL(clicked()), CNewSceneDialog, SLOT(pressOkay()));
        QObject::connect(m_pCancelButton, SIGNAL(clicked()), CNewSceneDialog, SLOT(pressCancel()));
        QObject::connect(m_pOkButton, SIGNAL(clicked()), CNewSceneDialog, SLOT(accept()));
        QObject::connect(m_pCancelButton, SIGNAL(clicked()), CNewSceneDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(CNewSceneDialog);
    } // setupUi

    void retranslateUi(QDialog *CNewSceneDialog)
    {
        CNewSceneDialog->setWindowTitle(QApplication::translate("CNewSceneDialog", "Create new scene.", 0));
        label_2->setText(QApplication::translate("CNewSceneDialog", "Y", 0));
        m_pEditY->setText(QApplication::translate("CNewSceneDialog", "4", 0));
        m_pEditY->setPlaceholderText(QApplication::translate("CNewSceneDialog", "Number of regions in Y", 0));
        label->setText(QApplication::translate("CNewSceneDialog", "X", 0));
        m_pEditX->setText(QApplication::translate("CNewSceneDialog", "4", 0));
        m_pEditX->setPlaceholderText(QApplication::translate("CNewSceneDialog", "Number of regions in X", 0));
        m_pOkButton->setText(QApplication::translate("CNewSceneDialog", "OK", 0));
        m_pCancelButton->setText(QApplication::translate("CNewSceneDialog", "Cancel", 0));
    } // retranslateUi

};

namespace Ui {
    class CNewSceneDialog: public Ui_CNewSceneDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_NEWSCENEDIALOG_H
