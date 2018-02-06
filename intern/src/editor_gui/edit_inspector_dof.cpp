
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

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutFloat(Near);
        NewMessage.PutFloat(Far);
        NewMessage.PutFloat(NearToFarRatio);
        NewMessage.PutFloat(FadeUnSmall);
        NewMessage.PutFloat(FadeSmallMedium);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Effect_DOF_Update, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorDOF::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Effect_DOF_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorDOF::OnEntityInfoDOF(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        float Near            = _rMessage.GetFloat();
        float Far             = _rMessage.GetFloat();
        float NearToFarRatio  = _rMessage.GetFloat();
        float FadeUnSmall     = _rMessage.GetFloat();
        float FadeSmallMedium = _rMessage.GetFloat();

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
