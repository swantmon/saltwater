
#pragma once

#include "editor_gui/ui_edit_tone_mapping.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <QFileSystemModel>
#include <QWidget>

namespace Edit
{
    class CToneMapping : public QWidget, public Ui::ToneMapping
    {
        Q_OBJECT

    public:

        CToneMapping(QWidget * parent = Q_NULLPTR);
        ~CToneMapping();

    public Q_SLOTS:

        void pickTintFromDialog();
        void pickDarkTintFromDialog();
        void pickRedFromDialog();
        void pickGreenFromDialog();
        void pickBlueFromDialog();


        void changeToneMappingSettings();
    };
} // namespace Edit