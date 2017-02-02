
#pragma once

namespace MR
{
    class CDepthSensorControl
    {
    public:

        virtual ~CDepthSensorControl() {};

        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual int GetWidth() = 0;
        virtual int GetHeight() = 0;
        virtual int GetPixelCount() = 0;
        virtual float GetFocalLengthX() = 0;
        virtual float GetFocalLengthY() = 0;
        virtual float GetFocalPointX() = 0;
        virtual float GetFocalPointY() = 0;
        virtual bool GetDepthBuffer(unsigned short* pBuffer) = 0;
    };
} // namespace MR