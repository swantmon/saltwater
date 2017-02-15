
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
    class CKinectControl : public IDepthSensorControl
    {
    public:

        virtual void Start() override;
        virtual void Stop() override;
        virtual int GetWidth() const override;
        virtual int GetHeight() const override;
        virtual int GetPixelCount() const override;
        virtual float GetFocalLengthX() const override;
        virtual float GetFocalLengthY() const override;
        virtual float GetFocalPointX() const override;
        virtual float GetFocalPointY() const override;
        virtual bool GetDepthBuffer(unsigned short* pBuffer) override;

    public:

        CKinectControl();
        virtual ~CKinectControl() override;

	private:

		IKinectSensor*            m_pKinect;
		IDepthFrameReader*        m_pDepthFrameReader;
    };
} // namespace MR