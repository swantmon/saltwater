
#include "base/base_vector3.h"

#include "editor_gui/edit_inspector_slam.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorSLAM::CInspectorSLAM(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);
    }

    // -----------------------------------------------------------------------------

    CInspectorSLAM::~CInspectorSLAM()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorSLAM::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        
        const float VolumeSize = m_pVolumeSizeLE->text().toFloat();
        const int Resolution = m_pVolumeResolutionLE->text().toInt();
        const float TruncatedDistance = m_pTruncatedDistanceLE->text().toFloat();
        const int MaxIntegrationWeight = m_pMaxIntegrationWeightLE->text().toInt();
        const int MinDepth = m_pMinDepthLE->text().toInt();
        const int MaxDepth = m_pMaxDepthLE->text().toInt();

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

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Info, NewMessage);
    }    
} // namespace Edit
