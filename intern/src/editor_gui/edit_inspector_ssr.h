#pragma once

#include "editor_gui/ui_edit_inspector_ssr.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorSSR : public QWidget, public Ui::InspectorSSR
    {
	    Q_OBJECT

    public:
	    CInspectorSSR(QWidget* _pParent = Q_NULLPTR);
	    ~CInspectorSSR();

    public Q_SLOTS:

        void valueChanged();

        void RequestInformation(Base::ID _EntityID);

    private:

        Base::ID m_CurrentEntityID;

    private:

        void OnEntityInfoSSR(Edit::CMessage& _rMessage);
    };
} // namespace Edit
