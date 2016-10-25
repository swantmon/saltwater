/********************************************************************************
** Form generated from reading UI file 'edit_inspector_transformation.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_TRANSFORMATION_H
#define UI_EDIT_INSPECTOR_TRANSFORMATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorForm
{
public:
    QWidget *m_pMainFrame;
    QGroupBox *m_pInspectorTransformationGroup;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label;
    QDoubleSpinBox *m_pTransformationPositionX;
    QDoubleSpinBox *m_pTransformationPositionY;
    QDoubleSpinBox *m_pTransformationPositionZ;
    QDoubleSpinBox *m_pTransformationScaleX;
    QDoubleSpinBox *m_pTransformationScaleY;
    QDoubleSpinBox *m_pTransformationScaleZ;
    QDoubleSpinBox *m_pTransformationRotationX;
    QDoubleSpinBox *m_pTransformationRotationY;
    QDoubleSpinBox *m_pTransformationRotationZ;

    void setupUi(QWidget *InspectorForm)
    {
        if (InspectorForm->objectName().isEmpty())
            InspectorForm->setObjectName(QStringLiteral("InspectorForm"));
        InspectorForm->resize(310, 127);
        m_pMainFrame = new QWidget(InspectorForm);
        m_pMainFrame->setObjectName(QStringLiteral("m_pMainFrame"));
        m_pMainFrame->setGeometry(QRect(0, 0, 311, 131));
        m_pInspectorTransformationGroup = new QGroupBox(m_pMainFrame);
        m_pInspectorTransformationGroup->setObjectName(QStringLiteral("m_pInspectorTransformationGroup"));
        m_pInspectorTransformationGroup->setGeometry(QRect(0, 0, 310, 131));
        gridLayoutWidget = new QWidget(m_pInspectorTransformationGroup);
        gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(10, 20, 291, 101));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(gridLayoutWidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label_3 = new QLabel(gridLayoutWidget);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label = new QLabel(gridLayoutWidget);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        m_pTransformationPositionX = new QDoubleSpinBox(gridLayoutWidget);
        m_pTransformationPositionX->setObjectName(QStringLiteral("m_pTransformationPositionX"));

        gridLayout->addWidget(m_pTransformationPositionX, 0, 1, 1, 1);

        m_pTransformationPositionY = new QDoubleSpinBox(gridLayoutWidget);
        m_pTransformationPositionY->setObjectName(QStringLiteral("m_pTransformationPositionY"));

        gridLayout->addWidget(m_pTransformationPositionY, 0, 2, 1, 1);

        m_pTransformationPositionZ = new QDoubleSpinBox(gridLayoutWidget);
        m_pTransformationPositionZ->setObjectName(QStringLiteral("m_pTransformationPositionZ"));

        gridLayout->addWidget(m_pTransformationPositionZ, 0, 3, 1, 1);

        m_pTransformationScaleX = new QDoubleSpinBox(gridLayoutWidget);
        m_pTransformationScaleX->setObjectName(QStringLiteral("m_pTransformationScaleX"));

        gridLayout->addWidget(m_pTransformationScaleX, 1, 1, 1, 1);

        m_pTransformationScaleY = new QDoubleSpinBox(gridLayoutWidget);
        m_pTransformationScaleY->setObjectName(QStringLiteral("m_pTransformationScaleY"));

        gridLayout->addWidget(m_pTransformationScaleY, 1, 2, 1, 1);

        m_pTransformationScaleZ = new QDoubleSpinBox(gridLayoutWidget);
        m_pTransformationScaleZ->setObjectName(QStringLiteral("m_pTransformationScaleZ"));

        gridLayout->addWidget(m_pTransformationScaleZ, 1, 3, 1, 1);

        m_pTransformationRotationX = new QDoubleSpinBox(gridLayoutWidget);
        m_pTransformationRotationX->setObjectName(QStringLiteral("m_pTransformationRotationX"));

        gridLayout->addWidget(m_pTransformationRotationX, 2, 1, 1, 1);

        m_pTransformationRotationY = new QDoubleSpinBox(gridLayoutWidget);
        m_pTransformationRotationY->setObjectName(QStringLiteral("m_pTransformationRotationY"));

        gridLayout->addWidget(m_pTransformationRotationY, 2, 2, 1, 1);

        m_pTransformationRotationZ = new QDoubleSpinBox(gridLayoutWidget);
        m_pTransformationRotationZ->setObjectName(QStringLiteral("m_pTransformationRotationZ"));

        gridLayout->addWidget(m_pTransformationRotationZ, 2, 3, 1, 1);


        retranslateUi(InspectorForm);
        QObject::connect(m_pTransformationPositionX, SIGNAL(valueChanged(double)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationPositionY, SIGNAL(valueChanged(double)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationPositionZ, SIGNAL(valueChanged(double)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationScaleX, SIGNAL(valueChanged(double)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationScaleY, SIGNAL(valueChanged(double)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationScaleZ, SIGNAL(valueChanged(double)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationRotationX, SIGNAL(valueChanged(double)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationRotationY, SIGNAL(valueChanged(double)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationRotationZ, SIGNAL(valueChanged(double)), InspectorForm, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorForm);
    } // setupUi

    void retranslateUi(QWidget *InspectorForm)
    {
        InspectorForm->setWindowTitle(QApplication::translate("InspectorForm", "Form", 0));
        m_pInspectorTransformationGroup->setTitle(QApplication::translate("InspectorForm", "Transformation", 0));
        label_2->setText(QApplication::translate("InspectorForm", "Scale", 0));
        label_3->setText(QApplication::translate("InspectorForm", "Rotation", 0));
        label->setText(QApplication::translate("InspectorForm", "Position", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorForm: public Ui_InspectorForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_TRANSFORMATION_H
