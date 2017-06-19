
#pragma once

namespace MR
{
    class IDepthSensorControl
    {
    public:

        virtual ~IDepthSensorControl() {};

        virtual int GetDepthWidth() const = 0 ;
        virtual int GetDepthHeight() const = 0;
        virtual int GetDepthPixelCount() const = 0;
        virtual float GetDepthFocalLengthX() const = 0;
        virtual float GetDepthFocalLengthY() const = 0;
		virtual float GetDepthFocalPointX() const = 0;
		virtual float GetDepthFocalPointY() const = 0;
		virtual float GetMinDepth() const = 0;
		virtual float GetMaxDepth() const = 0;
        virtual bool GetDepthBuffer(unsigned short* pBuffer) = 0;
    };
} // namespace MR