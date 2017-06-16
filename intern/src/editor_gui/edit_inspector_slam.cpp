
#include "base/base_vector3.h"

#include "editor_gui/edit_inspector_slam.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

#include "base/base_math_operations.h"
#include "mr/mr_slam_reconstructor.h"

namespace
{
    const int g_ResolutionCount = 8;

    int g_Resolutions[g_ResolutionCount] =
    {
        128,
        192,
        256,
        384,
        512,
        640,
        768,
        1024,
    };
}

namespace Edit
{
    CInspectorSLAM::CInspectorSLAM(QWidget* _pParent)
        : QWidget(_pParent)
        , m_IsIntegrationPaused(false)
        , m_IsTrackingPaused(false)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        MR::SReconstructionSettings DefaultSettings;
        
        m_pSizeHS->setRange(1, 1000);
        m_pSizeHS->setValue(DefaultSettings.m_VolumeSize * 100);
        m_pSizeTL->setText(QString::number(DefaultSettings.m_VolumeSize));

		m_pVoxelSizeHS->setRange(1, 10);
		m_pVoxelSizeHS->setValue(DefaultSettings.m_VoxelSize);
		m_pVoxelSizeTL->setText(QString::number(DefaultSettings.m_VoxelSize));

        m_pTruncatedDistanceHS->setRange(1, 100);
        m_pTruncatedDistanceHS->setValue(DefaultSettings.m_TruncatedDistance);
        m_pTruncatedDistanceTL->setText(QString::number(DefaultSettings.m_TruncatedDistance));

        m_pWeightHS->setRange(1, 1000);
        m_pWeightHS->setValue(DefaultSettings.m_MaxIntegrationWeight);
        m_pWeightTL->setText(QString::number(DefaultSettings.m_MaxIntegrationWeight));

        const int InitialSliderPosition = 4;
        m_pResolutionHS->setRange(0, g_ResolutionCount - 1);
        m_pResolutionHS->setValue(InitialSliderPosition);
        m_pResolutionHS->setPageStep(1);
        m_pResolutionTL->setText(QString::number(g_Resolutions[InitialSliderPosition]));
        
        m_pPauseIntegrationButton->setText(m_IsIntegrationPaused ? s_ResumeIntegrationText : s_PauseIntegrationText);
        m_pPauseTrackingButton->setText(m_IsTrackingPaused ? s_ResumeTrackingText : s_PauseTrackingText);
        
        m_pCaptureColorCB->setChecked(DefaultSettings.m_CaptureColor);

        m_pTrackingCameraCB->setChecked(true);

		const bool isScalable = true;

		m_pScalableCB->setChecked(isScalable);
		m_pSizeHS->setEnabled(!isScalable);
		m_pResolutionHS->setEnabled(!isScalable);
		m_pVoxelSizeHS->setEnabled(isScalable);
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
        
        const float VolumeSize = static_cast<float>(m_pSizeHS->value() / 100.0f);
        const int Resolution = g_Resolutions[m_pResolutionHS->value()];
		const int VoxelSize = static_cast<float>(m_pSizeHS->value());
        const float TruncatedDistance = static_cast<float>(m_pTruncatedDistanceHS->value());
        const int MaxIntegrationWeight = m_pWeightHS->value();
        const int MinDepth = m_pMinDepthLE->text().toInt();
        const int MaxDepth = m_pMaxDepthLE->text().toInt();
        const bool CaptureColor = m_pCaptureColorCB->checkState() == Qt::CheckState::Checked;
        
        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------

        Edit::CMessage NewMessage;
        
        NewMessage.PutFloat(VolumeSize);
        NewMessage.PutInt(Resolution);
		NewMessage.PutInt(VoxelSize);
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
        const int SliderPosition = m_pResolutionHS->value();
        assert(SliderPosition >= 0 && SliderPosition < g_ResolutionCount);

        m_pResolutionTL->setText(QString::number(g_Resolutions[SliderPosition]));
    }

    // -----------------------------------------------------------------------------

    void CInspectorSLAM::volumeSizeChanged(int _Value)
    {
        m_pSizeTL->setText(QString::number(_Value / 100.0f));
    }

	// -----------------------------------------------------------------------------

	void CInspectorSLAM::voxelSizeChanged(int _Value)
	{
		m_pVoxelSizeTL->setText(QString::number(_Value / 100.0f));
	}
    
    // -----------------------------------------------------------------------------

    void CInspectorSLAM::pauseIntegration()
    {
        m_IsIntegrationPaused = !m_IsIntegrationPaused;

        m_pPauseIntegrationButton->setText(m_IsIntegrationPaused ? s_ResumeIntegrationText : s_PauseIntegrationText);

        Edit::CMessage NewMessage;
        NewMessage.PutBool(m_IsIntegrationPaused);
        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Pause_Integration, NewMessage);
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

    void CInspectorSLAM::changeCamera()
    {
        const bool UseTrackingCamera = m_pTrackingCameraCB->checkState() == Qt::CheckState::Checked;

        Edit::CMessage NewMessage;
        NewMessage.PutBool(UseTrackingCamera);
        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Change_Camera, NewMessage);
    }

    // -----------------------------------------------------------------------------

	void CInspectorSLAM::scalableChanged(int State)
	{
		const bool isScalable = State == Qt::CheckState::Checked;

		m_pSizeHS->setEnabled(!isScalable);
		m_pResolutionHS->setEnabled(!isScalable); 
		m_pVoxelSizeHS->setEnabled(isScalable);
	}

	// -----------------------------------------------------------------------------

    const QString CInspectorSLAM::s_PauseIntegrationText = "Pause Integration";
    const QString CInspectorSLAM::s_ResumeIntegrationText = "Resume Integration";

    const QString CInspectorSLAM::s_PauseTrackingText = "Pause Tracking";
    const QString CInspectorSLAM::s_ResumeTrackingText = "Resume Tracking";

    // -----------------------------------------------------------------------------

} // namespace Edit
