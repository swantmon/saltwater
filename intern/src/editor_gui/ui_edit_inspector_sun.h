/********************************************************************************
** Form generated from reading UI file 'edit_inspector_sun.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_SUN_H
#define UI_EDIT_INSPECTOR_SUN_H

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
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorSun
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QLabel *label_16;
    QComboBox *m_pShadowRefreshCB;
    QLabel *label;
    QLabel *label_3;
    QComboBox *m_pColorModeCB;
    QLabel *label_8;
    QHBoxLayout *horizontalLayout_4;
    QLineEdit *m_pIntensityEdit;
    QLabel *label_9;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *m_pTemperatureEdit;
    QLabel *label_10;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_13;
    QLineEdit *m_pDirectionXEdit;
    QLabel *label_12;
    QLineEdit *m_pDirectionYEdit;
    QLabel *label_11;
    QLineEdit *m_pDirectionZEdit;
    QLabel *label_7;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *m_pPickColorButton;
    QPushButton *m_pPickColorButton2;

    void setupUi(QWidget *InspectorSun)
    {
        if (InspectorSun->objectName().isEmpty())
            InspectorSun->setObjectName(QStringLiteral("InspectorSun"));
        InspectorSun->resize(335, 212);
        verticalLayout = new QVBoxLayout(InspectorSun);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorSun);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label_16 = new QLabel(groupBox);
        label_16->setObjectName(QStringLiteral("label_16"));

        gridLayout->addWidget(label_16, 6, 0, 1, 1);

        m_pShadowRefreshCB = new QComboBox(groupBox);
        m_pShadowRefreshCB->setObjectName(QStringLiteral("m_pShadowRefreshCB"));

        gridLayout->addWidget(m_pShadowRefreshCB, 6, 2, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        m_pColorModeCB = new QComboBox(groupBox);
        m_pColorModeCB->setObjectName(QStringLiteral("m_pColorModeCB"));

        gridLayout->addWidget(m_pColorModeCB, 0, 2, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 4, 0, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        m_pIntensityEdit = new QLineEdit(groupBox);
        m_pIntensityEdit->setObjectName(QStringLiteral("m_pIntensityEdit"));

        horizontalLayout_4->addWidget(m_pIntensityEdit);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QStringLiteral("label_9"));

        horizontalLayout_4->addWidget(label_9);


        gridLayout->addLayout(horizontalLayout_4, 4, 2, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        m_pTemperatureEdit = new QLineEdit(groupBox);
        m_pTemperatureEdit->setObjectName(QStringLiteral("m_pTemperatureEdit"));

        horizontalLayout_2->addWidget(m_pTemperatureEdit);

        label_10 = new QLabel(groupBox);
        label_10->setObjectName(QStringLiteral("label_10"));

        horizontalLayout_2->addWidget(label_10);


        gridLayout->addLayout(horizontalLayout_2, 2, 2, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        label_13 = new QLabel(groupBox);
        label_13->setObjectName(QStringLiteral("label_13"));

        horizontalLayout_3->addWidget(label_13);

        m_pDirectionXEdit = new QLineEdit(groupBox);
        m_pDirectionXEdit->setObjectName(QStringLiteral("m_pDirectionXEdit"));

        horizontalLayout_3->addWidget(m_pDirectionXEdit);

        label_12 = new QLabel(groupBox);
        label_12->setObjectName(QStringLiteral("label_12"));

        horizontalLayout_3->addWidget(label_12);

        m_pDirectionYEdit = new QLineEdit(groupBox);
        m_pDirectionYEdit->setObjectName(QStringLiteral("m_pDirectionYEdit"));

        horizontalLayout_3->addWidget(m_pDirectionYEdit);

        label_11 = new QLabel(groupBox);
        label_11->setObjectName(QStringLiteral("label_11"));

        horizontalLayout_3->addWidget(label_11);

        m_pDirectionZEdit = new QLineEdit(groupBox);
        m_pDirectionZEdit->setObjectName(QStringLiteral("m_pDirectionZEdit"));

        horizontalLayout_3->addWidget(m_pDirectionZEdit);


        gridLayout->addLayout(horizontalLayout_3, 5, 2, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout->addWidget(label_7, 5, 0, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        m_pPickColorButton = new QPushButton(groupBox);
        m_pPickColorButton->setObjectName(QStringLiteral("m_pPickColorButton"));
        QPalette palette;
        QBrush brush(QColor(255, 0, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
        m_pPickColorButton->setPalette(palette);
        m_pPickColorButton->setAutoFillBackground(true);
        m_pPickColorButton->setAutoDefault(false);
        m_pPickColorButton->setFlat(true);

        horizontalLayout_5->addWidget(m_pPickColorButton);

        m_pPickColorButton2 = new QPushButton(groupBox);
        m_pPickColorButton2->setObjectName(QStringLiteral("m_pPickColorButton2"));
        m_pPickColorButton2->setMaximumSize(QSize(30, 23));

        horizontalLayout_5->addWidget(m_pPickColorButton2);


        gridLayout->addLayout(horizontalLayout_5, 1, 2, 1, 1);


        verticalLayout->addWidget(groupBox);


        retranslateUi(InspectorSun);
        QObject::connect(m_pColorModeCB, SIGNAL(currentIndexChanged(int)), InspectorSun, SLOT(valueChanged()));
        QObject::connect(m_pPickColorButton, SIGNAL(clicked()), InspectorSun, SLOT(pickColorFromDialog()));
        QObject::connect(m_pPickColorButton2, SIGNAL(clicked()), InspectorSun, SLOT(pickColorFromDialog()));
        QObject::connect(m_pTemperatureEdit, SIGNAL(textEdited(QString)), InspectorSun, SLOT(valueChanged()));
        QObject::connect(m_pIntensityEdit, SIGNAL(textEdited(QString)), InspectorSun, SLOT(valueChanged()));
        QObject::connect(m_pDirectionXEdit, SIGNAL(textEdited(QString)), InspectorSun, SLOT(valueChanged()));
        QObject::connect(m_pDirectionYEdit, SIGNAL(textEdited(QString)), InspectorSun, SLOT(valueChanged()));
        QObject::connect(m_pDirectionZEdit, SIGNAL(textEdited(QString)), InspectorSun, SLOT(valueChanged()));
        QObject::connect(m_pShadowRefreshCB, SIGNAL(currentIndexChanged(int)), InspectorSun, SLOT(valueChanged()));

        m_pPickColorButton->setDefault(false);


        QMetaObject::connectSlotsByName(InspectorSun);
    } // setupUi

    void retranslateUi(QWidget *InspectorSun)
    {
        InspectorSun->setWindowTitle(QApplication::translate("InspectorSun", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorSun", "Sun", 0));
        label_2->setText(QApplication::translate("InspectorSun", "Color", 0));
        label_16->setText(QApplication::translate("InspectorSun", "Shadow Refresh", 0));
        m_pShadowRefreshCB->clear();
        m_pShadowRefreshCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorSun", "Static", 0)
         << QApplication::translate("InspectorSun", "Dynamic", 0)
        );
        label->setText(QApplication::translate("InspectorSun", "Color Mode:", 0));
        label_3->setText(QApplication::translate("InspectorSun", "Temperature", 0));
        m_pColorModeCB->clear();
        m_pColorModeCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorSun", "Color", 0)
         << QApplication::translate("InspectorSun", "Temperature", 0)
        );
        label_8->setText(QApplication::translate("InspectorSun", "Intensity", 0));
        m_pIntensityEdit->setText(QApplication::translate("InspectorSun", "1200", 0));
        label_9->setText(QApplication::translate("InspectorSun", "Lux", 0));
        m_pTemperatureEdit->setText(QApplication::translate("InspectorSun", "5300", 0));
        label_10->setText(QApplication::translate("InspectorSun", "Kelvin", 0));
        label_13->setText(QApplication::translate("InspectorSun", "X", 0));
        m_pDirectionXEdit->setText(QApplication::translate("InspectorSun", "-1", 0));
        label_12->setText(QApplication::translate("InspectorSun", "Y", 0));
        m_pDirectionYEdit->setText(QApplication::translate("InspectorSun", "-1", 0));
        label_11->setText(QApplication::translate("InspectorSun", "Z", 0));
        m_pDirectionZEdit->setText(QApplication::translate("InspectorSun", "-1", 0));
        label_7->setText(QApplication::translate("InspectorSun", "Direction", 0));
        m_pPickColorButton->setText(QString());
        m_pPickColorButton2->setText(QApplication::translate("InspectorSun", "Pick", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorSun: public Ui_InspectorSun {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_SUN_H
