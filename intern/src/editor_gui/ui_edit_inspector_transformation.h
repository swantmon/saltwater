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
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorTransformation
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *m_pInspectorTransformationGroup;
    QGridLayout *gridLayout_2;
    QLabel *label_2;
    QLineEdit *m_pTransformationRotationY;
    QLabel *label_4;
    QLineEdit *m_pTransformationScaleY;
    QLabel *label_11;
    QLabel *label;
    QLabel *label_3;
    QLineEdit *m_pTransformationRotationX;
    QLabel *label_6;
    QLabel *label_10;
    QLabel *label_12;
    QLabel *label_8;
    QLabel *label_5;
    QLabel *label_7;
    QLabel *label_9;
    QLineEdit *m_pTransformationPositionZ;
    QLineEdit *m_pTransformationPositionY;
    QLineEdit *m_pTransformationRotationZ;
    QLineEdit *m_pTransformationPositionX;
    QLineEdit *m_pTransformationScaleX;
    QLineEdit *m_pTransformationScaleZ;

    void setupUi(QWidget *InspectorTransformation)
    {
        if (InspectorTransformation->objectName().isEmpty())
            InspectorTransformation->setObjectName(QStringLiteral("InspectorTransformation"));
        InspectorTransformation->resize(313, 118);
        InspectorTransformation->setMaximumSize(QSize(16777215, 118));
        verticalLayout = new QVBoxLayout(InspectorTransformation);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        m_pInspectorTransformationGroup = new QGroupBox(InspectorTransformation);
        m_pInspectorTransformationGroup->setObjectName(QStringLiteral("m_pInspectorTransformationGroup"));
        m_pInspectorTransformationGroup->setMaximumSize(QSize(16777215, 100));
        gridLayout_2 = new QGridLayout(m_pInspectorTransformationGroup);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_2 = new QLabel(m_pInspectorTransformationGroup);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout_2->addWidget(label_2, 2, 0, 1, 1);

        m_pTransformationRotationY = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationRotationY->setObjectName(QStringLiteral("m_pTransformationRotationY"));

        gridLayout_2->addWidget(m_pTransformationRotationY, 1, 4, 1, 1);

        label_4 = new QLabel(m_pInspectorTransformationGroup);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout_2->addWidget(label_4, 0, 1, 1, 1);

        m_pTransformationScaleY = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationScaleY->setObjectName(QStringLiteral("m_pTransformationScaleY"));

        gridLayout_2->addWidget(m_pTransformationScaleY, 2, 4, 1, 1);

        label_11 = new QLabel(m_pInspectorTransformationGroup);
        label_11->setObjectName(QStringLiteral("label_11"));

        gridLayout_2->addWidget(label_11, 0, 7, 1, 1);

        label = new QLabel(m_pInspectorTransformationGroup);
        label->setObjectName(QStringLiteral("label"));

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        label_3 = new QLabel(m_pInspectorTransformationGroup);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout_2->addWidget(label_3, 1, 0, 1, 1);

        m_pTransformationRotationX = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationRotationX->setObjectName(QStringLiteral("m_pTransformationRotationX"));

        gridLayout_2->addWidget(m_pTransformationRotationX, 1, 2, 1, 1);

        label_6 = new QLabel(m_pInspectorTransformationGroup);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout_2->addWidget(label_6, 1, 1, 1, 1);

        label_10 = new QLabel(m_pInspectorTransformationGroup);
        label_10->setObjectName(QStringLiteral("label_10"));

        gridLayout_2->addWidget(label_10, 1, 7, 1, 1);

        label_12 = new QLabel(m_pInspectorTransformationGroup);
        label_12->setObjectName(QStringLiteral("label_12"));

        gridLayout_2->addWidget(label_12, 2, 7, 1, 1);

        label_8 = new QLabel(m_pInspectorTransformationGroup);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout_2->addWidget(label_8, 0, 3, 1, 1);

        label_5 = new QLabel(m_pInspectorTransformationGroup);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout_2->addWidget(label_5, 2, 1, 1, 1);

        label_7 = new QLabel(m_pInspectorTransformationGroup);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout_2->addWidget(label_7, 1, 3, 1, 1);

        label_9 = new QLabel(m_pInspectorTransformationGroup);
        label_9->setObjectName(QStringLiteral("label_9"));

        gridLayout_2->addWidget(label_9, 2, 3, 1, 1);

        m_pTransformationPositionZ = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationPositionZ->setObjectName(QStringLiteral("m_pTransformationPositionZ"));
        m_pTransformationPositionZ->setDragEnabled(false);

        gridLayout_2->addWidget(m_pTransformationPositionZ, 0, 8, 1, 1);

        m_pTransformationPositionY = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationPositionY->setObjectName(QStringLiteral("m_pTransformationPositionY"));

        gridLayout_2->addWidget(m_pTransformationPositionY, 0, 4, 1, 1);

        m_pTransformationRotationZ = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationRotationZ->setObjectName(QStringLiteral("m_pTransformationRotationZ"));

        gridLayout_2->addWidget(m_pTransformationRotationZ, 1, 8, 1, 1);

        m_pTransformationPositionX = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationPositionX->setObjectName(QStringLiteral("m_pTransformationPositionX"));

        gridLayout_2->addWidget(m_pTransformationPositionX, 0, 2, 1, 1);

        m_pTransformationScaleX = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationScaleX->setObjectName(QStringLiteral("m_pTransformationScaleX"));

        gridLayout_2->addWidget(m_pTransformationScaleX, 2, 2, 1, 1);

        m_pTransformationScaleZ = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationScaleZ->setObjectName(QStringLiteral("m_pTransformationScaleZ"));

        gridLayout_2->addWidget(m_pTransformationScaleZ, 2, 8, 1, 1);


        verticalLayout->addWidget(m_pInspectorTransformationGroup);


        retranslateUi(InspectorTransformation);
        QObject::connect(m_pTransformationPositionX, SIGNAL(textEdited(QString)), InspectorTransformation, SLOT(valueChanged()));
        QObject::connect(m_pTransformationPositionY, SIGNAL(textEdited(QString)), InspectorTransformation, SLOT(valueChanged()));
        QObject::connect(m_pTransformationPositionZ, SIGNAL(textEdited(QString)), InspectorTransformation, SLOT(valueChanged()));
        QObject::connect(m_pTransformationRotationX, SIGNAL(textEdited(QString)), InspectorTransformation, SLOT(valueChanged()));
        QObject::connect(m_pTransformationRotationY, SIGNAL(textEdited(QString)), InspectorTransformation, SLOT(valueChanged()));
        QObject::connect(m_pTransformationRotationZ, SIGNAL(textEdited(QString)), InspectorTransformation, SLOT(valueChanged()));
        QObject::connect(m_pTransformationScaleX, SIGNAL(textEdited(QString)), InspectorTransformation, SLOT(valueChanged()));
        QObject::connect(m_pTransformationScaleY, SIGNAL(textEdited(QString)), InspectorTransformation, SLOT(valueChanged()));
        QObject::connect(m_pTransformationScaleZ, SIGNAL(textEdited(QString)), InspectorTransformation, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorTransformation);
    } // setupUi

    void retranslateUi(QWidget *InspectorTransformation)
    {
        InspectorTransformation->setWindowTitle(QApplication::translate("InspectorTransformation", "Form", 0));
        m_pInspectorTransformationGroup->setTitle(QApplication::translate("InspectorTransformation", "Transformation", 0));
        label_2->setText(QApplication::translate("InspectorTransformation", "Scale", 0));
        m_pTransformationRotationY->setText(QApplication::translate("InspectorTransformation", "0", 0));
        label_4->setText(QApplication::translate("InspectorTransformation", "X", 0));
        m_pTransformationScaleY->setText(QApplication::translate("InspectorTransformation", "1", 0));
        label_11->setText(QApplication::translate("InspectorTransformation", "Z", 0));
        label->setText(QApplication::translate("InspectorTransformation", "Position", 0));
        label_3->setText(QApplication::translate("InspectorTransformation", "Rotation", 0));
        m_pTransformationRotationX->setText(QApplication::translate("InspectorTransformation", "0", 0));
        label_6->setText(QApplication::translate("InspectorTransformation", "X", 0));
        label_10->setText(QApplication::translate("InspectorTransformation", "Z", 0));
        label_12->setText(QApplication::translate("InspectorTransformation", "Z", 0));
        label_8->setText(QApplication::translate("InspectorTransformation", "Y", 0));
        label_5->setText(QApplication::translate("InspectorTransformation", "X", 0));
        label_7->setText(QApplication::translate("InspectorTransformation", "Y", 0));
        label_9->setText(QApplication::translate("InspectorTransformation", "Y", 0));
        m_pTransformationPositionZ->setText(QApplication::translate("InspectorTransformation", "0", 0));
        m_pTransformationPositionY->setText(QApplication::translate("InspectorTransformation", "0", 0));
        m_pTransformationRotationZ->setText(QApplication::translate("InspectorTransformation", "0", 0));
        m_pTransformationPositionX->setText(QApplication::translate("InspectorTransformation", "0", 0));
        m_pTransformationScaleX->setText(QApplication::translate("InspectorTransformation", "1", 0));
        m_pTransformationScaleZ->setText(QApplication::translate("InspectorTransformation", "1", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorTransformation: public Ui_InspectorTransformation {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_TRANSFORMATION_H
