
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
        , m_IsPaused(false)
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
        
        m_pPauseButton->setText(m_IsPaused ? s_ResumeText : s_PauseText);
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

    void CInspectorSLAM::pause()
    {
        m_IsPaused = !m_IsPaused;

        m_pPauseButton->setText(m_IsPaused ? s_ResumeText : s_PauseText);

        Edit::CMessage NewMessage;
        NewMessage.PutBool(m_IsPaused);
        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Pause, NewMessage);
    }

    // -----------------------------------------------------------------------------

    const QString CInspectorSLAM::s_PauseText = "Pause Integration";
    const QString CInspectorSLAM::s_ResumeText = "Resume Integration";

    // -----------------------------------------------------------------------------

} // namespace Edit
