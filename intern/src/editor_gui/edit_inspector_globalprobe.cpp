
#include "editor_gui/edit_inspector_globalprobe.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspectorGlobalProbe::CInspectorGlobalProbe(QWidget* _pParent) 
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EntityInfoGlobalProbe, EDIT_RECEIVE_MESSAGE(&CInspectorGlobalProbe::OnEntityInfoGlobalProbe));
    }

    // -----------------------------------------------------------------------------

    CInspectorGlobalProbe::~CInspectorGlobalProbe() 
    {

    }

    void CInspectorGlobalProbe::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        int RefreshMode = m_pRefreshModeCB->currentIndex();

        int Type = m_pTypeCB->currentIndex();

        int Quality = m_pQualityCB->currentIndex();

        float Intensity = m_pIntensityEdit->text().toFloat();

        NewMessage.PutInt(RefreshMode);

        NewMessage.PutInt(Type);

        NewMessage.PutInt(Quality);

        NewMessage.PutFloat(Intensity);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::EntityInfoGlobalProbe, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorGlobalProbe::RequestInformation()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoGlobalProbe, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorGlobalProbe::OnEntityInfoGlobalProbe(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
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

