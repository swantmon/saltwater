
#pragma once

#include "engine/engine_config.h"

#include "engine/camera/cam_control.h"

namespace Cam
{
namespace ControlManager
{
    ENGINE_API void CreateControl(CControl::EType _Type);
    ENGINE_API void SetActiveControl(CControl::EType _Type);
    ENGINE_API CControl& GetActiveControl();

    ENGINE_API void Update();
} // namespace ControlManager
} // namespace Cam
