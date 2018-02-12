
#include "mr/mr_precompiled.h"

#include "base/base_program_parameters.h"

#include "mr_slam_reconstruction_settings.h"

namespace MR
{
    void SReconstructionSettings::SetDefaultSettings(SReconstructionSettings& _Settings)
    {
        _Settings.m_TruncatedDistance = 30.0f;
        _Settings.m_MaxIntegrationWeight = 200;
        _Settings.m_PyramidLevelCount = 3;
        _Settings.m_DepthThreshold = glm::ivec2(500, 8000);
        _Settings.m_UseFullVolumeIntegration = true;
        _Settings.m_UseReverseIntegration = true;

        _Settings.m_IsScalable = Base::CProgramParameters::GetInstance().Get("mr:slam:scalable", true);

        _Settings.m_VoxelSize = Base::CProgramParameters::GetInstance().Get("mr:slam:metrics:voxel_size", 0.004f);
        _Settings.m_VolumeSize = Base::CProgramParameters::GetInstance().Get("mr:slam:metrics:volume_size", 4.0f);
        _Settings.m_VolumeResolution = Base::CProgramParameters::GetInstance().Get("mr:slam:metrics:volume_resolution", 512);

        _Settings.m_PyramidLevelIterations[0] = 10;
        _Settings.m_PyramidLevelIterations[1] = 5;
        _Settings.m_PyramidLevelIterations[2] = 4;

        _Settings.m_GridResolutions[0] = 16;
        _Settings.m_GridResolutions[1] = 8;
        _Settings.m_GridResolutions[2] = 8;

        if (_Settings.m_IsScalable)
        {
            _Settings.m_VolumeSize = _Settings.m_VoxelSize;
        }

        for (int i = 0; i < GRID_LEVELS; ++i)
        {
            _Settings.m_VoxelsPerGrid[i] = _Settings.m_GridResolutions[i] * _Settings.m_GridResolutions[i] * _Settings.m_GridResolutions[i];
            if (_Settings.m_IsScalable)
            {
                _Settings.m_VolumeSize *= _Settings.m_GridResolutions[i];
            }
        }

        _Settings.m_CaptureColor = Base::CProgramParameters::GetInstance().Get("mr:slam:capture_color", false);
    }
} // namespace MR
