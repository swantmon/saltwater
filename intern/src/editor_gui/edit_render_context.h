
#pragma once

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <QWidget>
#include <QKeyEvent>

namespace Edit
{
    class CRenderContext : public QWidget
    {
        Q_OBJECT

    public:

        CRenderContext(QWidget * parent = Q_NULLPTR);
        ~CRenderContext();

    private:

        void keyPressEvent(QKeyEvent* _pKeyEvent);
        void keyReleaseEvent(QKeyEvent* _pKeyEvent);
        void mouseMoveEvent(QMouseEvent* _pMouseEvent);
        void mousePressEvent(QMouseEvent* _pMouseEvent);
        void mouseReleaseEvent(QMouseEvent* _pMouseEvent);
    };
} // namespace Edit