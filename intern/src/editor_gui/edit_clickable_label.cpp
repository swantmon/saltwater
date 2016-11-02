
#include "editor_gui/edit_clickable_label.h"


namespace Edit
{
    CClickableLabel::CClickableLabel(QWidget* _pParent)
        : QLabel(_pParent)
    {

    }

    // -----------------------------------------------------------------------------

    CClickableLabel::~CClickableLabel()
    {

    }

    // -----------------------------------------------------------------------------

    void CClickableLabel::mousePressEvent(QMouseEvent* _pEvent)
    {
        emit clicked();
    }
} // namespace Edit