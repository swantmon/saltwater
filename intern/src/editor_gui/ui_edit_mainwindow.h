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
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
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
    QAction *m_pActionGlobalProbe;
    QAction *m_pActionCamera;
    QAction *m_pActionSun;
    QAction *m_pActionHistogram;
    QAction *actionConsole;
    QAction *actionScene_Hraph;
    QAction *actionInspector;
    QAction *m_pActionBloom;
    QAction *m_pActionDOF;
    QAction *m_pActionFXAA;
    QAction *m_pActionSSR;
    QAction *m_pActionVolumeFog;
    QWidget *m_pCentralWidget;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QPushButton *m_pPlayButton;
    QSpacerItem *horizontalSpacer;
    QLineEdit *m_pMailAdressEdit;
    QPushButton *m_pScreenshotButton;
    QHBoxLayout *horizontalLayout_2;
    Edit::CRenderContext *m_pEditorRenderContext;
    QMenuBar *m_pMenuBar;
    QMenu *m_pMenuFile;
    QMenu *m_pMenuEdit;
    QMenu *m_pMenuAssets;
    QMenu *m_pMenuEntity;
    QMenu *menuActors;
    QMenu *menuLights;
    QMenu *menuEffects;
    QMenu *m_pMenuComponent;
    QMenu *m_pMenuWindow;
    QMenu *m_pMenuHelp;
    QStatusBar *m_pStatusBar;
    QDockWidget *m_pSceneGraphDockWidget;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout;
    Edit::CSceneGraph *m_pScenegraph;
    QDockWidget *m_pInspectorDockWidget;
    QWidget *dockWidgetContents_2;
    QVBoxLayout *verticalLayout_2;
    Edit::CInspector *m_pInspector;
    QDockWidget *m_pConsoleDockWidget;
    QWidget *dockWidgetContents_4;
    QVBoxLayout *verticalLayout_4;
    QTextEdit *m_pConsoleOutputEdit;
    QHBoxLayout *horizontalLayout_3;
    QLineEdit *m_pConsoleCommandEdit;
    QPushButton *m_pSendCommandButton;
    QDockWidget *m_pHistogramDockWidget;
    QWidget *dockWidgetContents_3;
    QVBoxLayout *verticalLayout_5;
    QGridLayout *gridLayout;
    QLabel *label_6;
    QLineEdit *m_pHistogramUpperBoundEdit;
    QLineEdit *m_pHistogramLogMaxEdit;
    QLineEdit *m_pHistogramEyeDownEdit;
    QLineEdit *m_pHistogramLogMinEdit;
    QLineEdit *m_pHistogramLowerBoundEdit;
    QLineEdit *m_pHistogramEyeUpEdit;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_4;
    QLabel *label_3;
    QLabel *label_5;
    QSpacerItem *verticalSpacer;

    void setupUi(QMainWindow *CMainWindow)
    {
        if (CMainWindow->objectName().isEmpty())
            CMainWindow->setObjectName(QStringLiteral("CMainWindow"));
        CMainWindow->setWindowModality(Qt::NonModal);
        CMainWindow->resize(1924, 959);
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
        m_pActionGlobalProbe = new QAction(CMainWindow);
        m_pActionGlobalProbe->setObjectName(QStringLiteral("m_pActionGlobalProbe"));
        m_pActionCamera = new QAction(CMainWindow);
        m_pActionCamera->setObjectName(QStringLiteral("m_pActionCamera"));
        m_pActionSun = new QAction(CMainWindow);
        m_pActionSun->setObjectName(QStringLiteral("m_pActionSun"));
        m_pActionHistogram = new QAction(CMainWindow);
        m_pActionHistogram->setObjectName(QStringLiteral("m_pActionHistogram"));
        m_pActionHistogram->setCheckable(false);
        actionConsole = new QAction(CMainWindow);
        actionConsole->setObjectName(QStringLiteral("actionConsole"));
        actionScene_Hraph = new QAction(CMainWindow);
        actionScene_Hraph->setObjectName(QStringLiteral("actionScene_Hraph"));
        actionInspector = new QAction(CMainWindow);
        actionInspector->setObjectName(QStringLiteral("actionInspector"));
        m_pActionBloom = new QAction(CMainWindow);
        m_pActionBloom->setObjectName(QStringLiteral("m_pActionBloom"));
        m_pActionDOF = new QAction(CMainWindow);
        m_pActionDOF->setObjectName(QStringLiteral("m_pActionDOF"));
        m_pActionFXAA = new QAction(CMainWindow);
        m_pActionFXAA->setObjectName(QStringLiteral("m_pActionFXAA"));
        m_pActionSSR = new QAction(CMainWindow);
        m_pActionSSR->setObjectName(QStringLiteral("m_pActionSSR"));
        m_pActionVolumeFog = new QAction(CMainWindow);
        m_pActionVolumeFog->setObjectName(QStringLiteral("m_pActionVolumeFog"));
        m_pCentralWidget = new QWidget(CMainWindow);
        m_pCentralWidget->setObjectName(QStringLiteral("m_pCentralWidget"));
        verticalLayout_3 = new QVBoxLayout(m_pCentralWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setSizeConstraint(QLayout::SetDefaultConstraint);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        m_pPlayButton = new QPushButton(m_pCentralWidget);
        m_pPlayButton->setObjectName(QStringLiteral("m_pPlayButton"));

        horizontalLayout->addWidget(m_pPlayButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        m_pMailAdressEdit = new QLineEdit(m_pCentralWidget);
        m_pMailAdressEdit->setObjectName(QStringLiteral("m_pMailAdressEdit"));
        m_pMailAdressEdit->setMaximumSize(QSize(200, 16777215));

        horizontalLayout->addWidget(m_pMailAdressEdit);

        m_pScreenshotButton = new QPushButton(m_pCentralWidget);
        m_pScreenshotButton->setObjectName(QStringLiteral("m_pScreenshotButton"));

        horizontalLayout->addWidget(m_pScreenshotButton);


        verticalLayout_3->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        m_pEditorRenderContext = new Edit::CRenderContext(m_pCentralWidget);
        m_pEditorRenderContext->setObjectName(QStringLiteral("m_pEditorRenderContext"));
        m_pEditorRenderContext->setMinimumSize(QSize(640, 480));
        m_pEditorRenderContext->setMaximumSize(QSize(16777215, 16777215));
        m_pEditorRenderContext->setCursor(QCursor(Qt::ArrowCursor));
        m_pEditorRenderContext->setMouseTracking(true);
        m_pEditorRenderContext->setAutoFillBackground(false);
        m_pEditorRenderContext->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));

        horizontalLayout_2->addWidget(m_pEditorRenderContext);


        verticalLayout_3->addLayout(horizontalLayout_2);

        CMainWindow->setCentralWidget(m_pCentralWidget);
        m_pMenuBar = new QMenuBar(CMainWindow);
        m_pMenuBar->setObjectName(QStringLiteral("m_pMenuBar"));
        m_pMenuBar->setGeometry(QRect(0, 0, 1924, 21));
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
        menuEffects = new QMenu(m_pMenuEntity);
        menuEffects->setObjectName(QStringLiteral("menuEffects"));
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
        m_pSceneGraphDockWidget = new QDockWidget(CMainWindow);
        m_pSceneGraphDockWidget->setObjectName(QStringLiteral("m_pSceneGraphDockWidget"));
        m_pSceneGraphDockWidget->setMinimumSize(QSize(89, 111));
        m_pSceneGraphDockWidget->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
        verticalLayout = new QVBoxLayout(dockWidgetContents);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        m_pScenegraph = new Edit::CSceneGraph(dockWidgetContents);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(1, QStringLiteral("2"));
        __qtreewidgetitem->setText(0, QStringLiteral("1"));
        m_pScenegraph->setHeaderItem(__qtreewidgetitem);
        m_pScenegraph->setObjectName(QStringLiteral("m_pScenegraph"));
        m_pScenegraph->setDragEnabled(false);
        m_pScenegraph->setDragDropMode(QAbstractItemView::InternalMove);
        m_pScenegraph->setAlternatingRowColors(false);
        m_pScenegraph->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_pScenegraph->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_pScenegraph->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
        m_pScenegraph->setUniformRowHeights(true);
        m_pScenegraph->setColumnCount(2);
        m_pScenegraph->header()->setVisible(false);

        verticalLayout->addWidget(m_pScenegraph);

        m_pSceneGraphDockWidget->setWidget(dockWidgetContents);
        CMainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), m_pSceneGraphDockWidget);
        m_pInspectorDockWidget = new QDockWidget(CMainWindow);
        m_pInspectorDockWidget->setObjectName(QStringLiteral("m_pInspectorDockWidget"));
        m_pInspectorDockWidget->setMinimumSize(QSize(80, 50));
        m_pInspectorDockWidget->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
        dockWidgetContents_2 = new QWidget();
        dockWidgetContents_2->setObjectName(QStringLiteral("dockWidgetContents_2"));
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents_2);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        m_pInspector = new Edit::CInspector(dockWidgetContents_2);
        m_pInspector->setObjectName(QStringLiteral("m_pInspector"));

        verticalLayout_2->addWidget(m_pInspector, 0, Qt::AlignTop);

        m_pInspectorDockWidget->setWidget(dockWidgetContents_2);
        CMainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), m_pInspectorDockWidget);
        m_pConsoleDockWidget = new QDockWidget(CMainWindow);
        m_pConsoleDockWidget->setObjectName(QStringLiteral("m_pConsoleDockWidget"));
        dockWidgetContents_4 = new QWidget();
        dockWidgetContents_4->setObjectName(QStringLiteral("dockWidgetContents_4"));
        verticalLayout_4 = new QVBoxLayout(dockWidgetContents_4);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        m_pConsoleOutputEdit = new QTextEdit(dockWidgetContents_4);
        m_pConsoleOutputEdit->setObjectName(QStringLiteral("m_pConsoleOutputEdit"));

        verticalLayout_4->addWidget(m_pConsoleOutputEdit);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        m_pConsoleCommandEdit = new QLineEdit(dockWidgetContents_4);
        m_pConsoleCommandEdit->setObjectName(QStringLiteral("m_pConsoleCommandEdit"));

        horizontalLayout_3->addWidget(m_pConsoleCommandEdit);

        m_pSendCommandButton = new QPushButton(dockWidgetContents_4);
        m_pSendCommandButton->setObjectName(QStringLiteral("m_pSendCommandButton"));

        horizontalLayout_3->addWidget(m_pSendCommandButton);


        verticalLayout_4->addLayout(horizontalLayout_3);

        m_pConsoleDockWidget->setWidget(dockWidgetContents_4);
        CMainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(8), m_pConsoleDockWidget);
        m_pHistogramDockWidget = new QDockWidget(CMainWindow);
        m_pHistogramDockWidget->setObjectName(QStringLiteral("m_pHistogramDockWidget"));
        m_pHistogramDockWidget->setFloating(false);
        m_pHistogramDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
        dockWidgetContents_3 = new QWidget();
        dockWidgetContents_3->setObjectName(QStringLiteral("dockWidgetContents_3"));
        verticalLayout_5 = new QVBoxLayout(dockWidgetContents_3);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_6 = new QLabel(dockWidgetContents_3);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout->addWidget(label_6, 5, 0, 1, 1);

        m_pHistogramUpperBoundEdit = new QLineEdit(dockWidgetContents_3);
        m_pHistogramUpperBoundEdit->setObjectName(QStringLiteral("m_pHistogramUpperBoundEdit"));

        gridLayout->addWidget(m_pHistogramUpperBoundEdit, 1, 1, 1, 1);

        m_pHistogramLogMaxEdit = new QLineEdit(dockWidgetContents_3);
        m_pHistogramLogMaxEdit->setObjectName(QStringLiteral("m_pHistogramLogMaxEdit"));

        gridLayout->addWidget(m_pHistogramLogMaxEdit, 3, 1, 1, 1);

        m_pHistogramEyeDownEdit = new QLineEdit(dockWidgetContents_3);
        m_pHistogramEyeDownEdit->setObjectName(QStringLiteral("m_pHistogramEyeDownEdit"));

        gridLayout->addWidget(m_pHistogramEyeDownEdit, 5, 1, 1, 1);

        m_pHistogramLogMinEdit = new QLineEdit(dockWidgetContents_3);
        m_pHistogramLogMinEdit->setObjectName(QStringLiteral("m_pHistogramLogMinEdit"));

        gridLayout->addWidget(m_pHistogramLogMinEdit, 2, 1, 1, 1);

        m_pHistogramLowerBoundEdit = new QLineEdit(dockWidgetContents_3);
        m_pHistogramLowerBoundEdit->setObjectName(QStringLiteral("m_pHistogramLowerBoundEdit"));

        gridLayout->addWidget(m_pHistogramLowerBoundEdit, 0, 1, 1, 1);

        m_pHistogramEyeUpEdit = new QLineEdit(dockWidgetContents_3);
        m_pHistogramEyeUpEdit->setObjectName(QStringLiteral("m_pHistogramEyeUpEdit"));

        gridLayout->addWidget(m_pHistogramEyeUpEdit, 4, 1, 1, 1);

        label = new QLabel(dockWidgetContents_3);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_2 = new QLabel(dockWidgetContents_3);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        label_4 = new QLabel(dockWidgetContents_3);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        label_3 = new QLabel(dockWidgetContents_3);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        label_5 = new QLabel(dockWidgetContents_3);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 4, 0, 1, 1);


        verticalLayout_5->addLayout(gridLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer);

        m_pHistogramDockWidget->setWidget(dockWidgetContents_3);
        CMainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), m_pHistogramDockWidget);

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
        m_pMenuEntity->addAction(menuEffects->menuAction());
        menuActors->addAction(m_pActionModel);
        menuActors->addAction(m_pActionCamera);
        menuLights->addAction(m_pActionPoint);
        menuLights->addAction(m_pActionSun);
        menuLights->addAction(m_pActionEnvironment);
        menuLights->addAction(m_pActionGlobalProbe);
        menuEffects->addAction(m_pActionBloom);
        menuEffects->addAction(m_pActionDOF);
        menuEffects->addAction(m_pActionFXAA);
        menuEffects->addAction(m_pActionSSR);
        menuEffects->addAction(m_pActionVolumeFog);
        m_pMenuWindow->addAction(m_pActionHistogram);
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
        QObject::connect(m_pActionSun, SIGNAL(triggered()), CMainWindow, SLOT(createNewLightSun()));
        QObject::connect(m_pActionEnvironment, SIGNAL(triggered()), CMainWindow, SLOT(createNewLightEnvironment()));
        QObject::connect(m_pActionPoint, SIGNAL(triggered()), CMainWindow, SLOT(createNewLightPoint()));
        QObject::connect(m_pActionGlobalProbe, SIGNAL(triggered()), CMainWindow, SLOT(createNewLightGlobalProbe()));
        QObject::connect(m_pScenegraph, SIGNAL(entitySelected(int)), m_pInspector, SLOT(updateContentForEntity(int)));
        QObject::connect(m_pHistogramLowerBoundEdit, SIGNAL(editingFinished()), CMainWindow, SLOT(changeHistogramSettings()));
        QObject::connect(m_pHistogramUpperBoundEdit, SIGNAL(editingFinished()), CMainWindow, SLOT(changeHistogramSettings()));
        QObject::connect(m_pHistogramLogMinEdit, SIGNAL(editingFinished()), CMainWindow, SLOT(changeHistogramSettings()));
        QObject::connect(m_pHistogramLogMaxEdit, SIGNAL(editingFinished()), CMainWindow, SLOT(changeHistogramSettings()));
        QObject::connect(m_pHistogramEyeUpEdit, SIGNAL(editingFinished()), CMainWindow, SLOT(changeHistogramSettings()));
        QObject::connect(m_pHistogramEyeDownEdit, SIGNAL(editingFinished()), CMainWindow, SLOT(changeHistogramSettings()));
        QObject::connect(m_pActionHistogram, SIGNAL(triggered()), CMainWindow, SLOT(toggleHistogramDock()));
        QObject::connect(m_pActionDOF, SIGNAL(triggered()), CMainWindow, SLOT(createNewEntityDOF()));
        QObject::connect(m_pActionFXAA, SIGNAL(triggered()), CMainWindow, SLOT(createNewEntityFXAA()));
        QObject::connect(m_pActionBloom, SIGNAL(triggered()), CMainWindow, SLOT(createNewEntityBloom()));
        QObject::connect(m_pActionSSR, SIGNAL(triggered()), CMainWindow, SLOT(createNewEntitySSR()));
        QObject::connect(m_pActionVolumeFog, SIGNAL(triggered()), CMainWindow, SLOT(createNewEntityVolumeFog()));

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
        m_pActionGlobalProbe->setText(QApplication::translate("CMainWindow", "Global Probe", 0));
        m_pActionCamera->setText(QApplication::translate("CMainWindow", "Camera", 0));
        m_pActionSun->setText(QApplication::translate("CMainWindow", "Sun", 0));
        m_pActionHistogram->setText(QApplication::translate("CMainWindow", "Histogram", 0));
        actionConsole->setText(QApplication::translate("CMainWindow", "Console", 0));
        actionScene_Hraph->setText(QApplication::translate("CMainWindow", "Scene Hraph", 0));
        actionInspector->setText(QApplication::translate("CMainWindow", "Inspector", 0));
        m_pActionBloom->setText(QApplication::translate("CMainWindow", "Bloom", 0));
        m_pActionDOF->setText(QApplication::translate("CMainWindow", "DOF", 0));
        m_pActionFXAA->setText(QApplication::translate("CMainWindow", "FXAA", 0));
        m_pActionSSR->setText(QApplication::translate("CMainWindow", "SSR", 0));
        m_pActionVolumeFog->setText(QApplication::translate("CMainWindow", "Volume Fog", 0));
        m_pPlayButton->setText(QApplication::translate("CMainWindow", "Play", 0));
        m_pMailAdressEdit->setPlaceholderText(QApplication::translate("CMainWindow", "max.mustermann@mail.com", 0));
        m_pScreenshotButton->setText(QApplication::translate("CMainWindow", "Screenshot", 0));
        m_pMenuFile->setTitle(QApplication::translate("CMainWindow", "File", 0));
        m_pMenuEdit->setTitle(QApplication::translate("CMainWindow", "Edit", 0));
        m_pMenuAssets->setTitle(QApplication::translate("CMainWindow", "Assets", 0));
        m_pMenuEntity->setTitle(QApplication::translate("CMainWindow", "Entity", 0));
        menuActors->setTitle(QApplication::translate("CMainWindow", "Actors", 0));
        menuLights->setTitle(QApplication::translate("CMainWindow", "Lights", 0));
        menuEffects->setTitle(QApplication::translate("CMainWindow", "Effects", 0));
        m_pMenuComponent->setTitle(QApplication::translate("CMainWindow", "Component", 0));
        m_pMenuWindow->setTitle(QApplication::translate("CMainWindow", "Window", 0));
        m_pMenuHelp->setTitle(QApplication::translate("CMainWindow", "Help", 0));
        m_pSceneGraphDockWidget->setWindowTitle(QApplication::translate("CMainWindow", "Scene Graph", 0));
        m_pInspectorDockWidget->setWindowTitle(QApplication::translate("CMainWindow", "Inspector", 0));
        m_pConsoleDockWidget->setWindowTitle(QApplication::translate("CMainWindow", "Console", 0));
        m_pSendCommandButton->setText(QApplication::translate("CMainWindow", "Send", 0));
        m_pHistogramDockWidget->setWindowTitle(QApplication::translate("CMainWindow", "Histogram", 0));
        label_6->setText(QApplication::translate("CMainWindow", "Eye Adaption Down", 0));
        label->setText(QApplication::translate("CMainWindow", "Lower Bound", 0));
        label_2->setText(QApplication::translate("CMainWindow", "Log Min", 0));
        label_4->setText(QApplication::translate("CMainWindow", "Log Max", 0));
        label_3->setText(QApplication::translate("CMainWindow", "Upper Bound", 0));
        label_5->setText(QApplication::translate("CMainWindow", "Eye Adaption Up", 0));
    } // retranslateUi

};

namespace Ui {
    class CMainWindow: public Ui_CMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_MAINWINDOW_H
