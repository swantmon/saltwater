
#include "core/core_precompiled.h"

#include "base/base_console.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_lua_argument.h"
#include "core/core_lua_result.h"
#include "core/core_lua_state.h"
#include "core/core_lua_typedef.h"

#include "lua.hpp"

using namespace Core::Lua;

namespace 
{
    void OnLuaErrorCallback(lua_State* _pLuaState, int _StatusCode)
    {
        if (_StatusCode != 0)
        {
            BASE_CONSOLE_INFOV("%s\n", lua_tostring(_pLuaState, -1));

            lua_pop(_pLuaState, 1);
        }
    }
} // namespace 

namespace 
{
    class CLuaStateManager : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLuaStateManager)

    public:

        CLuaStateManager();
        ~CLuaStateManager();

    public:

        BState CreateState(const Base::Char* _pName);
        void DeleteState(BState _State);

        void LoadScript(BState _State, const Base::Char* _pFilename, int _Flags = 0);

        bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument** _ppArguments, unsigned int _NumberOfArguments, CResult* _pResult = nullptr);

        void RequireLibrary(BState _State, const Base::Char* _pLibraryName, FLuaCFunc _pFunction);

        void RegisterFunction(BState _State, const Base::Char* _pFunctionName, FLuaCFunc _pFunction);
        void RegisterLibrary(BState _State, const Base::Char* _pLibraryName, const Base::Char* _pFunctionName, FLuaCFunc _pFunction);
        void RegisterObject(BState _State, const Base::Char* _pObjectName, const Base::Char* _pFunctionName, FLuaCFunc _pFunction);

        void PushBoolean(BState _State, bool _Value);
        void PushSChar(BState _State, signed char _Value);
        void PushUChar(BState _State, unsigned char _Value);
        void PushSShort(BState _State, signed short _Value);
        void PushUShort(BState _State, unsigned short _Value);
        void PushSInt(BState _State, signed int _Value);
        void PushUInt(BState _State, unsigned int _Value);
        void PushSLong(BState _State, signed long _Value);
        void PushULong(BState _State, unsigned long _Value);
        void PushSLongLong(BState _State, signed long long _Value);
        void PushULongLong(BState _State, unsigned long long _Value);
        void PushFloat(BState _State, float _Value);
        void PushDouble(BState _State, double _Value);
        void PushLongDouble(BState _State, long double _Value);
        void PushString(BState _State, const Base::Char* _pString);
        void* PushUserData(BState _State, unsigned int _NumberOfBytes, const Base::Char* _pObjectName);

        bool GetBoolean(BState _State, int _IndexOfStack, bool _DefaultValue = false);
        signed char GetSChar(BState _State, int _IndexOfStack, signed char _DefaultValue = 0);
        unsigned char GetUChar(BState _State, int _IndexOfStack, unsigned char _DefaultValue = 0);
        signed short GetSShort(BState _State, int _IndexOfStack, signed short _DefaultValue = 0);
        unsigned short GetUShort(BState _State, int _IndexOfStack, unsigned short _DefaultValue = 0);
        signed int GetSInt(BState _State, int _IndexOfStack, signed int _DefaultValue = 0);
        unsigned int GetUInt(BState _State, int _IndexOfStack, unsigned int _DefaultValue = 0);
        signed long GetSLong(BState _State, int _IndexOfStack, signed long _DefaultValue = 0);
        unsigned long GetULong(BState _State, int _IndexOfStack, unsigned long _DefaultValue = 0);
        signed long long GetSLongLong(BState _State, int _IndexOfStack, signed long long _DefaultValue = 0);
        unsigned long long GetULongLong(BState _State, int _IndexOfStack, unsigned long long _DefaultValue = 0);
        float GetFloat(BState _State, int _IndexOfStack, float _DefaultValue = 0);
        double GetDouble(BState _State, int _IndexOfStack, double _DefaultValue = 0);
        long double GetLongDouble(BState _State, int _IndexOfStack, long double _DefaultValue = 0);
        const Base::Char* GetString(BState _State, int _IndexOfStack);
        void* GetUserData(BState _State, int _IndexOfStack);

        bool IsBoolean(BState _State, int _IndexOfStack);
        bool IsNumber(BState _State, int _IndexOfStack);
        bool IsString(BState _State, int _IndexOfStack);
        bool IsPointer(BState _State, int _IndexOfStack);

    private:

        lua_State* GetNativeState(BState _State);
        void Push(lua_State* _pState, const CArgument& _rArgument);
        void Pop(lua_State* _pState, CResult& _rResult);
    };
} // namespace 

