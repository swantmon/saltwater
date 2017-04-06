
#pragma once

#include "mr/mr_control.h"

#include "mr_rgbd_camera_control.h"

#include "base/base_matrix4x4.h"
#include "base/base_console.h"

#define NOMINMAX
#include <windows.h>
#include <Kinect.h>
#include <NuiKinectFusionApi.h>

#include <cassert>
#include <mutex>

namespace MR
{
    class CKinectControl : public IRGBDCameraControl
    {
    public:

        virtual int GetCameraWidth() const;
        virtual int GetCameraHeight() const;
        virtual int GetCameraPixelCount() const;
        virtual float GetCameraFocalLengthX() const;
        virtual float GetCameraFocalLengthY() const;
        virtual float GetCameraFocalPointX() const;
        virtual float GetCameraFocalPointY() const;
        virtual bool GetCameraFrame(Base::Byte4* pBuffer);
                
        virtual int GetDepthWidth() const override;
        virtual int GetDepthHeight() const override;
        virtual int GetDepthPixelCount() const override;
        virtual float GetDepthFocalLengthX() const override;
        virtual float GetDepthFocalLengthY() const override;
        virtual float GetDepthFocalPointX() const override;
        virtual float GetDepthFocalPointY() const override;
        virtual bool GetDepthBuffer(unsigned short* pBuffer) override;

    public:

        CKinectControl();
        virtual ~CKinectControl() override;

	private:

        void Start();
        void Stop();

        IKinectSensor*     m_pKinect;
        IColorFrameReader* m_pColorFrameReader;
        IDepthFrameReader* m_pDepthFrameReader;
        ICoordinateMapper* m_pMapper;

        std::vector<Base::Byte4> m_CameraFrameBuffer;
        std::vector<UINT16> m_DepthBuffer;
        std::vector<ColorSpacePoint> m_ColorSpacePoints;
    };
} // namespace MR