#pragma once

#include <QLabel>

namespace Edit
{
    class CClickableLabel : public QLabel
    {
        Q_OBJECT

    public:
        CClickableLabel(QWidget* _pParent = Q_NULLPTR);
        ~CClickableLabel();

    Q_SIGNALS:
        void clicked();

    protected:
        void mousePressEvent(QMouseEvent* _pEvent);
    };
} // namespace Edit
