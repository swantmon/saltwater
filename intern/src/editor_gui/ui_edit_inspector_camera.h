/********************************************************************************
** Form generated from reading UI file 'edit_inspector_camera.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_CAMERA_H
#define UI_EDIT_INSPECTOR_CAMERA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorCamera
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QLineEdit *lineEdit_13;
    QCheckBox *checkBox;
    QLineEdit *lineEdit_12;
    QLabel *label_16;
    QLabel *label_21;
    QLabel *label_3;
    QLabel *label_5;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QComboBox *comboBox;
    QLineEdit *lineEdit_9;
    QComboBox *comboBox_2;
    QLabel *label;
    QLabel *label_8;
    QLabel *label_9;
    QCheckBox *checkBox_2;
    QLabel *label_10;
    QLineEdit *lineEdit_4;
    QComboBox *comboBox_3;
    QHBoxLayout *horizontalLayout_2;
    QSlider *horizontalSlider;
    QLineEdit *lineEdit;
    QLabel *label_4;
    QLabel *label_17;
    QLabel *label_19;
    QHBoxLayout *horizontalLayout_4;
    QLineEdit *lineEdit_11;
    QLabel *label_23;
    QLabel *label_20;
    QHBoxLayout *horizontalLayout_3;
    QLineEdit *lineEdit_10;
    QLabel *label_22;
    QLabel *label_18;
    QGridLayout *gridLayout_2;
    QLabel *label_6;
    QLabel *label_7;
    QLineEdit *lineEdit_2;
    QLineEdit *lineEdit_3;
    QLabel *label_24;
    QLineEdit *lineEdit_14;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_11;
    QGridLayout *gridLayout_3;
    QLabel *label_14;
    QLabel *label_12;
    QLabel *label_13;
    QLabel *label_15;
    QLineEdit *lineEdit_5;
    QLineEdit *lineEdit_6;
    QLineEdit *lineEdit_7;
    QLineEdit *lineEdit_8;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *InspectorCamera)
    {
        if (InspectorCamera->objectName().isEmpty())
            InspectorCamera->setObjectName(QStringLiteral("InspectorCamera"));
        InspectorCamera->resize(792, 876);
        verticalLayout = new QVBoxLayout(InspectorCamera);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorCamera);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        lineEdit_13 = new QLineEdit(groupBox);
        lineEdit_13->setObjectName(QStringLiteral("lineEdit_13"));

        gridLayout->addWidget(lineEdit_13, 14, 1, 1, 1);

        checkBox = new QCheckBox(groupBox);
        checkBox->setObjectName(QStringLiteral("checkBox"));
        checkBox->setChecked(true);

        gridLayout->addWidget(checkBox, 8, 1, 1, 1);

        lineEdit_12 = new QLineEdit(groupBox);
        lineEdit_12->setObjectName(QStringLiteral("lineEdit_12"));

        gridLayout->addWidget(lineEdit_12, 13, 1, 1, 1);

        label_16 = new QLabel(groupBox);
        label_16->setObjectName(QStringLiteral("label_16"));

        gridLayout->addWidget(label_16, 9, 0, 1, 1);

        label_21 = new QLabel(groupBox);
        label_21->setObjectName(QStringLiteral("label_21"));

        gridLayout->addWidget(label_21, 14, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 4, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        gridLayout->addWidget(label_5, 7, 0, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        pushButton = new QPushButton(groupBox);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
        pushButton->setPalette(palette);
        pushButton->setAutoFillBackground(true);
        pushButton->setFlat(true);

        horizontalLayout->addWidget(pushButton);

        pushButton_2 = new QPushButton(groupBox);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(pushButton_2);


        gridLayout->addLayout(horizontalLayout, 2, 1, 1, 1);

        comboBox = new QComboBox(groupBox);
        comboBox->setObjectName(QStringLiteral("comboBox"));

        gridLayout->addWidget(comboBox, 1, 1, 1, 1);

        lineEdit_9 = new QLineEdit(groupBox);
        lineEdit_9->setObjectName(QStringLiteral("lineEdit_9"));

        gridLayout->addWidget(lineEdit_9, 9, 1, 1, 1);

        comboBox_2 = new QComboBox(groupBox);
        comboBox_2->setObjectName(QStringLiteral("comboBox_2"));

        gridLayout->addWidget(comboBox_2, 4, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 8, 0, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QStringLiteral("label_9"));

        gridLayout->addWidget(label_9, 0, 0, 1, 1);

        checkBox_2 = new QCheckBox(groupBox);
        checkBox_2->setObjectName(QStringLiteral("checkBox_2"));

        gridLayout->addWidget(checkBox_2, 0, 1, 1, 1);

        label_10 = new QLabel(groupBox);
        label_10->setObjectName(QStringLiteral("label_10"));

        gridLayout->addWidget(label_10, 6, 0, 1, 1);

        lineEdit_4 = new QLineEdit(groupBox);
        lineEdit_4->setObjectName(QStringLiteral("lineEdit_4"));

        gridLayout->addWidget(lineEdit_4, 6, 1, 1, 1);

        comboBox_3 = new QComboBox(groupBox);
        comboBox_3->setObjectName(QStringLiteral("comboBox_3"));

        gridLayout->addWidget(comboBox_3, 10, 1, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalSlider = new QSlider(groupBox);
        horizontalSlider->setObjectName(QStringLiteral("horizontalSlider"));
        horizontalSlider->setMinimum(1);
        horizontalSlider->setMaximum(179);
        horizontalSlider->setValue(60);
        horizontalSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(horizontalSlider);

        lineEdit = new QLineEdit(groupBox);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_2->addWidget(lineEdit);


        gridLayout->addLayout(horizontalLayout_2, 5, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 5, 0, 1, 1);

        label_17 = new QLabel(groupBox);
        label_17->setObjectName(QStringLiteral("label_17"));

        gridLayout->addWidget(label_17, 10, 0, 1, 1);

        label_19 = new QLabel(groupBox);
        label_19->setObjectName(QStringLiteral("label_19"));

        gridLayout->addWidget(label_19, 12, 0, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        lineEdit_11 = new QLineEdit(groupBox);
        lineEdit_11->setObjectName(QStringLiteral("lineEdit_11"));

        horizontalLayout_4->addWidget(lineEdit_11);

        label_23 = new QLabel(groupBox);
        label_23->setObjectName(QStringLiteral("label_23"));

        horizontalLayout_4->addWidget(label_23);


        gridLayout->addLayout(horizontalLayout_4, 12, 1, 1, 1);

        label_20 = new QLabel(groupBox);
        label_20->setObjectName(QStringLiteral("label_20"));

        gridLayout->addWidget(label_20, 13, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        lineEdit_10 = new QLineEdit(groupBox);
        lineEdit_10->setObjectName(QStringLiteral("lineEdit_10"));

        horizontalLayout_3->addWidget(lineEdit_10);

        label_22 = new QLabel(groupBox);
        label_22->setObjectName(QStringLiteral("label_22"));

        horizontalLayout_3->addWidget(label_22);


        gridLayout->addLayout(horizontalLayout_3, 11, 1, 1, 1);

        label_18 = new QLabel(groupBox);
        label_18->setObjectName(QStringLiteral("label_18"));

        gridLayout->addWidget(label_18, 11, 0, 1, 1);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout_2->addWidget(label_6, 0, 0, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout_2->addWidget(label_7, 1, 0, 1, 1);

        lineEdit_2 = new QLineEdit(groupBox);
        lineEdit_2->setObjectName(QStringLiteral("lineEdit_2"));

        gridLayout_2->addWidget(lineEdit_2, 0, 1, 1, 1);

        lineEdit_3 = new QLineEdit(groupBox);
        lineEdit_3->setObjectName(QStringLiteral("lineEdit_3"));

        gridLayout_2->addWidget(lineEdit_3, 1, 1, 1, 1);


        gridLayout->addLayout(gridLayout_2, 7, 1, 1, 1);

        label_24 = new QLabel(groupBox);
        label_24->setObjectName(QStringLiteral("label_24"));

        gridLayout->addWidget(label_24, 3, 0, 1, 1);

        lineEdit_14 = new QLineEdit(groupBox);
        lineEdit_14->setObjectName(QStringLiteral("lineEdit_14"));

        gridLayout->addWidget(lineEdit_14, 3, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        label_11 = new QLabel(groupBox);
        label_11->setObjectName(QStringLiteral("label_11"));
        label_11->setStyleSheet(QStringLiteral(""));

        verticalLayout_3->addWidget(label_11);

        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        label_14 = new QLabel(groupBox);
        label_14->setObjectName(QStringLiteral("label_14"));

        gridLayout_3->addWidget(label_14, 0, 3, 1, 1);

        label_12 = new QLabel(groupBox);
        label_12->setObjectName(QStringLiteral("label_12"));

        gridLayout_3->addWidget(label_12, 0, 1, 1, 1);

        label_13 = new QLabel(groupBox);
        label_13->setObjectName(QStringLiteral("label_13"));

        gridLayout_3->addWidget(label_13, 1, 1, 1, 1);

        label_15 = new QLabel(groupBox);
        label_15->setObjectName(QStringLiteral("label_15"));

        gridLayout_3->addWidget(label_15, 1, 3, 1, 1);

        lineEdit_5 = new QLineEdit(groupBox);
        lineEdit_5->setObjectName(QStringLiteral("lineEdit_5"));

        gridLayout_3->addWidget(lineEdit_5, 0, 2, 1, 1);

        lineEdit_6 = new QLineEdit(groupBox);
        lineEdit_6->setObjectName(QStringLiteral("lineEdit_6"));

        gridLayout_3->addWidget(lineEdit_6, 1, 2, 1, 1);

        lineEdit_7 = new QLineEdit(groupBox);
        lineEdit_7->setObjectName(QStringLiteral("lineEdit_7"));

        gridLayout_3->addWidget(lineEdit_7, 0, 4, 1, 1);

        lineEdit_8 = new QLineEdit(groupBox);
        lineEdit_8->setObjectName(QStringLiteral("lineEdit_8"));

        gridLayout_3->addWidget(lineEdit_8, 1, 4, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer, 0, 0, 1, 1);


        verticalLayout_3->addLayout(gridLayout_3);


        verticalLayout_2->addLayout(verticalLayout_3);


        verticalLayout->addWidget(groupBox);


        retranslateUi(InspectorCamera);

        QMetaObject::connectSlotsByName(InspectorCamera);
    } // setupUi

    void retranslateUi(QWidget *InspectorCamera)
    {
        InspectorCamera->setWindowTitle(QApplication::translate("InspectorCamera", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorCamera", "Camera", 0));
        lineEdit_13->setText(QApplication::translate("InspectorCamera", "0", 0));
        checkBox->setText(QString());
        lineEdit_12->setText(QApplication::translate("InspectorCamera", "100", 0));
        label_16->setText(QApplication::translate("InspectorCamera", "Depth", 0));
        label_21->setText(QApplication::translate("InspectorCamera", "EC", 0));
        label_3->setText(QApplication::translate("InspectorCamera", "Projection", 0));
        label_5->setText(QApplication::translate("InspectorCamera", "Clipping Planes", 0));
        label_2->setText(QApplication::translate("InspectorCamera", "Solid Color", 0));
        pushButton->setText(QString());
        pushButton_2->setText(QApplication::translate("InspectorCamera", "Pick", 0));
        comboBox->clear();
        comboBox->insertItems(0, QStringList()
         << QApplication::translate("InspectorCamera", "Skybox", 0)
         << QApplication::translate("InspectorCamera", "Texture", 0)
         << QApplication::translate("InspectorCamera", "Solid Color", 0)
         << QApplication::translate("InspectorCamera", "No clear", 0)
        );
        lineEdit_9->setText(QApplication::translate("InspectorCamera", "-1", 0));
        comboBox_2->clear();
        comboBox_2->insertItems(0, QStringList()
         << QApplication::translate("InspectorCamera", "Perspective", 0)
         << QApplication::translate("InspectorCamera", "Orthographic", 0)
        );
        label->setText(QApplication::translate("InspectorCamera", "Clear Flags", 0));
        label_8->setText(QApplication::translate("InspectorCamera", "Occlusion Culling", 0));
        label_9->setText(QApplication::translate("InspectorCamera", "Main Camera", 0));
        checkBox_2->setText(QString());
        label_10->setText(QApplication::translate("InspectorCamera", "Size", 0));
        comboBox_3->clear();
        comboBox_3->insertItems(0, QStringList()
         << QApplication::translate("InspectorCamera", "Auto", 0)
         << QApplication::translate("InspectorCamera", "Manual", 0)
        );
        lineEdit->setText(QApplication::translate("InspectorCamera", "60", 0));
        label_4->setText(QApplication::translate("InspectorCamera", "Field of View", 0));
        label_17->setText(QApplication::translate("InspectorCamera", "Camera Mode", 0));
        label_19->setText(QApplication::translate("InspectorCamera", "Aperture", 0));
        lineEdit_11->setText(QApplication::translate("InspectorCamera", "16", 0));
        label_23->setText(QApplication::translate("InspectorCamera", "flops", 0));
        label_20->setText(QApplication::translate("InspectorCamera", "ISO", 0));
        lineEdit_10->setText(QApplication::translate("InspectorCamera", "0.01", 0));
        label_22->setText(QApplication::translate("InspectorCamera", "seconds", 0));
        label_18->setText(QApplication::translate("InspectorCamera", "Shutter Speed", 0));
        label_6->setText(QApplication::translate("InspectorCamera", "Near", 0));
        label_7->setText(QApplication::translate("InspectorCamera", "Far", 0));
        lineEdit_2->setText(QApplication::translate("InspectorCamera", "0.3", 0));
        lineEdit_3->setText(QApplication::translate("InspectorCamera", "1024", 0));
        label_24->setText(QApplication::translate("InspectorCamera", "Backround Texture", 0));
        label_11->setText(QApplication::translate("InspectorCamera", "Viewport Rect", 0));
        label_14->setText(QApplication::translate("InspectorCamera", "Y", 0));
        label_12->setText(QApplication::translate("InspectorCamera", "X", 0));
        label_13->setText(QApplication::translate("InspectorCamera", "W", 0));
        label_15->setText(QApplication::translate("InspectorCamera", "H", 0));
        lineEdit_5->setText(QApplication::translate("InspectorCamera", "0", 0));
        lineEdit_6->setText(QApplication::translate("InspectorCamera", "1", 0));
        lineEdit_7->setText(QApplication::translate("InspectorCamera", "0", 0));
        lineEdit_8->setText(QApplication::translate("InspectorCamera", "1", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorCamera: public Ui_InspectorCamera {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_CAMERA_H
