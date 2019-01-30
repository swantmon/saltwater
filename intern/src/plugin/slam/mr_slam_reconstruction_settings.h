
#pragma once

#include "base/base_include_glm.h"

namespace MR
{
	struct SReconstructionSettings
	{
		const static int MAX_PYRAMIDLEVELS = 8;
		const static int GRID_LEVELS = 3;

		float m_VolumeSize;
		int m_VolumeResolution;
		float m_VoxelSize;
		float m_TruncatedDistance;
		int m_MaxIntegrationWeight;
		int m_PyramidLevelCount;
        glm::ivec3 m_PyramidLevelIterations;
		glm::ivec2 m_DepthThreshold;
		bool m_CaptureColor;
        
		glm::ivec3  m_GridResolutions;
        glm::ivec3  m_VoxelsPerGrid;
                
        static void SetDefaultSettings(SReconstructionSettings& _Settings);

    private:

        static void SetupDefaultSettings();

        static bool s_IsInitialized;
        static SReconstructionSettings s_DefaultSettings;
	};
} // namespace MR
