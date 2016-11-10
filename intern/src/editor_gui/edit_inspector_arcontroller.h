
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

        void RequestInformation(unsigned int _EntityID);

    private:

        unsigned int m_CurrentEntityID;

    private:

        void OnEntityInfoARController(Edit::CMessage& _rMessage);
    };
} // namespace Edit

