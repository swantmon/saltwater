/********************************************************************************
** Form generated from reading UI file 'edit_newscenedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
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
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CNewSceneDialog
{
public:
    QWidget *layoutWidget;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *m_pOkButton;
    QPushButton *m_pCancelButton;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QPlainTextEdit *m_pEditX;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QPlainTextEdit *m_pEditY;
    QSpacerItem *verticalSpacer_2;

    void setupUi(QDialog *CNewSceneDialog)
    {
        if (CNewSceneDialog->objectName().isEmpty())
            CNewSceneDialog->setObjectName(QStringLiteral("CNewSceneDialog"));
        CNewSceneDialog->resize(271, 134);
        CNewSceneDialog->setContextMenuPolicy(Qt::PreventContextMenu);
        layoutWidget = new QWidget(CNewSceneDialog);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 92, 251, 33));
        hboxLayout = new QHBoxLayout(layoutWidget);
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        m_pOkButton = new QPushButton(layoutWidget);
        m_pOkButton->setObjectName(QStringLiteral("m_pOkButton"));

        hboxLayout->addWidget(m_pOkButton);

        m_pCancelButton = new QPushButton(layoutWidget);
        m_pCancelButton->setObjectName(QStringLiteral("m_pCancelButton"));

        hboxLayout->addWidget(m_pCancelButton);

        verticalLayoutWidget = new QWidget(CNewSceneDialog);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 10, 251, 71));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(verticalLayoutWidget);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        m_pEditX = new QPlainTextEdit(verticalLayoutWidget);
        m_pEditX->setObjectName(QStringLiteral("m_pEditX"));

        horizontalLayout->addWidget(m_pEditX);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label_2 = new QLabel(verticalLayoutWidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_2->addWidget(label_2);

        m_pEditY = new QPlainTextEdit(verticalLayoutWidget);
        m_pEditY->setObjectName(QStringLiteral("m_pEditY"));

        horizontalLayout_2->addWidget(m_pEditY);


        verticalLayout->addLayout(horizontalLayout_2);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);


        retranslateUi(CNewSceneDialog);
        QObject::connect(m_pOkButton, SIGNAL(clicked()), CNewSceneDialog, SLOT(pressOkay()));
        QObject::connect(m_pCancelButton, SIGNAL(clicked()), CNewSceneDialog, SLOT(pressCancel()));
        QObject::connect(m_pOkButton, SIGNAL(clicked()), CNewSceneDialog, SLOT(accept()));
        QObject::connect(m_pCancelButton, SIGNAL(clicked()), CNewSceneDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(CNewSceneDialog);
    } // setupUi

    void retranslateUi(QDialog *CNewSceneDialog)
    {
        CNewSceneDialog->setWindowTitle(QApplication::translate("CNewSceneDialog", "Dialog", 0));
        m_pOkButton->setText(QApplication::translate("CNewSceneDialog", "OK", 0));
        m_pCancelButton->setText(QApplication::translate("CNewSceneDialog", "Cancel", 0));
        label->setText(QApplication::translate("CNewSceneDialog", "X", 0));
        label_2->setText(QApplication::translate("CNewSceneDialog", "Y", 0));
    } // retranslateUi

};

namespace Ui {
    class CNewSceneDialog: public Ui_CNewSceneDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_NEWSCENEDIALOG_H
