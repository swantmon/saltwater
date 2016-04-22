
#pragma once

#include "mr/mr_control.h"
#include "mr/mr_control_description.h"

namespace MR
{
namespace ControlManager
{
    void OnStart();
    void OnExit();

    void Update();

    bool IsActive();

    CControl* GetActiveControl();
} // namespace ControlManager
} // namespace MR
