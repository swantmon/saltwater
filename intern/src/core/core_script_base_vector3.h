
#pragma once

#include "base/base_vector3.h"

#include "core/core_lua_export.h"
#include "core/core_lua_main.h"
#include "core/core_lua_state.h"

#include <string>

// -----------------------------------------------------------------------------
// Object
// -----------------------------------------------------------------------------
Core::Lua::CStaticFunctionList BaseFloat3ObjFuncs;

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, SetX)
{
    Base::Float3* pVector = static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    float X = static_cast<float>(Core::Lua::State::GetFloat(_State, 2));

    (*pVector)[0] = X;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, SetY)
{
    Base::Float3* pVector = static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    float Y = static_cast<float>(Core::Lua::State::GetFloat(_State, 2));

    (*pVector)[1] = Y;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, SetZ)
{
    Base::Float3* pVector = static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    float Z = static_cast<float>(Core::Lua::State::GetFloat(_State, 2));

    (*pVector)[2] = Z;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, GetX)
{
    Base::Float3* pVector = static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, (*pVector)[0]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, GetY)
{
    Base::Float3* pVector = static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, (*pVector)[1]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, GetZ)
{
    Base::Float3* pVector = static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, (*pVector)[2]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, __add)
{
    Base::Float3* pVector1 = static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3* pVector2 = static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, __tostring)
{
    Base::Float3* pVector = static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    std::string String;

    String = "Float3 (" + std::to_string((*pVector)[0]) + "; " + std::to_string((*pVector)[1]) + "; " + std::to_string((*pVector)[2]) + ")";

    Core::Lua::State::PushString(_State, String.c_str());

    return 1;
}

LUA_REGISTER_OBJECT(Core::Lua::Main::GetMainState(), BaseFloat3ObjFuncs, Base_Vector3)

// -----------------------------------------------------------------------------
// Library
// -----------------------------------------------------------------------------
Core::Lua::CStaticFunctionList BaseFloat3LibFuncs;

LUA_DEFINE_FUNCTION(BaseFloat3LibFuncs, New)
{
    Base::Float3* pNewEntityInScript;

    float X = Core::Lua::State::GetFloat(_State, 1, 0.0f);
    float Y = Core::Lua::State::GetFloat(_State, 2, 0.0f);
    float Z = Core::Lua::State::GetFloat(_State, 3, 0.0f);

    // -----------------------------------------------------------------------------
    // Create new full user data for a entity
    // -----------------------------------------------------------------------------
    pNewEntityInScript = static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    pNewEntityInScript->Set(X, Y, Z);

    return 1;
}

LUA_REGISTER_LIBRARY(Core::Lua::Main::GetMainState(), BaseFloat3LibFuncs, vector3)
