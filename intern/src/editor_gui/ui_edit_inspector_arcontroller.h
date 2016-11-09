/********************************************************************************
** Form generated from reading UI file 'edit_inspector_arcontroller.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_ARCONTROLLER_H
#define UI_EDIT_INSPECTOR_ARCONTROLLER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorARController
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_3;
    QGridLayout *gridLayout_2;
    QLabel *label_6;
    QLabel *label_3;
    QComboBox *comboBox;
    QLabel *label_4;
    QLabel *label_2;
    QLabel *label;
    QLabel *label_5;
    QLabel *label_7;
    QLineEdit *lineEdit;
    QLineEdit *lineEdit_2;
    QLineEdit *lineEdit_3;
    QLineEdit *lineEdit_4;
    QLineEdit *lineEdit_5;
    QLineEdit *lineEdit_6;

    void setupUi(QWidget *InspectorARController)
    {
        if (InspectorARController->objectName().isEmpty())
            InspectorARController->setObjectName(QStringLiteral("InspectorARController"));
        InspectorARController->resize(526, 390);
        verticalLayout = new QVBoxLayout(InspectorARController);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorARController);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_3 = new QVBoxLayout(groupBox);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout_2->addWidget(label_6, 5, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout_2->addWidget(label_3, 1, 0, 1, 1);

        comboBox = new QComboBox(groupBox);
        comboBox->setObjectName(QStringLiteral("comboBox"));

        gridLayout_2->addWidget(comboBox, 0, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout_2->addWidget(label_4, 2, 0, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout_2->addWidget(label_2, 0, 0, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout_2->addWidget(label, 3, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout_2->addWidget(label_5, 4, 0, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout_2->addWidget(label_7, 6, 0, 1, 1);

        lineEdit = new QLineEdit(groupBox);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        gridLayout_2->addWidget(lineEdit, 1, 1, 1, 1);

        lineEdit_2 = new QLineEdit(groupBox);
        lineEdit_2->setObjectName(QStringLiteral("lineEdit_2"));

        gridLayout_2->addWidget(lineEdit_2, 2, 1, 1, 1);

        lineEdit_3 = new QLineEdit(groupBox);
        lineEdit_3->setObjectName(QStringLiteral("lineEdit_3"));

        gridLayout_2->addWidget(lineEdit_3, 3, 1, 1, 1);

        lineEdit_4 = new QLineEdit(groupBox);
        lineEdit_4->setObjectName(QStringLiteral("lineEdit_4"));
        lineEdit_4->setReadOnly(true);

        gridLayout_2->addWidget(lineEdit_4, 4, 1, 1, 1);

        lineEdit_5 = new QLineEdit(groupBox);
        lineEdit_5->setObjectName(QStringLiteral("lineEdit_5"));
        lineEdit_5->setReadOnly(true);

        gridLayout_2->addWidget(lineEdit_5, 5, 1, 1, 1);

        lineEdit_6 = new QLineEdit(groupBox);
        lineEdit_6->setObjectName(QStringLiteral("lineEdit_6"));

        gridLayout_2->addWidget(lineEdit_6, 6, 1, 1, 1);


        verticalLayout_3->addLayout(gridLayout_2);


        verticalLayout->addWidget(groupBox);


        retranslateUi(InspectorARController);

        QMetaObject::connectSlotsByName(InspectorARController);
    } // setupUi

    void retranslateUi(QWidget *InspectorARController)
    {
        InspectorARController->setWindowTitle(QApplication::translate("InspectorARController", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorARController", "AR Controller", 0));
        label_6->setText(QApplication::translate("InspectorARController", "Output Cubemap", 0));
        label_3->setText(QApplication::translate("InspectorARController", "Parameter File", 0));
        comboBox->clear();
        comboBox->insertItems(0, QStringList()
         << QApplication::translate("InspectorARController", "Webcam", 0)
         << QApplication::translate("InspectorARController", "Kinect", 0)
        );
        label_4->setText(QApplication::translate("InspectorARController", "Device Number", 0));
        label_2->setText(QApplication::translate("InspectorARController", "Device", 0));
        label->setText(QApplication::translate("InspectorARController", "Camera Entity ID", 0));
        label_5->setText(QApplication::translate("InspectorARController", "Output Background", 0));
        label_7->setText(QApplication::translate("InspectorARController", "Number of Marker", 0));
        lineEdit->setText(QApplication::translate("InspectorARController", "../assets/", 0));
        lineEdit_2->setText(QApplication::translate("InspectorARController", "0", 0));
        lineEdit_3->setText(QApplication::translate("InspectorARController", "0", 0));
        lineEdit_4->setText(QApplication::translate("InspectorARController", "0", 0));
        lineEdit_5->setText(QApplication::translate("InspectorARController", "0", 0));
        lineEdit_6->setText(QApplication::translate("InspectorARController", "0", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorARController: public Ui_InspectorARController {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_ARCONTROLLER_H
