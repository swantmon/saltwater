
#pragma once

#include "mr/mr_control.h"

#include "mr_depth_sensor_control.h"

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
    class CKinectControl : public CDepthSensorControl
    {
    public:

        virtual void Start() override;
        virtual void Stop() override;
        virtual int GetWidth() override;
        virtual int GetHeight() override;
        virtual int GetPixelCount() override;
        virtual float GetFocalLengthX() override;
        virtual float GetFocalLengthY() override;
        virtual float GetFocalPointX() override;
        virtual float GetFocalPointY() override;
        virtual bool GetDepthBuffer(unsigned short* pBuffer) override;

    public:

        CKinectControl();
        virtual ~CKinectControl() override;

	private:

		IKinectSensor*            m_pKinect;
		IDepthFrameReader*        m_pDepthFrameReader;
    };
} // namespace MR