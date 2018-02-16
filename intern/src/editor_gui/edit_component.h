
#pragma once

#include "editor_gui/ui_edit_component.h"

#include <QWidget>

namespace Edit
{
    class CComponent : public QWidget, public Ui::Component
    {
        Q_OBJECT

    public:

        CComponent(QWidget * parent = Q_NULLPTR);
        ~CComponent();
    };
} // namespace Edit