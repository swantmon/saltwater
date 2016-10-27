
#include "edit_inspector_pointlight.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorPointlight::CInspectorPointlight(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Color picker
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pPickColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Qt::white));

        m_pPickColorButton->setPalette(ButtonPalette);

        m_pPickColorButton->update();
    }

    // -----------------------------------------------------------------------------

    CInspectorPointlight::~CInspectorPointlight()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorPointlight::pickColorFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pPickColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pPickColorButton->setPalette(ButtonPalette);

        m_pPickColorButton->update();
    }
} // namespace Edit
