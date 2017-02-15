
#pragma once

namespace MR
{
    class IDepthSensorControl
    {
    public:

        virtual ~IDepthSensorControl() {};

        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual int GetWidth() const = 0 ;
        virtual int GetHeight() const = 0;
        virtual int GetPixelCount() const = 0;
        virtual float GetFocalLengthX() const = 0;
        virtual float GetFocalLengthY() const = 0;
        virtual float GetFocalPointX() const = 0;
        virtual float GetFocalPointY() const = 0;
        virtual bool GetDepthBuffer(unsigned short* pBuffer) = 0;
    };
} // namespace MR