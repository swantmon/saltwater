
#pragma once

#include "mr/mr_control.h"
#include "mr/mr_control_description.h"

namespace MR
{
namespace ControlManager
{
    void CreateControl(CControl::EType _Type);
    void SetActiveControl(CControl::EType _Type);
    CControl& GetActiveControl();

    // -----------------------------------------------------------------------------

    void Start(const SControlDescription& _rDescriptor);
    void Stop();

    void Update();

    bool IsActive();
} // namespace ControlManager
} // namespace MR
