
#include "editor_gui/edit_inspector_pointlight.h"

#include "editor_port/edit_message_manager.h"

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

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EntityInfoPointlight, EDIT_RECEIVE_MESSAGE(&CInspectorPointlight::OnEntityInfoPointlight));
    }

    // -----------------------------------------------------------------------------

    CInspectorPointlight::~CInspectorPointlight()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorPointlight::valueChanged()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::EntityInfoPointlight, NewMessage);
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

    // -----------------------------------------------------------------------------

    void CInspectorPointlight::RequestInformation()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoPointlight, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorPointlight::OnEntityInfoPointlight(Edit::CMessage& _rMessage)
    {

    }
} // namespace Edit
