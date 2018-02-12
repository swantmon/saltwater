
#include "mr/mr_precompiled.h"

#include "base/base_program_parameters.h"

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
        s_DefaultSettings.m_TruncatedDistance = 30.0f;
        s_DefaultSettings.m_MaxIntegrationWeight = 200;
        s_DefaultSettings.m_PyramidLevelCount = 3;
        s_DefaultSettings.m_DepthThreshold = glm::ivec2(500, 8000);
        s_DefaultSettings.m_UseFullVolumeIntegration = true;
        s_DefaultSettings.m_UseReverseIntegration = true;

        s_DefaultSettings.m_IsScalable = Base::CProgramParameters::GetInstance().Get("mr:slam:scalable", true);

        s_DefaultSettings.m_VoxelSize = Base::CProgramParameters::GetInstance().Get("mr:slam:metrics:voxel_size", 0.004f);
        s_DefaultSettings.m_VolumeSize = Base::CProgramParameters::GetInstance().Get("mr:slam:metrics:volume_size", 4.0f);
        s_DefaultSettings.m_VolumeResolution = Base::CProgramParameters::GetInstance().Get("mr:slam:metrics:volume_resolution", 512);

        std::vector<int> PyramidLevelIterations = Base::CProgramParameters::GetInstance().Get<std::vector<int>>("mr:slam:tracking_iterations", { 10, 5, 4 });

        assert(PyramidLevelIterations.size() == s_DefaultSettings.m_PyramidLevelCount);

        s_DefaultSettings.m_PyramidLevelIterations[0] = PyramidLevelIterations[0];
        s_DefaultSettings.m_PyramidLevelIterations[1] = PyramidLevelIterations[1];
        s_DefaultSettings.m_PyramidLevelIterations[2] = PyramidLevelIterations[2];

        s_DefaultSettings.m_GridResolutions[0] = 16;
        s_DefaultSettings.m_GridResolutions[1] = 8;
        s_DefaultSettings.m_GridResolutions[2] = 8;

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

        s_DefaultSettings.m_CaptureColor = Base::CProgramParameters::GetInstance().Get("mr:slam:capture_color", false);

        s_IsInitialized = true;
    }

} // namespace MR
