
#pragma once

#include "engine/engine_config.h"

#include "engine/gui/gui_event_delegate.h"

namespace Base
{
    class CInputEvent;
} // namespace Base

namespace Gui
{
namespace EventHandler
{
    ENGINE_API void OnUserEvent(const Base::CInputEvent& _rEvent);

    ENGINE_API void RegisterDirectUserListener(const CInputEventDelegate& _rListener);
    ENGINE_API void UnregisterDirectUserListener(const CInputEventDelegate& _rListener);
    ENGINE_API void UnregisterAllDirectUserListeners();
    ENGINE_API bool ContainsDirectUserListener(const CInputEventDelegate& _rListener);
} // namespace EventHandler
} // namespace Gui
