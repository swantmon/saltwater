
#pragma once

#include "mr_camera_control.h"
#include "mr_depth_sensor_control.h"

namespace MR
{
    class IRGBDCameraControl : public ICameraControl, public IDepthSensorControl
    {
    public:

        virtual ~IRGBDCameraControl() {};
    };
} // namespace MR