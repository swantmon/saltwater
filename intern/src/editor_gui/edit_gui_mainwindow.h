#pragma once
#include <QMainWindow>
#include "ui_edit_gui_mainwindow.h"

class edit_gui_mainwindow : public QMainWindow {
	Q_OBJECT

public:
	edit_gui_mainwindow(QWidget * parent = Q_NULLPTR);
	~edit_gui_mainwindow();

    public Q_SLOTS:

    void slot1();

private:
	Ui::edit_gui_mainwindow ui;
};
