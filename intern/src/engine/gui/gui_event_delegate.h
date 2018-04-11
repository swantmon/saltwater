
#pragma once

#include <functional>

namespace Base
{
    class CInputEvent;
} // namespace Base

namespace Gui
{
    typedef std::function<void(const Base::CInputEvent&)> CInputEventDelegate;
} // namespace Gui

#define GUI_BIND_INPUT_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1)