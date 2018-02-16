
#include "edit_component.h"

namespace Edit
{
    CComponent::CComponent(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);
    }

    // -----------------------------------------------------------------------------

    CComponent::~CComponent()
    {
    }
} // namespace Edit