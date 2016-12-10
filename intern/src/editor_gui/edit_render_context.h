
#pragma once

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <QWidget>

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
        void resizeEvent(QResizeEvent* _pResizeEvent);

        void dragEnterEvent(QDragEnterEvent* _pEvent);
        void dropEvent(QDropEvent* _pEvent);
    };
} // namespace Edit