
#pragma once

#ifndef __ANDROID__

#include "core/core_lua_export.h"

#include "base/base_matrix4x4.h"

#include "core/core_lua_export.h"
#include "core/core_lua_main.h"
#include "core/core_lua_state.h"

#include <string>

Core::Lua::CStaticFunctionList BaseMatrix4x4ObjFuncs;
Core::Lua::CStaticFunctionList BaseMatrix4x4LibFuncs;

// -----------------------------------------------------------------------------
// Object
// -----------------------------------------------------------------------------
LUA_DEFINE_FUNCTION(BaseMatrix4x4ObjFuncs, SetZero)
{
    Base::Float4x4& rMatrix = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 1));

    rMatrix.Set(0.0f);

    return 0;
}

LUA_DEFINE_FUNCTION(BaseMatrix4x4ObjFuncs, IsEqual)
{
    Base::Float4x4& rMatrix1 = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float4x4& rMatrix2 = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 2));;

    float Epsilon = Core::Lua::State::GetFloat(_State, 3, 0.0f);

    Core::Lua::State::PushBoolean(_State, rMatrix1.IsEqual(rMatrix2, Epsilon));

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix4x4ObjFuncs, Set)
{
    Base::Float4x4& rMatrix = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 1));

    unsigned int Row    = Core::Lua::State::GetSInt(_State, 2, 0) - 1;
    unsigned int Column = Core::Lua::State::GetSInt(_State, 3, 0) - 1;
    float        Value  = Core::Lua::State::GetFloat(_State, 4);

    rMatrix[Row][Column] = Value;

    return 0;
}

LUA_DEFINE_FUNCTION(BaseMatrix4x4ObjFuncs, Get)
{
    Base::Float4x4& rMatrix = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 1));

    unsigned int Row    = Core::Lua::State::GetSInt(_State, 2, 0) - 1;
    unsigned int Column = Core::Lua::State::GetSInt(_State, 3, 0) - 1;

    Core::Lua::State::PushFloat(_State, rMatrix[Row][Column]);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix4x4ObjFuncs, __add)
{
    Base::Float4x4& rMatrix1 = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float4x4& rMatrix2 = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float4x4& rResult = *static_cast<Base::Float4x4*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float4x4), "Base_Matrix4x4"));

    rResult = rMatrix1 + rMatrix2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix4x4ObjFuncs, __sub)
{
    Base::Float4x4& rMatrix1 = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float4x4& rMatrix2 = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float4x4& rResult = *static_cast<Base::Float4x4*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float4x4), "Base_Matrix4x4"));

    rResult = rMatrix1 - rMatrix2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix4x4ObjFuncs, __mul)
{
    Base::Float4x4& rMatrix1 = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float4x4& rMatrix2 = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 2));

    Base::Float4x4& rResult = *static_cast<Base::Float4x4*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float4x4), "Base_Matrix4x4"));

    rResult = rMatrix1 * rMatrix2;

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix4x4ObjFuncs, __eq)
{
    Base::Float4x4& rMatrix1 = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 1));
    Base::Float4x4& rMatrix2 = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 2));

    Core::Lua::State::PushBoolean(_State, rMatrix1 == rMatrix2);

    return 1;
}

LUA_DEFINE_FUNCTION(BaseMatrix4x4ObjFuncs, __tostring)
{
    Base::Float4x4& rMatrix = *static_cast<Base::Float4x4*>(Core::Lua::State::GetUserData(_State, 1));

    std::string String;

    String = 
        "{{" + std::to_string(rMatrix[0][0]) + ", " + std::to_string(rMatrix[0][1]) + ", " + std::to_string(rMatrix[0][2]) + ", " + std::to_string(rMatrix[0][3]) + "},"
         "{" + std::to_string(rMatrix[1][0]) + ", " + std::to_string(rMatrix[1][1]) + ", " + std::to_string(rMatrix[1][2]) + ", " + std::to_string(rMatrix[1][3]) + "},"
         "{" + std::to_string(rMatrix[2][0]) + ", " + std::to_string(rMatrix[2][1]) + ", " + std::to_string(rMatrix[2][2]) + ", " + std::to_string(rMatrix[2][3]) + "},"
         "{" + std::to_string(rMatrix[3][0]) + ", " + std::to_string(rMatrix[3][1]) + ", " + std::to_string(rMatrix[3][2]) + ", " + std::to_string(rMatrix[3][3]) + "}}";

    Core::Lua::State::PushString(_State, String.c_str());

    return 1;
}

// -----------------------------------------------------------------------------
// Library
// -----------------------------------------------------------------------------
LUA_DEFINE_FUNCTION(BaseMatrix4x4LibFuncs, New)
{
    // -----------------------------------------------------------------------------
    // Create new full user data for a entity
    // -----------------------------------------------------------------------------
    Base::Float4x4& rNew = *static_cast<Base::Float4x4*>(Core::Lua::State::PushUserData(_State, sizeof(Base::Float4x4), "Base_Matrix4x4"));

    rNew.SetIdentity();

    return 1;
}

#endif