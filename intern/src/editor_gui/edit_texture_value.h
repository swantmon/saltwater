#pragma once

#include "editor_gui/ui_edit_texture_value.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CTextureValue : public QWidget, public Ui::TextureValue
    {
        Q_OBJECT

    public:

        enum ELayout
        {
            All       = 0,
            NoPreview = 1,
            NoFile    = 2
        };

    public:

        CTextureValue(QWidget* _pParent = Q_NULLPTR);
        ~CTextureValue();

    public:

        void SetLayout(unsigned int _Layout);

        void SetTextureFile(const QString& _rTextureFile);
        const QString& GetTextureFile() const;


    Q_SIGNALS:

        void fileChanged(const QString& _rFile);

    public Q_SLOTS:

        void fileValueChanged();

    private:

        QString m_File;

    private:

        void dragEnterEvent(QDragEnterEvent* _pEvent);
        void dropEvent(QDropEvent* _pEvent);

    private:

        void OnTextureInfo(Edit::CMessage& _rMessage);

    private:

        void LoadTexture(const QString& _rPathToTexture);
    };
} // namespace Edit
