
#include "edit_render_context.h"

#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMimeData>
#include <QResizeEvent>
#include <QUrl>

#include <assert.h>

namespace Edit
{
    CRenderContext::CRenderContext(QWidget* _pParent)
        : QWidget(_pParent)
    {
        setAcceptDrops(true);
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

        NewMessage.Put((int)(CharCode));
        NewMessage.Put(_pKeyEvent->modifiers());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Input_KeyPressed, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::keyReleaseEvent(QKeyEvent* _pKeyEvent)
    {
        CMessage NewMessage;

        char CharCode = _pKeyEvent->text()[0].toLatin1();

        NewMessage.Put((int)(CharCode));
        NewMessage.Put(_pKeyEvent->modifiers());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Input_KeyReleased, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::mouseMoveEvent(QMouseEvent* _pMouseEvent)
    {
        CMessage NewMessage;

        NewMessage.Put(_pMouseEvent->globalX());
        NewMessage.Put(_pMouseEvent->globalY());
        NewMessage.Put(_pMouseEvent->x());
        NewMessage.Put(_pMouseEvent->y());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Input_MouseMove, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::mousePressEvent(QMouseEvent* _pMouseEvent)
    {
        setFocus();

        CMessage NewMessage;

        NewMessage.Put(_pMouseEvent->globalX());
        NewMessage.Put(_pMouseEvent->globalY());
        NewMessage.Put(_pMouseEvent->x());
        NewMessage.Put(_pMouseEvent->y());

        NewMessage.Reset();

        switch (_pMouseEvent->button())
        {
        case Qt::LeftButton:
            MessageManager::SendMessage(SGUIMessageType::Input_MouseLeftPressed, NewMessage);
            break;
        case Qt::MiddleButton:
            MessageManager::SendMessage(SGUIMessageType::Input_MouseMiddlePressed, NewMessage);
            break;
        case Qt::RightButton:
            MessageManager::SendMessage(SGUIMessageType::Input_MouseRightPressed, NewMessage);
            break;
        }
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::mouseReleaseEvent(QMouseEvent* _pMouseEvent)
    {
        CMessage NewMessage;

        NewMessage.Put(_pMouseEvent->globalX());
        NewMessage.Put(_pMouseEvent->globalY());
        NewMessage.Put(_pMouseEvent->x());
        NewMessage.Put(_pMouseEvent->y());

        NewMessage.Reset();

        switch (_pMouseEvent->button())
        {
        case Qt::LeftButton:
            MessageManager::SendMessage(SGUIMessageType::Input_MouseLeftReleased, NewMessage);
            break;
        case Qt::MiddleButton:
            MessageManager::SendMessage(SGUIMessageType::Input_MouseMiddleReleased, NewMessage);
            break;
        case Qt::RightButton:
            MessageManager::SendMessage(SGUIMessageType::Input_MouseRightReleased, NewMessage);
            break;
        }
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::wheelEvent(QWheelEvent* _pWheelEvent)
    {
        CMessage NewMessage;

        NewMessage.Put(_pWheelEvent->orientation() == Qt::Vertical);
        NewMessage.Put(_pWheelEvent->delta());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Input_MouseWheel, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::resizeEvent(QResizeEvent* _pResizeEvent)
    {
        CMessage NewMessage;

        NewMessage.Put(_pResizeEvent->size().width());
        NewMessage.Put(_pResizeEvent->size().height());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Window_Resize, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::dragEnterEvent(QDragEnterEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        if (pMimeData->hasFormat("SW_MODEL_REL_PATH") == false) return;

        _pEvent->acceptProposedAction();
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::dropEvent(QDropEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        assert(pMimeData->hasFormat("SW_MODEL_REL_PATH"));

        QString RelativePathToFile = pMimeData->data("SW_MODEL_REL_PATH");

        QByteArray ModelFileBinary = RelativePathToFile.toLatin1();

        CMessage NewMessage;

        NewMessage.PutString(ModelFileBinary.data());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Load, NewMessage);
    }
} // namespace Edit