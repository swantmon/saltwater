
#pragma once

#include "camera/cam_control.h"

namespace Cam
{
namespace ControlManager
{
    void CreateControl(CControl::EType _Type);
    void SetActiveControl(CControl::EType _Type);
    CControl& GetActiveControl();

    void Update();
} // namespace ControlManager
} // namespace Cam
