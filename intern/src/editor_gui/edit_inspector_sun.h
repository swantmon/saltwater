#pragma once

#include "editor_gui/ui_edit_inspector_sun.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorSun : public QWidget, public Ui::InspectorSun
    {
        Q_OBJECT

    public:

        CInspectorSun(QWidget* _pParent = Q_NULLPTR);
        ~CInspectorSun();

   public Q_SLOTS:

        void valueChanged();

        void pickColorFromDialog();

        void RequestInformation();

    private:

        void OnEntityInfoSun(Edit::CMessage& _rMessage);
    };
} // namespace Edit

