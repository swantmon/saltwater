
#include "editor_gui/edit_inspector_slam.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

#include "base/base_program_parameters.h"
#include "mr/mr_slam_reconstructor.h"

namespace
{
    const int g_ResolutionCount = 11;

    int g_Resolutions[g_ResolutionCount] =
    {
		32,
		64,
		92,
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
        MR::SReconstructionSettings::SetDefaultSettings(DefaultSettings);
        
        m_pSizeHS->setRange(1, 1000);

        if (DefaultSettings.m_IsScalable)
        {
            int VoxelCount = 1;
            for (int i = 0; i < DefaultSettings.GRID_LEVELS; ++i)
            {
                VoxelCount *= DefaultSettings.m_GridResolutions[i];
            }

            float VolumeSize = DefaultSettings.m_VoxelSize * VoxelCount;

            m_pSizeTL->setText(QString::number(VolumeSize));
            m_pSizeHS->setValue(VolumeSize * 100);
        }
        else
        {
            m_pSizeHS->setValue(DefaultSettings.m_VolumeSize * 100);
            m_pSizeTL->setText(QString::number(DefaultSettings.m_VolumeSize));
        }

		m_pVoxelSizeHS->setRange(1, 10);
		m_pVoxelSizeHS->setValue(DefaultSettings.m_VoxelSize * 1000.0f); // meter to millimeter
		m_pVoxelSizeHS->setPageStep(1);
		m_pVoxelSizeTL->setText(QString::number(DefaultSettings.m_VoxelSize * 1000.0f));

        m_pTruncatedDistanceHS->setRange(1, 100);
        m_pTruncatedDistanceHS->setValue(DefaultSettings.m_TruncatedDistance);
        m_pTruncatedDistanceTL->setText(QString::number(DefaultSettings.m_TruncatedDistance));

        m_pWeightHS->setRange(1, 1000);
        m_pWeightHS->setValue(DefaultSettings.m_MaxIntegrationWeight);
        m_pWeightTL->setText(QString::number(DefaultSettings.m_MaxIntegrationWeight));

        const int InitialSliderPosition = 5;
        m_pResolutionHS->setRange(0, g_ResolutionCount - 1);
        m_pResolutionHS->setValue(InitialSliderPosition);
        m_pResolutionHS->setPageStep(1);
        m_pResolutionTL->setText(QString::number(g_Resolutions[InitialSliderPosition]));
        
        m_pPauseIntegrationButton->setText(m_IsIntegrationPaused ? s_ResumeIntegrationText : s_PauseIntegrationText);
        m_pPauseTrackingButton->setText(m_IsTrackingPaused ? s_ResumeTrackingText : s_PauseTrackingText);
        
        m_pCaptureColorCB->setChecked(DefaultSettings.m_CaptureColor);

        m_pTrackingCameraCB->setChecked(Base::CProgramParameters::GetInstance().Get("mr:slam:rendering:use_tracking_camera", true));

		const bool IsScalable = DefaultSettings.m_IsScalable;

		m_pScalableCB->setChecked(IsScalable);
		m_pSizeHS->setEnabled(!IsScalable);
		m_pResolutionHS->setEnabled(!IsScalable);
		m_pVoxelSizeHS->setEnabled(IsScalable);
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
        
        float VolumeSize = static_cast<float>(m_pSizeHS->value() / 100.0f);
        int Resolution = g_Resolutions[m_pResolutionHS->value()];
		float VoxelSize = static_cast<float>(m_pVoxelSizeHS->value() / 1000.0f);
        float TruncatedDistance = static_cast<float>(m_pTruncatedDistanceHS->value());
        int MaxIntegrationWeight = m_pWeightHS->value();
        int MinDepth = m_pMinDepthLE->text().toInt();
        int MaxDepth = m_pMaxDepthLE->text().toInt();
        bool CaptureColor = m_pCaptureColorCB->checkState() == Qt::CheckState::Checked;
		bool IsScalable = m_pScalableCB->checkState() == Qt::CheckState::Checked;

        if (IsScalable)
        {
            MR::SReconstructionSettings DefaultSettings;

            int VoxelCount = 1;
            for (int i = 0; i < DefaultSettings.GRID_LEVELS; ++ i)
            {
                VoxelCount *= DefaultSettings.m_GridResolutions[i];
            }

            VolumeSize = VoxelSize * VoxelCount;

            m_pSizeTL->setText(QString::number(VolumeSize));
            m_pSizeHS->setValue(VolumeSize * 100);
        }
        
        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------

        Edit::CMessage NewMessage;
        
        NewMessage.Put<float>(VolumeSize);
        NewMessage.Put<int>(Resolution);
		NewMessage.Put<float>(VoxelSize);
        NewMessage.Put<float>(TruncatedDistance);
        NewMessage.Put<int>(MaxIntegrationWeight);
        NewMessage.Put<int>(MinDepth);
        NewMessage.Put<int>(MaxDepth);
        NewMessage.Put<bool>(CaptureColor);
		NewMessage.Put<bool>(IsScalable);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorSLAM::volumeResolutionChanged(int _Value)
    {
        (void)_Value;

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
        NewMessage.Put<bool>(m_IsIntegrationPaused);
        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Pause_Integration, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorSLAM::pauseTracking()
    {
        m_IsTrackingPaused = !m_IsTrackingPaused;

        m_pPauseTrackingButton->setText(m_IsTrackingPaused ? s_ResumeTrackingText : s_PauseTrackingText);

        Edit::CMessage NewMessage;
        NewMessage.Put<bool>(m_IsTrackingPaused);
        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Pause_Tracking, NewMessage);
    }

    void CInspectorSLAM::changeCamera()
    {
        const bool UseTrackingCamera = m_pTrackingCameraCB->checkState() == Qt::CheckState::Checked;

        Edit::CMessage NewMessage;
        NewMessage.Put<bool>(UseTrackingCamera);
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
