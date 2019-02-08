
#pragma once

#include "base/base_delegate.h"

#include "engine/engine_config.h"

#include <functional>

namespace Engine
{
    enum class EEvent
    {
        Engine_OnStartup,
        Engine_OnShutdown,
        Engine_OnPause,
        Engine_OnResume,
        Engine_OnUpdate,

        Gfx_OnUpdate,
        Gfx_OnRenderGBuffer,
        Gfx_OnRenderLighting,
        Gfx_OnRenderForward,
        Gfx_OnRenderUI,

        NumberOfEvents
    };

    using CEventDelegates = Base::CDelegates<static_cast<int>(EEvent::NumberOfEvents)>;
} // namespace Engine

namespace Engine
{
    ENGINE_API void Startup();

    ENGINE_API void Shutdown();

    ENGINE_API void Update();

    ENGINE_API void Resume();

    ENGINE_API void Pause();

    ENGINE_API CEventDelegates::HandleType RegisterEventHandler(EEvent _Event, CEventDelegates::FunctionType _Function);

    ENGINE_API void RaiseEvent(EEvent _Event);
} // namespace Engine