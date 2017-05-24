
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
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Light_Probe_Info, EDIT_RECEIVE_MESSAGE(&CInspectorGlobalProbe::OnEntityInfoGlobalProbe));
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

        float Near = m_pNearEdit->text().toFloat();

        float Far = m_pFarEdit->text().toFloat();

        bool ParallaxCorrection = m_pParallaxCorrectionCB->isChecked();

        float BoxSizeX = m_pBoxSizeXEdit->text().toFloat();

        float BoxSizeY = m_pBoxSizeYEdit->text().toFloat();

        float BoxSizeZ = m_pBoxSizeZEdit->text().toFloat();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutInt(RefreshMode);

        NewMessage.PutInt(Type);

        NewMessage.PutInt(Quality);

        NewMessage.PutFloat(Intensity);

        NewMessage.PutFloat(Near);

        NewMessage.PutFloat(Far);

        NewMessage.PutBool(ParallaxCorrection);

        NewMessage.PutFloat(BoxSizeX);

        NewMessage.PutFloat(BoxSizeY);

        NewMessage.PutFloat(BoxSizeZ);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Light_Probe_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorGlobalProbe::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Probe_Info, NewMessage);
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

        float Near = _rMessage.GetFloat();

        float Far = _rMessage.GetFloat();

        bool ParallaxCorrection = _rMessage.GetBool();

        float BoxSizeX = _rMessage.GetFloat();

        float BoxSizeY = _rMessage.GetFloat();

        float BoxSizeZ = _rMessage.GetFloat();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pRefreshModeCB       ->blockSignals(true);
        m_pTypeCB              ->blockSignals(true);
        m_pQualityCB           ->blockSignals(true);
        m_pParallaxCorrectionCB->blockSignals(true);

        m_pRefreshModeCB->setCurrentIndex(RefreshMode);

        m_pTypeCB->setCurrentIndex(Type);

        m_pQualityCB->setCurrentIndex(Quality);

        m_pIntensityEdit->setText(QString::number(Intensity));

        m_pNearEdit->setText(QString::number(Near));

        m_pFarEdit->setText(QString::number(Far));

        m_pParallaxCorrectionCB->setChecked(ParallaxCorrection);

        m_pBoxSizeXEdit->setText(QString::number(BoxSizeX));

        m_pBoxSizeYEdit->setText(QString::number(BoxSizeY));

        m_pBoxSizeZEdit->setText(QString::number(BoxSizeZ));

        m_pRefreshModeCB       ->blockSignals(false);
        m_pTypeCB              ->blockSignals(false);
        m_pQualityCB           ->blockSignals(false);
        m_pParallaxCorrectionCB->blockSignals(false);
    }
} // namespace Edit

