
#pragma once

#include "editor_gui/ui_edit_mainwindow.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <QCloseEvent>
#include <QLabel>
#include <QMainWindow>

namespace Edit
{
    class CMainWindow : public QMainWindow
    {
        Q_OBJECT

    public:

        CMainWindow(QWidget* _pParent = Q_NULLPTR);
        ~CMainWindow();

    public:

        void* GetEditorWindowHandle();

    private:

        Ui::CMainWindow m_UserInterface;
        QLabel*         m_pStatusLabel;

    private:

        void closeEvent(QCloseEvent* _pEvent);
        void keyPressEvent(QKeyEvent* _pKeyEvent);
        void keyReleaseEvent(QKeyEvent* _pKeyEvent);
        void mouseMoveEvent(QMouseEvent* _pMouseEvent);
        void mousePressEvent(QMouseEvent* _pMouseEvent);
        void mouseReleaseEvent(QMouseEvent* _pMouseEvent);

    private:

        void OnFramesPerSecond(Edit::CMessage& _rMessage);
    };
} // namespace Edit


