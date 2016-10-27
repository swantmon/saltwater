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
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>
#include "editor_gui/edit_inspector.h"
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
    QAction *m_pActionModel;
    QAction *actionPoint;
    QAction *m_pActionPoint;
    QAction *m_pActionEnvironment;
    QAction *m_pActionReflection;
    QAction *m_pActionCamera;
    QAction *m_pActionDirectional;
    QWidget *m_pCentralWidget;
    QTabWidget *m_pSceneGraphTabWidget;
    QWidget *m_pSceneGraphTab;
    Edit::CSceneGraph *m_pScenegraph;
    QTabWidget *m_pGameTabWidget;
    QWidget *m_pGameTab;
    QPushButton *m_pPlayButton;
    Edit::CRenderContext *m_pEditorRenderContext;
    QPushButton *m_pScreenshotButton;
    QLineEdit *m_pMailAdressEdit;
    QTabWidget *m_pInspectorTabWidget;
    QWidget *m_pInspectorTab;
    Edit::CInspector *m_pInspector;
    QMenuBar *m_pMenuBar;
    QMenu *m_pMenuFile;
    QMenu *m_pMenuEdit;
    QMenu *m_pMenuAssets;
    QMenu *m_pMenuEntity;
    QMenu *menuActors;
    QMenu *menuLights;
    QMenu *m_pMenuComponent;
    QMenu *m_pMenuWindow;
    QMenu *m_pMenuHelp;
    QStatusBar *m_pStatusBar;

    void setupUi(QMainWindow *CMainWindow)
    {
        if (CMainWindow->objectName().isEmpty())
            CMainWindow->setObjectName(QStringLiteral("CMainWindow"));
        CMainWindow->setWindowModality(Qt::NonModal);
        CMainWindow->resize(1933, 916);
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
        m_pActionModel = new QAction(CMainWindow);
        m_pActionModel->setObjectName(QStringLiteral("m_pActionModel"));
        actionPoint = new QAction(CMainWindow);
        actionPoint->setObjectName(QStringLiteral("actionPoint"));
        m_pActionPoint = new QAction(CMainWindow);
        m_pActionPoint->setObjectName(QStringLiteral("m_pActionPoint"));
        m_pActionEnvironment = new QAction(CMainWindow);
        m_pActionEnvironment->setObjectName(QStringLiteral("m_pActionEnvironment"));
        m_pActionReflection = new QAction(CMainWindow);
        m_pActionReflection->setObjectName(QStringLiteral("m_pActionReflection"));
        m_pActionCamera = new QAction(CMainWindow);
        m_pActionCamera->setObjectName(QStringLiteral("m_pActionCamera"));
        m_pActionDirectional = new QAction(CMainWindow);
        m_pActionDirectional->setObjectName(QStringLiteral("m_pActionDirectional"));
        m_pCentralWidget = new QWidget(CMainWindow);
        m_pCentralWidget->setObjectName(QStringLiteral("m_pCentralWidget"));
        m_pSceneGraphTabWidget = new QTabWidget(m_pCentralWidget);
        m_pSceneGraphTabWidget->setObjectName(QStringLiteral("m_pSceneGraphTabWidget"));
        m_pSceneGraphTabWidget->setGeometry(QRect(0, 0, 320, 791));
        m_pSceneGraphTab = new QWidget();
        m_pSceneGraphTab->setObjectName(QStringLiteral("m_pSceneGraphTab"));
        m_pScenegraph = new Edit::CSceneGraph(m_pSceneGraphTab);
        m_pScenegraph->setObjectName(QStringLiteral("m_pScenegraph"));
        m_pScenegraph->setGeometry(QRect(0, 0, 311, 761));
        m_pScenegraph->setDragEnabled(false);
        m_pScenegraph->setAlternatingRowColors(true);
        m_pScenegraph->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_pScenegraph->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_pScenegraph->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
        m_pSceneGraphTabWidget->addTab(m_pSceneGraphTab, QString());
        m_pGameTabWidget = new QTabWidget(m_pCentralWidget);
        m_pGameTabWidget->setObjectName(QStringLiteral("m_pGameTabWidget"));
        m_pGameTabWidget->setGeometry(QRect(320, 0, 1291, 791));
        m_pGameTabWidget->setTabBarAutoHide(false);
        m_pGameTab = new QWidget();
        m_pGameTab->setObjectName(QStringLiteral("m_pGameTab"));
        m_pPlayButton = new QPushButton(m_pGameTab);
        m_pPlayButton->setObjectName(QStringLiteral("m_pPlayButton"));
        m_pPlayButton->setGeometry(QRect(570, 10, 75, 23));
        m_pEditorRenderContext = new Edit::CRenderContext(m_pGameTab);
        m_pEditorRenderContext->setObjectName(QStringLiteral("m_pEditorRenderContext"));
        m_pEditorRenderContext->setGeometry(QRect(0, 40, 1280, 720));
        m_pEditorRenderContext->setMinimumSize(QSize(1280, 0));
        m_pEditorRenderContext->setCursor(QCursor(Qt::ArrowCursor));
        m_pEditorRenderContext->setMouseTracking(true);
        m_pEditorRenderContext->setAutoFillBackground(false);
        m_pEditorRenderContext->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_pScreenshotButton = new QPushButton(m_pGameTab);
        m_pScreenshotButton->setObjectName(QStringLiteral("m_pScreenshotButton"));
        m_pScreenshotButton->setGeometry(QRect(1200, 10, 75, 23));
        m_pMailAdressEdit = new QLineEdit(m_pGameTab);
        m_pMailAdressEdit->setObjectName(QStringLiteral("m_pMailAdressEdit"));
        m_pMailAdressEdit->setGeometry(QRect(970, 10, 221, 20));
        m_pGameTabWidget->addTab(m_pGameTab, QString());
        m_pInspectorTabWidget = new QTabWidget(m_pCentralWidget);
        m_pInspectorTabWidget->setObjectName(QStringLiteral("m_pInspectorTabWidget"));
        m_pInspectorTabWidget->setGeometry(QRect(1610, 0, 320, 791));
        m_pInspectorTab = new QWidget();
        m_pInspectorTab->setObjectName(QStringLiteral("m_pInspectorTab"));
        m_pInspector = new Edit::CInspector(m_pInspectorTab);
        m_pInspector->setObjectName(QStringLiteral("m_pInspector"));
        m_pInspector->setGeometry(QRect(0, 0, 311, 761));
        m_pInspectorTabWidget->addTab(m_pInspectorTab, QString());
        CMainWindow->setCentralWidget(m_pCentralWidget);
        m_pMenuBar = new QMenuBar(CMainWindow);
        m_pMenuBar->setObjectName(QStringLiteral("m_pMenuBar"));
        m_pMenuBar->setGeometry(QRect(0, 0, 1933, 38));
        m_pMenuFile = new QMenu(m_pMenuBar);
        m_pMenuFile->setObjectName(QStringLiteral("m_pMenuFile"));
        m_pMenuEdit = new QMenu(m_pMenuBar);
        m_pMenuEdit->setObjectName(QStringLiteral("m_pMenuEdit"));
        m_pMenuAssets = new QMenu(m_pMenuBar);
        m_pMenuAssets->setObjectName(QStringLiteral("m_pMenuAssets"));
        m_pMenuEntity = new QMenu(m_pMenuBar);
        m_pMenuEntity->setObjectName(QStringLiteral("m_pMenuEntity"));
        menuActors = new QMenu(m_pMenuEntity);
        menuActors->setObjectName(QStringLiteral("menuActors"));
        menuLights = new QMenu(m_pMenuEntity);
        menuLights->setObjectName(QStringLiteral("menuLights"));
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
        m_pMenuEntity->addAction(menuActors->menuAction());
        m_pMenuEntity->addAction(menuLights->menuAction());
        menuActors->addAction(m_pActionModel);
        menuActors->addAction(m_pActionCamera);
        menuLights->addAction(m_pActionDirectional);
        menuLights->addAction(m_pActionPoint);
        menuLights->addAction(m_pActionEnvironment);
        menuLights->addAction(m_pActionReflection);
        m_pMenuHelp->addAction(m_pActionAboutSaltwater);
        m_pMenuHelp->addSeparator();
        m_pMenuHelp->addAction(m_pActionReleaseNotes);
        m_pMenuHelp->addAction(m_pActionReportBug);

        retranslateUi(CMainWindow);
        QObject::connect(m_pActionExit, SIGNAL(triggered()), CMainWindow, SLOT(close()));
        QObject::connect(m_pPlayButton, SIGNAL(clicked()), CMainWindow, SLOT(switchPlayingCurrentScene()));
        QObject::connect(m_pScreenshotButton, SIGNAL(clicked()), CMainWindow, SLOT(takeScreenshot()));
        QObject::connect(m_pActionNew, SIGNAL(triggered()), CMainWindow, SLOT(openNewSceneDialog()));
        QObject::connect(m_pScenegraph, SIGNAL(itemClicked(QTreeWidgetItem*,int)), m_pScenegraph, SLOT(itemSelected(QTreeWidgetItem*)));
        QObject::connect(m_pActionModel, SIGNAL(triggered()), CMainWindow, SLOT(openNewActorModelDialog()));
        QObject::connect(m_pActionDirectional, SIGNAL(triggered()), CMainWindow, SLOT(createNewLightDirectional()));
        QObject::connect(m_pActionEnvironment, SIGNAL(triggered()), CMainWindow, SLOT(createNewLightEnvironment()));
        QObject::connect(m_pActionPoint, SIGNAL(triggered()), CMainWindow, SLOT(createNewLightPoint()));
        QObject::connect(m_pActionReflection, SIGNAL(triggered()), CMainWindow, SLOT(createNewLightReflection()));
        QObject::connect(m_pScenegraph, SIGNAL(entitySelected(int)), m_pInspector, SLOT(updateContentForEntity(int)));

        m_pSceneGraphTabWidget->setCurrentIndex(0);
        m_pGameTabWidget->setCurrentIndex(0);
        m_pInspectorTabWidget->setCurrentIndex(0);


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
        m_pActionModel->setText(QApplication::translate("CMainWindow", "Model", 0));
        m_pActionModel->setShortcut(QApplication::translate("CMainWindow", "Ctrl+Shift+N", 0));
        actionPoint->setText(QApplication::translate("CMainWindow", "Point", 0));
        m_pActionPoint->setText(QApplication::translate("CMainWindow", "Point", 0));
        m_pActionEnvironment->setText(QApplication::translate("CMainWindow", "Environment", 0));
        m_pActionReflection->setText(QApplication::translate("CMainWindow", "Reflection", 0));
        m_pActionCamera->setText(QApplication::translate("CMainWindow", "Camera", 0));
        m_pActionDirectional->setText(QApplication::translate("CMainWindow", "Directional", 0));
        QTreeWidgetItem *___qtreewidgetitem = m_pScenegraph->headerItem();
        ___qtreewidgetitem->setText(2, QApplication::translate("CMainWindow", "Type", 0));
        ___qtreewidgetitem->setText(1, QApplication::translate("CMainWindow", "Entity", 0));
        ___qtreewidgetitem->setText(0, QApplication::translate("CMainWindow", "ID", 0));
        m_pSceneGraphTabWidget->setTabText(m_pSceneGraphTabWidget->indexOf(m_pSceneGraphTab), QApplication::translate("CMainWindow", "Scene Graph", 0));
        m_pPlayButton->setText(QApplication::translate("CMainWindow", "Play", 0));
        m_pScreenshotButton->setText(QApplication::translate("CMainWindow", "Screenshot", 0));
        m_pMailAdressEdit->setPlaceholderText(QApplication::translate("CMainWindow", "max.mustermann@mail.com", 0));
        m_pGameTabWidget->setTabText(m_pGameTabWidget->indexOf(m_pGameTab), QApplication::translate("CMainWindow", "Game", 0));
        m_pInspectorTabWidget->setTabText(m_pInspectorTabWidget->indexOf(m_pInspectorTab), QApplication::translate("CMainWindow", "Inspector", 0));
        m_pMenuFile->setTitle(QApplication::translate("CMainWindow", "File", 0));
        m_pMenuEdit->setTitle(QApplication::translate("CMainWindow", "Edit", 0));
        m_pMenuAssets->setTitle(QApplication::translate("CMainWindow", "Assets", 0));
        m_pMenuEntity->setTitle(QApplication::translate("CMainWindow", "Entity", 0));
        menuActors->setTitle(QApplication::translate("CMainWindow", "Actors", 0));
        menuLights->setTitle(QApplication::translate("CMainWindow", "Lights", 0));
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
