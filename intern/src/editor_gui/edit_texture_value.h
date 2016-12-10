#pragma once

#include "editor_gui/ui_edit_texture_value.h"

#include <QWidget>

namespace Edit
{
    class CTextureValue : public QWidget, public Ui::TextureValue
    {
        Q_OBJECT

    public:

        CTextureValue(QWidget* _pParent = Q_NULLPTR);
        ~CTextureValue();

    private:

        QString m_CurrentTextureFile;
        unsigned int m_CurrentTextureHash;

    private:

        void dragEnterEvent(QDragEnterEvent* _pEvent);
        void dropEvent(QDropEvent* _pEvent);
    };
} // namespace Edit
