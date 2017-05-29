#pragma once

#include "editor_gui/ui_edit_inspector_light_probe.h"

#include "editor_port/edit_message.h"

#include <QWidget>


namespace Edit
{
    class CInspectorLightProbe : public QWidget, public Ui::InspectorLightProbe
    {
        Q_OBJECT

    public:
        CInspectorLightProbe(QWidget* _pParent = Q_NULLPTR);
        ~CInspectorLightProbe();

    public Q_SLOTS:

        void valueChanged();

        void RequestInformation(unsigned int _EntityID);

    private:

        unsigned int m_CurrentEntityID;

    private:

        void OnEntityInfoGlobalProbe(Edit::CMessage& _rMessage);
    };
} // namespace Edit

