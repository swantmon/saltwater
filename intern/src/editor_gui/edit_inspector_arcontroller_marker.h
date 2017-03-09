
#pragma once

#include "editor_gui/ui_edit_inspector_arcontroller_marker.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorARControllerMarker : public QWidget, public Ui::InspectorARControllerMarker
    {
        Q_OBJECT

    public:
        CInspectorARControllerMarker(QWidget* _pParent = Q_NULLPTR);
        ~CInspectorARControllerMarker();

    public Q_SLOTS:

        void valueChanged();

        void RequestInformation(unsigned int _EntityID, unsigned int _MarkerID);

    private:

        unsigned int m_CurrentEntityID;
        unsigned int m_CurrentMarkerID;

    private:

        void OnInfoARControllerMarker(Edit::CMessage& _rMessage);
    };
} // namespace Edit

