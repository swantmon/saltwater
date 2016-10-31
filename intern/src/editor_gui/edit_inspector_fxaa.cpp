
#include "base/base_vector3.h"

#include "editor_gui/edit_inspector_fxaa.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorFXAA::CInspectorFXAA(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Color picker
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pLumaColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Qt::white));

        m_pLumaColorButton->setPalette(ButtonPalette);

        m_pLumaColorButton->update();

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EntityInfoFXAA, EDIT_RECEIVE_MESSAGE(&CInspectorFXAA::OnEntityInfoFXAA));
    }

    // -----------------------------------------------------------------------------

    CInspectorFXAA::~CInspectorFXAA()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorFXAA::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pLumaColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        Base::Float3 Color = Base::Float3(RGB.blue() / 255.0f, RGB.green() / 255.0f, RGB.red() / 255.0f);

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutFloat(Color[0]);
        NewMessage.PutFloat(Color[1]);
        NewMessage.PutFloat(Color[2]);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::EntityInfoFXAA, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorFXAA::pickColorFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pLumaColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pLumaColorButton->setPalette(ButtonPalette);

        m_pLumaColorButton->update();

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorFXAA::RequestInformation()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoFXAA, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorFXAA::OnEntityInfoFXAA(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::Int3 Color = Base::Int3(_rMessage.GetFloat() * 255, _rMessage.GetFloat() * 255, _rMessage.GetFloat() * 255);

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pLumaColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Color[0], Color[1], Color[2]));

        m_pLumaColorButton->setPalette(ButtonPalette);

        m_pLumaColorButton->update();
    }
} // namespace Edit
