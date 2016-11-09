/********************************************************************************
** Form generated from reading UI file 'edit_inspector_arcontroller_marker.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_ARCONTROLLER_MARKER_H
#define UI_EDIT_INSPECTOR_ARCONTROLLER_MARKER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorARControllerMarker
{
public:
    QGridLayout *gridLayout;
    QLabel *label_3;
    QLabel *label_2;
    QLabel *label_4;
    QLabel *label;
    QLineEdit *lineEdit;
    QComboBox *comboBox;
    QLineEdit *lineEdit_2;
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEdit_3;
    QLabel *label_5;

    void setupUi(QWidget *InspectorARControllerMarker)
    {
        if (InspectorARControllerMarker->objectName().isEmpty())
            InspectorARControllerMarker->setObjectName(QStringLiteral("InspectorARControllerMarker"));
        InspectorARControllerMarker->resize(400, 198);
        gridLayout = new QGridLayout(InspectorARControllerMarker);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_3 = new QLabel(InspectorARControllerMarker);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_2 = new QLabel(InspectorARControllerMarker);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label_4 = new QLabel(InspectorARControllerMarker);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        label = new QLabel(InspectorARControllerMarker);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        lineEdit = new QLineEdit(InspectorARControllerMarker);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        gridLayout->addWidget(lineEdit, 0, 1, 1, 1);

        comboBox = new QComboBox(InspectorARControllerMarker);
        comboBox->setObjectName(QStringLiteral("comboBox"));

        gridLayout->addWidget(comboBox, 1, 1, 1, 1);

        lineEdit_2 = new QLineEdit(InspectorARControllerMarker);
        lineEdit_2->setObjectName(QStringLiteral("lineEdit_2"));

        gridLayout->addWidget(lineEdit_2, 2, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        lineEdit_3 = new QLineEdit(InspectorARControllerMarker);
        lineEdit_3->setObjectName(QStringLiteral("lineEdit_3"));

        horizontalLayout->addWidget(lineEdit_3);

        label_5 = new QLabel(InspectorARControllerMarker);
        label_5->setObjectName(QStringLiteral("label_5"));

        horizontalLayout->addWidget(label_5);


        gridLayout->addLayout(horizontalLayout, 3, 1, 1, 1);


        retranslateUi(InspectorARControllerMarker);

        QMetaObject::connectSlotsByName(InspectorARControllerMarker);
    } // setupUi

    void retranslateUi(QWidget *InspectorARControllerMarker)
    {
        InspectorARControllerMarker->setWindowTitle(QApplication::translate("InspectorARControllerMarker", "Form", 0));
        label_3->setText(QApplication::translate("InspectorARControllerMarker", "Pattern File", 0));
        label_2->setText(QApplication::translate("InspectorARControllerMarker", "Type", 0));
        label_4->setText(QApplication::translate("InspectorARControllerMarker", "Width", 0));
        label->setText(QApplication::translate("InspectorARControllerMarker", "UID", 0));
        lineEdit->setText(QApplication::translate("InspectorARControllerMarker", "0", 0));
        comboBox->clear();
        comboBox->insertItems(0, QStringList()
         << QApplication::translate("InspectorARControllerMarker", "Square", 0)
         << QApplication::translate("InspectorARControllerMarker", "Square Barcode", 0)
         << QApplication::translate("InspectorARControllerMarker", "Multimarker", 0)
         << QApplication::translate("InspectorARControllerMarker", "NFT", 0)
        );
        lineEdit_3->setText(QApplication::translate("InspectorARControllerMarker", "0.06", 0));
        label_5->setText(QApplication::translate("InspectorARControllerMarker", "m", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorARControllerMarker: public Ui_InspectorARControllerMarker {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_ARCONTROLLER_MARKER_H
