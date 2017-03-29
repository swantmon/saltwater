#pragma once

#include "editor_gui/ui_edit_inspector_slam.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorSLAM : public QWidget, public Ui::InspectorSLAM
    {
	    Q_OBJECT

    public:
        CInspectorSLAM(QWidget* _pParent = Q_NULLPTR);
	    ~CInspectorSLAM();

    public Q_SLOTS:

        void valueChanged();
        void sliderValueChanged(int);
    };
} // namespace Edit
