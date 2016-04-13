
#pragma once

#include "gui/gui_event_delegate.h"

namespace Base
{
    class CInputEvent;
} // namespace Base

namespace Gui
{
namespace EventHandler
{
    void OnUserEvent(const Base::CInputEvent& _rEvent);

    void RegisterDirectUserListener(const CInputEventDelegate& _rListener);
    void UnregisterDirectUserListener(const CInputEventDelegate& _rListener);
    void UnregisterAllDirectUserListeners();
    bool ContainsDirectUserListener(const CInputEventDelegate& _rListener);
} // namespace EventHandler
} // namespace Gui
