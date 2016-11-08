#pragma once

#include "editor_gui/ui_edit_inspector_fxaa.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorFXAA : public QWidget, public Ui::InspectorFXAA
    {
	    Q_OBJECT

    public:
	    CInspectorFXAA(QWidget* _pParent = Q_NULLPTR);
	    ~CInspectorFXAA();

    public Q_SLOTS:

        void valueChanged();

        void pickColorFromDialog();

        void RequestInformation(unsigned int _EntityID);

    private:

        unsigned int m_CurrentEntityID;

    private:

        void OnEntityInfoFXAA(Edit::CMessage& _rMessage);
    };
} // namespace Edit
