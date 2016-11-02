#pragma once

#include "editor_gui/ui_edit_inspector_transformation.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <QWidget>

namespace Edit
{
    class CInspectorTransformation : public QWidget, public Ui::InspectorTransformation
    {
	    Q_OBJECT

    public:

	    CInspectorTransformation(QWidget* _pParent = Q_NULLPTR);
	    ~CInspectorTransformation();

    public Q_SLOTS:

        void valueChanged();

        void mousePositionDifference(QPoint _Difference);

        void RequestInformation();

    private:

        void OnEntityInfoTransformation(Edit::CMessage& _rMessage);
    };
} // namespace Edit