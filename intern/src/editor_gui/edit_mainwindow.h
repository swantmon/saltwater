
#pragma once

#include "editor_gui/ui_edit_mainwindow.h"

#include <QCloseEvent>
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

    public Q_SLOTS:

        void slot1();

    private:

        Ui::CMainWindow m_UserInterface;

    private:

        void closeEvent(QCloseEvent* _pEvent);
        void keyPressEvent(QKeyEvent* _pKeyEvent);
        void keyReleaseEvent(QKeyEvent* _pKeyEvent);
        void mouseMoveEvent(QMouseEvent* _pMouseEvent);
        void mousePressEvent(QMouseEvent* _pMouseEvent);
        void mouseReleaseEvent(QMouseEvent* _pMouseEvent);
    };
} // namespace Edit


