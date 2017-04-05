
#pragma once

#include "base/base_vector4.h"

namespace MR
{
    class ICameraControl
    {
    public:

        virtual ~ICameraControl() {};

        virtual int GetCameraWidth() const = 0;
        virtual int GetCameraHeight() const = 0;
        virtual int GetCameraPixelCount() const = 0;
        virtual float GetCameraFocalLengthX() const = 0;
        virtual float GetCameraFocalLengthY() const = 0;
        virtual float GetCameraFocalPointX() const = 0;
        virtual float GetCameraFocalPointY() const = 0;
        virtual bool GetCameraFrame(Base::Byte4* pBuffer) = 0;
    };
} // namespace MR