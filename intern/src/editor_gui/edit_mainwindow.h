
#pragma once

#include "editor_gui/ui_edit_mainwindow.h"
#include "editor_gui/edit_newscenedialog.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <QCloseEvent>
#include <QLabel>
#include <QMainWindow>

namespace Edit
{
    class CMainWindow : public QMainWindow, public Ui::CMainWindow
    {
        Q_OBJECT

    public:

        CMainWindow(QWidget* _pParent = Q_NULLPTR);
        ~CMainWindow();

    public:

        void* GetEditorWindowHandle();

        void OnStart();
        void OnExit();

        void SetRenderSize(int _Width, int _Height);

    public Q_SLOTS:

        void switchPlayingCurrentScene();
        void openNewSceneDialog();
        void createNewLightPoint();
        void createNewLightSun();
        void createNewLightEnvironment();
        void createNewLightGlobalProbe();
        void createNewLightArea();
        void createNewEntityBloom();
        void createNewEntityDOF();
        void createNewEntityPostAA();
        void createNewEntitySSR();
        void createNewEntityVolumeFog();
        void createNewPluginARController();
        void createNewSLAMReconstruction();
        void reloadRenderer();
        void reloadAllShader();
        void changeHistogramSettings();
        void toggleHistogramDock();
        void toggleSceneGraphDock();
        void toggleInspectorDock();
        void toggleConsoleDock();
        void toggleAssetsDock();
        void changeResolution(int _Index);

    private:

        QLabel*               m_pStatusLabel;
        CNewSceneDialog*      m_pNewSceneDialog;
        bool                  m_IsPlaying;
        bool                  m_IsSceneLoaded;

    private:

        void closeEvent(QCloseEvent* _pEvent);
        void mousePressEvent(QMouseEvent* _pMouseEvent);

    private:

        void OnStateChange(Edit::CMessage& _rMessage);
        void OnFramesPerSecond(Edit::CMessage& _rMessage);
        void OnHistogramInfo(Edit::CMessage& _rMessage);
    };
} // namespace Edit


