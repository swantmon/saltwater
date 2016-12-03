/********************************************************************************
** Form generated from reading UI file 'edit_inspector_fxaa.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_FXAA_H
#define UI_EDIT_INSPECTOR_FXAA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorFXAA
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QPushButton *m_pLumaColorButton;
    QPushButton *m_pLumaPickButton;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *InspectorFXAA)
    {
        if (InspectorFXAA->objectName().isEmpty())
            InspectorFXAA->setObjectName(QStringLiteral("InspectorFXAA"));
        InspectorFXAA->resize(376, 84);
        verticalLayout = new QVBoxLayout(InspectorFXAA);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorFXAA);
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
        m_pLumaColorButton = new QPushButton(groupBox);
        m_pLumaColorButton->setObjectName(QStringLiteral("m_pLumaColorButton"));
        QPalette palette;
        QBrush brush(QColor(255, 0, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
        m_pLumaColorButton->setPalette(palette);
        m_pLumaColorButton->setAutoFillBackground(true);
        m_pLumaColorButton->setFlat(true);

        horizontalLayout->addWidget(m_pLumaColorButton);

        m_pLumaPickButton = new QPushButton(groupBox);
        m_pLumaPickButton->setObjectName(QStringLiteral("m_pLumaPickButton"));
        m_pLumaPickButton->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(m_pLumaPickButton);


        gridLayout->addLayout(horizontalLayout, 0, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout);


        verticalLayout->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(InspectorFXAA);
        QObject::connect(m_pLumaColorButton, SIGNAL(clicked()), InspectorFXAA, SLOT(pickColorFromDialog()));
        QObject::connect(m_pLumaPickButton, SIGNAL(clicked()), InspectorFXAA, SLOT(pickColorFromDialog()));

        QMetaObject::connectSlotsByName(InspectorFXAA);
    } // setupUi

    void retranslateUi(QWidget *InspectorFXAA)
    {
        InspectorFXAA->setWindowTitle(QApplication::translate("InspectorFXAA", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorFXAA", "FXAA", 0));
        label->setText(QApplication::translate("InspectorFXAA", "Luma", 0));
        m_pLumaColorButton->setText(QString());
        m_pLumaPickButton->setText(QApplication::translate("InspectorFXAA", "Pick", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorFXAA: public Ui_InspectorFXAA {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_FXAA_H
