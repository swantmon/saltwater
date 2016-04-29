
#pragma once

#include "base/base_defines.h"
#include "base/base_typedef.h"

#include "logic/lua_typedef.h"

// -----------------------------------------------------------------------------
// Define to help registering different modules like functions, libraries and
// so on.
// -----------------------------------------------------------------------------
#define LUA_DEFINE_FUNCTION(FunctionList, FunctionName)                                                                                                                     \
    static int BASE_CONCAT(LuaFunction, BASE_CONCAT(FunctionList, FunctionName))(LUA::BState _State);                                                                       \
    struct BASE_CONCAT(SAddLuaFunction, BASE_CONCAT(FunctionList, FunctionName))                                                                                            \
    {                                                                                                                                                                       \
        const Base::Char* m_pFunctionName;                                                                                                                                  \
        LUA::FLuaCFunc    m_pLuaCFtr;                                                                                                                                       \
        void*             m_pNext;                                                                                                                                          \
        BASE_CONCAT(SAddLuaFunction, BASE_CONCAT(FunctionList, FunctionName))(const Base::Char* _pFunctionName, LUA::FLuaCFunc _pLuaCFtr)                                   \
            : m_pFunctionName(_pFunctionName)                                                                                                                               \
            , m_pLuaCFtr     (_pLuaCFtr)                                                                                                                                    \
            , m_pNext        (nullptr)                                                                                                                                      \
        {                                                                                                                                                                   \
            ::LUA::Private::AddFunction(FunctionList, this);                                                                                                                \
        }                                                                                                                                                                   \
    } const BASE_CONCAT(g_AddLuaFunction, BASE_CONCAT(FunctionList, FunctionName))(#FunctionName, BASE_CONCAT(LuaFunction, BASE_CONCAT(FunctionList, FunctionName)));       \
    static int BASE_CONCAT(LuaFunction, BASE_CONCAT(FunctionList, FunctionName))(LUA::BState _State)

#define LUA_REGISTER_FUNCTIONS(LuaState, FunctionList)                                                                                                                      \
struct BASE_CONCAT(SRegGlobalFunc, FunctionList)                                                                                                                            \
{                                                                                                                                                                           \
    BASE_CONCAT(SRegGlobalFunc, FunctionList)()                                                                                                                             \
    {                                                                                                                                                                       \
        ::LUA::Private::RegisterFunctions(LuaState, FunctionList);                                                                                                          \
    }                                                                                                                                                                       \
} const BASE_CONCAT(g_RegGlobalFunc, FunctionList);

// -----------------------------------------------------------------------------

#define LUA_REGISTER_OBJECT(LuaState, FunctionList, ObjectName)                                                                                                             \
struct BASE_CONCAT(SRegObject, BASE_CONCAT(FunctionList, ObjectName))                                                                                                       \
{                                                                                                                                                                           \
    BASE_CONCAT(SRegObject, BASE_CONCAT(FunctionList, ObjectName))()                                                                                                        \
    {                                                                                                                                                                       \
        ::LUA::Private::RegisterObject(LuaState, FunctionList, #ObjectName);                                                                                                \
    }                                                                                                                                                                       \
} const BASE_CONCAT(g_RegObject, BASE_CONCAT(FunctionList, ObjectName));

// -----------------------------------------------------------------------------

#define LUA_REGISTER_LIBRARY(LuaState, FunctionList, LibraryName)                                                                                                           \
static int BASE_CONCAT(LuaFunction, BASE_CONCAT(FunctionList, LibraryName))(LUA::BState _State)                                                                             \
{                                                                                                                                                                           \
    ::LUA::Private::RegisterLibrary(LuaState, FunctionList, #LibraryName); return 1;                                                                                        \
}                                                                                                                                                                           \
struct BASE_CONCAT(SRegLibrary, BASE_CONCAT(FunctionList, LibraryName))                                                                                                     \
{                                                                                                                                                                           \
    BASE_CONCAT(SRegLibrary, BASE_CONCAT(FunctionList, LibraryName))()                                                                                                      \
    {                                                                                                                                                                       \
        ::LUA::Private::RequireLibrary(LuaState, #LibraryName, BASE_CONCAT(LuaFunction, BASE_CONCAT(FunctionList, LibraryName)));                                           \
    }                                                                                                                                                                       \
} const BASE_CONCAT(g_RegLibrary, BASE_CONCAT(FunctionList, LibraryName));

// -----------------------------------------------------------------------------
// Functionality
// -----------------------------------------------------------------------------
namespace LUA
{
    class CStaticFunctionList
    {
    public:

        CStaticFunctionList();

    protected:

        void* m_pFirst;
    };
} // namespace LUA

namespace LUA
{
namespace Private
{
    void RequireLibrary(BState _State, const Base::Char* _pLibraryName, FLuaCFunc _pFunction);

    void AddFunction(CStaticFunctionList& _rFunctionList, void* _pFunctionInfo);
    void RegisterFunctions(BState _State, CStaticFunctionList& _rFunctionList);
    void RegisterLibrary(BState _State, CStaticFunctionList& _rFunctionList, const Base::Char* _pLibraryName);
    void RegisterObject(BState _State, CStaticFunctionList& _rFunctionList, const Base::Char* _pObjectName);
} // namespace Private
} // namespace LUA