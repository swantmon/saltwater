
#include "editor_gui/edit_clickable_label.h"

#include <QMouseEvent>

namespace Edit
{
    CClickableLabel::CClickableLabel(QWidget* _pParent)
        : QLabel             (_pParent)
        , m_LastMousePosition()
    {

    }

    // -----------------------------------------------------------------------------

    CClickableLabel::~CClickableLabel()
    {

    }

    // -----------------------------------------------------------------------------

    void CClickableLabel::mousePressEvent(QMouseEvent* _pEvent)
    {
        m_LastMousePosition = _pEvent->pos();

        emit clicked();
    }

    // -----------------------------------------------------------------------------

    void CClickableLabel::mouseMoveEvent(QMouseEvent* _pEvent)
    {
        QPoint CurrentMousePosition = _pEvent->pos();

        emit mousePositionDifference(CurrentMousePosition - m_LastMousePosition);
    }
} // namespace Edit