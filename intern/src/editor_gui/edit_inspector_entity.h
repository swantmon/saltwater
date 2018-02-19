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

        void enableEntity(bool _Flag);

        void RequestInformation(Base::ID _EntityID);

    private:

        Base::ID m_CurrentEntityID;

    private:

        void OnEntityInfoEntity(Edit::CMessage& _rMessage);
    };

} // namespace Edit
