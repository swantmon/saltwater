/********************************************************************************
** Form generated from reading UI file 'edit_inspector_entity.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_ENTITY_H
#define UI_EDIT_INSPECTOR_ENTITY_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorEntity
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QGraphicsView *graphicsView;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *m_pEntityNameEdit;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLineEdit *m_pEntityIDEdit;

    void setupUi(QWidget *InspectorEntity)
    {
        if (InspectorEntity->objectName().isEmpty())
            InspectorEntity->setObjectName(QStringLiteral("InspectorEntity"));
        InspectorEntity->resize(400, 70);
        InspectorEntity->setAutoFillBackground(false);
        InspectorEntity->setStyleSheet(QStringLiteral(""));
        verticalLayout = new QVBoxLayout(InspectorEntity);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        graphicsView = new QGraphicsView(InspectorEntity);
        graphicsView->setObjectName(QStringLiteral("graphicsView"));
        graphicsView->setMaximumSize(QSize(50, 50));

        horizontalLayout->addWidget(graphicsView);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        m_pEntityNameEdit = new QLineEdit(InspectorEntity);
        m_pEntityNameEdit->setObjectName(QStringLiteral("m_pEntityNameEdit"));

        verticalLayout_2->addWidget(m_pEntityNameEdit);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label = new QLabel(InspectorEntity);
        label->setObjectName(QStringLiteral("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(label);

        m_pEntityIDEdit = new QLineEdit(InspectorEntity);
        m_pEntityIDEdit->setObjectName(QStringLiteral("m_pEntityIDEdit"));
        m_pEntityIDEdit->setMaximumSize(QSize(50, 16777215));
        m_pEntityIDEdit->setReadOnly(true);

        horizontalLayout_2->addWidget(m_pEntityIDEdit);


        verticalLayout_2->addLayout(horizontalLayout_2);


        horizontalLayout->addLayout(verticalLayout_2);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(InspectorEntity);
        QObject::connect(m_pEntityNameEdit, SIGNAL(textEdited(QString)), InspectorEntity, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorEntity);
    } // setupUi

    void retranslateUi(QWidget *InspectorEntity)
    {
        InspectorEntity->setWindowTitle(QApplication::translate("InspectorEntity", "Entity", 0));
        label->setText(QApplication::translate("InspectorEntity", "ID", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorEntity: public Ui_InspectorEntity {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_ENTITY_H
