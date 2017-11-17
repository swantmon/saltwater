
#pragma once

#include "core/core_lua_export.h"

#include "base/base_vector3.h"
#include "base/base_matrix3x3.h"

#include "core/core_lua_export.h"
#include "core/core_lua_main.h"
#include "core/core_lua_state.h"

#include <string>

Core::Lua::CStaticFunctionList BaseFloat3ObjFuncs;
Core::Lua::CStaticFunctionList BaseFloat3LibFuncs;

// -----------------------------------------------------------------------------
// Object
// -----------------------------------------------------------------------------
LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, Set)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    float X = Core::Lua::State::GetFloat(_State, 2, 0.0f);
    float Y = Core::Lua::State::GetFloat(_State, 3, 0.0f);
    float Z = Core::Lua::State::GetFloat(_State, 4, 0.0f);

    rVector[0] = X;
    rVector[1] = Y;
    rVector[2] = Z;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, SetZero)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    rVector[0] = 0.0f;
    rVector[1] = 0.0f;
    rVector[2] = 0.0f;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, IsEqual)
{
    Base::Float3& rVector1 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3& rVector2 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));;

    float Epsilon = Core::Lua::State::GetFloat(_State, 3, 0.0f);

    Core::Lua::State::PushBoolean(_State, rVector1.IsEqual(rVector2, Epsilon));

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, SetX)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    float X = static_cast<float>(Core::Lua::State::GetFloat(_State, 2));

    rVector[0] = X;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, SetY)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    float Y = static_cast<float>(Core::Lua::State::GetFloat(_State, 2));

    rVector[1] = Y;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, SetZ)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    float Z = static_cast<float>(Core::Lua::State::GetFloat(_State, 2));

    rVector[2] = Z;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, GetX)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, rVector[0]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, GetY)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, rVector[1]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, GetZ)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, rVector[2]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, Length)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, rVector.Length());

    return 1;
}


LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, CrossProduct)
{
    Base::Float3& rVector1 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3& rVector2 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float3& rResult = *static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    rResult = rVector1.CrossProduct(rVector2);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, DotProduct)
{
    Base::Float3& rVector1 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3& rVector2 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));

    Core::Lua::State::PushFloat(_State, rVector1.DotProduct(rVector2));

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, Normalize)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    Base::Float3& rResult = *static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    rResult = rVector.Normalize();

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, SquaredLength)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    Core::Lua::State::PushFloat(_State, rVector.SquaredLength());

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, MultiplyMatrix3x3)
{
    Base::Float3&   rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3x3& rMatrix = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float3& rResult = *static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    rResult = rVector * rMatrix;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, __add)
{
    Base::Float3& rVector1 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3& rVector2 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float3& rResult = *static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    rResult = rVector1 + rVector2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, __sub)
{
    Base::Float3& rVector1 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3& rVector2 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float3& rResult = *static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    rResult = rVector1 - rVector2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, __mul)
{
    Base::Float3& rVector1 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3& rVector2 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float3& rResult = *static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    rResult = rVector1 * rVector2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, __div)
{
    Base::Float3& rVector1 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3& rVector2 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float3& rResult = *static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    rResult = rVector1 / rVector2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, __eq)
{
    Base::Float3& rVector1 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3& rVector2 = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 2));

    Core::Lua::State::PushBoolean(_State, rVector1 == rVector2);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseFloat3ObjFuncs, __tostring)
{
    Base::Float3& rVector = *static_cast<Base::Float3*>(Core::Lua::State::GetUserData(_State, 1));

    std::string String;

    // String = "{" + std::to_string(rVector[0]) + ", " + std::to_string(rVector[1]) + ", " + std::to_string(rVector[2]) + "}";

    Core::Lua::State::PushString(_State, String.c_str());

    return 1;
}

// -----------------------------------------------------------------------------
// Library
// -----------------------------------------------------------------------------
LUA_DEFINE_FUNCTION(BaseFloat3LibFuncs, New)
{
    float X = Core::Lua::State::GetFloat(_State, 1, 0.0f);
    float Y = Core::Lua::State::GetFloat(_State, 2, 0.0f);
    float Z = Core::Lua::State::GetFloat(_State, 3, 0.0f);

    // -----------------------------------------------------------------------------
    // Create new full user data for a entity
    // -----------------------------------------------------------------------------
    Base::Float3& rNew = *static_cast<Base::Float3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3), "Base_Vector3"));

    rNew.Set(X, Y, Z);

    return 1;
}