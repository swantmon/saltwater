#pragma once

#include "editor_gui/ui_edit_inspector_pointlight.h"

#include <QWidget>

namespace Edit
{
    class CInspectorPointlight : public QWidget, public Ui::InspectorPointlight
    {
	    Q_OBJECT

    public:
	    CInspectorPointlight(QWidget* _pParent = Q_NULLPTR);
	    ~CInspectorPointlight();

    public Q_SLOTS:

        void pickColorFromDialog();
    };
} // namespace Edit
