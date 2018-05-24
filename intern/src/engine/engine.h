
#pragma once

#include "engine/engine_config.h"

#include <functional>

namespace Engine
{
    enum 
    {
        Gfx_OnStart,
        Gfx_OnUpdate,
        Gfx_OnExit,
        Gfx_OnRenderGBuffer,
        Gfx_OnRenderLighting,
        Gfx_OnRenderForward,
        Gfx_OnRenderUI,
    };

    typedef std::function<void()> CEventDelegate;
} // namespace Engine

#define ENGINE_BIND_EVENT_METHOD(_Method) std::bind(_Method, this)

namespace Engine
{
    ENGINE_API void Startup();

    ENGINE_API void Shutdown();

    ENGINE_API void Update();

    ENGINE_API void Resume();

    ENGINE_API void Pause();

    ENGINE_API void RegisterEventHandler(int _EventID, CEventDelegate _Delegate);

    ENGINE_API void RaiseEvent(int _EventID);
} // namespace Engine