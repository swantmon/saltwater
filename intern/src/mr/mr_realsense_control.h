
#pragma once

#include "mr/mr_depth_sensor_control.h"

#include "base/base_console.h"

#include "RealSense/Capture.h"
#include "RealSense/SenseManager.h"

#include <cassert>

namespace MR
{
    class CRealSenseControl : public CDepthSensorControl
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

        template<typename T>
        void SafeRelease(T& pObject)
        {
            if (pObject != nullptr)
            {
                pObject->Release();
                pObject = nullptr;
            }
        }
        
        Intel::RealSense::SenseManager* m_pSenseManager;
        Intel::RealSense::CaptureManager* m_pCaptureManager;
        Intel::RealSense::Device* m_pDevice;

        Intel::RealSense::PointF32 m_FocalLength;
        Intel::RealSense::PointF32 m_FocalPoint;

        const static int s_Width = 628;
        const static int s_Height = 468;
    };
} // namespace MR