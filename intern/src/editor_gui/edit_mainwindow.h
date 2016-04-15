
#pragma once

#include "editor_gui/ui_edit_mainwindow.h"

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

        void CreateContext();

        void SwapWindow();

    public Q_SLOTS:

        void slot1();

    private:

        Ui::CMainWindow m_UserInterface;
    };
} // namespace Edit


