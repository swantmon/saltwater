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
        void mousePositionDifference(QPoint _Difference);

    protected:

        void mousePressEvent(QMouseEvent* _pEvent);
        void mouseMoveEvent(QMouseEvent* _pEvent);

    private:

        QPoint m_LastMousePosition;
    };
} // namespace Edit
