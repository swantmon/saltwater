
#pragma once

#include "base/base_input_event.h"

namespace Gui
{
namespace InputManager
{
    void OnStart();
    void OnExit();

    void Update();

    bool IsAnyKey();
    bool IsAnyKeyDown();
    bool IsAnyKeyUp();

    bool GetKey(Base::CInputEvent::EKey _Key);
    bool GetKeyDown(Base::CInputEvent::EKey _Key);
    bool GetKeyUp(Base::CInputEvent::EKey _Key);

    const glm::vec2& GetGlobalMousePosition();
    const glm::vec2& GetLocalMousePosition();
    float GetMouseScrollDelta();
} // namespace InputManager
} // namespace Gui
