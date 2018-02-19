#pragma once

#include "editor_gui/ui_edit_inspector_volumefog.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorVolumeFog : public QWidget, public Ui::InspectorVolumeFog
    {
        Q_OBJECT

    public:
        CInspectorVolumeFog(QWidget* _pParent = Q_NULLPTR);
        ~CInspectorVolumeFog();

    public Q_SLOTS:

        void valueChanged();

        void pickColorFromDialog();

        void RequestInformation(Base::ID _EntityID);

    private:

        Base::ID m_CurrentEntityID;

    private:

        void OnEntityInfoVolumeFog(Edit::CMessage& _rMessage);
    };
} // namespace Edit
