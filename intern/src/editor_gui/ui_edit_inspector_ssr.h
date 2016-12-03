/********************************************************************************
** Form generated from reading UI file 'edit_inspector_ssr.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_SSR_H
#define UI_EDIT_INSPECTOR_SSR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorSSR
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QLabel *label_3;
    QLabel *label_2;
    QLineEdit *m_pIntensityEdit;
    QLabel *label;
    QLabel *label_4;
    QLineEdit *m_pRoughnessMaskEdit;
    QLineEdit *m_pDistanceEdit;
    QCheckBox *m_pDoubleReflectionsCB;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *InspectorSSR)
    {
        if (InspectorSSR->objectName().isEmpty())
            InspectorSSR->setObjectName(QStringLiteral("InspectorSSR"));
        InspectorSSR->resize(400, 150);
        verticalLayout = new QVBoxLayout(InspectorSSR);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorSSR);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        m_pIntensityEdit = new QLineEdit(groupBox);
        m_pIntensityEdit->setObjectName(QStringLiteral("m_pIntensityEdit"));

        gridLayout->addWidget(m_pIntensityEdit, 0, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        m_pRoughnessMaskEdit = new QLineEdit(groupBox);
        m_pRoughnessMaskEdit->setObjectName(QStringLiteral("m_pRoughnessMaskEdit"));

        gridLayout->addWidget(m_pRoughnessMaskEdit, 1, 1, 1, 1);

        m_pDistanceEdit = new QLineEdit(groupBox);
        m_pDistanceEdit->setObjectName(QStringLiteral("m_pDistanceEdit"));

        gridLayout->addWidget(m_pDistanceEdit, 2, 1, 1, 1);

        m_pDoubleReflectionsCB = new QCheckBox(groupBox);
        m_pDoubleReflectionsCB->setObjectName(QStringLiteral("m_pDoubleReflectionsCB"));

        gridLayout->addWidget(m_pDoubleReflectionsCB, 3, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout);


        verticalLayout->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(InspectorSSR);
        QObject::connect(m_pIntensityEdit, SIGNAL(textEdited(QString)), InspectorSSR, SLOT(valueChanged()));
        QObject::connect(m_pRoughnessMaskEdit, SIGNAL(textEdited(QString)), InspectorSSR, SLOT(valueChanged()));
        QObject::connect(m_pDistanceEdit, SIGNAL(textEdited(QString)), InspectorSSR, SLOT(valueChanged()));
        QObject::connect(m_pDoubleReflectionsCB, SIGNAL(clicked()), InspectorSSR, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorSSR);
    } // setupUi

    void retranslateUi(QWidget *InspectorSSR)
    {
        InspectorSSR->setWindowTitle(QApplication::translate("InspectorSSR", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorSSR", "Screen Space Reflections (SSR)", 0));
        label_3->setText(QApplication::translate("InspectorSSR", "Distance", 0));
        label_2->setText(QApplication::translate("InspectorSSR", "Roughness Mask", 0));
        label->setText(QApplication::translate("InspectorSSR", "Intensity", 0));
        label_4->setText(QApplication::translate("InspectorSSR", "Double Reflections", 0));
        m_pDoubleReflectionsCB->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class InspectorSSR: public Ui_InspectorSSR {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_SSR_H
