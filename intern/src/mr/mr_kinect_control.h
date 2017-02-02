
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

        CKinectControl();
        virtual ~CKinectControl();

    public:

        virtual void Start();
        virtual void Stop();

        virtual bool GetDepthBuffer(unsigned short* pBuffer);

        virtual int GetWidth();
        virtual int GetHeight();
        virtual int GetPixelCount();
        
        virtual float GetFocalLengthX();
        virtual float GetFocalLengthY();
        virtual float GetFocalPointX();
        virtual float GetFocalPointY();
	private:

		IKinectSensor*            m_pKinect;
		IDepthFrameReader*        m_pDepthFrameReader;
    };
} // namespace MR