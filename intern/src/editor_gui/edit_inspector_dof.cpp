
#include "editor_gui/edit_inspector_dof.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorDOF::CInspectorDOF(QWidget* _pParent)
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
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Effect_DOF_Info, EDIT_RECEIVE_MESSAGE(&CInspectorDOF::OnEntityInfoDOF));
    }

    // -----------------------------------------------------------------------------

    CInspectorDOF::~CInspectorDOF()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorDOF::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        float Near            = m_pNearEdit->text().toFloat();
        float Far             = m_pFarEdit->text().toFloat();
        float NearToFarRatio  = m_pNearToFarRatioEdit->text().toFloat();
        float FadeUnSmall     = m_pFadeUnSmallEdit->text().toFloat();
        float FadeSmallMedium = m_pFadeSmallMediumEdit->text().toFloat();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Put(Near);
        NewMessage.Put(Far);
        NewMessage.Put(NearToFarRatio);
        NewMessage.Put(FadeUnSmall);
        NewMessage.Put(FadeSmallMedium);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Effect_DOF_Update, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorDOF::RequestInformation(Base::ID _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Effect_DOF_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorDOF::OnEntityInfoDOF(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        if (EntityID != m_CurrentEntityID) return;

        float Near            = _rMessage.Get<float>();
        float Far             = _rMessage.Get<float>();
        float NearToFarRatio  = _rMessage.Get<float>();
        float FadeUnSmall     = _rMessage.Get<float>();
        float FadeSmallMedium = _rMessage.Get<float>();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pNearEdit->setText(QString::number(Near));
        m_pFarEdit->setText(QString::number(Far));
        m_pNearToFarRatioEdit->setText(QString::number(NearToFarRatio));
        m_pFadeUnSmallEdit->setText(QString::number(FadeUnSmall));
        m_pFadeSmallMediumEdit->setText(QString::number(FadeSmallMedium));
    }
} // namespace Edit
