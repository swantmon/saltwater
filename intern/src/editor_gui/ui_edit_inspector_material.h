/********************************************************************************
** Form generated from reading UI file 'edit_inspector_material.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_MATERIAL_H
#define UI_EDIT_INSPECTOR_MATERIAL_H

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
#include <QtWidgets/QSlider>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorMaterial
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_13;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_15;
    QLineEdit *m_pTilingXEdit;
    QLabel *label_16;
    QLineEdit *m_pTilingYEdit;
    QLabel *label_8;
    QLabel *label_7;
    QHBoxLayout *horizontalLayout_2;
    QSlider *m_pRoughnessSlider;
    QLineEdit *m_pRoughnessEdit;
    QLabel *label_9;
    QLabel *label_5;
    QLabel *label_4;
    QHBoxLayout *horizontalLayout_3;
    QSlider *m_pReflectanceSlider;
    QLineEdit *m_pReflectanceEdit;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_17;
    QLineEdit *m_pOffsetXEdit;
    QLabel *label_18;
    QLineEdit *m_pOffsetYEdit;
    QLabel *label;
    QLineEdit *m_pNormalTextureEdit;
    QLineEdit *m_pAlbedoTextureEdit;
    QLabel *label_6;
    QLineEdit *m_pRoughnessTextureEdit;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_11;
    QHBoxLayout *horizontalLayout;
    QPushButton *m_pAlbedoColorButton;
    QPushButton *m_pAlbedoPickButton;
    QLineEdit *m_pMetallicTextureEdit;
    QLabel *label_19;
    QHBoxLayout *horizontalLayout_6;
    QSlider *m_pMetallicSlider;
    QLineEdit *m_pMetallicEdit;
    QLabel *label_14;
    QLabel *label_10;
    QLabel *label_12;
    QLabel *label_20;

    void setupUi(QWidget *InspectorMaterial)
    {
        if (InspectorMaterial->objectName().isEmpty())
            InspectorMaterial->setObjectName(QStringLiteral("InspectorMaterial"));
        InspectorMaterial->resize(343, 485);
        verticalLayout = new QVBoxLayout(InspectorMaterial);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorMaterial);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setCheckable(false);
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_13 = new QLabel(groupBox);
        label_13->setObjectName(QStringLiteral("label_13"));

        gridLayout->addWidget(label_13, 19, 0, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        label_15 = new QLabel(groupBox);
        label_15->setObjectName(QStringLiteral("label_15"));

        horizontalLayout_4->addWidget(label_15);

        m_pTilingXEdit = new QLineEdit(groupBox);
        m_pTilingXEdit->setObjectName(QStringLiteral("m_pTilingXEdit"));

        horizontalLayout_4->addWidget(m_pTilingXEdit);

        label_16 = new QLabel(groupBox);
        label_16->setObjectName(QStringLiteral("label_16"));

        horizontalLayout_4->addWidget(label_16);

        m_pTilingYEdit = new QLineEdit(groupBox);
        m_pTilingYEdit->setObjectName(QStringLiteral("m_pTilingYEdit"));

        horizontalLayout_4->addWidget(m_pTilingYEdit);


        gridLayout->addLayout(horizontalLayout_4, 19, 1, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 11, 0, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout->addWidget(label_7, 10, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        m_pRoughnessSlider = new QSlider(groupBox);
        m_pRoughnessSlider->setObjectName(QStringLiteral("m_pRoughnessSlider"));
        m_pRoughnessSlider->setMaximum(100);
        m_pRoughnessSlider->setValue(100);
        m_pRoughnessSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(m_pRoughnessSlider);

        m_pRoughnessEdit = new QLineEdit(groupBox);
        m_pRoughnessEdit->setObjectName(QStringLiteral("m_pRoughnessEdit"));
        m_pRoughnessEdit->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_2->addWidget(m_pRoughnessEdit);


        gridLayout->addLayout(horizontalLayout_2, 11, 1, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setStyleSheet(QStringLiteral("font-weight:bold;"));

        gridLayout->addWidget(label_9, 12, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 8, 0, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setStyleSheet(QStringLiteral("font-weight:bold;"));

        gridLayout->addWidget(label_4, 7, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        m_pReflectanceSlider = new QSlider(groupBox);
        m_pReflectanceSlider->setObjectName(QStringLiteral("m_pReflectanceSlider"));
        m_pReflectanceSlider->setMaximum(100);
        m_pReflectanceSlider->setOrientation(Qt::Horizontal);
        m_pReflectanceSlider->setTickPosition(QSlider::NoTicks);

        horizontalLayout_3->addWidget(m_pReflectanceSlider);

        m_pReflectanceEdit = new QLineEdit(groupBox);
        m_pReflectanceEdit->setObjectName(QStringLiteral("m_pReflectanceEdit"));
        m_pReflectanceEdit->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_3->addWidget(m_pReflectanceEdit);


        gridLayout->addLayout(horizontalLayout_3, 16, 1, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        label_17 = new QLabel(groupBox);
        label_17->setObjectName(QStringLiteral("label_17"));

        horizontalLayout_5->addWidget(label_17);

        m_pOffsetXEdit = new QLineEdit(groupBox);
        m_pOffsetXEdit->setObjectName(QStringLiteral("m_pOffsetXEdit"));

        horizontalLayout_5->addWidget(m_pOffsetXEdit);

        label_18 = new QLabel(groupBox);
        label_18->setObjectName(QStringLiteral("label_18"));

        horizontalLayout_5->addWidget(label_18);

        m_pOffsetYEdit = new QLineEdit(groupBox);
        m_pOffsetYEdit->setObjectName(QStringLiteral("m_pOffsetYEdit"));

        horizontalLayout_5->addWidget(m_pOffsetYEdit);


        gridLayout->addLayout(horizontalLayout_5, 20, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        label->setStyleSheet(QStringLiteral("font-weight:bold;"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        m_pNormalTextureEdit = new QLineEdit(groupBox);
        m_pNormalTextureEdit->setObjectName(QStringLiteral("m_pNormalTextureEdit"));

        gridLayout->addWidget(m_pNormalTextureEdit, 8, 1, 1, 1);

        m_pAlbedoTextureEdit = new QLineEdit(groupBox);
        m_pAlbedoTextureEdit->setObjectName(QStringLiteral("m_pAlbedoTextureEdit"));

        gridLayout->addWidget(m_pAlbedoTextureEdit, 2, 1, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setStyleSheet(QStringLiteral("font-weight:bold;"));

        gridLayout->addWidget(label_6, 9, 0, 1, 1);

        m_pRoughnessTextureEdit = new QLineEdit(groupBox);
        m_pRoughnessTextureEdit->setObjectName(QStringLiteral("m_pRoughnessTextureEdit"));

        gridLayout->addWidget(m_pRoughnessTextureEdit, 10, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        label_11 = new QLabel(groupBox);
        label_11->setObjectName(QStringLiteral("label_11"));

        gridLayout->addWidget(label_11, 13, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        m_pAlbedoColorButton = new QPushButton(groupBox);
        m_pAlbedoColorButton->setObjectName(QStringLiteral("m_pAlbedoColorButton"));
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
        m_pAlbedoColorButton->setPalette(palette);
        m_pAlbedoColorButton->setAutoFillBackground(true);
        m_pAlbedoColorButton->setCheckable(false);
        m_pAlbedoColorButton->setFlat(true);

        horizontalLayout->addWidget(m_pAlbedoColorButton);

        m_pAlbedoPickButton = new QPushButton(groupBox);
        m_pAlbedoPickButton->setObjectName(QStringLiteral("m_pAlbedoPickButton"));
        m_pAlbedoPickButton->setMaximumSize(QSize(30, 20));

        horizontalLayout->addWidget(m_pAlbedoPickButton);


        gridLayout->addLayout(horizontalLayout, 3, 1, 1, 1);

        m_pMetallicTextureEdit = new QLineEdit(groupBox);
        m_pMetallicTextureEdit->setObjectName(QStringLiteral("m_pMetallicTextureEdit"));

        gridLayout->addWidget(m_pMetallicTextureEdit, 13, 1, 1, 1);

        label_19 = new QLabel(groupBox);
        label_19->setObjectName(QStringLiteral("label_19"));

        gridLayout->addWidget(label_19, 16, 0, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        m_pMetallicSlider = new QSlider(groupBox);
        m_pMetallicSlider->setObjectName(QStringLiteral("m_pMetallicSlider"));
        m_pMetallicSlider->setMaximum(100);
        m_pMetallicSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_6->addWidget(m_pMetallicSlider);

        m_pMetallicEdit = new QLineEdit(groupBox);
        m_pMetallicEdit->setObjectName(QStringLiteral("m_pMetallicEdit"));
        m_pMetallicEdit->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_6->addWidget(m_pMetallicEdit);


        gridLayout->addLayout(horizontalLayout_6, 14, 1, 1, 1);

        label_14 = new QLabel(groupBox);
        label_14->setObjectName(QStringLiteral("label_14"));

        gridLayout->addWidget(label_14, 20, 0, 1, 1);

        label_10 = new QLabel(groupBox);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setStyleSheet(QStringLiteral("font-weight:bold;"));

        gridLayout->addWidget(label_10, 15, 0, 1, 1);

        label_12 = new QLabel(groupBox);
        label_12->setObjectName(QStringLiteral("label_12"));
        label_12->setStyleSheet(QStringLiteral("font-weight:bold;"));

        gridLayout->addWidget(label_12, 18, 0, 1, 1);

        label_20 = new QLabel(groupBox);
        label_20->setObjectName(QStringLiteral("label_20"));

        gridLayout->addWidget(label_20, 14, 0, 1, 1);


        verticalLayout->addWidget(groupBox);

        QWidget::setTabOrder(m_pAlbedoTextureEdit, m_pAlbedoColorButton);
        QWidget::setTabOrder(m_pAlbedoColorButton, m_pAlbedoPickButton);
        QWidget::setTabOrder(m_pAlbedoPickButton, m_pNormalTextureEdit);
        QWidget::setTabOrder(m_pNormalTextureEdit, m_pRoughnessTextureEdit);
        QWidget::setTabOrder(m_pRoughnessTextureEdit, m_pRoughnessSlider);
        QWidget::setTabOrder(m_pRoughnessSlider, m_pRoughnessEdit);
        QWidget::setTabOrder(m_pRoughnessEdit, m_pMetallicTextureEdit);
        QWidget::setTabOrder(m_pMetallicTextureEdit, m_pMetallicSlider);
        QWidget::setTabOrder(m_pMetallicSlider, m_pMetallicEdit);
        QWidget::setTabOrder(m_pMetallicEdit, m_pReflectanceSlider);
        QWidget::setTabOrder(m_pReflectanceSlider, m_pReflectanceEdit);
        QWidget::setTabOrder(m_pReflectanceEdit, m_pTilingXEdit);
        QWidget::setTabOrder(m_pTilingXEdit, m_pTilingYEdit);
        QWidget::setTabOrder(m_pTilingYEdit, m_pOffsetXEdit);
        QWidget::setTabOrder(m_pOffsetXEdit, m_pOffsetYEdit);

        retranslateUi(InspectorMaterial);
        QObject::connect(m_pAlbedoPickButton, SIGNAL(clicked()), InspectorMaterial, SLOT(pickColorFromDialog()));
        QObject::connect(m_pAlbedoColorButton, SIGNAL(clicked()), InspectorMaterial, SLOT(pickColorFromDialog()));
        QObject::connect(m_pAlbedoTextureEdit, SIGNAL(textEdited(QString)), InspectorMaterial, SLOT(valueChanged()));
        QObject::connect(m_pNormalTextureEdit, SIGNAL(textEdited(QString)), InspectorMaterial, SLOT(valueChanged()));
        QObject::connect(m_pRoughnessTextureEdit, SIGNAL(textEdited(QString)), InspectorMaterial, SLOT(valueChanged()));
        QObject::connect(m_pMetallicTextureEdit, SIGNAL(textEdited(QString)), InspectorMaterial, SLOT(valueChanged()));
        QObject::connect(m_pTilingXEdit, SIGNAL(textEdited(QString)), InspectorMaterial, SLOT(valueChanged()));
        QObject::connect(m_pTilingYEdit, SIGNAL(textEdited(QString)), InspectorMaterial, SLOT(valueChanged()));
        QObject::connect(m_pOffsetXEdit, SIGNAL(textEdited(QString)), InspectorMaterial, SLOT(valueChanged()));
        QObject::connect(m_pOffsetYEdit, SIGNAL(textEdited(QString)), InspectorMaterial, SLOT(valueChanged()));
        QObject::connect(m_pRoughnessEdit, SIGNAL(textChanged(QString)), InspectorMaterial, SLOT(valueChanged()));
        QObject::connect(m_pMetallicEdit, SIGNAL(textChanged(QString)), InspectorMaterial, SLOT(valueChanged()));
        QObject::connect(m_pReflectanceEdit, SIGNAL(textChanged(QString)), InspectorMaterial, SLOT(valueChanged()));
        QObject::connect(m_pRoughnessSlider, SIGNAL(valueChanged(int)), InspectorMaterial, SLOT(roughnessValueChanged(int)));
        QObject::connect(m_pMetallicSlider, SIGNAL(valueChanged(int)), InspectorMaterial, SLOT(metallicValueChanged(int)));
        QObject::connect(m_pReflectanceSlider, SIGNAL(valueChanged(int)), InspectorMaterial, SLOT(reflectanceValueChanged(int)));

        QMetaObject::connectSlotsByName(InspectorMaterial);
    } // setupUi

    void retranslateUi(QWidget *InspectorMaterial)
    {
        InspectorMaterial->setWindowTitle(QApplication::translate("InspectorMaterial", "Material", 0));
        groupBox->setTitle(QApplication::translate("InspectorMaterial", "Material", 0));
        label_13->setText(QApplication::translate("InspectorMaterial", "Tiling", 0));
        label_15->setText(QApplication::translate("InspectorMaterial", "X", 0));
        m_pTilingXEdit->setText(QApplication::translate("InspectorMaterial", "0", 0));
        label_16->setText(QApplication::translate("InspectorMaterial", "Y", 0));
        m_pTilingYEdit->setText(QApplication::translate("InspectorMaterial", "0", 0));
        label_8->setText(QApplication::translate("InspectorMaterial", "Value", 0));
        label_7->setText(QApplication::translate("InspectorMaterial", "Texture", 0));
        m_pRoughnessEdit->setText(QApplication::translate("InspectorMaterial", "1", 0));
        label_9->setText(QApplication::translate("InspectorMaterial", "Metallic", 0));
        label_5->setText(QApplication::translate("InspectorMaterial", "Texture", 0));
        label_4->setText(QApplication::translate("InspectorMaterial", "Normal", 0));
        m_pReflectanceEdit->setText(QApplication::translate("InspectorMaterial", "0", 0));
        label_17->setText(QApplication::translate("InspectorMaterial", "X", 0));
        m_pOffsetXEdit->setText(QApplication::translate("InspectorMaterial", "1", 0));
        label_18->setText(QApplication::translate("InspectorMaterial", "Y", 0));
        m_pOffsetYEdit->setText(QApplication::translate("InspectorMaterial", "1", 0));
        label->setText(QApplication::translate("InspectorMaterial", "Albedo", 0));
        label_6->setText(QApplication::translate("InspectorMaterial", "Roughness", 0));
        label_2->setText(QApplication::translate("InspectorMaterial", "Texture", 0));
        label_3->setText(QApplication::translate("InspectorMaterial", "Color", 0));
        label_11->setText(QApplication::translate("InspectorMaterial", "Texture", 0));
        m_pAlbedoColorButton->setText(QString());
        m_pAlbedoPickButton->setText(QApplication::translate("InspectorMaterial", "Pick", 0));
        label_19->setText(QApplication::translate("InspectorMaterial", "Value", 0));
        m_pMetallicEdit->setText(QApplication::translate("InspectorMaterial", "0", 0));
        label_14->setText(QApplication::translate("InspectorMaterial", "Offset", 0));
        label_10->setText(QApplication::translate("InspectorMaterial", "Reflectance", 0));
        label_12->setText(QApplication::translate("InspectorMaterial", "Tiling & Offset", 0));
        label_20->setText(QApplication::translate("InspectorMaterial", "Value", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorMaterial: public Ui_InspectorMaterial {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_MATERIAL_H
