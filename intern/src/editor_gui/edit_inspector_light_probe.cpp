
#include "editor_gui/edit_inspector_light_probe.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspectorLightProbe::CInspectorLightProbe(QWidget* _pParent) 
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
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Light_Probe_Info, EDIT_RECEIVE_MESSAGE(&CInspectorLightProbe::OnEntityInfoGlobalProbe));
    }

    // -----------------------------------------------------------------------------

    CInspectorLightProbe::~CInspectorLightProbe() 
    {

    }

    void CInspectorLightProbe::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // read values
        // -----------------------------------------------------------------------------
        int RefreshMode = m_pRefreshModeCB->currentIndex();

        int Type = m_pTypeCB->currentIndex();

        int Quality = m_pQualityCB->currentIndex();

        int ClearFlag = m_pClearFlagCB->currentIndex();

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

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Put(RefreshMode);

        NewMessage.Put(Type);

        NewMessage.Put(Quality);

        NewMessage.Put(ClearFlag);

        NewMessage.Put(Intensity);

        NewMessage.Put(Near);

        NewMessage.Put(Far);

        NewMessage.Put(ParallaxCorrection);

        NewMessage.Put(BoxSizeX);

        NewMessage.Put(BoxSizeY);

        NewMessage.Put(BoxSizeZ);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Light_Probe_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorLightProbe::RequestInformation(Base::ID _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Probe_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorLightProbe::OnEntityInfoGlobalProbe(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        if (EntityID != m_CurrentEntityID) return;

        int RefreshMode = _rMessage.Get<int>();

        int Type = _rMessage.Get<int>();

        int Quality = _rMessage.Get<int>();

        int ClearFlag = _rMessage.Get<int>();

        float Intensity = _rMessage.Get<float>();

        float Near = _rMessage.Get<float>();

        float Far = _rMessage.Get<float>();

        bool ParallaxCorrection = _rMessage.Get<bool>();

        float BoxSizeX = _rMessage.Get<float>();

        float BoxSizeY = _rMessage.Get<float>();

        float BoxSizeZ = _rMessage.Get<float>();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pRefreshModeCB       ->blockSignals(true);
        m_pTypeCB              ->blockSignals(true);
        m_pQualityCB           ->blockSignals(true);
        m_pClearFlagCB         ->blockSignals(true);
        m_pParallaxCorrectionCB->blockSignals(true);

        m_pRefreshModeCB->setCurrentIndex(RefreshMode);

        m_pTypeCB->setCurrentIndex(Type);

        m_pQualityCB->setCurrentIndex(Quality);

        m_pClearFlagCB->setCurrentIndex(ClearFlag);

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
        m_pClearFlagCB         ->blockSignals(false);
        m_pParallaxCorrectionCB->blockSignals(false);
    }
} // namespace Edit

