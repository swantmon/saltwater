
#pragma once

#include "editor_gui/ui_edit_inspector_arcontroller.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorARController : public QWidget, public Ui::InspectorARController
    {
        Q_OBJECT

    public:
        CInspectorARController(QWidget* _pParent = Q_NULLPTR);
        ~CInspectorARController();

    public Q_SLOTS:

        void valueChanged();

        void numberOfMarkerChanged();

        void RequestInformation(Base::ID _EntityID);

    private:

        Base::ID m_CurrentEntityID;

    private:

        void OnEntityInfoARController(Edit::CMessage& _rMessage);
    };
} // namespace Edit

