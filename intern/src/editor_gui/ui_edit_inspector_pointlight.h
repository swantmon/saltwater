/********************************************************************************
** Form generated from reading UI file 'edit_inspector_pointlight.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_POINTLIGHT_H
#define UI_EDIT_INSPECTOR_POINTLIGHT_H

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

class Ui_InspectorPointlight
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_4;
    QLabel *label_7;
    QComboBox *m_pColorModeCB;
    QLabel *label_3;
    QLabel *label_6;
    QLineEdit *m_pOuterConeAngleEdit;
    QLineEdit *m_pInnerConeAngleEdit;
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
    QComboBox *m_pShadowTypeCB;
    QLabel *label_2;
    QLineEdit *m_pAttenuationRadiusEdit;
    QHBoxLayout *horizontalLayout_4;
    QLineEdit *m_pIntensityEdit;
    QLabel *label_9;
    QLabel *label_15;
    QLabel *label_16;
    QLabel *label;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *m_pPickColorButton;
    QPushButton *m_pPickColorButton2;
    QLabel *label_5;
    QLabel *label_8;
    QComboBox *m_pShadowRefreshCB;
    QComboBox *m_pShadowQualityCB;
    QLabel *label_14;

    void setupUi(QWidget *InspectorPointlight)
    {
        if (InspectorPointlight->objectName().isEmpty())
            InspectorPointlight->setObjectName(QStringLiteral("InspectorPointlight"));
        InspectorPointlight->resize(401, 342);
        verticalLayout = new QVBoxLayout(InspectorPointlight);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorPointlight);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 6, 0, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout->addWidget(label_7, 5, 0, 1, 1);

        m_pColorModeCB = new QComboBox(groupBox);
        m_pColorModeCB->setObjectName(QStringLiteral("m_pColorModeCB"));

        gridLayout->addWidget(m_pColorModeCB, 0, 2, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout->addWidget(label_6, 8, 0, 1, 1);

        m_pOuterConeAngleEdit = new QLineEdit(groupBox);
        m_pOuterConeAngleEdit->setObjectName(QStringLiteral("m_pOuterConeAngleEdit"));

        gridLayout->addWidget(m_pOuterConeAngleEdit, 8, 2, 1, 1);

        m_pInnerConeAngleEdit = new QLineEdit(groupBox);
        m_pInnerConeAngleEdit->setObjectName(QStringLiteral("m_pInnerConeAngleEdit"));

        gridLayout->addWidget(m_pInnerConeAngleEdit, 7, 2, 1, 1);

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

        m_pShadowTypeCB = new QComboBox(groupBox);
        m_pShadowTypeCB->setObjectName(QStringLiteral("m_pShadowTypeCB"));

        gridLayout->addWidget(m_pShadowTypeCB, 9, 2, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        m_pAttenuationRadiusEdit = new QLineEdit(groupBox);
        m_pAttenuationRadiusEdit->setObjectName(QStringLiteral("m_pAttenuationRadiusEdit"));

        gridLayout->addWidget(m_pAttenuationRadiusEdit, 6, 2, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        m_pIntensityEdit = new QLineEdit(groupBox);
        m_pIntensityEdit->setObjectName(QStringLiteral("m_pIntensityEdit"));

        horizontalLayout_4->addWidget(m_pIntensityEdit);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QStringLiteral("label_9"));

        horizontalLayout_4->addWidget(label_9);


        gridLayout->addLayout(horizontalLayout_4, 4, 2, 1, 1);

        label_15 = new QLabel(groupBox);
        label_15->setObjectName(QStringLiteral("label_15"));

        gridLayout->addWidget(label_15, 10, 0, 1, 1);

        label_16 = new QLabel(groupBox);
        label_16->setObjectName(QStringLiteral("label_16"));

        gridLayout->addWidget(label_16, 11, 0, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        m_pPickColorButton = new QPushButton(groupBox);
        m_pPickColorButton->setObjectName(QStringLiteral("m_pPickColorButton"));
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
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

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 7, 0, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 4, 0, 1, 1);

        m_pShadowRefreshCB = new QComboBox(groupBox);
        m_pShadowRefreshCB->setObjectName(QStringLiteral("m_pShadowRefreshCB"));

        gridLayout->addWidget(m_pShadowRefreshCB, 11, 2, 1, 1);

        m_pShadowQualityCB = new QComboBox(groupBox);
        m_pShadowQualityCB->setObjectName(QStringLiteral("m_pShadowQualityCB"));

        gridLayout->addWidget(m_pShadowQualityCB, 10, 2, 1, 1);

        label_14 = new QLabel(groupBox);
        label_14->setObjectName(QStringLiteral("label_14"));

        gridLayout->addWidget(label_14, 9, 0, 1, 1);


        verticalLayout->addWidget(groupBox);


        retranslateUi(InspectorPointlight);
        QObject::connect(m_pPickColorButton, SIGNAL(clicked()), InspectorPointlight, SLOT(pickColorFromDialog()));
        QObject::connect(m_pPickColorButton2, SIGNAL(clicked()), InspectorPointlight, SLOT(pickColorFromDialog()));
        QObject::connect(m_pColorModeCB, SIGNAL(currentIndexChanged(int)), InspectorPointlight, SLOT(valueChanged()));
        QObject::connect(m_pTemperatureEdit, SIGNAL(textEdited(QString)), InspectorPointlight, SLOT(valueChanged()));
        QObject::connect(m_pIntensityEdit, SIGNAL(textEdited(QString)), InspectorPointlight, SLOT(valueChanged()));
        QObject::connect(m_pDirectionXEdit, SIGNAL(textEdited(QString)), InspectorPointlight, SLOT(valueChanged()));
        QObject::connect(m_pDirectionYEdit, SIGNAL(textEdited(QString)), InspectorPointlight, SLOT(valueChanged()));
        QObject::connect(m_pDirectionZEdit, SIGNAL(textEdited(QString)), InspectorPointlight, SLOT(valueChanged()));
        QObject::connect(m_pAttenuationRadiusEdit, SIGNAL(textEdited(QString)), InspectorPointlight, SLOT(valueChanged()));
        QObject::connect(m_pInnerConeAngleEdit, SIGNAL(textEdited(QString)), InspectorPointlight, SLOT(valueChanged()));
        QObject::connect(m_pOuterConeAngleEdit, SIGNAL(textEdited(QString)), InspectorPointlight, SLOT(valueChanged()));
        QObject::connect(m_pShadowTypeCB, SIGNAL(currentIndexChanged(int)), InspectorPointlight, SLOT(valueChanged()));
        QObject::connect(m_pShadowQualityCB, SIGNAL(currentIndexChanged(int)), InspectorPointlight, SLOT(valueChanged()));
        QObject::connect(m_pShadowRefreshCB, SIGNAL(currentIndexChanged(int)), InspectorPointlight, SLOT(valueChanged()));

        m_pPickColorButton->setDefault(false);


        QMetaObject::connectSlotsByName(InspectorPointlight);
    } // setupUi

    void retranslateUi(QWidget *InspectorPointlight)
    {
        InspectorPointlight->setWindowTitle(QApplication::translate("InspectorPointlight", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorPointlight", "Pointlight", 0));
        label_4->setText(QApplication::translate("InspectorPointlight", "Attenuation Radius", 0));
        label_7->setText(QApplication::translate("InspectorPointlight", "Direction", 0));
        m_pColorModeCB->clear();
        m_pColorModeCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorPointlight", "Color", 0)
         << QApplication::translate("InspectorPointlight", "Temperature", 0)
        );
        label_3->setText(QApplication::translate("InspectorPointlight", "Temperature", 0));
        label_6->setText(QApplication::translate("InspectorPointlight", "Outer Cone Angle", 0));
        m_pOuterConeAngleEdit->setText(QApplication::translate("InspectorPointlight", "90", 0));
        m_pInnerConeAngleEdit->setText(QApplication::translate("InspectorPointlight", "45", 0));
        m_pTemperatureEdit->setText(QApplication::translate("InspectorPointlight", "5300", 0));
        label_10->setText(QApplication::translate("InspectorPointlight", "Kelvin", 0));
        label_13->setText(QApplication::translate("InspectorPointlight", "X", 0));
        m_pDirectionXEdit->setText(QApplication::translate("InspectorPointlight", "-1", 0));
        label_12->setText(QApplication::translate("InspectorPointlight", "Y", 0));
        m_pDirectionYEdit->setText(QApplication::translate("InspectorPointlight", "-1", 0));
        label_11->setText(QApplication::translate("InspectorPointlight", "Z", 0));
        m_pDirectionZEdit->setText(QApplication::translate("InspectorPointlight", "-1", 0));
        m_pShadowTypeCB->clear();
        m_pShadowTypeCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorPointlight", "No Shadows", 0)
         << QApplication::translate("InspectorPointlight", "Hard Shadows", 0)
         << QApplication::translate("InspectorPointlight", "GlobalIllumination", 0)
        );
        label_2->setText(QApplication::translate("InspectorPointlight", "Color", 0));
        m_pAttenuationRadiusEdit->setText(QApplication::translate("InspectorPointlight", "10", 0));
        m_pIntensityEdit->setText(QApplication::translate("InspectorPointlight", "1200", 0));
        label_9->setText(QApplication::translate("InspectorPointlight", "Lux", 0));
        label_15->setText(QApplication::translate("InspectorPointlight", "Shadow Quality", 0));
        label_16->setText(QApplication::translate("InspectorPointlight", "Shadow Refresh", 0));
        label->setText(QApplication::translate("InspectorPointlight", "Color Mode:", 0));
        m_pPickColorButton->setText(QString());
        m_pPickColorButton2->setText(QApplication::translate("InspectorPointlight", "Pick", 0));
        label_5->setText(QApplication::translate("InspectorPointlight", "Inner Cone Angle", 0));
        label_8->setText(QApplication::translate("InspectorPointlight", "Intensity", 0));
        m_pShadowRefreshCB->clear();
        m_pShadowRefreshCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorPointlight", "Static", 0)
         << QApplication::translate("InspectorPointlight", "Dynamic", 0)
        );
        m_pShadowQualityCB->clear();
        m_pShadowQualityCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorPointlight", "Low", 0)
         << QApplication::translate("InspectorPointlight", "Medium", 0)
         << QApplication::translate("InspectorPointlight", "High", 0)
         << QApplication::translate("InspectorPointlight", "Very High", 0)
        );
        label_14->setText(QApplication::translate("InspectorPointlight", "Shadow Type", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorPointlight: public Ui_InspectorPointlight {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_POINTLIGHT_H
