#include "edit_render_context.h"

namespace Edit
{
    CRenderContext::CRenderContext(QWidget * parent) 
        : QWidget(parent) 
    {
    }

    // -----------------------------------------------------------------------------

    CRenderContext::~CRenderContext() 
    {

    }

    // -----------------------------------------------------------------------------

    void CRenderContext::keyPressEvent(QKeyEvent* _pKeyEvent)
    {
        CMessage NewMessage;

        char CharCode = _pKeyEvent->text()[0].toLatin1();

        NewMessage.PutInt((int)(CharCode));
        NewMessage.PutInt(_pKeyEvent->modifiers());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::KeyPressed, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::keyReleaseEvent(QKeyEvent* _pKeyEvent)
    {
        CMessage NewMessage;

        char CharCode = _pKeyEvent->text()[0].toLatin1();

        NewMessage.PutInt((int)(CharCode));
        NewMessage.PutInt(_pKeyEvent->modifiers());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::KeyReleased, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::mouseMoveEvent(QMouseEvent* _pMouseEvent)
    {
        CMessage NewMessage;

        NewMessage.PutInt(_pMouseEvent->globalX());
        NewMessage.PutInt(_pMouseEvent->globalY());
        NewMessage.PutInt(_pMouseEvent->x());
        NewMessage.PutInt(_pMouseEvent->y());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::MouseMove, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::mousePressEvent(QMouseEvent* _pMouseEvent)
    {
        setFocus();

        CMessage NewMessage;

        NewMessage.PutInt(_pMouseEvent->globalX());
        NewMessage.PutInt(_pMouseEvent->globalY());
        NewMessage.PutInt(_pMouseEvent->x());
        NewMessage.PutInt(_pMouseEvent->y());

        NewMessage.Reset();

        switch (_pMouseEvent->button())
        {
        case Qt::LeftButton:
            MessageManager::SendMessage(SGUIMessageType::MouseLeftPressed, NewMessage);
            break;
        case Qt::MiddleButton:
            MessageManager::SendMessage(SGUIMessageType::MouseMiddlePressed, NewMessage);
            break;
        case Qt::RightButton:
            MessageManager::SendMessage(SGUIMessageType::MouseRightPressed, NewMessage);
            break;
        }
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::mouseReleaseEvent(QMouseEvent* _pMouseEvent)
    {
        CMessage NewMessage;

        NewMessage.PutInt(_pMouseEvent->globalX());
        NewMessage.PutInt(_pMouseEvent->globalY());
        NewMessage.PutInt(_pMouseEvent->x());
        NewMessage.PutInt(_pMouseEvent->y());

        NewMessage.Reset();

        switch (_pMouseEvent->button())
        {
        case Qt::LeftButton:
            MessageManager::SendMessage(SGUIMessageType::MouseLeftReleased, NewMessage);
            break;
        case Qt::MiddleButton:
            MessageManager::SendMessage(SGUIMessageType::MouseMiddleReleased, NewMessage);
            break;
        case Qt::RightButton:
            MessageManager::SendMessage(SGUIMessageType::MouseRightReleased, NewMessage);
            break;
        }
    }
} // namespace Edit