//
//  mr_slam_reconstructor.h
//  graphic
//
//  Created by Tobias Schwandt on 12/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_vector2.h"

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
		int m_PyramidLevelIterations[MAX_PYRAMIDLEVELS];
		Base::Int2 m_DepthThreshold;
		bool m_CaptureColor;

		bool m_IsScalable;

		int m_GridResolutions[GRID_LEVELS];
        int m_VoxelsPerGrid[GRID_LEVELS];

        bool m_UseFullVolumeIntegration;
        bool m_UseReverseIntegration;
        
        static void SetDefaultSettings(SReconstructionSettings& _Settings);
	};
} // namespace MR
