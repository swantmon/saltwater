
#pragma once

#include "mr/mr_control.h"

struct INuiFusionReconstruction;
struct IKinectSensor;
struct IDepthFrameReader;
struct _Matrix4;
typedef _Matrix4 Matrix4;
struct _NUI_FUSION_IMAGE_FRAME;
typedef _NUI_FUSION_IMAGE_FRAME NUI_FUSION_IMAGE_FRAME;

namespace MR
{
    class CKinectControl
    {
    public:

		static const int VoxelCountX = 256;
		static const int VoxelCountY = 256;
		static const int VoxelCountZ = 256;
		static const int VoxelsPerMeter = 256;
		static const int VoxelCount = VoxelCountX * VoxelCountY * VoxelCountZ;

        CKinectControl();
        ~CKinectControl();

    public:

        void Start();
        void Stop();

        void Update();
		void ExportVolumeBlock(short* pVolumeBlock);

	private:

		IKinectSensor*            m_pKinect;
		INuiFusionReconstruction* m_pVolume;
		IDepthFrameReader*        m_pDepthFrameReader;
		NUI_FUSION_IMAGE_FRAME*   m_pDepthImageFrame;
		NUI_FUSION_IMAGE_FRAME*   m_pPointCloud;
		NUI_FUSION_IMAGE_FRAME*   m_pShadedSurface;
		Matrix4*                  m_pTransform;
    };
} // namespace MR