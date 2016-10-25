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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include "editor_gui/edit_render_context.h"
#include "editor_gui/edit_scenegraph.h"

QT_BEGIN_NAMESPACE

class Ui_CMainWindow
{
public:
    QAction *m_pActionNew;
    QAction *m_pActionOpen;
    QAction *m_pActionExit;
    QAction *m_pActionAboutSaltwater;
    QAction *m_pActionReleaseNotes;
    QAction *m_pActionReportBug;
    QWidget *m_pCentralWidget;
    QPushButton *m_pPlayButton;
    QPushButton *m_pScreenshotButton;
    QLineEdit *m_pMailAdressEdit;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    Edit::CSceneGraph *m_pScenegraph;
    Edit::CRenderContext *m_pEditorRenderContext;
    QListView *listView_2;
    QMenuBar *m_pMenuBar;
    QMenu *m_pMenuFile;
    QMenu *m_pMenuEdit;
    QMenu *m_pMenuAssets;
    QMenu *m_pMenuEntity;
    QMenu *m_pMenuComponent;
    QMenu *m_pMenuWindow;
    QMenu *m_pMenuHelp;
    QStatusBar *m_pStatusBar;

    void setupUi(QMainWindow *CMainWindow)
    {
        if (CMainWindow->objectName().isEmpty())
            CMainWindow->setObjectName(QStringLiteral("CMainWindow"));
        CMainWindow->setWindowModality(Qt::NonModal);
        CMainWindow->resize(1702, 798);
        CMainWindow->setCursor(QCursor(Qt::ArrowCursor));
        CMainWindow->setMouseTracking(true);
        m_pActionNew = new QAction(CMainWindow);
        m_pActionNew->setObjectName(QStringLiteral("m_pActionNew"));
        m_pActionOpen = new QAction(CMainWindow);
        m_pActionOpen->setObjectName(QStringLiteral("m_pActionOpen"));
        m_pActionOpen->setEnabled(false);
        m_pActionExit = new QAction(CMainWindow);
        m_pActionExit->setObjectName(QStringLiteral("m_pActionExit"));
        m_pActionAboutSaltwater = new QAction(CMainWindow);
        m_pActionAboutSaltwater->setObjectName(QStringLiteral("m_pActionAboutSaltwater"));
        m_pActionReleaseNotes = new QAction(CMainWindow);
        m_pActionReleaseNotes->setObjectName(QStringLiteral("m_pActionReleaseNotes"));
        m_pActionReportBug = new QAction(CMainWindow);
        m_pActionReportBug->setObjectName(QStringLiteral("m_pActionReportBug"));
        m_pCentralWidget = new QWidget(CMainWindow);
        m_pCentralWidget->setObjectName(QStringLiteral("m_pCentralWidget"));
        m_pPlayButton = new QPushButton(m_pCentralWidget);
        m_pPlayButton->setObjectName(QStringLiteral("m_pPlayButton"));
        m_pPlayButton->setGeometry(QRect(10, 10, 75, 23));
        m_pScreenshotButton = new QPushButton(m_pCentralWidget);
        m_pScreenshotButton->setObjectName(QStringLiteral("m_pScreenshotButton"));
        m_pScreenshotButton->setGeometry(QRect(1610, 10, 75, 23));
        m_pMailAdressEdit = new QLineEdit(m_pCentralWidget);
        m_pMailAdressEdit->setObjectName(QStringLiteral("m_pMailAdressEdit"));
        m_pMailAdressEdit->setGeometry(QRect(1382, 10, 221, 20));
        horizontalLayoutWidget = new QWidget(m_pCentralWidget);
        horizontalLayoutWidget->setObjectName(QStringLiteral("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(9, 39, 1681, 711));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        m_pScenegraph = new Edit::CSceneGraph(horizontalLayoutWidget);
        m_pScenegraph->setObjectName(QStringLiteral("m_pScenegraph"));
        m_pScenegraph->setDragEnabled(false);
        m_pScenegraph->setAlternatingRowColors(true);
        m_pScenegraph->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_pScenegraph->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_pScenegraph->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
        m_pScenegraph->setModelColumn(0);

        horizontalLayout->addWidget(m_pScenegraph);

        m_pEditorRenderContext = new Edit::CRenderContext(horizontalLayoutWidget);
        m_pEditorRenderContext->setObjectName(QStringLiteral("m_pEditorRenderContext"));
        m_pEditorRenderContext->setMinimumSize(QSize(1280, 0));
        m_pEditorRenderContext->setCursor(QCursor(Qt::ArrowCursor));
        m_pEditorRenderContext->setMouseTracking(true);
        m_pEditorRenderContext->setAutoFillBackground(false);
        m_pEditorRenderContext->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));

        horizontalLayout->addWidget(m_pEditorRenderContext);

        listView_2 = new QListView(horizontalLayoutWidget);
        listView_2->setObjectName(QStringLiteral("listView_2"));

        horizontalLayout->addWidget(listView_2);

        CMainWindow->setCentralWidget(m_pCentralWidget);
        m_pMenuBar = new QMenuBar(CMainWindow);
        m_pMenuBar->setObjectName(QStringLiteral("m_pMenuBar"));
        m_pMenuBar->setGeometry(QRect(0, 0, 1702, 21));
        m_pMenuFile = new QMenu(m_pMenuBar);
        m_pMenuFile->setObjectName(QStringLiteral("m_pMenuFile"));
        m_pMenuEdit = new QMenu(m_pMenuBar);
        m_pMenuEdit->setObjectName(QStringLiteral("m_pMenuEdit"));
        m_pMenuAssets = new QMenu(m_pMenuBar);
        m_pMenuAssets->setObjectName(QStringLiteral("m_pMenuAssets"));
        m_pMenuEntity = new QMenu(m_pMenuBar);
        m_pMenuEntity->setObjectName(QStringLiteral("m_pMenuEntity"));
        m_pMenuComponent = new QMenu(m_pMenuBar);
        m_pMenuComponent->setObjectName(QStringLiteral("m_pMenuComponent"));
        m_pMenuWindow = new QMenu(m_pMenuBar);
        m_pMenuWindow->setObjectName(QStringLiteral("m_pMenuWindow"));
        m_pMenuHelp = new QMenu(m_pMenuBar);
        m_pMenuHelp->setObjectName(QStringLiteral("m_pMenuHelp"));
        CMainWindow->setMenuBar(m_pMenuBar);
        m_pStatusBar = new QStatusBar(CMainWindow);
        m_pStatusBar->setObjectName(QStringLiteral("m_pStatusBar"));
        CMainWindow->setStatusBar(m_pStatusBar);

        m_pMenuBar->addAction(m_pMenuFile->menuAction());
        m_pMenuBar->addAction(m_pMenuEdit->menuAction());
        m_pMenuBar->addAction(m_pMenuAssets->menuAction());
        m_pMenuBar->addAction(m_pMenuEntity->menuAction());
        m_pMenuBar->addAction(m_pMenuComponent->menuAction());
        m_pMenuBar->addAction(m_pMenuWindow->menuAction());
        m_pMenuBar->addAction(m_pMenuHelp->menuAction());
        m_pMenuFile->addAction(m_pActionNew);
        m_pMenuFile->addAction(m_pActionOpen);
        m_pMenuFile->addSeparator();
        m_pMenuFile->addAction(m_pActionExit);
        m_pMenuHelp->addAction(m_pActionAboutSaltwater);
        m_pMenuHelp->addSeparator();
        m_pMenuHelp->addAction(m_pActionReleaseNotes);
        m_pMenuHelp->addAction(m_pActionReportBug);

        retranslateUi(CMainWindow);
        QObject::connect(m_pActionExit, SIGNAL(triggered()), CMainWindow, SLOT(close()));
        QObject::connect(m_pPlayButton, SIGNAL(clicked()), CMainWindow, SLOT(switchPlayingCurrentScene()));
        QObject::connect(m_pScreenshotButton, SIGNAL(clicked()), CMainWindow, SLOT(takeScreenshot()));
        QObject::connect(m_pActionNew, SIGNAL(triggered()), CMainWindow, SLOT(createNewScene()));

        QMetaObject::connectSlotsByName(CMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *CMainWindow)
    {
        CMainWindow->setWindowTitle(QApplication::translate("CMainWindow", "Saltwater Editor", 0));
        m_pActionNew->setText(QApplication::translate("CMainWindow", "New Scene", 0));
        m_pActionNew->setShortcut(QApplication::translate("CMainWindow", "Ctrl+N", 0));
        m_pActionOpen->setText(QApplication::translate("CMainWindow", "Open Scene", 0));
        m_pActionOpen->setShortcut(QApplication::translate("CMainWindow", "Ctrl+O", 0));
        m_pActionExit->setText(QApplication::translate("CMainWindow", "Exit", 0));
        m_pActionAboutSaltwater->setText(QApplication::translate("CMainWindow", "About Saltwater", 0));
        m_pActionReleaseNotes->setText(QApplication::translate("CMainWindow", "Release Notes", 0));
        m_pActionReportBug->setText(QApplication::translate("CMainWindow", "Report a Bug...", 0));
        m_pPlayButton->setText(QApplication::translate("CMainWindow", "Play", 0));
        m_pScreenshotButton->setText(QApplication::translate("CMainWindow", "Screenshot", 0));
        m_pMailAdressEdit->setPlaceholderText(QApplication::translate("CMainWindow", "max.mustermann@mail.com", 0));
        m_pMenuFile->setTitle(QApplication::translate("CMainWindow", "File", 0));
        m_pMenuEdit->setTitle(QApplication::translate("CMainWindow", "Edit", 0));
        m_pMenuAssets->setTitle(QApplication::translate("CMainWindow", "Assets", 0));
        m_pMenuEntity->setTitle(QApplication::translate("CMainWindow", "Entity", 0));
        m_pMenuComponent->setTitle(QApplication::translate("CMainWindow", "Component", 0));
        m_pMenuWindow->setTitle(QApplication::translate("CMainWindow", "Window", 0));
        m_pMenuHelp->setTitle(QApplication::translate("CMainWindow", "Help", 0));
    } // retranslateUi

};

namespace Ui {
    class CMainWindow: public Ui_CMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_MAINWINDOW_H
