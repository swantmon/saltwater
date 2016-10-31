
#include "base/base_vector3.h"

#include "editor_gui/edit_inspector_ssr.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorSSR::CInspectorSSR(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EntityInfoSSR, EDIT_RECEIVE_MESSAGE(&CInspectorSSR::OnEntityInfoSSR));
    }

    // -----------------------------------------------------------------------------

    CInspectorSSR::~CInspectorSSR()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorSSR::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        float Intensity            = m_pIntensityEdit->text().toFloat();
        float RoughnessMask        = m_pRoughnessMaskEdit->text().toFloat();
        float Distance             = m_pDistanceEdit->text().toFloat();
        bool  UseDoubleReflections = m_pDoubleReflectionsCB->isChecked();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutFloat(Intensity);
        NewMessage.PutFloat(RoughnessMask);
        NewMessage.PutFloat(Distance);
        NewMessage.PutBool(UseDoubleReflections);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::EntityInfoSSR, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorSSR::RequestInformation()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoSSR, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorSSR::OnEntityInfoSSR(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        float Intensity            = _rMessage.GetFloat();
        float RoughnessMask        = _rMessage.GetFloat();
        float Distance             = _rMessage.GetFloat();
        bool  UseDoubleReflections = _rMessage.GetBool();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pIntensityEdit      ->setText(QString::number(Intensity));
        m_pRoughnessMaskEdit  ->setText(QString::number(RoughnessMask));
        m_pDistanceEdit       ->setText(QString::number(Distance));
        m_pDoubleReflectionsCB->setChecked(UseDoubleReflections);
    }
} // namespace Edit
