
#include "plugin/slam/slam_precompiled.h"

#include "engine/core/core_program_parameters.h"

#include "mr_slam_reconstruction_settings.h"

#include <vector>

namespace MR
{
    void SReconstructionSettings::SetDefaultSettings(SReconstructionSettings& _Settings)
    {
        if (!s_IsInitialized)
        {
            SetupDefaultSettings();
        }
        _Settings = s_DefaultSettings;
    }

    bool SReconstructionSettings::s_IsInitialized = false;
    SReconstructionSettings SReconstructionSettings::s_DefaultSettings = {};

    void SReconstructionSettings::SetupDefaultSettings()
    {
        s_DefaultSettings.m_TruncatedDistance = Core::CProgramParameters::GetInstance().Get("mr:slam:truncated_distance", 30.0f);
        s_DefaultSettings.m_MaxIntegrationWeight = Core::CProgramParameters::GetInstance().Get("mr:slam:max_weight", 200);
        s_DefaultSettings.m_PyramidLevelCount = 3;
        s_DefaultSettings.m_DepthThreshold = Core::CProgramParameters::GetInstance().Get("mr:slam:depth_threshold", glm::ivec2(500, 3000));

        s_DefaultSettings.m_IsScalable = Core::CProgramParameters::GetInstance().Get("mr:slam:scalable", true);

        s_DefaultSettings.m_VoxelSize = Core::CProgramParameters::GetInstance().Get("mr:slam:metrics:voxel_size", 0.002f);
        s_DefaultSettings.m_VolumeSize = Core::CProgramParameters::GetInstance().Get("mr:slam:metrics:volume_size", 4.0f);
        s_DefaultSettings.m_VolumeResolution = Core::CProgramParameters::GetInstance().Get("mr:slam:metrics:volume_resolution", 512);

        s_DefaultSettings.m_PyramidLevelIterations = Core::CProgramParameters::GetInstance().Get("mr:slam:tracking_iterations", glm::ivec3(5, 5, 4));
        
        s_DefaultSettings.m_GridResolutions = glm::ivec3(16, 8, 8);

        if (s_DefaultSettings.m_IsScalable)
        {
            s_DefaultSettings.m_VolumeSize = s_DefaultSettings.m_VoxelSize;
        }

        for (int i = 0; i < GRID_LEVELS; ++i)
        {
            s_DefaultSettings.m_VoxelsPerGrid[i] =
                s_DefaultSettings.m_GridResolutions[i] *
                s_DefaultSettings.m_GridResolutions[i] *
                s_DefaultSettings.m_GridResolutions[i];

            if (s_DefaultSettings.m_IsScalable)
            {
                s_DefaultSettings.m_VolumeSize *= s_DefaultSettings.m_GridResolutions[i];
            }
        }

        s_DefaultSettings.m_CaptureColor = Core::CProgramParameters::GetInstance().Get("mr:slam:capture_color", true);

        s_IsInitialized = true;
    }

} // namespace MR
