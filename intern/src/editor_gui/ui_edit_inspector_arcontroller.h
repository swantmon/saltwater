/********************************************************************************
** Form generated from reading UI file 'edit_inspector_arcontroller.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
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
    QLineEdit *m_pOutputCubemapEdit;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *m_pParameterFile;
    QLineEdit *m_pCameraEntityIDEdit;
    QLabel *label_3;
    QLabel *label_5;
    QLabel *label_7;
    QLineEdit *m_pOutputBackgroundEdit;
    QLineEdit *m_pNumberOfMarkerEdit;
    QLabel *label_8;
    QLineEdit *m_pConfigurationEdit;
    QComboBox *m_pDeviceCB;
    QLabel *label_6;
    QVBoxLayout *m_pMarkerLayout;

    void setupUi(QWidget *InspectorARController)
    {
        if (InspectorARController->objectName().isEmpty())
            InspectorARController->setObjectName(QStringLiteral("InspectorARController"));
        InspectorARController->resize(303, 237);
        verticalLayout = new QVBoxLayout(InspectorARController);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorARController);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_3 = new QVBoxLayout(groupBox);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        m_pOutputCubemapEdit = new QLineEdit(groupBox);
        m_pOutputCubemapEdit->setObjectName(QStringLiteral("m_pOutputCubemapEdit"));
        m_pOutputCubemapEdit->setReadOnly(true);

        gridLayout_2->addWidget(m_pOutputCubemapEdit, 5, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout_2->addWidget(label, 3, 0, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout_2->addWidget(label_2, 0, 0, 1, 1);

        m_pParameterFile = new QLineEdit(groupBox);
        m_pParameterFile->setObjectName(QStringLiteral("m_pParameterFile"));

        gridLayout_2->addWidget(m_pParameterFile, 2, 1, 1, 1);

        m_pCameraEntityIDEdit = new QLineEdit(groupBox);
        m_pCameraEntityIDEdit->setObjectName(QStringLiteral("m_pCameraEntityIDEdit"));

        gridLayout_2->addWidget(m_pCameraEntityIDEdit, 3, 1, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout_2->addWidget(label_3, 2, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout_2->addWidget(label_5, 4, 0, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout_2->addWidget(label_7, 6, 0, 1, 1);

        m_pOutputBackgroundEdit = new QLineEdit(groupBox);
        m_pOutputBackgroundEdit->setObjectName(QStringLiteral("m_pOutputBackgroundEdit"));
        m_pOutputBackgroundEdit->setReadOnly(true);

        gridLayout_2->addWidget(m_pOutputBackgroundEdit, 4, 1, 1, 1);

        m_pNumberOfMarkerEdit = new QLineEdit(groupBox);
        m_pNumberOfMarkerEdit->setObjectName(QStringLiteral("m_pNumberOfMarkerEdit"));

        gridLayout_2->addWidget(m_pNumberOfMarkerEdit, 6, 1, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout_2->addWidget(label_8, 1, 0, 1, 1);

        m_pConfigurationEdit = new QLineEdit(groupBox);
        m_pConfigurationEdit->setObjectName(QStringLiteral("m_pConfigurationEdit"));

        gridLayout_2->addWidget(m_pConfigurationEdit, 1, 1, 1, 1);

        m_pDeviceCB = new QComboBox(groupBox);
        m_pDeviceCB->setObjectName(QStringLiteral("m_pDeviceCB"));

        gridLayout_2->addWidget(m_pDeviceCB, 0, 1, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout_2->addWidget(label_6, 5, 0, 1, 1);


        verticalLayout_3->addLayout(gridLayout_2);

        m_pMarkerLayout = new QVBoxLayout();
        m_pMarkerLayout->setSpacing(6);
        m_pMarkerLayout->setObjectName(QStringLiteral("m_pMarkerLayout"));

        verticalLayout_3->addLayout(m_pMarkerLayout);


        verticalLayout->addWidget(groupBox);

        QWidget::setTabOrder(m_pDeviceCB, m_pConfigurationEdit);
        QWidget::setTabOrder(m_pConfigurationEdit, m_pParameterFile);
        QWidget::setTabOrder(m_pParameterFile, m_pCameraEntityIDEdit);
        QWidget::setTabOrder(m_pCameraEntityIDEdit, m_pOutputBackgroundEdit);
        QWidget::setTabOrder(m_pOutputBackgroundEdit, m_pOutputCubemapEdit);
        QWidget::setTabOrder(m_pOutputCubemapEdit, m_pNumberOfMarkerEdit);

        retranslateUi(InspectorARController);
        QObject::connect(m_pDeviceCB, SIGNAL(currentIndexChanged(int)), InspectorARController, SLOT(valueChanged()));
        QObject::connect(m_pParameterFile, SIGNAL(textEdited(QString)), InspectorARController, SLOT(valueChanged()));
        QObject::connect(m_pCameraEntityIDEdit, SIGNAL(textEdited(QString)), InspectorARController, SLOT(valueChanged()));
        QObject::connect(m_pOutputBackgroundEdit, SIGNAL(textEdited(QString)), InspectorARController, SLOT(valueChanged()));
        QObject::connect(m_pOutputCubemapEdit, SIGNAL(textEdited(QString)), InspectorARController, SLOT(valueChanged()));
        QObject::connect(m_pNumberOfMarkerEdit, SIGNAL(textEdited(QString)), InspectorARController, SLOT(valueChanged()));
        QObject::connect(m_pNumberOfMarkerEdit, SIGNAL(textChanged(QString)), InspectorARController, SLOT(numberOfMarkerChanged()));
        QObject::connect(m_pConfigurationEdit, SIGNAL(textEdited(QString)), InspectorARController, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorARController);
    } // setupUi

    void retranslateUi(QWidget *InspectorARController)
    {
        InspectorARController->setWindowTitle(QApplication::translate("InspectorARController", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorARController", "AR Controller", 0));
        m_pOutputCubemapEdit->setText(QString());
        label->setText(QApplication::translate("InspectorARController", "Camera Entity ID", 0));
        label_2->setText(QApplication::translate("InspectorARController", "Device", 0));
        m_pParameterFile->setText(QApplication::translate("InspectorARController", "../assets/", 0));
        m_pCameraEntityIDEdit->setText(QString());
        label_3->setText(QApplication::translate("InspectorARController", "Parameter File", 0));
        label_5->setText(QApplication::translate("InspectorARController", "Output Background", 0));
        label_7->setText(QApplication::translate("InspectorARController", "Number of Marker", 0));
        m_pOutputBackgroundEdit->setText(QString());
        m_pNumberOfMarkerEdit->setText(QString());
        label_8->setText(QApplication::translate("InspectorARController", "Configuration", 0));
        m_pConfigurationEdit->setText(QApplication::translate("InspectorARController", "-device=WinDS -showDialog -flipV -devNum=1", 0));
        m_pDeviceCB->clear();
        m_pDeviceCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorARController", "Webcam", 0)
         << QApplication::translate("InspectorARController", "Kinect", 0)
        );
        label_6->setText(QApplication::translate("InspectorARController", "Output Cubemap", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorARController: public Ui_InspectorARController {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_ARCONTROLLER_H
