
#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "logic/lg_script_manager.h"

#include "lua.hpp"

#include <assert.h>
#include <fstream>

namespace 
{
    void LuaErrorCallback(lua_State* _pLuaState, int _StatusCode)
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
    int ScriptFuncSummarize(lua_State* _pLuaState)
    {
        int NumberOfArguments = lua_gettop(_pLuaState);

        int Summe = 0;

        for (int IndexOfArgument = 1; IndexOfArgument <= NumberOfArguments; ++IndexOfArgument)
        {
            Summe += (int)lua_tointeger(_pLuaState, IndexOfArgument);
        }

        lua_pushnumber(_pLuaState, Summe);

        return 1;
    }





    struct SEntity
    {
        float m_X;
        float m_Y;
    };


    static SEntity StaticEntity;

    


    int ScriptFuncEntity(lua_State* _pLuaState)
    {
        lua_pushlightuserdata(_pLuaState, &StaticEntity);

        return 1;
    }

    int ScriptFuncEntitySetXY(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(lua_touserdata(_pLuaState, 1));

        float X = lua_tonumber(_pLuaState, 2);
        float Y = lua_tonumber(_pLuaState, 3);

        pEntity->m_X = X;
        pEntity->m_Y = Y;

        return 0;
    }

    int ScriptFuncEntitySetX(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(lua_touserdata(_pLuaState, 1));

        float X = lua_tonumber(_pLuaState, 2);

        pEntity->m_X = X;

        return 0;
    }

    int ScriptFuncEntitySetY(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(lua_touserdata(_pLuaState, 1));

        float Y = lua_tonumber(_pLuaState, 2);

        pEntity->m_Y = Y;

        return 0;
    }

    int ScriptFuncEntityGetX(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(lua_touserdata(_pLuaState, 1));

        lua_pushnumber(_pLuaState, pEntity->m_X);

        return 1;
    }

    int ScriptFuncEntityGetY(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(lua_touserdata(_pLuaState, 1));

        lua_pushnumber(_pLuaState, pEntity->m_Y);

        return 1;
    }

    int ScriptFuncEntityGetXY(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(lua_touserdata(_pLuaState, 1));

        lua_pushnumber(_pLuaState, pEntity->m_X);
        lua_pushnumber(_pLuaState, pEntity->m_Y);

        return 2;
    }
} // namespace 

namespace 
{
    void lua_register_Entity(lua_State* _pLuaState)
    {
        lua_register(_pLuaState, "Entity"     , ScriptFuncEntity);
        lua_register(_pLuaState, "EntitySetX" , ScriptFuncEntitySetX);
        lua_register(_pLuaState, "EntitySetY" , ScriptFuncEntitySetY);
        lua_register(_pLuaState, "EntitySetXY", ScriptFuncEntitySetXY);
        lua_register(_pLuaState, "EntityGetX" , ScriptFuncEntityGetX);
        lua_register(_pLuaState, "EntityGetY" , ScriptFuncEntityGetY);
        lua_register(_pLuaState, "EntityGetXY", ScriptFuncEntityGetXY);
    }
} // namespace 

namespace
{
    static const char* g_PathToAssets = "../assets/";
} // namespace

namespace 
{
    class CLgScriptManager : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLgScriptManager)

    public:

        CLgScriptManager();
        ~CLgScriptManager();

    public:

        void OnStart();
        void OnExit();

        void Clear();

        void Update();

    private:

        lua_State* m_pLuaState;
    };
} // namespace 

namespace 
{
    CLgScriptManager::CLgScriptManager()
        : m_pLuaState(0)
    {
        StaticEntity.m_X = 5.0f;
        StaticEntity.m_Y = 5.0f;
    }

    // -----------------------------------------------------------------------------

    CLgScriptManager::~CLgScriptManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Create state
        // -----------------------------------------------------------------------------
        m_pLuaState = luaL_newstate();

        // -----------------------------------------------------------------------------
        // Load libraries
        // -----------------------------------------------------------------------------
        luaopen_io    (m_pLuaState);
        luaopen_base  (m_pLuaState);
        luaopen_table (m_pLuaState);
        luaopen_string(m_pLuaState);
        luaopen_math  (m_pLuaState);

        luaL_openlibs(m_pLuaState);

        // -----------------------------------------------------------------------------
        // Register C-Functions
        // -----------------------------------------------------------------------------
        lua_register_Entity(m_pLuaState);

        // -----------------------------------------------------------------------------
        // Load a script
        // -----------------------------------------------------------------------------
        std::string PathToScript = std::string(g_PathToAssets) + "test.lua";

        luaL_loadfile(m_pLuaState, PathToScript.c_str());

        int Script = lua_pcall(m_pLuaState, 0, 0, 0);

        LuaErrorCallback(m_pLuaState, Script);

        // -----------------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------------
        Script = luaL_loadfile(m_pLuaState, PathToScript.c_str());

        lua_pcall(m_pLuaState, 0, 0, 0);

        lua_getglobal(m_pLuaState, "OnStart");

        if (Script == 0)
        {
            Script = lua_pcall(m_pLuaState, 0, LUA_MULTRET, 0);

            LuaErrorCallback(m_pLuaState, Script);
        }

        lua_getglobal(m_pLuaState, "Update");

        if (Script == 0)
        {
            Script = lua_pcall(m_pLuaState, 0, LUA_MULTRET, 0);

            LuaErrorCallback(m_pLuaState, Script);
        }

        lua_getglobal(m_pLuaState, "OnExit");

        if (Script == 0)
        {
            Script = lua_pcall(m_pLuaState, 0, LUA_MULTRET, 0);

            LuaErrorCallback(m_pLuaState, Script);
        }
    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::OnExit()
    {
        Clear();

        // -----------------------------------------------------------------------------
        // Cleanup
        // -----------------------------------------------------------------------------
        lua_close(m_pLuaState);
    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::Clear()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::Update()
    {
        
    }
} // namespace 

namespace Lg
{
namespace ScriptManager
{
    void OnStart()
    {
        CLgScriptManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CLgScriptManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CLgScriptManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CLgScriptManager::GetInstance().Update();
    }
} // namespace Name2
} // namespace Lg