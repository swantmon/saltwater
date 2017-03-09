
#pragma once

#include "mr/mr_control.h"

namespace MR
{
namespace ControlManager
{
    void OnStart();
    void OnExit();

    void Clear();

    void Update();

    bool IsActive();
    bool IsOriginTracked();

    CControl* GetActiveControl();
} // namespace ControlManager
} // namespace MR
