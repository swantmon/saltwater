/********************************************************************************
** Form generated from reading UI file 'edit_inspector_dof.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_DOF_H
#define UI_EDIT_INSPECTOR_DOF_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorDOF
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QLineEdit *m_pNearToFarRatioEdit;
    QLineEdit *m_pFadeUnSmallEdit;
    QLineEdit *m_pFarEdit;
    QLabel *label_5;
    QLineEdit *m_pFadeSmallMediumEdit;
    QLabel *label_2;
    QLineEdit *m_pNearEdit;
    QLabel *label;
    QLabel *label_4;
    QLabel *label_3;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *InspectorDOF)
    {
        if (InspectorDOF->objectName().isEmpty())
            InspectorDOF->setObjectName(QStringLiteral("InspectorDOF"));
        InspectorDOF->resize(400, 183);
        verticalLayout = new QVBoxLayout(InspectorDOF);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorDOF);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        m_pNearToFarRatioEdit = new QLineEdit(groupBox);
        m_pNearToFarRatioEdit->setObjectName(QStringLiteral("m_pNearToFarRatioEdit"));

        gridLayout->addWidget(m_pNearToFarRatioEdit, 2, 1, 1, 1);

        m_pFadeUnSmallEdit = new QLineEdit(groupBox);
        m_pFadeUnSmallEdit->setObjectName(QStringLiteral("m_pFadeUnSmallEdit"));

        gridLayout->addWidget(m_pFadeUnSmallEdit, 3, 1, 1, 1);

        m_pFarEdit = new QLineEdit(groupBox);
        m_pFarEdit->setObjectName(QStringLiteral("m_pFarEdit"));

        gridLayout->addWidget(m_pFarEdit, 1, 1, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 4, 0, 1, 1);

        m_pFadeSmallMediumEdit = new QLineEdit(groupBox);
        m_pFadeSmallMediumEdit->setObjectName(QStringLiteral("m_pFadeSmallMediumEdit"));

        gridLayout->addWidget(m_pFadeSmallMediumEdit, 4, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        m_pNearEdit = new QLineEdit(groupBox);
        m_pNearEdit->setObjectName(QStringLiteral("m_pNearEdit"));

        gridLayout->addWidget(m_pNearEdit, 0, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);


        verticalLayout_2->addLayout(gridLayout);


        verticalLayout->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(InspectorDOF);
        QObject::connect(m_pNearEdit, SIGNAL(textEdited(QString)), InspectorDOF, SLOT(valueChanged()));
        QObject::connect(m_pFarEdit, SIGNAL(textEdited(QString)), InspectorDOF, SLOT(valueChanged()));
        QObject::connect(m_pNearToFarRatioEdit, SIGNAL(textEdited(QString)), InspectorDOF, SLOT(valueChanged()));
        QObject::connect(m_pFadeUnSmallEdit, SIGNAL(textEdited(QString)), InspectorDOF, SLOT(valueChanged()));
        QObject::connect(m_pFadeSmallMediumEdit, SIGNAL(textEdited(QString)), InspectorDOF, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorDOF);
    } // setupUi

    void retranslateUi(QWidget *InspectorDOF)
    {
        InspectorDOF->setWindowTitle(QApplication::translate("InspectorDOF", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorDOF", "Depth of Field (DOF)", 0));
        label_5->setText(QApplication::translate("InspectorDOF", "Fade Small- to medium blur", 0));
        label_2->setText(QApplication::translate("InspectorDOF", "Far Distance", 0));
        label->setText(QApplication::translate("InspectorDOF", "Near Distance ", 0));
        label_4->setText(QApplication::translate("InspectorDOF", "Fade Un- to Small blur", 0));
        label_3->setText(QApplication::translate("InspectorDOF", "Near to far ratio", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorDOF: public Ui_InspectorDOF {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_DOF_H
