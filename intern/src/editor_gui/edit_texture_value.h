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

    public:

        void SetTextureFile(const QString& _rTextureFile);
        const QString& GetTextureFile();

        void SetTextureHash(unsigned int _Hash);
        unsigned int GetTextureHash();

    Q_SIGNALS:

        void hashChanged(unsigned int _Hash);
        void fileChanged(const QString& _rFile);

    public Q_SLOTS:

        void hashValueChanged();
        void fileValueChanged();

    private:

        QString m_CurrentTextureFile;
        unsigned int m_CurrentTextureHash;

    private:

        void dragEnterEvent(QDragEnterEvent* _pEvent);
        void dropEvent(QDropEvent* _pEvent);
    };
} // namespace Edit
