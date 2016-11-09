
#include "editor_gui/edit_inspector_globalprobe.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspectorGlobalProbe::CInspectorGlobalProbe(QWidget* _pParent) 
        : QWidget          (_pParent)
        , m_CurrentEntityID(-1)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::LightInfoGlobalProbe, EDIT_RECEIVE_MESSAGE(&CInspectorGlobalProbe::OnEntityInfoGlobalProbe));
    }

    // -----------------------------------------------------------------------------

    CInspectorGlobalProbe::~CInspectorGlobalProbe() 
    {

    }

    void CInspectorGlobalProbe::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // read values
        // -----------------------------------------------------------------------------
        int RefreshMode = m_pRefreshModeCB->currentIndex();

        int Type = m_pTypeCB->currentIndex();

        int Quality = m_pQualityCB->currentIndex();

        float Intensity = m_pIntensityEdit->text().toFloat();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutInt(RefreshMode);

        NewMessage.PutInt(Type);

        NewMessage.PutInt(Quality);

        NewMessage.PutFloat(Intensity);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::LightInfoGlobalProbe, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorGlobalProbe::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestLightInfoGlobalProbe, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorGlobalProbe::OnEntityInfoGlobalProbe(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        int RefreshMode = _rMessage.GetInt();

        int Type = _rMessage.GetInt();

        int Quality = _rMessage.GetInt();

        float Intensity = _rMessage.GetFloat();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pRefreshModeCB->blockSignals(true);
        m_pTypeCB       ->blockSignals(true);
        m_pQualityCB    ->blockSignals(true);

        m_pRefreshModeCB->setCurrentIndex(RefreshMode);

        m_pTypeCB->setCurrentIndex(Type);

        m_pQualityCB->setCurrentIndex(Quality);

        m_pIntensityEdit->setText(QString::number(Intensity));

        m_pRefreshModeCB->blockSignals(false);
        m_pTypeCB       ->blockSignals(false);
        m_pQualityCB    ->blockSignals(false);
    }
} // namespace Edit