namespace 
{
    CLuaStateManager::CLuaStateManager()
    {

    }

    // -----------------------------------------------------------------------------

    CLuaStateManager::~CLuaStateManager()
    {

    }

    // -----------------------------------------------------------------------------

    BState CLuaStateManager::CreateState(const Base::Char* _pName)
    {
        BASE_UNUSED(_pName);

        lua_State* pLuaState;

        // -----------------------------------------------------------------------------
        // Create state
        // -----------------------------------------------------------------------------
        pLuaState = luaL_newstate();

        // -----------------------------------------------------------------------------
        // Load libraries
        // -----------------------------------------------------------------------------
        luaopen_io    (pLuaState);
        luaopen_base  (pLuaState);
        luaopen_table (pLuaState);
        luaopen_string(pLuaState);
        luaopen_math  (pLuaState);

        luaL_openlibs(pLuaState);

        return pLuaState;
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::DeleteState(BState _State)
    {
        lua_State* pNativeState = GetNativeState(_State);

        lua_close(pNativeState);
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::LoadScript(BState _State, const Base::Char* _pFilename, int _Flags)
    {
        int        Result;
        lua_State* pNativeState = GetNativeState(_State);

        if (_pFilename != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Load file
            // -----------------------------------------------------------------------------
            Result = luaL_loadfile(pNativeState, _pFilename);

            if (Result != 0)
            {
                OnLuaErrorCallback(pNativeState, Result);
            }
            else
            {
                // -----------------------------------------------------------------------------
                // Call script once
                // -----------------------------------------------------------------------------
                if (!(_Flags & SLoadFlags::DoNotExecute) == 1)
                {
                    lua_pcall(pNativeState, 0, 0, 0);
                }
            }
        }
    }

    // -----------------------------------------------------------------------------

    bool CLuaStateManager::CallFunction(BState _State, const Base::Char* _pFunction, const CArgument** _ppArguments, unsigned int _NumberOfArguments, CResult* _pResult)
    {
        int        Result;
        lua_State* pNativeState = GetNativeState(_State);

        if (_pFunction != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Get stack position of this function
            // -----------------------------------------------------------------------------
            lua_getglobal(pNativeState, _pFunction);

            // -----------------------------------------------------------------------------
            // Set arguments
            // -----------------------------------------------------------------------------
            for (unsigned int IndexOfArgument = 0; IndexOfArgument < _NumberOfArguments; ++ IndexOfArgument)
            {
                Push(pNativeState, *_ppArguments[IndexOfArgument]);
            }

            // -----------------------------------------------------------------------------
            // Call function
            // -----------------------------------------------------------------------------
            int NumberOfArguments = _NumberOfArguments;

            Result = lua_pcall(pNativeState, NumberOfArguments, (_pResult == nullptr) ? 0 : 1, 0);

            if (Result != 0)
            {
                OnLuaErrorCallback(pNativeState, Result);
            }

            // -----------------------------------------------------------------------------
            // Get result
            // -----------------------------------------------------------------------------
            if (_pResult != nullptr)
            {
                Pop(pNativeState, *_pResult);
            }

            return true;
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::RequireLibrary(BState _State, const Base::Char* _pLibraryName, FLuaCFunc _pFunction)
    {
        assert(_pLibraryName != 0 && _pFunction != 0);

        lua_State* pNativeState = GetNativeState(_State);

        luaL_requiref(pNativeState, _pLibraryName, reinterpret_cast<lua_CFunction>(_pFunction), 1);
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::RegisterFunction(BState _State, const Base::Char* _pFunctionName, FLuaCFunc _pFunction)
    {
        assert(_pFunctionName != 0 && _pFunction != 0);

        lua_State* pNativeState = GetNativeState(_State);

        lua_register(pNativeState, _pFunctionName, reinterpret_cast<lua_CFunction>(_pFunction));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::RegisterLibrary(BState _State, const Base::Char* _pLibraryName, const Base::Char* _pFunctionName, FLuaCFunc _pFunction)
    {
        assert(_pLibraryName != 0 && _pFunctionName != 0 && _pFunction != 0);

        lua_State* pNativeState = GetNativeState(_State);

        // -----------------------------------------------------------------------------
        // Get table from global environment
        // -----------------------------------------------------------------------------
        lua_getglobal(pNativeState, _pLibraryName);

        // -----------------------------------------------------------------------------
        // Check if table exists
        // -----------------------------------------------------------------------------
        if (lua_isnil(pNativeState, -1)) 
        {
            // -----------------------------------------------------------------------------
            // Pop error form stack
            // -----------------------------------------------------------------------------
            lua_pop(pNativeState, 1);

            // -----------------------------------------------------------------------------
            // Create new table and set to global environment
            // -----------------------------------------------------------------------------
            lua_newtable(pNativeState);

            lua_setglobal(pNativeState, _pLibraryName);

            // -----------------------------------------------------------------------------
            // Pop current table
            // -----------------------------------------------------------------------------
            lua_pop(pNativeState, 1);

            // -----------------------------------------------------------------------------
            // Get table from global environment
            // -----------------------------------------------------------------------------
            lua_getglobal(pNativeState, _pLibraryName);
        }

        // -----------------------------------------------------------------------------
        // Set specific function on this table
        // -----------------------------------------------------------------------------
        lua_pushstring(pNativeState, _pFunctionName);
        lua_pushcfunction(pNativeState, reinterpret_cast<lua_CFunction>(_pFunction));
        lua_settable(pNativeState, -3);
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::RegisterObject(BState _State, const Base::Char* _pObjectName, const Base::Char* _pFunctionName, FLuaCFunc _pFunction)
    {
        lua_State* pNativeState = GetNativeState(_State);

        // -----------------------------------------------------------------------------
        // Create a new meta table for this Lua script
        // -----------------------------------------------------------------------------
        if (luaL_getmetatable(pNativeState, _pObjectName) == LUA_TNIL)
        {
            // -----------------------------------------------------------------------------
            // Create new meta table
            // -----------------------------------------------------------------------------
            luaL_newmetatable(pNativeState, _pObjectName);
        }

        // -----------------------------------------------------------------------------
        // Set meta table and load index
        // -----------------------------------------------------------------------------
        luaL_setmetatable(pNativeState, _pObjectName);
        lua_pushvalue(pNativeState, -1);
        lua_setfield(pNativeState, -2, "__index");

        // -----------------------------------------------------------------------------
        // Set a function that loads the function on table
        // -----------------------------------------------------------------------------
        lua_pushcfunction(pNativeState, reinterpret_cast<lua_CFunction>(_pFunction));
        lua_setfield(pNativeState, -2, _pFunctionName);

        // -----------------------------------------------------------------------------
        // Pop metatable
        // -----------------------------------------------------------------------------
        lua_pop(pNativeState, 1);
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushBoolean(BState _State, bool _Value)
    {
        ::lua_pushboolean(GetNativeState(_State), _Value);
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushSChar(BState _State, signed char _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushUChar(BState _State, unsigned char _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushSShort(BState _State, signed short _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushUShort(BState _State, unsigned short _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushSInt(BState _State, signed int _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushUInt(BState _State, unsigned int _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushSLong(BState _State, signed long _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushULong(BState _State, unsigned long _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushSLongLong(BState _State, signed long long _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushULongLong(BState _State, unsigned long long _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushFloat(BState _State, float _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushDouble(BState _State, double _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushLongDouble(BState _State, long double _Value)
    {
        ::lua_pushnumber(GetNativeState(_State), static_cast<lua_Number>(_Value));
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::PushString(BState _State, const Base::Char* _pString)
    {
        ::lua_pushlstring(GetNativeState(_State), _pString, strlen(_pString));
    }
    
    // -----------------------------------------------------------------------------

    void* CLuaStateManager::PushUserData(BState _State, unsigned int _NumberOfBytes, const Base::Char* _pObjectName)
    {
        void* pUserData = 0;

        pUserData = lua_newuserdata(GetNativeState(_State), _NumberOfBytes);
        luaL_getmetatable(GetNativeState(_State), _pObjectName);
        lua_setmetatable(GetNativeState(_State), -2);

        return pUserData;
    }

    // -----------------------------------------------------------------------------

    bool CLuaStateManager::GetBoolean(BState _State, int _IndexOfStack, bool _DefaultValue)
    {
        BASE_UNUSED(_DefaultValue);

        return ::lua_toboolean(GetNativeState(_State), _IndexOfStack) != 0;
    }

    // -----------------------------------------------------------------------------

    signed char CLuaStateManager::GetSChar(BState _State, int _IndexOfStack, signed char _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<signed char>(Value);
    }

    // -----------------------------------------------------------------------------

    unsigned char CLuaStateManager::GetUChar(BState _State, int _IndexOfStack, unsigned char _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<unsigned char>(Value);
    }

    // -----------------------------------------------------------------------------

    signed short CLuaStateManager::GetSShort(BState _State, int _IndexOfStack, signed short _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<signed short>(Value);
    }

    // -----------------------------------------------------------------------------

    unsigned short CLuaStateManager::GetUShort(BState _State, int _IndexOfStack, unsigned short _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<unsigned short>(Value);
    }

    // -----------------------------------------------------------------------------

    signed int CLuaStateManager::GetSInt(BState _State, int _IndexOfStack, signed int _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<signed int>(Value);
    }

    // -----------------------------------------------------------------------------

    unsigned int CLuaStateManager::GetUInt(BState _State, int _IndexOfStack, unsigned int _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<unsigned int>(Value);
    }

    // -----------------------------------------------------------------------------

    signed long CLuaStateManager::GetSLong(BState _State, int _IndexOfStack, signed long _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<signed long>(Value);
    }

    // -----------------------------------------------------------------------------

    unsigned long CLuaStateManager::GetULong(BState _State, int _IndexOfStack, unsigned long _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<unsigned long>(Value);
    }

    // -----------------------------------------------------------------------------

    signed long long CLuaStateManager::GetSLongLong(BState _State, int _IndexOfStack, signed long long _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<signed long long>(Value);
    }

    // -----------------------------------------------------------------------------

    unsigned long long CLuaStateManager::GetULongLong(BState _State, int _IndexOfStack, unsigned long long _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<unsigned long long>(Value);
    }

    // -----------------------------------------------------------------------------

    float CLuaStateManager::GetFloat(BState _State, int _IndexOfStack, float _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<float>(Value);
    }

    // -----------------------------------------------------------------------------

    double CLuaStateManager::GetDouble(BState _State, int _IndexOfStack, double _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<double>(Value);
    }

    // -----------------------------------------------------------------------------

    long double CLuaStateManager::GetLongDouble(BState _State, int _IndexOfStack, long double _DefaultValue)
    {
        const lua_Number Value = ::luaL_optnumber(GetNativeState(_State), _IndexOfStack, static_cast<lua_Number>(_DefaultValue));

        return static_cast<long double>(Value);
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CLuaStateManager::GetString(BState _State, int _IndexOfStack)
    {
        return luaL_optstring(GetNativeState(_State), _IndexOfStack, "");
    }

    // -----------------------------------------------------------------------------

    void* CLuaStateManager::GetUserData(BState _State, int _IndexOfStack)
    {
        return lua_touserdata(GetNativeState(_State), _IndexOfStack);
    }

    // -----------------------------------------------------------------------------

    bool CLuaStateManager::IsBoolean(BState _State, int _IndexOfStack)
    {
        return lua_isboolean(GetNativeState(_State), _IndexOfStack) == true;
    }

    // -----------------------------------------------------------------------------

    bool CLuaStateManager::IsNumber(BState _State, int _IndexOfStack)
    {
        return ::lua_isnumber(GetNativeState(_State), _IndexOfStack) == 1;
    }

    // -----------------------------------------------------------------------------

    bool CLuaStateManager::IsString(BState _State, int _IndexOfStack)
    {
        return ::lua_isstring(GetNativeState(_State), _IndexOfStack) == 1;
    }

    // -----------------------------------------------------------------------------

    bool CLuaStateManager::IsPointer(BState _State, int _IndexOfStack)
    {
        return lua_islightuserdata(GetNativeState(_State), _IndexOfStack) == 1;
    }

    // -----------------------------------------------------------------------------

    lua_State* CLuaStateManager::GetNativeState(BState _State)
    {
        return static_cast<lua_State*>(_State);
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::Push(lua_State* _pState, const CArgument& _rArgument)
    {
        const CArgument& rArgument = static_cast<const CArgument&>(_rArgument);

        switch (rArgument.GetType())
        {
        case SValueType::Nil:     ::lua_pushnil(_pState); break;
        case SValueType::Boolean: ::lua_pushboolean(_pState, rArgument.GetBoolean()); break;
        case SValueType::Number:  ::lua_pushnumber(_pState, rArgument.GetNumber()); break;
        case SValueType::String:  ::lua_pushstring(_pState, rArgument.GetString().c_str()); break;
        case SValueType::Pointer: ::lua_pushlightuserdata(_pState, rArgument.GetPointer()); break;

        default:
        {
            ::lua_pushnil(_pState);
        }
        break;
        }
    }

    // -----------------------------------------------------------------------------

    void CLuaStateManager::Pop(lua_State* _pState, CResult& _rResult)
    {
        CResult& rResult = static_cast<CResult&>(_rResult);

        assert(rResult.IsEmpty());

        switch (::lua_type(_pState, -1))
        {
        case LUA_TBOOLEAN: rResult.Set(::lua_toboolean(_pState, -1) ? true : false); break;
        case LUA_TNUMBER:  rResult.Set(::lua_tonumber(_pState, -1)); break;
        case LUA_TSTRING:  rResult.Set(::lua_tostring(_pState, -1)); break;
        case LUA_TNIL:                                                                               break;

        default:
        {
            rResult.Clear();
        }
        break;
        }

        ::lua_pop(_pState, 1);
    }
} // namespace 

namespace Core
{
namespace Lua
{
namespace State
{
    BState CreateState(const Base::Char* _pName)
    {
        return CLuaStateManager::GetInstance().CreateState(_pName);
    }

    // -----------------------------------------------------------------------------

    void DeleteState(BState _State)
    {
        CLuaStateManager::GetInstance().DeleteState(_State);
    }

    // -----------------------------------------------------------------------------

    void LoadScript(BState _State, const Base::Char* _pFilename, int _Flags)
    {
        return CLuaStateManager::GetInstance().LoadScript(_State, _pFilename, _Flags);
    }

    // -----------------------------------------------------------------------------

    bool CallFunction(BState _State, const Base::Char* _pFunction, CResult* _pResult)
    {
        return CLuaStateManager::GetInstance().CallFunction(_State, _pFunction, 0, 0, _pResult);
    }

    // -----------------------------------------------------------------------------

    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument& _rArgument1, CResult* _pResult)
    {
        const CArgument* ppArguments[] = { &_rArgument1 };

        return CLuaStateManager::GetInstance().CallFunction(_State, _pFunction, ppArguments, 1, _pResult);
    }

    // -----------------------------------------------------------------------------

    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument& _rArgument1, const CArgument& _rArgument2, CResult* _pResult)
    {
        const CArgument* ppArguments[] = { &_rArgument1, &_rArgument2 };

        return CLuaStateManager::GetInstance().CallFunction(_State, _pFunction, ppArguments, 2, _pResult);
    }

    // -----------------------------------------------------------------------------

    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument& _rArgument1, const CArgument& _rArgument2, const CArgument& _rArgument3, CResult* _pResult)
    {
        const CArgument* ppArguments[] = { &_rArgument1, &_rArgument2, &_rArgument3 };

        return CLuaStateManager::GetInstance().CallFunction(_State, _pFunction, ppArguments, 3, _pResult);
    }

    // -----------------------------------------------------------------------------

    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument& _rArgument1, const CArgument& _rArgument2, const CArgument& _rArgument3, const CArgument& _rArgument4, CResult* _pResult)
    {
        const CArgument* ppArguments[] = { &_rArgument1, &_rArgument2, &_rArgument3, &_rArgument4 };

        return CLuaStateManager::GetInstance().CallFunction(_State, _pFunction, ppArguments, 4, _pResult);
    }

    // -----------------------------------------------------------------------------

    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument& _rArgument1, const CArgument& _rArgument2, const CArgument& _rArgument3, const CArgument& _rArgument4, const CArgument& _rArgument5, CResult* _pResult)
    {
        const CArgument* ppArguments[] = { &_rArgument1, &_rArgument2, &_rArgument3, &_rArgument4, &_rArgument5 };

        return CLuaStateManager::GetInstance().CallFunction(_State, _pFunction, ppArguments, 5, _pResult);
    }

    // -----------------------------------------------------------------------------

    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument** _ppArguments, unsigned int _NumberOfArguments, CResult* _pResult)
    {
        return CLuaStateManager::GetInstance().CallFunction(_State, _pFunction, _ppArguments, _NumberOfArguments, _pResult);
    }

    // -----------------------------------------------------------------------------

    void RequireLibrary(BState _State, const Base::Char* _pLibraryName, FLuaCFunc _pFunction)
    {
        CLuaStateManager::GetInstance().RequireLibrary(_State, _pLibraryName, _pFunction);
    }

    // -----------------------------------------------------------------------------

    void RegisterFunction(BState _State, const Base::Char* _pFunctionName, FLuaCFunc _pFunction)
    {
        CLuaStateManager::GetInstance().RegisterFunction(_State, _pFunctionName, _pFunction);
    }

    // -----------------------------------------------------------------------------

    void RegisterLibrary(BState _State, const Base::Char* _pLibraryName, const Base::Char* _pFunctionName, FLuaCFunc _pFunction)
    {
        CLuaStateManager::GetInstance().RegisterLibrary(_State, _pLibraryName, _pFunctionName, _pFunction);
    }

    // -----------------------------------------------------------------------------

    void RegisterObject(BState _State, const Base::Char* _pObjectName, const Base::Char* _pFunctionName, FLuaCFunc _pFunction)
    {
        CLuaStateManager::GetInstance().RegisterObject(_State, _pObjectName, _pFunctionName, _pFunction);
    }

    // -----------------------------------------------------------------------------

    void PushBoolean(BState _State, bool _Value)
    {
        CLuaStateManager::GetInstance().PushBoolean(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushSChar(BState _State, signed char _Value)
    {
        CLuaStateManager::GetInstance().PushSChar(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushUChar(BState _State, unsigned char _Value)
    {
        CLuaStateManager::GetInstance().PushUChar(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushSShort(BState _State, signed short _Value)
    {
        CLuaStateManager::GetInstance().PushSShort(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushUShort(BState _State, unsigned short _Value)
    {
        CLuaStateManager::GetInstance().PushUShort(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushSInt(BState _State, signed int _Value)
    {
        CLuaStateManager::GetInstance().PushSInt(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushUInt(BState _State, unsigned int _Value)
    {
        CLuaStateManager::GetInstance().PushUInt(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushSLong(BState _State, signed long _Value)
    {
        CLuaStateManager::GetInstance().PushSLong(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushULong(BState _State, unsigned long _Value)
    {
        CLuaStateManager::GetInstance().PushULong(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushSLongLong(BState _State, signed long long _Value)
    {
        CLuaStateManager::GetInstance().PushSLongLong(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushULongLong(BState _State, unsigned long long _Value)
    {
        CLuaStateManager::GetInstance().PushULongLong(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushFloat(BState _State, float _Value)
    {
        CLuaStateManager::GetInstance().PushFloat(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushDouble(BState _State, double _Value)
    {
        CLuaStateManager::GetInstance().PushDouble(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushLongDouble(BState _State, long double _Value)
    {
        CLuaStateManager::GetInstance().PushLongDouble(_State, _Value);
    }

    // -----------------------------------------------------------------------------

    void PushString(BState _State, const Base::Char* _pString)
    {
        CLuaStateManager::GetInstance().PushString(_State, _pString);
    }

    // -----------------------------------------------------------------------------

    void* PushUserData(BState _State, unsigned int _NumberOfBytes, const Base::Char* _pObjectName)
    {
        return CLuaStateManager::GetInstance().PushUserData(_State, _NumberOfBytes, _pObjectName);
    }

    // -----------------------------------------------------------------------------

    bool GetBoolean(BState _State, int _IndexOfStack, bool _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetBoolean(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    signed char GetSChar(BState _State, int _IndexOfStack, signed char _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetSChar(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    unsigned char GetUChar(BState _State, int _IndexOfStack, unsigned char _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetUChar(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    signed short GetSShort(BState _State, int _IndexOfStack, signed short _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetSShort(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    unsigned short GetUShort(BState _State, int _IndexOfStack, unsigned short _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetUShort(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    signed int GetSInt(BState _State, int _IndexOfStack, signed int _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetSInt(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    unsigned int GetUInt(BState _State, int _IndexOfStack, unsigned int _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetUInt(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    signed long GetSLong(BState _State, int _IndexOfStack, signed long _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetSLong(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    unsigned long GetULong(BState _State, int _IndexOfStack, unsigned long _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetULong(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    signed long long GetSLongLong(BState _State, int _IndexOfStack, signed long long _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetSLongLong(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    unsigned long long GetULongLong(BState _State, int _IndexOfStack, unsigned long long _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetULongLong(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    float GetFloat(BState _State, int _IndexOfStack, float _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetFloat(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    double GetDouble(BState _State, int _IndexOfStack, double _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetDouble(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    long double GetLongDouble(BState _State, int _IndexOfStack, long double _DefaultValue)
    {
        return CLuaStateManager::GetInstance().GetLongDouble(_State, _IndexOfStack, _DefaultValue);
    }

    // -----------------------------------------------------------------------------

    const Base::Char* GetString(BState _State, int _IndexOfStack)
    {
        return CLuaStateManager::GetInstance().GetString(_State, _IndexOfStack);
    }

    // -----------------------------------------------------------------------------

    void* GetUserData(BState _State, int _IndexOfStack)
    {
        return CLuaStateManager::GetInstance().GetUserData(_State, _IndexOfStack);
    }

    // -----------------------------------------------------------------------------

    bool IsBoolean(BState _State, int _IndexOfStack)
    {
        return CLuaStateManager::GetInstance().IsBoolean(_State, _IndexOfStack);
    }

    // -----------------------------------------------------------------------------

    bool IsNumber(BState _State, int _IndexOfStack)
    {
        return CLuaStateManager::GetInstance().IsNumber(_State, _IndexOfStack);
    }

    // -----------------------------------------------------------------------------

    bool IsString(BState _State, int _IndexOfStack)
    {
        return CLuaStateManager::GetInstance().IsString(_State, _IndexOfStack);
    }
} // namespace State
} // namespace Lua
} // namespace Core