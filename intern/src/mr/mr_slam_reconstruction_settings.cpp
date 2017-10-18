
#include "mr/mr_precompiled.h"

#include "base/base_program_parameters.h"

#include "mr_slam_reconstruction_settings.h"

namespace MR
{    
    SReconstructionSettings::SReconstructionSettings()
        : m_VolumeSize(5.0f)
        , m_VolumeResolution(256)
		, m_VoxelSize(0.004f)
        , m_TruncatedDistance(30.0f)
        , m_MaxIntegrationWeight(200)
        , m_PyramidLevelCount(3)
        , m_DepthThreshold(500, 8000)
        , m_CaptureColor(false)
		, m_IsScalable(false)
        , m_UseFullVolumeIntegration(true)
        , m_UseReverseIntegration(true)
    {
        m_IsScalable = Base::CProgramParameters::GetInstance().GetBoolean("scalable_slam");

        m_PyramidLevelIterations[0] = 10;
        m_PyramidLevelIterations[1] = 5;
        m_PyramidLevelIterations[2] = 4;

		m_GridResolutions[0] = 16;
		m_GridResolutions[1] =  8;
		m_GridResolutions[2] =  8;

        for (int i = 0; i < GRID_LEVELS; ++ i)
        {
            m_VoxelsPerGrid[i] = m_GridResolutions[i] * m_GridResolutions[i] * m_GridResolutions[i];
        }
    }
} // namespace MR
