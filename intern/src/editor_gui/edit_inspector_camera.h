#pragma once

#include "editor_gui/ui_edit_inspector_camera.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorCamera : public QWidget, public Ui::InspectorCamera
    {
        Q_OBJECT

    public:

        CInspectorCamera(QWidget* _pParent = Q_NULLPTR);
        ~CInspectorCamera();

    public Q_SLOTS:

        void valueChanged();

        void fieldOfViewValueChanged(int _Value);

        void pickColorFromDialog();

        void RequestInformation(Base::ID _EntityID);

    private:

        Base::ID m_CurrentEntityID;

    private:

        void OnEntityInfoCamera(Edit::CMessage& _rMessage);

    };
} // namespace Edit


