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
#include "editor_gui/edit_clickable_label.h"

QT_BEGIN_NAMESPACE

class Ui_InspectorTransformation
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *m_pInspectorTransformationGroup;
    QGridLayout *gridLayout_2;
    Edit::CClickableLabel *m_pPositionXLabel;
    Edit::CClickableLabel *m_pRotationXLabel;
    QLineEdit *m_pTransformationPositionZ;
    QLineEdit *m_pTransformationScaleZ;
    QLineEdit *m_pTransformationRotationZ;
    QLineEdit *m_pTransformationScaleY;
    Edit::CClickableLabel *m_pPositionYLabel;
    Edit::CClickableLabel *m_pScaleYLabel;
    QLabel *label_3;
    Edit::CClickableLabel *m_pScaleZLabel;
    QLineEdit *m_pTransformationRotationY;
    QLabel *label_2;
    QLineEdit *m_pTransformationRotationX;
    QLineEdit *m_pTransformationScaleX;
    Edit::CClickableLabel *m_pRotationZLabel;
    Edit::CClickableLabel *m_pPositionZLabel;
    QLineEdit *m_pTransformationPositionX;
    Edit::CClickableLabel *m_pRotationYLabel;
    QLabel *label;
    QLineEdit *m_pTransformationPositionY;
    Edit::CClickableLabel *m_pScaleXLabel;

    void setupUi(QWidget *InspectorTransformation)
    {
        if (InspectorTransformation->objectName().isEmpty())
            InspectorTransformation->setObjectName(QStringLiteral("InspectorTransformation"));
        InspectorTransformation->resize(295, 118);
        InspectorTransformation->setMaximumSize(QSize(16777215, 118));
        verticalLayout = new QVBoxLayout(InspectorTransformation);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        m_pInspectorTransformationGroup = new QGroupBox(InspectorTransformation);
        m_pInspectorTransformationGroup->setObjectName(QStringLiteral("m_pInspectorTransformationGroup"));
        m_pInspectorTransformationGroup->setMaximumSize(QSize(16777215, 100));
        gridLayout_2 = new QGridLayout(m_pInspectorTransformationGroup);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        m_pPositionXLabel = new Edit::CClickableLabel(m_pInspectorTransformationGroup);
        m_pPositionXLabel->setObjectName(QStringLiteral("m_pPositionXLabel"));
        m_pPositionXLabel->setCursor(QCursor(Qt::SizeHorCursor));

        gridLayout_2->addWidget(m_pPositionXLabel, 0, 1, 1, 1);

        m_pRotationXLabel = new Edit::CClickableLabel(m_pInspectorTransformationGroup);
        m_pRotationXLabel->setObjectName(QStringLiteral("m_pRotationXLabel"));
        m_pRotationXLabel->setCursor(QCursor(Qt::SizeHorCursor));

        gridLayout_2->addWidget(m_pRotationXLabel, 1, 1, 1, 1);

        m_pTransformationPositionZ = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationPositionZ->setObjectName(QStringLiteral("m_pTransformationPositionZ"));
        m_pTransformationPositionZ->setDragEnabled(false);

        gridLayout_2->addWidget(m_pTransformationPositionZ, 0, 8, 1, 1);

        m_pTransformationScaleZ = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationScaleZ->setObjectName(QStringLiteral("m_pTransformationScaleZ"));

        gridLayout_2->addWidget(m_pTransformationScaleZ, 2, 8, 1, 1);

        m_pTransformationRotationZ = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationRotationZ->setObjectName(QStringLiteral("m_pTransformationRotationZ"));

        gridLayout_2->addWidget(m_pTransformationRotationZ, 1, 8, 1, 1);

        m_pTransformationScaleY = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationScaleY->setObjectName(QStringLiteral("m_pTransformationScaleY"));

        gridLayout_2->addWidget(m_pTransformationScaleY, 2, 4, 1, 1);

        m_pPositionYLabel = new Edit::CClickableLabel(m_pInspectorTransformationGroup);
        m_pPositionYLabel->setObjectName(QStringLiteral("m_pPositionYLabel"));
        m_pPositionYLabel->setCursor(QCursor(Qt::SizeHorCursor));

        gridLayout_2->addWidget(m_pPositionYLabel, 0, 3, 1, 1);

        m_pScaleYLabel = new Edit::CClickableLabel(m_pInspectorTransformationGroup);
        m_pScaleYLabel->setObjectName(QStringLiteral("m_pScaleYLabel"));
        m_pScaleYLabel->setCursor(QCursor(Qt::SizeHorCursor));

        gridLayout_2->addWidget(m_pScaleYLabel, 2, 3, 1, 1);

        label_3 = new QLabel(m_pInspectorTransformationGroup);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout_2->addWidget(label_3, 1, 0, 1, 1);

        m_pScaleZLabel = new Edit::CClickableLabel(m_pInspectorTransformationGroup);
        m_pScaleZLabel->setObjectName(QStringLiteral("m_pScaleZLabel"));
        m_pScaleZLabel->setCursor(QCursor(Qt::SizeHorCursor));

        gridLayout_2->addWidget(m_pScaleZLabel, 2, 7, 1, 1);

        m_pTransformationRotationY = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationRotationY->setObjectName(QStringLiteral("m_pTransformationRotationY"));

        gridLayout_2->addWidget(m_pTransformationRotationY, 1, 4, 1, 1);

        label_2 = new QLabel(m_pInspectorTransformationGroup);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout_2->addWidget(label_2, 2, 0, 1, 1);

        m_pTransformationRotationX = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationRotationX->setObjectName(QStringLiteral("m_pTransformationRotationX"));

        gridLayout_2->addWidget(m_pTransformationRotationX, 1, 2, 1, 1);

        m_pTransformationScaleX = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationScaleX->setObjectName(QStringLiteral("m_pTransformationScaleX"));

        gridLayout_2->addWidget(m_pTransformationScaleX, 2, 2, 1, 1);

        m_pRotationZLabel = new Edit::CClickableLabel(m_pInspectorTransformationGroup);
        m_pRotationZLabel->setObjectName(QStringLiteral("m_pRotationZLabel"));
        m_pRotationZLabel->setCursor(QCursor(Qt::SizeHorCursor));

        gridLayout_2->addWidget(m_pRotationZLabel, 1, 7, 1, 1);

        m_pPositionZLabel = new Edit::CClickableLabel(m_pInspectorTransformationGroup);
        m_pPositionZLabel->setObjectName(QStringLiteral("m_pPositionZLabel"));
        m_pPositionZLabel->setCursor(QCursor(Qt::SizeHorCursor));

        gridLayout_2->addWidget(m_pPositionZLabel, 0, 7, 1, 1);

        m_pTransformationPositionX = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationPositionX->setObjectName(QStringLiteral("m_pTransformationPositionX"));

        gridLayout_2->addWidget(m_pTransformationPositionX, 0, 2, 1, 1);

        m_pRotationYLabel = new Edit::CClickableLabel(m_pInspectorTransformationGroup);
        m_pRotationYLabel->setObjectName(QStringLiteral("m_pRotationYLabel"));
        m_pRotationYLabel->setCursor(QCursor(Qt::SizeHorCursor));

        gridLayout_2->addWidget(m_pRotationYLabel, 1, 3, 1, 1);

        label = new QLabel(m_pInspectorTransformationGroup);
        label->setObjectName(QStringLiteral("label"));

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        m_pTransformationPositionY = new QLineEdit(m_pInspectorTransformationGroup);
        m_pTransformationPositionY->setObjectName(QStringLiteral("m_pTransformationPositionY"));

        gridLayout_2->addWidget(m_pTransformationPositionY, 0, 4, 1, 1);

        m_pScaleXLabel = new Edit::CClickableLabel(m_pInspectorTransformationGroup);
        m_pScaleXLabel->setObjectName(QStringLiteral("m_pScaleXLabel"));
        m_pScaleXLabel->setCursor(QCursor(Qt::SizeHorCursor));

        gridLayout_2->addWidget(m_pScaleXLabel, 2, 1, 1, 1);


        verticalLayout->addWidget(m_pInspectorTransformationGroup);

        QWidget::setTabOrder(m_pTransformationPositionX, m_pTransformationPositionY);
        QWidget::setTabOrder(m_pTransformationPositionY, m_pTransformationPositionZ);
        QWidget::setTabOrder(m_pTransformationPositionZ, m_pTransformationRotationX);
        QWidget::setTabOrder(m_pTransformationRotationX, m_pTransformationRotationY);
        QWidget::setTabOrder(m_pTransformationRotationY, m_pTransformationRotationZ);
        QWidget::setTabOrder(m_pTransformationRotationZ, m_pTransformationScaleX);
        QWidget::setTabOrder(m_pTransformationScaleX, m_pTransformationScaleY);
        QWidget::setTabOrder(m_pTransformationScaleY, m_pTransformationScaleZ);

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
        QObject::connect(m_pPositionXLabel, SIGNAL(mousePositionDifference(QPoint)), InspectorTransformation, SLOT(changePositionX(QPoint)));
        QObject::connect(m_pPositionYLabel, SIGNAL(mousePositionDifference(QPoint)), InspectorTransformation, SLOT(changePositionY(QPoint)));
        QObject::connect(m_pPositionZLabel, SIGNAL(mousePositionDifference(QPoint)), InspectorTransformation, SLOT(changePositionZ(QPoint)));
        QObject::connect(m_pRotationXLabel, SIGNAL(mousePositionDifference(QPoint)), InspectorTransformation, SLOT(changeRotationX(QPoint)));
        QObject::connect(m_pRotationYLabel, SIGNAL(mousePositionDifference(QPoint)), InspectorTransformation, SLOT(changeRotationY(QPoint)));
        QObject::connect(m_pRotationZLabel, SIGNAL(mousePositionDifference(QPoint)), InspectorTransformation, SLOT(changeRotationZ(QPoint)));
        QObject::connect(m_pScaleXLabel, SIGNAL(mousePositionDifference(QPoint)), InspectorTransformation, SLOT(changeScaleX(QPoint)));
        QObject::connect(m_pScaleYLabel, SIGNAL(mousePositionDifference(QPoint)), InspectorTransformation, SLOT(changeScaleY(QPoint)));
        QObject::connect(m_pScaleZLabel, SIGNAL(mousePositionDifference(QPoint)), InspectorTransformation, SLOT(changeScaleZ(QPoint)));

        QMetaObject::connectSlotsByName(InspectorTransformation);
    } // setupUi

    void retranslateUi(QWidget *InspectorTransformation)
    {
        InspectorTransformation->setWindowTitle(QApplication::translate("InspectorTransformation", "Form", 0));
        m_pInspectorTransformationGroup->setTitle(QApplication::translate("InspectorTransformation", "Transformation", 0));
        m_pPositionXLabel->setText(QApplication::translate("InspectorTransformation", "X", 0));
        m_pRotationXLabel->setText(QApplication::translate("InspectorTransformation", "X", 0));
        m_pTransformationPositionZ->setText(QApplication::translate("InspectorTransformation", "0", 0));
        m_pTransformationScaleZ->setText(QApplication::translate("InspectorTransformation", "1", 0));
        m_pTransformationRotationZ->setText(QApplication::translate("InspectorTransformation", "0", 0));
        m_pTransformationScaleY->setText(QApplication::translate("InspectorTransformation", "1", 0));
        m_pPositionYLabel->setText(QApplication::translate("InspectorTransformation", "Y", 0));
        m_pScaleYLabel->setText(QApplication::translate("InspectorTransformation", "Y", 0));
        label_3->setText(QApplication::translate("InspectorTransformation", "Rotation", 0));
        m_pScaleZLabel->setText(QApplication::translate("InspectorTransformation", "Z", 0));
        m_pTransformationRotationY->setText(QApplication::translate("InspectorTransformation", "0", 0));
        label_2->setText(QApplication::translate("InspectorTransformation", "Scale", 0));
        m_pTransformationRotationX->setText(QApplication::translate("InspectorTransformation", "0", 0));
        m_pTransformationScaleX->setText(QApplication::translate("InspectorTransformation", "1", 0));
        m_pRotationZLabel->setText(QApplication::translate("InspectorTransformation", "Z", 0));
        m_pPositionZLabel->setText(QApplication::translate("InspectorTransformation", "Z", 0));
        m_pTransformationPositionX->setText(QApplication::translate("InspectorTransformation", "0", 0));
        m_pRotationYLabel->setText(QApplication::translate("InspectorTransformation", "Y", 0));
        label->setText(QApplication::translate("InspectorTransformation", "Position", 0));
        m_pTransformationPositionY->setText(QApplication::translate("InspectorTransformation", "0", 0));
        m_pScaleXLabel->setText(QApplication::translate("InspectorTransformation", "X", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorTransformation: public Ui_InspectorTransformation {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_TRANSFORMATION_H
