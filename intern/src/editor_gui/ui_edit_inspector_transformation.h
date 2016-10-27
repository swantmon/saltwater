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
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorForm
{
public:
    QWidget *m_pMainFrame;
    QGroupBox *m_pInspectorTransformationGroup;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLineEdit *m_pTransformationPositionX;
    QLineEdit *m_pTransformationPositionZ;
    QLineEdit *m_pTransformationRotationY;
    QLineEdit *m_pTransformationPositionY;
    QLabel *label_3;
    QLabel *label_4;
    QLineEdit *m_pTransformationRotationZ;
    QLabel *label;
    QLineEdit *m_pTransformationRotationX;
    QLabel *label_9;
    QLabel *label_6;
    QLabel *label_12;
    QLabel *label_7;
    QLabel *label_10;
    QLabel *label_2;
    QLabel *label_5;
    QLineEdit *m_pTransformationScaleX;
    QLineEdit *m_pTransformationScaleY;
    QLabel *label_8;
    QLineEdit *m_pTransformationScaleZ;
    QLabel *label_11;

    void setupUi(QWidget *InspectorForm)
    {
        if (InspectorForm->objectName().isEmpty())
            InspectorForm->setObjectName(QStringLiteral("InspectorForm"));
        InspectorForm->resize(393, 137);
        m_pMainFrame = new QWidget(InspectorForm);
        m_pMainFrame->setObjectName(QStringLiteral("m_pMainFrame"));
        m_pMainFrame->setGeometry(QRect(0, 0, 391, 131));
        m_pInspectorTransformationGroup = new QGroupBox(m_pMainFrame);
        m_pInspectorTransformationGroup->setObjectName(QStringLiteral("m_pInspectorTransformationGroup"));
        m_pInspectorTransformationGroup->setGeometry(QRect(0, 0, 381, 131));
        gridLayoutWidget = new QWidget(m_pInspectorTransformationGroup);
        gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(10, 20, 368, 101));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        m_pTransformationPositionX = new QLineEdit(gridLayoutWidget);
        m_pTransformationPositionX->setObjectName(QStringLiteral("m_pTransformationPositionX"));

        gridLayout->addWidget(m_pTransformationPositionX, 0, 2, 1, 1);

        m_pTransformationPositionZ = new QLineEdit(gridLayoutWidget);
        m_pTransformationPositionZ->setObjectName(QStringLiteral("m_pTransformationPositionZ"));
        m_pTransformationPositionZ->setDragEnabled(false);

        gridLayout->addWidget(m_pTransformationPositionZ, 0, 6, 1, 1);

        m_pTransformationRotationY = new QLineEdit(gridLayoutWidget);
        m_pTransformationRotationY->setObjectName(QStringLiteral("m_pTransformationRotationY"));

        gridLayout->addWidget(m_pTransformationRotationY, 2, 4, 1, 1);

        m_pTransformationPositionY = new QLineEdit(gridLayoutWidget);
        m_pTransformationPositionY->setObjectName(QStringLiteral("m_pTransformationPositionY"));

        gridLayout->addWidget(m_pTransformationPositionY, 0, 4, 1, 1);

        label_3 = new QLabel(gridLayoutWidget);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_4 = new QLabel(gridLayoutWidget);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 0, 1, 1, 1);

        m_pTransformationRotationZ = new QLineEdit(gridLayoutWidget);
        m_pTransformationRotationZ->setObjectName(QStringLiteral("m_pTransformationRotationZ"));

        gridLayout->addWidget(m_pTransformationRotationZ, 2, 6, 1, 1);

        label = new QLabel(gridLayoutWidget);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        m_pTransformationRotationX = new QLineEdit(gridLayoutWidget);
        m_pTransformationRotationX->setObjectName(QStringLiteral("m_pTransformationRotationX"));

        gridLayout->addWidget(m_pTransformationRotationX, 2, 2, 1, 1);

        label_9 = new QLabel(gridLayoutWidget);
        label_9->setObjectName(QStringLiteral("label_9"));

        gridLayout->addWidget(label_9, 2, 3, 1, 1);

        label_6 = new QLabel(gridLayoutWidget);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout->addWidget(label_6, 2, 1, 1, 1);

        label_12 = new QLabel(gridLayoutWidget);
        label_12->setObjectName(QStringLiteral("label_12"));

        gridLayout->addWidget(label_12, 2, 5, 1, 1);

        label_7 = new QLabel(gridLayoutWidget);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout->addWidget(label_7, 0, 3, 1, 1);

        label_10 = new QLabel(gridLayoutWidget);
        label_10->setObjectName(QStringLiteral("label_10"));

        gridLayout->addWidget(label_10, 0, 5, 1, 1);

        label_2 = new QLabel(gridLayoutWidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 3, 0, 1, 1);

        label_5 = new QLabel(gridLayoutWidget);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 3, 1, 1, 1);

        m_pTransformationScaleX = new QLineEdit(gridLayoutWidget);
        m_pTransformationScaleX->setObjectName(QStringLiteral("m_pTransformationScaleX"));

        gridLayout->addWidget(m_pTransformationScaleX, 3, 2, 1, 1);

        m_pTransformationScaleY = new QLineEdit(gridLayoutWidget);
        m_pTransformationScaleY->setObjectName(QStringLiteral("m_pTransformationScaleY"));

        gridLayout->addWidget(m_pTransformationScaleY, 3, 4, 1, 1);

        label_8 = new QLabel(gridLayoutWidget);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 3, 3, 1, 1);

        m_pTransformationScaleZ = new QLineEdit(gridLayoutWidget);
        m_pTransformationScaleZ->setObjectName(QStringLiteral("m_pTransformationScaleZ"));

        gridLayout->addWidget(m_pTransformationScaleZ, 3, 6, 1, 1);

        label_11 = new QLabel(gridLayoutWidget);
        label_11->setObjectName(QStringLiteral("label_11"));

        gridLayout->addWidget(label_11, 3, 5, 1, 1);

        QWidget::setTabOrder(m_pTransformationPositionX, m_pTransformationPositionY);
        QWidget::setTabOrder(m_pTransformationPositionY, m_pTransformationPositionZ);
        QWidget::setTabOrder(m_pTransformationPositionZ, m_pTransformationRotationX);
        QWidget::setTabOrder(m_pTransformationRotationX, m_pTransformationRotationY);
        QWidget::setTabOrder(m_pTransformationRotationY, m_pTransformationRotationZ);
        QWidget::setTabOrder(m_pTransformationRotationZ, m_pTransformationScaleX);
        QWidget::setTabOrder(m_pTransformationScaleX, m_pTransformationScaleY);
        QWidget::setTabOrder(m_pTransformationScaleY, m_pTransformationScaleZ);

        retranslateUi(InspectorForm);
        QObject::connect(m_pTransformationPositionX, SIGNAL(textEdited(QString)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationPositionY, SIGNAL(textEdited(QString)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationPositionZ, SIGNAL(textEdited(QString)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationRotationX, SIGNAL(textEdited(QString)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationRotationY, SIGNAL(textEdited(QString)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationRotationZ, SIGNAL(textEdited(QString)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationScaleX, SIGNAL(textEdited(QString)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationScaleY, SIGNAL(textEdited(QString)), InspectorForm, SLOT(valueChanged()));
        QObject::connect(m_pTransformationScaleZ, SIGNAL(textEdited(QString)), InspectorForm, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorForm);
    } // setupUi

    void retranslateUi(QWidget *InspectorForm)
    {
        InspectorForm->setWindowTitle(QApplication::translate("InspectorForm", "Form", 0));
        m_pInspectorTransformationGroup->setTitle(QApplication::translate("InspectorForm", "Transformation", 0));
        m_pTransformationPositionX->setText(QApplication::translate("InspectorForm", "0", 0));
        m_pTransformationPositionZ->setText(QApplication::translate("InspectorForm", "0", 0));
        m_pTransformationRotationY->setText(QApplication::translate("InspectorForm", "0", 0));
        m_pTransformationPositionY->setText(QApplication::translate("InspectorForm", "0", 0));
        label_3->setText(QApplication::translate("InspectorForm", "Rotation", 0));
        label_4->setText(QApplication::translate("InspectorForm", "X", 0));
        m_pTransformationRotationZ->setText(QApplication::translate("InspectorForm", "0", 0));
        label->setText(QApplication::translate("InspectorForm", "Position", 0));
        m_pTransformationRotationX->setText(QApplication::translate("InspectorForm", "0", 0));
        label_9->setText(QApplication::translate("InspectorForm", "Y", 0));
        label_6->setText(QApplication::translate("InspectorForm", "X", 0));
        label_12->setText(QApplication::translate("InspectorForm", "Z", 0));
        label_7->setText(QApplication::translate("InspectorForm", "Y", 0));
        label_10->setText(QApplication::translate("InspectorForm", "Z", 0));
        label_2->setText(QApplication::translate("InspectorForm", "Scale", 0));
        label_5->setText(QApplication::translate("InspectorForm", "X", 0));
        m_pTransformationScaleX->setText(QApplication::translate("InspectorForm", "1", 0));
        m_pTransformationScaleY->setText(QApplication::translate("InspectorForm", "1", 0));
        label_8->setText(QApplication::translate("InspectorForm", "Y", 0));
        m_pTransformationScaleZ->setText(QApplication::translate("InspectorForm", "1", 0));
        label_11->setText(QApplication::translate("InspectorForm", "Z", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorForm: public Ui_InspectorForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_TRANSFORMATION_H
