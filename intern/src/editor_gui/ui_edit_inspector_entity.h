/********************************************************************************
** Form generated from reading UI file 'edit_inspector_entity.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_ENTITY_H
#define UI_EDIT_INSPECTOR_ENTITY_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
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
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *m_pEnabledCB;
    QLineEdit *m_pEntityNameEdit;
    QLabel *label;
    QLineEdit *m_pEntityIDEdit;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QLabel *label_2;
    QComboBox *m_pLayerCB;
    QFrame *line;
    QLabel *label_3;
    QComboBox *m_pCategoryCB;

    void setupUi(QWidget *InspectorEntity)
    {
        if (InspectorEntity->objectName().isEmpty())
            InspectorEntity->setObjectName(QStringLiteral("InspectorEntity"));
        InspectorEntity->resize(400, 72);
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
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        m_pEnabledCB = new QCheckBox(InspectorEntity);
        m_pEnabledCB->setObjectName(QStringLiteral("m_pEnabledCB"));

        horizontalLayout_3->addWidget(m_pEnabledCB);

        m_pEntityNameEdit = new QLineEdit(InspectorEntity);
        m_pEntityNameEdit->setObjectName(QStringLiteral("m_pEntityNameEdit"));

        horizontalLayout_3->addWidget(m_pEntityNameEdit);

        label = new QLabel(InspectorEntity);
        label->setObjectName(QStringLiteral("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label);

        m_pEntityIDEdit = new QLineEdit(InspectorEntity);
        m_pEntityIDEdit->setObjectName(QStringLiteral("m_pEntityIDEdit"));
        m_pEntityIDEdit->setMaximumSize(QSize(50, 16777215));
        m_pEntityIDEdit->setReadOnly(true);

        horizontalLayout_3->addWidget(m_pEntityIDEdit);


        verticalLayout_2->addLayout(horizontalLayout_3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        label_2 = new QLabel(InspectorEntity);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_2->addWidget(label_2);

        m_pLayerCB = new QComboBox(InspectorEntity);
        m_pLayerCB->setObjectName(QStringLiteral("m_pLayerCB"));

        horizontalLayout_2->addWidget(m_pLayerCB);

        line = new QFrame(InspectorEntity);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout_2->addWidget(line);

        label_3 = new QLabel(InspectorEntity);
        label_3->setObjectName(QStringLiteral("label_3"));

        horizontalLayout_2->addWidget(label_3);

        m_pCategoryCB = new QComboBox(InspectorEntity);
        m_pCategoryCB->setObjectName(QStringLiteral("m_pCategoryCB"));
        m_pCategoryCB->setEnabled(false);
        m_pCategoryCB->setEditable(false);

        horizontalLayout_2->addWidget(m_pCategoryCB);


        verticalLayout_2->addLayout(horizontalLayout_2);


        horizontalLayout->addLayout(verticalLayout_2);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(InspectorEntity);
        QObject::connect(m_pEntityNameEdit, SIGNAL(textEdited(QString)), InspectorEntity, SLOT(valueChanged()));
        QObject::connect(m_pEnabledCB, SIGNAL(clicked()), InspectorEntity, SLOT(valueChanged()));
        QObject::connect(m_pCategoryCB, SIGNAL(currentIndexChanged(int)), InspectorEntity, SLOT(valueChanged()));
        QObject::connect(m_pLayerCB, SIGNAL(currentIndexChanged(int)), InspectorEntity, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorEntity);
    } // setupUi

    void retranslateUi(QWidget *InspectorEntity)
    {
        InspectorEntity->setWindowTitle(QApplication::translate("InspectorEntity", "Entity", 0));
        m_pEnabledCB->setText(QString());
        label->setText(QApplication::translate("InspectorEntity", "ID", 0));
        label_2->setText(QApplication::translate("InspectorEntity", "Layer", 0));
        m_pLayerCB->clear();
        m_pLayerCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorEntity", "Default", 0)
         << QApplication::translate("InspectorEntity", "AR", 0)
         << QApplication::translate("InspectorEntity", "TransparentFX", 0)
         << QApplication::translate("InspectorEntity", "IgnoreRaycast", 0)
         << QApplication::translate("InspectorEntity", "Water", 0)
         << QApplication::translate("InspectorEntity", "UI", 0)
        );
        label_3->setText(QApplication::translate("InspectorEntity", "Category", 0));
        m_pCategoryCB->clear();
        m_pCategoryCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorEntity", "Actor", 0)
         << QApplication::translate("InspectorEntity", "Light", 0)
         << QApplication::translate("InspectorEntity", "FX", 0)
         << QApplication::translate("InspectorEntity", "Plugin", 0)
        );
    } // retranslateUi

};

namespace Ui {
    class InspectorEntity: public Ui_InspectorEntity {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_ENTITY_H
