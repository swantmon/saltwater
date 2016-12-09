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

    // -----------------------------------------------------------------------------

    void CRenderContext::resizeEvent(QResizeEvent* _pResizeEvent)
    {
        CMessage NewMessage;

        NewMessage.PutInt(_pResizeEvent->size().width());
        NewMessage.PutInt(_pResizeEvent->size().height());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::ResizeMapEditWindow, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::dragEnterEvent(QDragEnterEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        if (pMimeData->hasText())
        {
            QString Text = pMimeData->text();

            QFileInfo FileInfo(Text);

            if (FileInfo.completeSuffix() == "dae" || FileInfo.completeSuffix() == "obj")
            {
                _pEvent->acceptProposedAction();
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::dropEvent(QDropEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        if (pMimeData->hasUrls())
        {
            QUrl Url = pMimeData->urls()[0];

            QFileInfo FileInfo(Url.toLocalFile());

            if (FileInfo.completeSuffix() == "dae" || FileInfo.completeSuffix() == "obj")
            {
                QDir Directory("../assets/");

                QString AbsPath = FileInfo.absoluteFilePath();

                QByteArray ModelFileBinary = Directory.relativeFilePath(AbsPath).toLatin1();

                CMessage NewMessage;

                NewMessage.PutString(ModelFileBinary.data());

                NewMessage.Reset();

                MessageManager::SendMessage(SGUIMessageType::NewActorModel, NewMessage);
            }
        }
    }
} // namespace Edit