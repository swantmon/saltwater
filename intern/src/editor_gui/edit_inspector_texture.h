#pragma once

#include "editor_gui/ui_edit_inspector_texture.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorTexture : public QWidget, public Ui::InspectorTexture
    {
        Q_OBJECT

    public:

        CInspectorTexture(QWidget* _pParent = Q_NULLPTR);
        ~CInspectorTexture();

    public Q_SLOTS:

        void valueChanged();

        void RequestInformation(int _Hash);

        void RequestInformation(const QString& _rRelPathToTexture);

    private:

        unsigned int m_TextureHash;

    private:

        void OnTextureInfo(Edit::CMessage& _rMessage);
    };
} // namespace Edit

