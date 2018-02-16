#pragma once

#include "editor_gui/ui_edit_inspector_post_aa.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorPostAA : public QWidget, public Ui::InspectorPostAA
    {
	    Q_OBJECT

    public:
	    CInspectorPostAA(QWidget* _pParent = Q_NULLPTR);
	    ~CInspectorPostAA();

    public Q_SLOTS:

        void valueChanged();

        void RequestInformation(Base::ID _EntityID);

    private:

        Base::ID m_CurrentEntityID;

    private:

        void OnEntityInfoPostAA(Edit::CMessage& _rMessage);
    };
} // namespace Edit
