
#include "base/base_vector3.h"

#include "editor_gui/edit_inspector_slam.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

#include "base/base_math_operations.h"
#include "mr/mr_slam_reconstructor.h"

namespace Edit
{
    CInspectorSLAM::CInspectorSLAM(QWidget* _pParent)
        : QWidget(_pParent)
        , m_IsDepthPaused(false)
        , m_IsColorPaused(false)
        , m_IsTrackingPaused(false)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        MR::CSLAMReconstructor::SReconstructionSettings DefaultSettings;
                
        m_pSizeHS->setRange(0, 5);
        m_pSizeHS->setValue(1);
        m_pSizeTL->setText(QString::number(DefaultSettings.m_VolumeSize));

        m_pTruncatedDistanceHS->setRange(1, 100);
        m_pTruncatedDistanceHS->setValue(DefaultSettings.m_TruncatedDistance);
        m_pTruncatedDistanceTL->setText(QString::number(DefaultSettings.m_TruncatedDistance));

        m_pWeightHS->setRange(1, 1000);
        m_pWeightHS->setValue(DefaultSettings.m_MaxIntegrationWeight);
        m_pWeightTL->setText(QString::number(DefaultSettings.m_MaxIntegrationWeight));

        int InitialSliderPosition = static_cast<int>(Base::Log2(static_cast<float>(DefaultSettings.m_VolumeResolution)));
        m_pResolutionHS->setRange(0, 11);
        m_pResolutionHS->setValue(InitialSliderPosition);
        m_pResolutionHS->setPageStep(1);
        m_pResolutionTL->setText(QString::number(1 << InitialSliderPosition));
        
        m_pPauseDepthIntegrationButton->setText(m_IsDepthPaused ? s_ResumeDepthText : s_PauseDepthText);
        m_pPauseColorIntegrationButton->setText(m_IsColorPaused ? s_ResumeColorText : s_PauseColorText);
        m_pPauseTrackingButton->setText(m_IsTrackingPaused ? s_ResumeTrackingText : s_PauseTrackingText);

        m_pPauseColorIntegrationButton->setVisible(DefaultSettings.m_CaptureColor);

        m_pCaptureColorCB->setChecked(DefaultSettings.m_CaptureColor);
    }

    // -----------------------------------------------------------------------------

    CInspectorSLAM::~CInspectorSLAM()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorSLAM::reset()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        
        const float VolumeSize = static_cast<float>(m_pSizeHS->value());
        const int Resolution = 1 << m_pResolutionHS->value();
        const float TruncatedDistance = static_cast<float>(m_pTruncatedDistanceHS->value());
        const int MaxIntegrationWeight = m_pWeightHS->value();
        const int MinDepth = m_pMinDepthLE->text().toInt();
        const int MaxDepth = m_pMaxDepthLE->text().toInt();
        const bool CaptureColor = m_pCaptureColorCB->checkState() == Qt::CheckState::Checked;

        m_pPauseColorIntegrationButton->setVisible(CaptureColor);

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------

        Edit::CMessage NewMessage;
        
        NewMessage.PutFloat(VolumeSize);
        NewMessage.PutInt(Resolution);
        NewMessage.PutFloat(TruncatedDistance);
        NewMessage.PutInt(MaxIntegrationWeight);
        NewMessage.PutInt(MinDepth);
        NewMessage.PutInt(MaxDepth);
        NewMessage.PutBool(CaptureColor);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorSLAM::volumeResolutionChanged(int _Value)
    {
        m_pResolutionTL->setText(QString::number(1 << _Value));
    }

    // -----------------------------------------------------------------------------

    void CInspectorSLAM::pauseDepthIntegration()
    {
        m_IsDepthPaused = !m_IsDepthPaused;

        m_pPauseDepthIntegrationButton->setText(m_IsDepthPaused ? s_ResumeDepthText : s_PauseDepthText);

        Edit::CMessage NewMessage;
        NewMessage.PutBool(m_IsDepthPaused);
        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Pause_Depth, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorSLAM::pauseColorIntegration()
    {
        m_IsColorPaused = !m_IsColorPaused;

        m_pPauseColorIntegrationButton->setText(m_IsColorPaused ? s_ResumeColorText : s_PauseColorText);

        Edit::CMessage NewMessage;
        NewMessage.PutBool(m_IsColorPaused);
        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Pause_Color, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorSLAM::pauseTracking()
    {
        m_IsTrackingPaused = !m_IsTrackingPaused;

        m_pPauseTrackingButton->setText(m_IsTrackingPaused ? s_ResumeTrackingText : s_PauseTrackingText);

        Edit::CMessage NewMessage;
        NewMessage.PutBool(m_IsTrackingPaused);
        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Pause_Tracking, NewMessage);
    }

    // -----------------------------------------------------------------------------

    const QString CInspectorSLAM::s_PauseDepthText = "Pause Depth Integration";
    const QString CInspectorSLAM::s_ResumeDepthText = "Resume Depth Integration";

    const QString CInspectorSLAM::s_PauseColorText = "Pause Color Integration";
    const QString CInspectorSLAM::s_ResumeColorText = "Resume Color Integration";

    const QString CInspectorSLAM::s_PauseTrackingText = "Pause Tracking";
    const QString CInspectorSLAM::s_ResumeTrackingText = "Resume Tracking";

    // -----------------------------------------------------------------------------

} // namespace Edit
