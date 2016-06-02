
#pragma once

#include "core/core_lua_export.h"

#include "base/base_matrix3x3.h"

#include "core/core_lua_export.h"
#include "core/core_lua_main.h"
#include "core/core_lua_state.h"

#include <string>

Core::Lua::CStaticFunctionList BaseMatrix3x3ObjFuncs;
Core::Lua::CStaticFunctionList BaseMatrix3x3LibFuncs;

// -----------------------------------------------------------------------------
// Object
// -----------------------------------------------------------------------------
LUA_DEFINE_FUNCTION(BaseMatrix3x3ObjFuncs, Set)
{
    Base::Float3x3& rMatrix = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 1));

    float A1 = Core::Lua::State::GetFloat(_State,  2, 0.0f);
    float A2 = Core::Lua::State::GetFloat(_State,  3, 0.0f);
    float A3 = Core::Lua::State::GetFloat(_State,  4, 0.0f);

    float B1 = Core::Lua::State::GetFloat(_State,  5, 0.0f);
    float B2 = Core::Lua::State::GetFloat(_State,  6, 0.0f);
    float B3 = Core::Lua::State::GetFloat(_State,  7, 0.0f);

    float C1 = Core::Lua::State::GetFloat(_State,  8, 0.0f);
    float C2 = Core::Lua::State::GetFloat(_State,  9, 0.0f);
    float C3 = Core::Lua::State::GetFloat(_State, 10, 0.0f);

    rMatrix.Set(A1, A2, A3, B1, B2, B3, C1, C2, C3);

    return 0;
}

LUA_DEFINE_FUNCTION(BaseMatrix3x3ObjFuncs, SetZero)
{
    Base::Float3x3& rMatrix = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 1));

    rMatrix.Set(0.0f);

    return 0;
}

LUA_DEFINE_FUNCTION(BaseMatrix3x3ObjFuncs, IsEqual)
{
    Base::Float3x3& rMatrix1 = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3x3& rMatrix2 = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 2));;

    float Epsilon = Core::Lua::State::GetFloat(_State, 3, 0.0f);

    Core::Lua::State::PushBoolean(_State, rMatrix1.IsEqual(rMatrix2, Epsilon));

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix3x3ObjFuncs, SetElement)
{
    Base::Float3x3& rMatrix = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 1));

    unsigned int Row    = Core::Lua::State::GetSInt(_State, 2, 0);
    unsigned int Column = Core::Lua::State::GetSInt(_State, 3, 0);
    float        Value  = Core::Lua::State::GetFloat(_State, 4);

    rMatrix[Row][Column] = Value;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseMatrix3x3ObjFuncs, GetElement)
{
    Base::Float3x3& rMatrix = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 1));

    unsigned int Row    = Core::Lua::State::GetSInt(_State, 2, 0);
    unsigned int Column = Core::Lua::State::GetSInt(_State, 3, 0);

    Core::Lua::State::PushFloat(_State, rMatrix[Row][Column]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix3x3ObjFuncs, __add)
{
    Base::Float3x3& rMatrix1 = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3x3& rMatrix2 = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float3x3& rResult = *static_cast<Base::Float3x3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3x3), "Base_Matrix3x3"));

    rResult = rMatrix1 + rMatrix2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix3x3ObjFuncs, __sub)
{
    Base::Float3x3& rMatrix1 = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3x3& rMatrix2 = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float3x3& rResult = *static_cast<Base::Float3x3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3x3), "Base_Matrix3x3"));

    rResult = rMatrix1 - rMatrix2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix3x3ObjFuncs, __mul)
{
    Base::Float3x3& rMatrix1 = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3x3& rMatrix2 = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float3x3& rResult = *static_cast<Base::Float3x3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3x3), "Base_Matrix3x3"));

    rResult = rMatrix1 * rMatrix2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix3x3ObjFuncs, __eq)
{
    Base::Float3x3& rMatrix1 = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float3x3& rMatrix2 = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 2));

    Core::Lua::State::PushBoolean(_State, rMatrix1 == rMatrix2);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix3x3ObjFuncs, __tostring)
{
    Base::Float3x3& rMatrix = *static_cast<Base::Float3x3*>(Core::Lua::State::GetUserData(_State, 1));

    std::string String;

    String = 
        "{{" + std::to_string(rMatrix[0][0]) + ", " + std::to_string(rMatrix[0][1]) + ", " + std::to_string(rMatrix[0][2]) + "}," 
        "{" + std::to_string(rMatrix[1][0]) + ", " + std::to_string(rMatrix[1][1]) + ", " + std::to_string(rMatrix[1][2]) + "},"
        "{" + std::to_string(rMatrix[2][0]) + ", " + std::to_string(rMatrix[2][1]) + ", " + std::to_string(rMatrix[2][2]) + "}}";

    Core::Lua::State::PushString(_State, String.c_str());

    return 1;
}

// -----------------------------------------------------------------------------
// Library
// -----------------------------------------------------------------------------
LUA_DEFINE_FUNCTION(BaseMatrix3x3LibFuncs, New)
{
    // -----------------------------------------------------------------------------
    // Create new full user data for a entity
    // -----------------------------------------------------------------------------
    Base::Float3x3& rNew = *static_cast<Base::Float3x3*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float3x3), "Base_Matrix3x3"));

    rNew.SetIdentity();

    return 1;
}