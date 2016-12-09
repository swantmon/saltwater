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

        MessageManager::SendMessage(SGUIMessageType::Input_KeyPressed, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CRenderContext::keyReleaseEvent(QKeyEvent* _pKeyEvent)
    {
        CMessage NewMessage;

        char CharCode = _pKeyEvent->text()[0].toLatin1();

        NewMessage.PutInt((int)(CharCode));
        NewMessage.PutInt(_pKeyEvent->modifiers());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Input_KeyReleased, NewMessage);
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

        MessageManager::SendMessage(SGUIMessageType::Input_MouseMove, NewMessage);
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

        NewMessage.PutInt(_pMouseEvent->globalX());
        NewMessage.PutInt(_pMouseEvent->globalY());
        NewMessage.PutInt(_pMouseEvent->x());
        NewMessage.PutInt(_pMouseEvent->y());

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

    void CRenderContext::resizeEvent(QResizeEvent* _pResizeEvent)
    {
        CMessage NewMessage;

        NewMessage.PutInt(_pResizeEvent->size().width());
        NewMessage.PutInt(_pResizeEvent->size().height());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Window_Resize, NewMessage);
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

                MessageManager::SendMessage(SGUIMessageType::Actor_Model_New, NewMessage);
            }
        }
    }
} // namespace Edit