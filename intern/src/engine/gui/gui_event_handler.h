
#pragma once

#include "base/base_delegate.h"

#include "engine/engine_config.h"

namespace Base
{
    class CInputEvent;
} // namespace Base

namespace Gui
{
namespace EventHandler
{
    using CEventDelegate = Base::CDelegate<const Base::CInputEvent&>;
} // namespace EventHandler
} // namespace Gui

namespace Gui
{
namespace EventHandler
{
    ENGINE_API void OnEvent(const Base::CInputEvent& _rEvent);

    ENGINE_API CEventDelegate::HandleType RegisterEventHandler(CEventDelegate::FunctionType _Function);
} // namespace EventHandler
} // namespace Gui
