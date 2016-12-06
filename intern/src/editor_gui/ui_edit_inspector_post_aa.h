/********************************************************************************
** Form generated from reading UI file 'edit_inspector_post_aa.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_POST_AA_H
#define UI_EDIT_INSPECTOR_POST_AA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorPostAA
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QComboBox *m_pTypeCB;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *InspectorPostAA)
    {
        if (InspectorPostAA->objectName().isEmpty())
            InspectorPostAA->setObjectName(QStringLiteral("InspectorPostAA"));
        InspectorPostAA->resize(376, 88);
        verticalLayout = new QVBoxLayout(InspectorPostAA);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorPostAA);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        m_pTypeCB = new QComboBox(groupBox);
        m_pTypeCB->setObjectName(QStringLiteral("m_pTypeCB"));

        horizontalLayout->addWidget(m_pTypeCB);


        gridLayout->addLayout(horizontalLayout, 0, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout);


        verticalLayout->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(InspectorPostAA);
        QObject::connect(m_pTypeCB, SIGNAL(currentIndexChanged(int)), InspectorPostAA, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorPostAA);
    } // setupUi

    void retranslateUi(QWidget *InspectorPostAA)
    {
        InspectorPostAA->setWindowTitle(QApplication::translate("InspectorPostAA", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorPostAA", "AA", 0));
        label->setText(QApplication::translate("InspectorPostAA", "Type", 0));
        m_pTypeCB->clear();
        m_pTypeCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorPostAA", "SMAA", 0)
         << QApplication::translate("InspectorPostAA", "FXAA", 0)
        );
    } // retranslateUi

};

namespace Ui {
    class InspectorPostAA: public Ui_InspectorPostAA {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_POST_AA_H
