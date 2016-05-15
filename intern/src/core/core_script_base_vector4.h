
#pragma once

#include "base/base_vector4.h"

#include "core/core_script_base_vector4.h"
#include "core/core_lua_export.h"
#include "core/core_lua_main.h"
#include "core/core_lua_state.h"

#include <string>

Core::Lua::CStaticFunctionList BaseFloat4ObjFuncs;
Core::Lua::CStaticFunctionList BaseFloat4LibFuncs;

// -----------------------------------------------------------------------------
// Object
// -----------------------------------------------------------------------------
LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, Set)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    float X = Core::Lua::State::GetFloat(_State, 2, 0.0f);
    float Y = Core::Lua::State::GetFloat(_State, 3, 0.0f);
    float Z = Core::Lua::State::GetFloat(_State, 4, 0.0f);
    float W = Core::Lua::State::GetFloat(_State, 5, 0.0f);

    rVector[0] = X;
    rVector[1] = Y;
    rVector[2] = Z;
    rVector[2] = W;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, SetZero)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    rVector[0] = 0.0f;
    rVector[1] = 0.0f;
    rVector[2] = 0.0f;
    rVector[3] = 0.0f;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, IsEqual)
{
    Base::Float4& rVector1 = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float4& rVector2 = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 2));;

    float Epsilon = Core::Lua::State::GetFloat(_State, 3, 0.0f);

    Core::Lua::State::PushBoolean(_State, rVector1.IsEqual(rVector2, Epsilon));

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, SetX)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    float X = static_cast<float>(Core::Lua::State::GetFloat(_State, 2));

    rVector[0] = X;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, SetY)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    float Y = static_cast<float>(Core::Lua::State::GetFloat(_State, 2));

    rVector[1] = Y;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, SetZ)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    float Z = static_cast<float>(Core::Lua::State::GetFloat(_State, 2));

    rVector[2] = Z;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, SetW)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    float W = static_cast<float>(Core::Lua::State::GetFloat(_State, 2));

    rVector[3] = W;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, GetX)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, rVector[0]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, GetY)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, rVector[1]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, GetZ)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, rVector[2]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, GetW)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, rVector[3]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, Length)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, rVector.Length());

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, Normalize)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    Base::Float4& rResult = *static_cast<Base::Float4*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float4), "Base_Vector4"));

    rResult = rVector.Normalize();

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, __add)
{
    Base::Float4& rVector1 = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float4& rVector2 = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float4& rResult = *static_cast<Base::Float4*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float4), "Base_Vector4"));

    rResult = rVector1 + rVector2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, __mul)
{
    Base::Float4& rVector1 = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float4& rVector2 = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float4& rResult = *static_cast<Base::Float4*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float4), "Base_Vector4"));

    rResult = rVector1 * rVector2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, __div)
{
    Base::Float4& rVector1 = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float4& rVector2 = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float4& rResult = *static_cast<Base::Float4*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float4), "Base_Vector4"));

    rResult = rVector1 / rVector2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, __eq)
{
    Base::Float4& rVector1 = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float4& rVector2 = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 2));

    Core::Lua::State::PushBoolean(_State, rVector1 == rVector2);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat4ObjFuncs, __tostring)
{
    Base::Float4& rVector = *static_cast<Base::Float4*>(Core::Lua::State::GetUserData(_State, 1));

    std::string String;

    String = "Float4 (" + std::to_string(rVector[0]) + "; " + std::to_string(rVector[1]) + "; " + std::to_string(rVector[2]) + "; " + std::to_string(rVector[3]) + ")";

    Core::Lua::State::PushString(_State, String.c_str());

    return 1;
}

// -----------------------------------------------------------------------------
// Library
// -----------------------------------------------------------------------------
LUA_DEFINE_FUNCTION(BaseFloat4LibFuncs, New)
{
    float X = Core::Lua::State::GetFloat(_State, 1, 0.0f);
    float Y = Core::Lua::State::GetFloat(_State, 2, 0.0f);
    float Z = Core::Lua::State::GetFloat(_State, 3, 0.0f);
    float W = Core::Lua::State::GetFloat(_State, 4, 0.0f);

    // -----------------------------------------------------------------------------
    // Create new full user data for a entity
    // -----------------------------------------------------------------------------
    Base::Float4& rNew = *static_cast<Base::Float4*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float4), "Base_Vector4"));

    rNew.Set(X, Y, Z, W);

    return 1;
}