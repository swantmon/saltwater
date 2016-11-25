/********************************************************************************
** Form generated from reading UI file 'edit_inspector_environment.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_ENVIRONMENT_H
#define UI_EDIT_INSPECTOR_ENVIRONMENT_H

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
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorEnvironment
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *m_pTextureEdit;
    QPushButton *m_pTextureLoad;
    QLabel *label;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout;
    QLineEdit *m_pIntensityEdit;
    QLabel *label_4;
    QComboBox *m_pTypeCB;
    QLabel *label_5;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *InspectorEnvironment)
    {
        if (InspectorEnvironment->objectName().isEmpty())
            InspectorEnvironment->setObjectName(QStringLiteral("InspectorEnvironment"));
        InspectorEnvironment->resize(400, 138);
        verticalLayout = new QVBoxLayout(InspectorEnvironment);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorEnvironment);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        m_pTextureEdit = new QLineEdit(groupBox);
        m_pTextureEdit->setObjectName(QStringLiteral("m_pTextureEdit"));

        horizontalLayout_2->addWidget(m_pTextureEdit);

        m_pTextureLoad = new QPushButton(groupBox);
        m_pTextureLoad->setObjectName(QStringLiteral("m_pTextureLoad"));

        horizontalLayout_2->addWidget(m_pTextureLoad);


        gridLayout->addLayout(horizontalLayout_2, 1, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        m_pIntensityEdit = new QLineEdit(groupBox);
        m_pIntensityEdit->setObjectName(QStringLiteral("m_pIntensityEdit"));

        horizontalLayout->addWidget(m_pIntensityEdit);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        horizontalLayout->addWidget(label_4);


        gridLayout->addLayout(horizontalLayout, 2, 1, 1, 1);

        m_pTypeCB = new QComboBox(groupBox);
        m_pTypeCB->setObjectName(QStringLiteral("m_pTypeCB"));

        gridLayout->addWidget(m_pTypeCB, 0, 1, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 1, 0, 1, 1);


        verticalLayout_2->addLayout(gridLayout);


        verticalLayout->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(InspectorEnvironment);
        QObject::connect(m_pTypeCB, SIGNAL(currentIndexChanged(int)), InspectorEnvironment, SLOT(valueChanged()));
        QObject::connect(m_pTextureEdit, SIGNAL(textEdited(QString)), InspectorEnvironment, SLOT(valueChanged()));
        QObject::connect(m_pIntensityEdit, SIGNAL(textEdited(QString)), InspectorEnvironment, SLOT(valueChanged()));
        QObject::connect(m_pTextureLoad, SIGNAL(clicked()), InspectorEnvironment, SLOT(loadTextureFromDialog()));

        QMetaObject::connectSlotsByName(InspectorEnvironment);
    } // setupUi

    void retranslateUi(QWidget *InspectorEnvironment)
    {
        InspectorEnvironment->setWindowTitle(QApplication::translate("InspectorEnvironment", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorEnvironment", "Environment", 0));
        m_pTextureLoad->setText(QApplication::translate("InspectorEnvironment", "Load", 0));
        label->setText(QApplication::translate("InspectorEnvironment", "Type", 0));
        label_3->setText(QApplication::translate("InspectorEnvironment", "Intensity", 0));
        label_4->setText(QApplication::translate("InspectorEnvironment", "Lux", 0));
        m_pTypeCB->clear();
        m_pTypeCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorEnvironment", "Procedural", 0)
         << QApplication::translate("InspectorEnvironment", "Panorama", 0)
         << QApplication::translate("InspectorEnvironment", "Cubemap", 0)
         << QApplication::translate("InspectorEnvironment", "Texture", 0)
        );
        label_5->setText(QApplication::translate("InspectorEnvironment", "Texture", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorEnvironment: public Ui_InspectorEnvironment {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_ENVIRONMENT_H
