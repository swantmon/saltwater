
#pragma once

#include "mr/mr_control.h"

struct INuiFusionReconstruction;
struct IKinectSensor;
struct IDepthFrameReader;
struct _Matrix4;
typedef _Matrix4 Matrix4;
struct _NUI_FUSION_IMAGE_FRAME;
typedef _NUI_FUSION_IMAGE_FRAME NUI_FUSION_IMAGE_FRAME;
struct _DepthSpacePoint;
typedef _DepthSpacePoint DepthSpacePoint;

namespace MR
{
    class CKinectControl
    {
    public:

        CKinectControl();
        ~CKinectControl();

    public:

        void Start(int VoxelCountX, int VoxelCountY, int VoxelCountZ, float VoxelsPerMeter);
        void Stop();

        void Update();
		void ExportVolumeBlock(short* pVolumeBlock);

	private:

        int m_VoxelCountX;
        int m_VoxelCountY;
        int m_VoxelCountZ;
        float m_VoxelsPerMeter;
        int m_VoxelCount;

		IKinectSensor*            m_pKinect;
		INuiFusionReconstruction* m_pVolume;
		IDepthFrameReader*        m_pDepthFrameReader;
		NUI_FUSION_IMAGE_FRAME*   m_pDepthImageFrame;
		NUI_FUSION_IMAGE_FRAME*   m_pPointCloud;
		NUI_FUSION_IMAGE_FRAME*   m_pShadedSurface;
		Matrix4*                  m_pTransform;

        unsigned short* m_pDepthImagePixelBuffer;
        DepthSpacePoint* m_pDepthDistortionMap;
        unsigned int* m_pDepthDistortionLT;
        
        __int64 m_CoordinateMappingChangedEvent;

        double m_TimeSinceLastUpdate;
        bool m_VolumeExported;
    };
} // namespace MR