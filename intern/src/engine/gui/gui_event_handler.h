
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
	using CInputEventDelegate = Base::CDelegate<const Base::CInputEvent&>;
} // namespace EventHandler
} // namespace Gui

namespace Gui
{
namespace EventHandler
{
    ENGINE_API void OnUserEvent(const Base::CInputEvent& _rEvent);

    ENGINE_API CInputEventDelegate::HandleType RegisterDirectUserListener(CInputEventDelegate::FunctionType _Function);
} // namespace EventHandler
} // namespace Gui
