/********************************************************************************
** Form generated from reading UI file 'edit_inspector_volumefog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_VOLUMEFOG_H
#define UI_EDIT_INSPECTOR_VOLUMEFOG_H

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

class Ui_InspectorVolumeFog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_11;
    QLineEdit *m_pWindDirectionXEdit;
    QLabel *label_10;
    QLineEdit *m_pWindDirectionYEdit;
    QLabel *label_9;
    QLineEdit *m_pWindDirectionZEdit;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_8;
    QLabel *label;
    QLineEdit *m_pDensityAttenuationEdit;
    QLineEdit *m_pDensityLevelEdit;
    QLineEdit *m_pAbsorptionCoeffEdit;
    QLineEdit *m_pScatteringCoeffEdit;
    QLineEdit *m_pShadowIntensityEdit;
    QHBoxLayout *horizontalLayout;
    QPushButton *m_pFogColorButton;
    QPushButton *m_pFogPickButton;
    QLineEdit *m_pFrustumDepthEdit;
    QLabel *label_6;
    QLabel *label_5;
    QLabel *label_7;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *InspectorVolumeFog)
    {
        if (InspectorVolumeFog->objectName().isEmpty())
            InspectorVolumeFog->setObjectName(QStringLiteral("InspectorVolumeFog"));
        InspectorVolumeFog->resize(400, 268);
        verticalLayout = new QVBoxLayout(InspectorVolumeFog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorVolumeFog);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label_11 = new QLabel(groupBox);
        label_11->setObjectName(QStringLiteral("label_11"));

        horizontalLayout_2->addWidget(label_11);

        m_pWindDirectionXEdit = new QLineEdit(groupBox);
        m_pWindDirectionXEdit->setObjectName(QStringLiteral("m_pWindDirectionXEdit"));

        horizontalLayout_2->addWidget(m_pWindDirectionXEdit);

        label_10 = new QLabel(groupBox);
        label_10->setObjectName(QStringLiteral("label_10"));

        horizontalLayout_2->addWidget(label_10);

        m_pWindDirectionYEdit = new QLineEdit(groupBox);
        m_pWindDirectionYEdit->setObjectName(QStringLiteral("m_pWindDirectionYEdit"));

        horizontalLayout_2->addWidget(m_pWindDirectionYEdit);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QStringLiteral("label_9"));

        horizontalLayout_2->addWidget(label_9);

        m_pWindDirectionZEdit = new QLineEdit(groupBox);
        m_pWindDirectionZEdit->setObjectName(QStringLiteral("m_pWindDirectionZEdit"));

        horizontalLayout_2->addWidget(m_pWindDirectionZEdit);


        gridLayout->addLayout(horizontalLayout_2, 0, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 7, 0, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        m_pDensityAttenuationEdit = new QLineEdit(groupBox);
        m_pDensityAttenuationEdit->setObjectName(QStringLiteral("m_pDensityAttenuationEdit"));

        gridLayout->addWidget(m_pDensityAttenuationEdit, 7, 1, 1, 1);

        m_pDensityLevelEdit = new QLineEdit(groupBox);
        m_pDensityLevelEdit->setObjectName(QStringLiteral("m_pDensityLevelEdit"));

        gridLayout->addWidget(m_pDensityLevelEdit, 6, 1, 1, 1);

        m_pAbsorptionCoeffEdit = new QLineEdit(groupBox);
        m_pAbsorptionCoeffEdit->setObjectName(QStringLiteral("m_pAbsorptionCoeffEdit"));

        gridLayout->addWidget(m_pAbsorptionCoeffEdit, 5, 1, 1, 1);

        m_pScatteringCoeffEdit = new QLineEdit(groupBox);
        m_pScatteringCoeffEdit->setObjectName(QStringLiteral("m_pScatteringCoeffEdit"));

        gridLayout->addWidget(m_pScatteringCoeffEdit, 4, 1, 1, 1);

        m_pShadowIntensityEdit = new QLineEdit(groupBox);
        m_pShadowIntensityEdit->setObjectName(QStringLiteral("m_pShadowIntensityEdit"));

        gridLayout->addWidget(m_pShadowIntensityEdit, 3, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        m_pFogColorButton = new QPushButton(groupBox);
        m_pFogColorButton->setObjectName(QStringLiteral("m_pFogColorButton"));
        QPalette palette;
        QBrush brush(QColor(255, 0, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
        m_pFogColorButton->setPalette(palette);
        m_pFogColorButton->setAutoFillBackground(true);
        m_pFogColorButton->setFlat(true);

        horizontalLayout->addWidget(m_pFogColorButton);

        m_pFogPickButton = new QPushButton(groupBox);
        m_pFogPickButton->setObjectName(QStringLiteral("m_pFogPickButton"));
        m_pFogPickButton->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(m_pFogPickButton);


        gridLayout->addLayout(horizontalLayout, 1, 1, 1, 1);

        m_pFrustumDepthEdit = new QLineEdit(groupBox);
        m_pFrustumDepthEdit->setObjectName(QStringLiteral("m_pFrustumDepthEdit"));

        gridLayout->addWidget(m_pFrustumDepthEdit, 2, 1, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout->addWidget(label_6, 5, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 4, 0, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout->addWidget(label_7, 6, 0, 1, 1);


        verticalLayout_2->addLayout(gridLayout);


        verticalLayout->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(InspectorVolumeFog);
        QObject::connect(m_pWindDirectionXEdit, SIGNAL(textEdited(QString)), InspectorVolumeFog, SLOT(valueChanged()));
        QObject::connect(m_pWindDirectionYEdit, SIGNAL(textEdited(QString)), InspectorVolumeFog, SLOT(valueChanged()));
        QObject::connect(m_pWindDirectionZEdit, SIGNAL(textEdited(QString)), InspectorVolumeFog, SLOT(valueChanged()));
        QObject::connect(m_pFrustumDepthEdit, SIGNAL(textEdited(QString)), InspectorVolumeFog, SLOT(valueChanged()));
        QObject::connect(m_pShadowIntensityEdit, SIGNAL(textEdited(QString)), InspectorVolumeFog, SLOT(valueChanged()));
        QObject::connect(m_pScatteringCoeffEdit, SIGNAL(textEdited(QString)), InspectorVolumeFog, SLOT(valueChanged()));
        QObject::connect(m_pAbsorptionCoeffEdit, SIGNAL(textEdited(QString)), InspectorVolumeFog, SLOT(valueChanged()));
        QObject::connect(m_pDensityLevelEdit, SIGNAL(textEdited(QString)), InspectorVolumeFog, SLOT(valueChanged()));
        QObject::connect(m_pDensityAttenuationEdit, SIGNAL(textEdited(QString)), InspectorVolumeFog, SLOT(valueChanged()));
        QObject::connect(m_pFogColorButton, SIGNAL(clicked()), InspectorVolumeFog, SLOT(valueChanged()));
        QObject::connect(m_pFogPickButton, SIGNAL(clicked()), InspectorVolumeFog, SLOT(pickColorFromDialog()));
        QObject::connect(m_pFogColorButton, SIGNAL(clicked()), InspectorVolumeFog, SLOT(pickColorFromDialog()));

        QMetaObject::connectSlotsByName(InspectorVolumeFog);
    } // setupUi

    void retranslateUi(QWidget *InspectorVolumeFog)
    {
        InspectorVolumeFog->setWindowTitle(QApplication::translate("InspectorVolumeFog", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorVolumeFog", "Volumetric Fog", 0));
        label_11->setText(QApplication::translate("InspectorVolumeFog", "X", 0));
        label_10->setText(QApplication::translate("InspectorVolumeFog", "Y", 0));
        label_9->setText(QApplication::translate("InspectorVolumeFog", "Z", 0));
        label_2->setText(QApplication::translate("InspectorVolumeFog", "Fog Color", 0));
        label_3->setText(QApplication::translate("InspectorVolumeFog", "Frustum Depth", 0));
        label_4->setText(QApplication::translate("InspectorVolumeFog", "Shadow Intensity", 0));
        label_8->setText(QApplication::translate("InspectorVolumeFog", "Density Attenuation", 0));
        label->setText(QApplication::translate("InspectorVolumeFog", "Wind Direction", 0));
        m_pFogColorButton->setText(QString());
        m_pFogPickButton->setText(QApplication::translate("InspectorVolumeFog", "Pick", 0));
        label_6->setText(QApplication::translate("InspectorVolumeFog", "Absorption Coefficient", 0));
        label_5->setText(QApplication::translate("InspectorVolumeFog", "Scattering Coefficient", 0));
        label_7->setText(QApplication::translate("InspectorVolumeFog", "Density Level", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorVolumeFog: public Ui_InspectorVolumeFog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_VOLUMEFOG_H
