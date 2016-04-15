/********************************************************************************
** Form generated from reading UI file 'edit_mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_MAINWINDOW_H
#define UI_EDIT_MAINWINDOW_H

#include <QtCore/QLocale>
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

class Ui_CMainWindow
{
public:
    QWidget *centralWidget;
    QPushButton *pushButton;
    QWidget *m_pEditorRenderContext;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *CMainWindow)
    {
        if (CMainWindow->objectName().isEmpty())
            CMainWindow->setObjectName(QStringLiteral("CMainWindow"));
        CMainWindow->setWindowModality(Qt::NonModal);
        CMainWindow->resize(1494, 874);
        CMainWindow->setCursor(QCursor(Qt::ArrowCursor));
        CMainWindow->setMouseTracking(true);
        centralWidget = new QWidget(CMainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(900, 780, 75, 23));
        m_pEditorRenderContext = new QWidget(centralWidget);
        m_pEditorRenderContext->setObjectName(QStringLiteral("m_pEditorRenderContext"));
        m_pEditorRenderContext->setGeometry(QRect(130, 20, 1280, 720));
        m_pEditorRenderContext->setCursor(QCursor(Qt::ArrowCursor));
        m_pEditorRenderContext->setMouseTracking(true);
        m_pEditorRenderContext->setAutoFillBackground(false);
        m_pEditorRenderContext->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        CMainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(CMainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1494, 21));
        CMainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(CMainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        CMainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(CMainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        CMainWindow->setStatusBar(statusBar);

        retranslateUi(CMainWindow);
        QObject::connect(pushButton, SIGNAL(clicked()), CMainWindow, SLOT(slot1()));

        QMetaObject::connectSlotsByName(CMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *CMainWindow)
    {
        CMainWindow->setWindowTitle(QApplication::translate("CMainWindow", "Editor", 0));
        pushButton->setText(QApplication::translate("CMainWindow", "PushButton", 0));
    } // retranslateUi

};

namespace Ui {
    class CMainWindow: public Ui_CMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_MAINWINDOW_H
