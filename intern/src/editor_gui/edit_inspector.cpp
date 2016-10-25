
#include "editor_gui/edit_inspector.h"

#include "editor_gui/ui_edit_inspector_transformation.h"

namespace Edit
{
    CInspector::CInspector(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Create layout
        // -----------------------------------------------------------------------------
        QVBoxLayout* pInspectorLayout = new QVBoxLayout();

        // -----------------------------------------------------------------------------
        // Create transformation widget and add to layout
        // -----------------------------------------------------------------------------
        QWidget* pTransformWidget = new QWidget();

        Ui::InspectorForm InspectorUI;

        InspectorUI.setupUi(pTransformWidget); 

        pInspectorLayout->addWidget(pTransformWidget);

        // -----------------------------------------------------------------------------
        // Set layout
        // -----------------------------------------------------------------------------
        setLayout(pInspectorLayout);
    }

    CInspector::~CInspector() 
    {

    }
} // namespace Edit