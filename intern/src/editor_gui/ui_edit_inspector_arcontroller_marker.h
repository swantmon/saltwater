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
    QLineEdit *m_pUIDEdit;
    QComboBox *m_pTypeCB;
    QLineEdit *m_pPatternFileEdit;
    QHBoxLayout *horizontalLayout;
    QLineEdit *m_pWidthEdit;
    QLabel *label_5;

    void setupUi(QWidget *InspectorARControllerMarker)
    {
        if (InspectorARControllerMarker->objectName().isEmpty())
            InspectorARControllerMarker->setObjectName(QStringLiteral("InspectorARControllerMarker"));
        InspectorARControllerMarker->resize(181, 118);
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

        m_pUIDEdit = new QLineEdit(InspectorARControllerMarker);
        m_pUIDEdit->setObjectName(QStringLiteral("m_pUIDEdit"));

        gridLayout->addWidget(m_pUIDEdit, 0, 1, 1, 1);

        m_pTypeCB = new QComboBox(InspectorARControllerMarker);
        m_pTypeCB->setObjectName(QStringLiteral("m_pTypeCB"));

        gridLayout->addWidget(m_pTypeCB, 1, 1, 1, 1);

        m_pPatternFileEdit = new QLineEdit(InspectorARControllerMarker);
        m_pPatternFileEdit->setObjectName(QStringLiteral("m_pPatternFileEdit"));

        gridLayout->addWidget(m_pPatternFileEdit, 2, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        m_pWidthEdit = new QLineEdit(InspectorARControllerMarker);
        m_pWidthEdit->setObjectName(QStringLiteral("m_pWidthEdit"));

        horizontalLayout->addWidget(m_pWidthEdit);

        label_5 = new QLabel(InspectorARControllerMarker);
        label_5->setObjectName(QStringLiteral("label_5"));

        horizontalLayout->addWidget(label_5);


        gridLayout->addLayout(horizontalLayout, 3, 1, 1, 1);


        retranslateUi(InspectorARControllerMarker);
        QObject::connect(m_pUIDEdit, SIGNAL(textEdited(QString)), InspectorARControllerMarker, SLOT(valueChanged()));
        QObject::connect(m_pTypeCB, SIGNAL(currentIndexChanged(int)), InspectorARControllerMarker, SLOT(valueChanged()));
        QObject::connect(m_pPatternFileEdit, SIGNAL(textEdited(QString)), InspectorARControllerMarker, SLOT(valueChanged()));
        QObject::connect(m_pWidthEdit, SIGNAL(textEdited(QString)), InspectorARControllerMarker, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorARControllerMarker);
    } // setupUi

    void retranslateUi(QWidget *InspectorARControllerMarker)
    {
        InspectorARControllerMarker->setWindowTitle(QApplication::translate("InspectorARControllerMarker", "Form", 0));
        label_3->setText(QApplication::translate("InspectorARControllerMarker", "Pattern File", 0));
        label_2->setText(QApplication::translate("InspectorARControllerMarker", "Type", 0));
        label_4->setText(QApplication::translate("InspectorARControllerMarker", "Width", 0));
        label->setText(QApplication::translate("InspectorARControllerMarker", "UID", 0));
        m_pUIDEdit->setText(QApplication::translate("InspectorARControllerMarker", "0", 0));
        m_pTypeCB->clear();
        m_pTypeCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorARControllerMarker", "Square", 0)
         << QApplication::translate("InspectorARControllerMarker", "Square Barcode", 0)
         << QApplication::translate("InspectorARControllerMarker", "Multimarker", 0)
         << QApplication::translate("InspectorARControllerMarker", "NFT", 0)
        );
        m_pWidthEdit->setText(QApplication::translate("InspectorARControllerMarker", "0.06", 0));
        label_5->setText(QApplication::translate("InspectorARControllerMarker", "m", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorARControllerMarker: public Ui_InspectorARControllerMarker {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_ARCONTROLLER_MARKER_H
