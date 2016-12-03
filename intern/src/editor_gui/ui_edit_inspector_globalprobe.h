/********************************************************************************
** Form generated from reading UI file 'edit_inspector_globalprobe.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_GLOBALPROBE_H
#define UI_EDIT_INSPECTOR_GLOBALPROBE_H

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

class Ui_InspectorGlobalProbe
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QComboBox *m_pRefreshModeCB;
    QComboBox *m_pTypeCB;
    QComboBox *m_pQualityCB;
    QLineEdit *m_pIntensityEdit;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *m_pGenerateButton;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *InspectorGlobalProbe)
    {
        if (InspectorGlobalProbe->objectName().isEmpty())
            InspectorGlobalProbe->setObjectName(QStringLiteral("InspectorGlobalProbe"));
        InspectorGlobalProbe->resize(400, 188);
        verticalLayout = new QVBoxLayout(InspectorGlobalProbe);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorGlobalProbe);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        m_pRefreshModeCB = new QComboBox(groupBox);
        m_pRefreshModeCB->setObjectName(QStringLiteral("m_pRefreshModeCB"));

        gridLayout->addWidget(m_pRefreshModeCB, 0, 1, 1, 1);

        m_pTypeCB = new QComboBox(groupBox);
        m_pTypeCB->setObjectName(QStringLiteral("m_pTypeCB"));

        gridLayout->addWidget(m_pTypeCB, 1, 1, 1, 1);

        m_pQualityCB = new QComboBox(groupBox);
        m_pQualityCB->setObjectName(QStringLiteral("m_pQualityCB"));

        gridLayout->addWidget(m_pQualityCB, 2, 1, 1, 1);

        m_pIntensityEdit = new QLineEdit(groupBox);
        m_pIntensityEdit->setObjectName(QStringLiteral("m_pIntensityEdit"));

        gridLayout->addWidget(m_pIntensityEdit, 3, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        m_pGenerateButton = new QPushButton(groupBox);
        m_pGenerateButton->setObjectName(QStringLiteral("m_pGenerateButton"));

        horizontalLayout->addWidget(m_pGenerateButton);


        verticalLayout_2->addLayout(horizontalLayout);


        verticalLayout->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(InspectorGlobalProbe);
        QObject::connect(m_pGenerateButton, SIGNAL(clicked()), InspectorGlobalProbe, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorGlobalProbe);
    } // setupUi

    void retranslateUi(QWidget *InspectorGlobalProbe)
    {
        InspectorGlobalProbe->setWindowTitle(QApplication::translate("InspectorGlobalProbe", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorGlobalProbe", "Global Light Probe", 0));
        label->setText(QApplication::translate("InspectorGlobalProbe", "Refresh Mode:", 0));
        label_2->setText(QApplication::translate("InspectorGlobalProbe", "Type:", 0));
        label_3->setText(QApplication::translate("InspectorGlobalProbe", "Quality:", 0));
        label_4->setText(QApplication::translate("InspectorGlobalProbe", "Intensity:", 0));
        m_pRefreshModeCB->clear();
        m_pRefreshModeCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorGlobalProbe", "Static", 0)
         << QApplication::translate("InspectorGlobalProbe", "Dynamic", 0)
        );
        m_pTypeCB->clear();
        m_pTypeCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorGlobalProbe", "Sky", 0)
         << QApplication::translate("InspectorGlobalProbe", "Custom", 0)
        );
        m_pQualityCB->clear();
        m_pQualityCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorGlobalProbe", "128", 0)
         << QApplication::translate("InspectorGlobalProbe", "256", 0)
         << QApplication::translate("InspectorGlobalProbe", "512", 0)
         << QApplication::translate("InspectorGlobalProbe", "1024", 0)
         << QApplication::translate("InspectorGlobalProbe", "2048", 0)
        );
        m_pIntensityEdit->setText(QApplication::translate("InspectorGlobalProbe", "1.0", 0));
        m_pGenerateButton->setText(QApplication::translate("InspectorGlobalProbe", "Generate", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorGlobalProbe: public Ui_InspectorGlobalProbe {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_GLOBALPROBE_H
