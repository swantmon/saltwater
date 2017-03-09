#pragma once

#include "editor_gui/ui_edit_inspector_environment.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorEnvironment : public QWidget, public Ui::InspectorEnvironment
    {
        Q_OBJECT

    public:
        CInspectorEnvironment(QWidget* _pParent = Q_NULLPTR);
        ~CInspectorEnvironment();

    public Q_SLOTS:

        void valueChanged();

        void RequestInformation(unsigned int _EntityID);

    private:

        unsigned int m_CurrentEntityID;

    private:

        void OnEntityInfoEnvironment(Edit::CMessage& _rMessage);
    };
} // namespace Edit

