
#pragma once

#include "base/base_defines.h"
#include "base/base_typedef.h"

#include "core/core_lua_typedef.h"

// -----------------------------------------------------------------------------
// Define to help registering different modules like functions, libraries and
// so on.
// -----------------------------------------------------------------------------
#define LUA_DEFINE_FUNCTION(FunctionList, FunctionName)                                                                                                                     \
    static int BASE_CONCAT(LuaFunction, BASE_CONCAT(FunctionList, FunctionName))(Core::Lua::BState _State);                                                                 \
    struct BASE_CONCAT(SAddLuaFunction, BASE_CONCAT(FunctionList, FunctionName))                                                                                            \
    {                                                                                                                                                                       \
        const Base::Char*    m_pFunctionName;                                                                                                                               \
        Core::Lua::FLuaCFunc m_pLuaCFtr;                                                                                                                                    \
        void*                m_pNext;                                                                                                                                       \
        BASE_CONCAT(SAddLuaFunction, BASE_CONCAT(FunctionList, FunctionName))(const Base::Char* _pFunctionName, Core::Lua::FLuaCFunc _pLuaCFtr)                             \
            : m_pFunctionName(_pFunctionName)                                                                                                                               \
            , m_pLuaCFtr     (_pLuaCFtr)                                                                                                                                    \
            , m_pNext        (nullptr)                                                                                                                                      \
        {                                                                                                                                                                   \
            ::Core::Lua::Private::AddFunction(FunctionList, this);                                                                                                          \
        }                                                                                                                                                                   \
    } const BASE_CONCAT(g_AddLuaFunction, BASE_CONCAT(FunctionList, FunctionName))(#FunctionName, BASE_CONCAT(LuaFunction, BASE_CONCAT(FunctionList, FunctionName)));       \
    static int BASE_CONCAT(LuaFunction, BASE_CONCAT(FunctionList, FunctionName))(Core::Lua::BState _State)

#define LUA_REGISTER_FUNCTIONS(LuaState, FunctionList)                                                                                                                      \
    ::Core::Lua::Private::RegisterFunctions(LuaState, FunctionList)

// -----------------------------------------------------------------------------

#define LUA_REGISTER_OBJECT(LuaState, FunctionList, ObjectName)                                                                                                             \
    ::Core::Lua::Private::RegisterObject(LuaState, FunctionList, #ObjectName)

// -----------------------------------------------------------------------------

#define LUA_REQUIRE_LIBRARY_FUNC(FunctionList, LibraryName)                                                                                                                 \
static int BASE_CONCAT(LuaFunction, BASE_CONCAT(FunctionList, LibraryName))(Core::Lua::BState _State)                                                                       \
{                                                                                                                                                                           \
    ::Core::Lua::Private::RegisterLibrary(_State, FunctionList, #LibraryName); return 1;                                                                                    \
}

#define LUA_REGISTER_LIBRARY(LuaState, FunctionList, LibraryName)                                                                                                            \
    ::Core::Lua::Private::RequireLibrary(LuaState, #LibraryName, BASE_CONCAT(LuaFunction, BASE_CONCAT(FunctionList, LibraryName)))

// -----------------------------------------------------------------------------
// Functionality
// -----------------------------------------------------------------------------
namespace Core
{
namespace Lua
{
    class CStaticFunctionList
    {
    public:

        CStaticFunctionList();

    protected:

        void* m_pFirst;
    };
} // namespace Lua
} // namespace Core

namespace Core
{
namespace Lua
{
namespace Private
{
    void RequireLibrary(BState _State, const Base::Char* _pLibraryName, FLuaCFunc _pFunction);

    void AddFunction(CStaticFunctionList& _rFunctionList, void* _pFunctionInfo);
    void RegisterFunctions(BState _State, CStaticFunctionList& _rFunctionList);
    void RegisterLibrary(BState _State, CStaticFunctionList& _rFunctionList, const Base::Char* _pLibraryName);
    void RegisterObject(BState _State, CStaticFunctionList& _rFunctionList, const Base::Char* _pObjectName);
} // namespace Private
} // namespace Lua
} // namespace Core