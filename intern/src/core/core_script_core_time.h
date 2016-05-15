
#pragma once

#include "core/core_lua_export.h"

#include "core/core_lua_state.h"
#include "core/core_script_core_time.h"
#include "core/core_time.h"

Core::Lua::CStaticFunctionList CoreTimeLibFuncs;

// -----------------------------------------------------------------------------
// Library
// -----------------------------------------------------------------------------
LUA_DEFINE_FUNCTION(CoreTimeLibFuncs, DeltaTime)
{
    double DeltaTimeLastFrame = Core::Time::GetDeltaTimeLastFrame();

    Core::Lua::State::PushDouble(_State, DeltaTimeLastFrame);

    return 1;
}