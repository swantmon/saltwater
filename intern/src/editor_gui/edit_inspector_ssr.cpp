
#include "editor_gui/edit_inspector_ssr.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorSSR::CInspectorSSR(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID(static_cast<unsigned int>(-1))
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Effect_SSR_Info, EDIT_RECEIVE_MESSAGE(&CInspectorSSR::OnEntityInfoSSR));
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

        NewMessage.PutInt(m_CurrentEntityID);
        NewMessage.PutFloat(Intensity);
        NewMessage.PutFloat(RoughnessMask);
        NewMessage.PutFloat(Distance);
        NewMessage.PutBool(UseDoubleReflections);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Effect_SSR_Update, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorSSR::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Effect_SSR_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorSSR::OnEntityInfoSSR(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int   EntityID             = _rMessage.GetInt();
        float Intensity            = _rMessage.GetFloat();
        float RoughnessMask        = _rMessage.GetFloat();
        float Distance             = _rMessage.GetFloat();
        bool  UseDoubleReflections = _rMessage.GetBool();

        BASE_UNUSED(EntityID);

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pIntensityEdit      ->setText(QString::number(Intensity));
        m_pRoughnessMaskEdit  ->setText(QString::number(RoughnessMask));
        m_pDistanceEdit       ->setText(QString::number(Distance));
        m_pDoubleReflectionsCB->setChecked(UseDoubleReflections);
    }
} // namespace Edit
