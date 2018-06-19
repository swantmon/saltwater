
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

        NewMessage.Put(m_CurrentEntityID);
        NewMessage.Put(Intensity);
        NewMessage.Put(RoughnessMask);
        NewMessage.Put(Distance);
        NewMessage.Put(UseDoubleReflections);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Effect_SSR_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorSSR::RequestInformation(Base::ID _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Effect_SSR_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorSSR::OnEntityInfoSSR(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::ID EntityID             = _rMessage.Get<Base::ID>();
        float    Intensity            = _rMessage.Get<float>();
        float    RoughnessMask        = _rMessage.Get<float>();
        float    Distance             = _rMessage.Get<float>();
        bool     UseDoubleReflections = _rMessage.Get<bool>();

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
