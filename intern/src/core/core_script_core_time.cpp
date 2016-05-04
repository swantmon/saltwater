
#include "core/core_precompiled.h"

#include "core/core_lua_export.h"
#include "core/core_lua_main.h"
#include "core/core_lua_state.h"
#include "core/core_time.h"

// -----------------------------------------------------------------------------
// Library
// -----------------------------------------------------------------------------
Core::Lua::CStaticFunctionList CoreTimeLibFuncs;

LUA_DEFINE_FUNCTION(CoreTimeLibFuncs, DeltaTime)
{
    double DeltaTimeLastFrame = Core::Time::GetDeltaTimeLastFrame();

    Core::Lua::State::PushDouble(_State, DeltaTimeLastFrame);

    return 1;
}

LUA_REGISTER_LIBRARY(Core::Lua::Main::GetMainState(), CoreTimeLibFuncs, Time)
