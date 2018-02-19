#pragma once

#include "editor_gui/ui_edit_inspector_area_light.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorArealight : public QWidget, public Ui::InspectorAreaLight
    {
        Q_OBJECT

    public:
        CInspectorArealight(QWidget* _pParent = Q_NULLPTR);
        ~CInspectorArealight();

    public Q_SLOTS:

        void valueChanged();

        void pickColorFromDialog();

        void RequestInformation(Base::ID _EntityID);

    private:

        Base::ID m_CurrentEntityID;

    private:

        void OnEntityInfoArealight(Edit::CMessage& _rMessage);
    };
} // namespace Edit
