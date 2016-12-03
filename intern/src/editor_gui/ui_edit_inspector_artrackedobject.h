/********************************************************************************
** Form generated from reading UI file 'edit_inspector_artrackedobject.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_ARTRACKEDOBJECT_H
#define UI_EDIT_INSPECTOR_ARTRACKEDOBJECT_H

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
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Form
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QLabel *label_3;
    QLabel *label_2;
    QLabel *label;
    QLineEdit *m_pUIDEdit;
    QLineEdit *m_pAppearCounterEdit;
    QCheckBox *m_pIsFoundCB;

    void setupUi(QWidget *Form)
    {
        if (Form->objectName().isEmpty())
            Form->setObjectName(QStringLiteral("Form"));
        Form->resize(201, 118);
        verticalLayout = new QVBoxLayout(Form);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(Form);
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

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        m_pUIDEdit = new QLineEdit(groupBox);
        m_pUIDEdit->setObjectName(QStringLiteral("m_pUIDEdit"));

        gridLayout->addWidget(m_pUIDEdit, 0, 1, 1, 1);

        m_pAppearCounterEdit = new QLineEdit(groupBox);
        m_pAppearCounterEdit->setObjectName(QStringLiteral("m_pAppearCounterEdit"));
        m_pAppearCounterEdit->setReadOnly(true);

        gridLayout->addWidget(m_pAppearCounterEdit, 1, 1, 1, 1);

        m_pIsFoundCB = new QCheckBox(groupBox);
        m_pIsFoundCB->setObjectName(QStringLiteral("m_pIsFoundCB"));

        gridLayout->addWidget(m_pIsFoundCB, 2, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout);


        verticalLayout->addWidget(groupBox);


        retranslateUi(Form);

        QMetaObject::connectSlotsByName(Form);
    } // setupUi

    void retranslateUi(QWidget *Form)
    {
        Form->setWindowTitle(QApplication::translate("Form", "Form", 0));
        groupBox->setTitle(QApplication::translate("Form", "AR Tracked Object", 0));
        label_3->setText(QApplication::translate("Form", "Is Found", 0));
        label_2->setText(QApplication::translate("Form", "Appear Counter", 0));
        label->setText(QApplication::translate("Form", "UID", 0));
        m_pUIDEdit->setText(QApplication::translate("Form", "0", 0));
        m_pAppearCounterEdit->setText(QApplication::translate("Form", "0", 0));
        m_pIsFoundCB->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Form: public Ui_Form {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_ARTRACKEDOBJECT_H
