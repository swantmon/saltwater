
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

        CRealSenseControl();
        virtual ~CRealSenseControl() override;

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