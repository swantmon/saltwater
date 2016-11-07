#pragma once

#include "editor_gui/ui_edit_inspector_entity.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorEntity : public QWidget, public Ui::InspectorEntity
    {
        Q_OBJECT

    public:
        CInspectorEntity(QWidget* _pParent = Q_NULLPTR);
        ~CInspectorEntity();

    public Q_SLOTS:

        void valueChanged();

        void RequestInformation();

    private:

        void OnEntityInfoEntity(Edit::CMessage& _rMessage);
    };

} // namespace Edit
