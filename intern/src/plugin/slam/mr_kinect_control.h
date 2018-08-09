
#pragma once

#include "engine/core/core_console.h"

#define NOMINMAX
#include <windows.h>
#include <Kinect.h>
#include <NuiKinectFusionApi.h>

#include <cassert>
#include <vector>

namespace MR
{
    class CKinectControl
    {
    public:

        int GetCameraWidth() const;
        int GetCameraHeight() const;
        int GetCameraPixelCount() const;
        float GetCameraFocalLengthX() const;
        float GetCameraFocalLengthY() const;
        float GetCameraFocalPointX() const;
        float GetCameraFocalPointY() const;
        bool GetCameraFrame(char* pBuffer);
                
        int GetDepthWidth() const;
        int GetDepthHeight() const;
        int GetDepthPixelCount() const;
        float GetDepthFocalLengthX() const;
        float GetDepthFocalLengthY() const;
        float GetDepthFocalPointX() const;
        float GetDepthFocalPointY() const;
		float GetMinDepth() const;
		float GetMaxDepth() const;
        bool GetDepthBuffer(unsigned short* pBuffer);

    public:

        CKinectControl();
       ~CKinectControl();

	private:

        struct Byte4
        {
            BYTE r, g, b, a;
        };

        void Start();
        void Stop();

        IKinectSensor*     m_pKinect;
        IColorFrameReader* m_pColorFrameReader;
        IDepthFrameReader* m_pDepthFrameReader;
        ICoordinateMapper* m_pMapper;

        std::vector<Byte4> m_CameraFrameBuffer;
        std::vector<UINT16> m_DepthBuffer;
        std::vector<ColorSpacePoint> m_ColorSpacePoints;
    };
} // namespace MR