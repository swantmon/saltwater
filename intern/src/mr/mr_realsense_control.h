
#pragma once

#include "mr/mr_depth_sensor_control.h"

#include "base/base_console.h"

#include <cassert>

namespace MR
{
    class CRealSenseControl : CDepthSensorControl
    {
    public:

        CRealSenseControl();
        virtual ~CRealSenseControl();

        virtual void Start();
        virtual void Stop();
        virtual int GetWidth();
        virtual int GetHeight();
        virtual int GetPixelCount();
        virtual float GetFocalLengthX();
        virtual float GetFocalLengthY();
        virtual float GetFocalPointX();
        virtual float GetFocalPointY();
        virtual bool GetDepthBuffer(unsigned short* pBuffer);

    private:
		
    };
} // namespace MR