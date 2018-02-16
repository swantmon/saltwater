#pragma once

#include "editor_gui/ui_edit_inspector_dof.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorDOF : public QWidget, public Ui::InspectorDOF
    {
	    Q_OBJECT

    public:
	    CInspectorDOF(QWidget* _pParent = Q_NULLPTR);
	    ~CInspectorDOF();

    public Q_SLOTS:

        void valueChanged();

        void RequestInformation(Base::ID _EntityID);

    private:

        Base::ID m_CurrentEntityID;

    private:

        void OnEntityInfoDOF(Edit::CMessage& _rMessage);
    };
} // namespace Edit
