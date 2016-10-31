/********************************************************************************
** Form generated from reading UI file 'edit_inspector_bloom.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_BLOOM_H
#define UI_EDIT_INSPECTOR_BLOOM_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorBloom
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLineEdit *m_pIntensityEdit;
    QLabel *label;
    QLabel *label_5;
    QLineEdit *m_pTreshholdEdit;
    QLineEdit *m_pExposureScaleEdit;
    QLineEdit *m_pSizeEdit;
    QHBoxLayout *horizontalLayout;
    QPushButton *m_pTintColorButton;
    QPushButton *m_pTintPickButton;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *InspectorBloom)
    {
        if (InspectorBloom->objectName().isEmpty())
            InspectorBloom->setObjectName(QStringLiteral("InspectorBloom"));
        InspectorBloom->resize(403, 188);
        verticalLayout = new QVBoxLayout(InspectorBloom);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorBloom);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        m_pIntensityEdit = new QLineEdit(groupBox);
        m_pIntensityEdit->setObjectName(QStringLiteral("m_pIntensityEdit"));

        gridLayout->addWidget(m_pIntensityEdit, 1, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 4, 0, 1, 1);

        m_pTreshholdEdit = new QLineEdit(groupBox);
        m_pTreshholdEdit->setObjectName(QStringLiteral("m_pTreshholdEdit"));

        gridLayout->addWidget(m_pTreshholdEdit, 2, 1, 1, 1);

        m_pExposureScaleEdit = new QLineEdit(groupBox);
        m_pExposureScaleEdit->setObjectName(QStringLiteral("m_pExposureScaleEdit"));

        gridLayout->addWidget(m_pExposureScaleEdit, 3, 1, 1, 1);

        m_pSizeEdit = new QLineEdit(groupBox);
        m_pSizeEdit->setObjectName(QStringLiteral("m_pSizeEdit"));

        gridLayout->addWidget(m_pSizeEdit, 4, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        m_pTintColorButton = new QPushButton(groupBox);
        m_pTintColorButton->setObjectName(QStringLiteral("m_pTintColorButton"));
        QPalette palette;
        QBrush brush(QColor(255, 0, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
        m_pTintColorButton->setPalette(palette);
        m_pTintColorButton->setAutoFillBackground(true);
        m_pTintColorButton->setFlat(true);

        horizontalLayout->addWidget(m_pTintColorButton);

        m_pTintPickButton = new QPushButton(groupBox);
        m_pTintPickButton->setObjectName(QStringLiteral("m_pTintPickButton"));
        m_pTintPickButton->setMinimumSize(QSize(0, 0));
        m_pTintPickButton->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(m_pTintPickButton);


        gridLayout->addLayout(horizontalLayout, 0, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout);


        verticalLayout->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(InspectorBloom);
        QObject::connect(m_pTintColorButton, SIGNAL(clicked()), InspectorBloom, SLOT(pickColorFromDialog()));
        QObject::connect(m_pTintPickButton, SIGNAL(clicked()), InspectorBloom, SLOT(pickColorFromDialog()));
        QObject::connect(m_pIntensityEdit, SIGNAL(textEdited(QString)), InspectorBloom, SLOT(valueChanged()));
        QObject::connect(m_pTreshholdEdit, SIGNAL(textEdited(QString)), InspectorBloom, SLOT(valueChanged()));
        QObject::connect(m_pExposureScaleEdit, SIGNAL(textEdited(QString)), InspectorBloom, SLOT(valueChanged()));
        QObject::connect(m_pSizeEdit, SIGNAL(textEdited(QString)), InspectorBloom, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorBloom);
    } // setupUi

    void retranslateUi(QWidget *InspectorBloom)
    {
        InspectorBloom->setWindowTitle(QApplication::translate("InspectorBloom", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorBloom", "Bloom", 0));
        label_2->setText(QApplication::translate("InspectorBloom", "Intensity", 0));
        label_3->setText(QApplication::translate("InspectorBloom", "Treshhold", 0));
        label_4->setText(QApplication::translate("InspectorBloom", "Exposure Scale", 0));
        label->setText(QApplication::translate("InspectorBloom", "Tint", 0));
        label_5->setText(QApplication::translate("InspectorBloom", "Size", 0));
        m_pTintColorButton->setText(QString());
        m_pTintPickButton->setText(QApplication::translate("InspectorBloom", "Pick", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorBloom: public Ui_InspectorBloom {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_BLOOM_H
