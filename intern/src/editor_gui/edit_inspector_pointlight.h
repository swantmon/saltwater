#pragma once

#include "editor_gui/ui_edit_inspector_point_light.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorPointlight : public QWidget, public Ui::InspectorPointLight
    {
	    Q_OBJECT

    public:
	    CInspectorPointlight(QWidget* _pParent = Q_NULLPTR);
	    ~CInspectorPointlight();

    public Q_SLOTS:

        void valueChanged();

        void pickColorFromDialog();

        void RequestInformation(unsigned int _EntityID);

    private:

        int m_CurrentEntityID;

    private:

        void OnEntityInfoPointlight(Edit::CMessage& _rMessage);
    };
} // namespace Edit
