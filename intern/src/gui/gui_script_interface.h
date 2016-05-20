
#pragma once

#include "core/core_lua_export.h"
#include "core/core_lua_main.h"
#include "core/core_lua_state.h"

#include "gui/gui_input_manager.h"

Core::Lua::CStaticFunctionList GuiInputLibFuncs;

// -----------------------------------------------------------------------------
// Library
// -----------------------------------------------------------------------------
LUA_DEFINE_FUNCTION(GuiInputLibFuncs, IsAnyKey)
{
    Core::Lua::State::PushBoolean(_State, Gui::InputManager::IsAnyKey());

    return 1;
}

LUA_DEFINE_FUNCTION(GuiInputLibFuncs, IsAnyKeyDown)
{
    Core::Lua::State::PushBoolean(_State, Gui::InputManager::IsAnyKeyDown());

    return 1;
}

LUA_DEFINE_FUNCTION(GuiInputLibFuncs, IsAnyKeyUp)
{
    Core::Lua::State::PushBoolean(_State, Gui::InputManager::IsAnyKeyUp());

    return 1;
}

LUA_DEFINE_FUNCTION(GuiInputLibFuncs, GetKey)
{
    Base::CInputEvent::EKey Key = static_cast<Base::CInputEvent::EKey>(Core::Lua::State::GetUInt(_State, 1));

    Core::Lua::State::PushBoolean(_State, Gui::InputManager::GetKey(Key));

    return 1;
}

LUA_DEFINE_FUNCTION(GuiInputLibFuncs, GetKeyDown)
{
    Base::CInputEvent::EKey Key = static_cast<Base::CInputEvent::EKey>(Core::Lua::State::GetUInt(_State, 1));

    Core::Lua::State::PushBoolean(_State, Gui::InputManager::GetKeyDown(Key));

    return 1;
}

LUA_DEFINE_FUNCTION(GuiInputLibFuncs, GetKeyUp)
{
    Base::CInputEvent::EKey Key = static_cast<Base::CInputEvent::EKey>(Core::Lua::State::GetUInt(_State, 1));

    Core::Lua::State::PushBoolean(_State, Gui::InputManager::GetKeyUp(Key));

    return 1;
}

LUA_DEFINE_FUNCTION(GuiInputLibFuncs, GetMouseButton)
{
    Base::CInputEvent::EKey Key = static_cast<Base::CInputEvent::EKey>(Core::Lua::State::GetUInt(_State, 1));

    Core::Lua::State::PushBoolean(_State, Gui::InputManager::GetMouseButton(Key));

    return 1;
}

LUA_DEFINE_FUNCTION(GuiInputLibFuncs, GetMouseButtonDown)
{
    Base::CInputEvent::EKey Key = static_cast<Base::CInputEvent::EKey>(Core::Lua::State::GetUInt(_State, 1));

    Core::Lua::State::PushBoolean(_State, Gui::InputManager::GetMouseButtonDown(Key));

    return 1;
}

LUA_DEFINE_FUNCTION(GuiInputLibFuncs, GetMouseButtonUp)
{
    Base::CInputEvent::EKey Key = static_cast<Base::CInputEvent::EKey>(Core::Lua::State::GetUInt(_State, 1));

    Core::Lua::State::PushBoolean(_State, Gui::InputManager::GetMouseButtonUp(Key));

    return 1;
}

LUA_DEFINE_FUNCTION(GuiInputLibFuncs, GetMousePosition)
{
    Base::Short2& rMousePosition = Gui::InputManager::GetMousePosition();

    Core::Lua::State::PushSShort(_State, rMousePosition[0]);
    Core::Lua::State::PushSShort(_State, rMousePosition[1]);

    return 2;
}

LUA_DEFINE_FUNCTION(GuiInputLibFuncs, GetMouseScrollDelta)
{
    Core::Lua::State::PushFloat(_State, Gui::InputManager::GetMouseScrollDelta());

    return 1;
}