#pragma once

#include "editor_gui/ui_edit_texture_value.h"

#include <QRegularExpression>
#include <QWidget>

// TODO by tschwandt
// - Add remove texture possibility
// - Setup style

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
            NoHash    = 2,
            NoFile    = 4,
        };

    public:

        CTextureValue(QWidget* _pParent = Q_NULLPTR);
        ~CTextureValue();

    public:

        void SetLayout(unsigned int _Layout);

        void SetSupportedFiles(const QString& _rSupportedFiles);

        void SetTextureFile(const QString& _rTextureFile);
        const QString& GetTextureFile() const;

        void SetTextureHash(unsigned int _Hash);
        unsigned int GetTextureHash() const;

    Q_SIGNALS:

        void hashChanged(unsigned int _Hash);
        void fileChanged(const QString& _rFile);

    public Q_SLOTS:

        void hashValueChanged();
        void fileValueChanged();

    private:

        QRegularExpression m_SupportedFiles;
        QString            m_File;
        unsigned int       m_Hash;

    private:

        void dragEnterEvent(QDragEnterEvent* _pEvent);
        void dropEvent(QDropEvent* _pEvent);
    };
} // namespace Edit
