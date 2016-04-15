/********************************************************************************
** Form generated from reading UI file 'edit_gui_mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_GUI_MAINWINDOW_H
#define UI_EDIT_GUI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_edit_gui_mainwindow
{
public:
    QWidget *centralWidget;
    QPushButton *pushButton;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *edit_gui_mainwindow)
    {
        if (edit_gui_mainwindow->objectName().isEmpty())
            edit_gui_mainwindow->setObjectName(QStringLiteral("edit_gui_mainwindow"));
        edit_gui_mainwindow->resize(400, 300);
        centralWidget = new QWidget(edit_gui_mainwindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(160, 100, 75, 23));
        edit_gui_mainwindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(edit_gui_mainwindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 400, 21));
        edit_gui_mainwindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(edit_gui_mainwindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        edit_gui_mainwindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(edit_gui_mainwindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        edit_gui_mainwindow->setStatusBar(statusBar);

        retranslateUi(edit_gui_mainwindow);
        QObject::connect(pushButton, SIGNAL(clicked()), edit_gui_mainwindow, SLOT(slot1()));

        QMetaObject::connectSlotsByName(edit_gui_mainwindow);
    } // setupUi

    void retranslateUi(QMainWindow *edit_gui_mainwindow)
    {
        edit_gui_mainwindow->setWindowTitle(QApplication::translate("edit_gui_mainwindow", "edit_gui_mainwindow", 0));
        pushButton->setText(QApplication::translate("edit_gui_mainwindow", "PushButton", 0));
    } // retranslateUi

};

namespace Ui {
    class edit_gui_mainwindow: public Ui_edit_gui_mainwindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_GUI_MAINWINDOW_H
