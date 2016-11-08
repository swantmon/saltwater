#pragma once

#include "editor_gui/ui_edit_inspector_bloom.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorBloom : public QWidget, public Ui::InspectorBloom
    {
	    Q_OBJECT

    public:
	    CInspectorBloom(QWidget* _pParent = Q_NULLPTR);
	    ~CInspectorBloom();

    public Q_SLOTS:

        void valueChanged();

        void pickColorFromDialog();

        void RequestInformation(unsigned int _EntityID);

    private:

        unsigned int m_CurrentEntityID;

    private:

        void OnEntityInfoBloom(Edit::CMessage& _rMessage);
    };
} // namespace Edit
