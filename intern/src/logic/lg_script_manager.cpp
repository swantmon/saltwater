
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
    struct SEntity
    {
        float m_X;
        float m_Y;
    };

    int ScriptFuncEntityNew(lua_State* _pLuaState)
    {
        SEntity* pNewEntityInScript;

        // -----------------------------------------------------------------------------
        // Create new full user data for a entity
        // -----------------------------------------------------------------------------
        pNewEntityInScript = static_cast<SEntity*>(lua_newuserdata(_pLuaState, sizeof(SEntity)));
        pNewEntityInScript->m_X = 0;
        pNewEntityInScript->m_Y = 0;

        // -----------------------------------------------------------------------------
        // Set meta table for entities on this new entity
        // -----------------------------------------------------------------------------
        luaL_getmetatable(_pLuaState, "Data.Entity");
        lua_setmetatable(_pLuaState, -2);

        return 1;
    }

    int ScriptFuncEntitySetX(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(luaL_checkudata(_pLuaState, 1, "Data.Entity"));

        float X = lua_tonumber(_pLuaState, 2);

        pEntity->m_X = X;

        return 0;
    }

    int ScriptFuncEntitySetY(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(luaL_checkudata(_pLuaState, 1, "Data.Entity"));

        float Y = lua_tonumber(_pLuaState, 2);

        pEntity->m_Y = Y;

        return 0;
    }

    int ScriptFuncEntityGetX(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(luaL_checkudata(_pLuaState, 1, "Data.Entity"));

        lua_pushnumber(_pLuaState, pEntity->m_X);

        return 1;
    }

    int ScriptFuncEntityGetY(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(luaL_checkudata(_pLuaState, 1, "Data.Entity"));

        lua_pushnumber(_pLuaState, pEntity->m_Y);

        return 1;
    }

    static int ScriptFuncEntityToString(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(luaL_checkudata(_pLuaState, 1, "Data.Entity"));

        lua_pushfstring(_pLuaState, "Entity at position: x=%f, y=%f", pEntity->m_X, pEntity->m_Y);

        return 1;
    }


    // -----------------------------------------------------------------------------
    // Create a table with every function that is accessible inside
    // Lua. It is an array of name and function pointer.
    // -----------------------------------------------------------------------------
    static const struct luaL_Reg s_RequireEntityFunctionTable[] = 
    {
        { "new", ScriptFuncEntityNew },
        { NULL, NULL }
    };


    // -----------------------------------------------------------------------------
    // That is an meta table that describes the functionality of an entity.
    // Combination of name and function pointer is needed.
    // -----------------------------------------------------------------------------
    static const struct luaL_Reg s_ObjectEntityFunctionTable[] = 
    {
        { "__tostring", ScriptFuncEntityToString },
        { "x", ScriptFuncEntityGetX },
        { "y", ScriptFuncEntityGetY },
        { "setx", ScriptFuncEntitySetX },
        { "sety", ScriptFuncEntitySetY },
        { NULL, NULL }
    };

} // namespace 

namespace 
{
    int lua_reguire_entity(lua_State *L)
    {
        // -----------------------------------------------------------------------------
        // Create new table with informations from global informations
        // -----------------------------------------------------------------------------
        lua_newtable(L);
        lua_pushvalue(L, -2);
        lua_setfield(L, -2, "Data.Entity");

        // -----------------------------------------------------------------------------
        // Set specific function on this table
        // -----------------------------------------------------------------------------
        luaL_setfuncs(L, s_RequireEntityFunctionTable, 0);

        return 1;
    }

    int lua_register_entity_meta(lua_State *_pLuaState)
    {
        // -----------------------------------------------------------------------------
        // Create a new meta table for this Lua script
        // Namespace.UniqueName
        // -----------------------------------------------------------------------------
        luaL_newmetatable(_pLuaState, "Data.Entity");

        lua_pushvalue(_pLuaState, -1);
        lua_setfield(_pLuaState, -2, "__index");

        // -----------------------------------------------------------------------------
        // Set a function that loads the function table on the meta table as soon
        // as the scripts require an entity
        // -----------------------------------------------------------------------------
        luaL_setfuncs(_pLuaState, s_ObjectEntityFunctionTable, 0);
        luaL_requiref(_pLuaState, "data_entity", lua_reguire_entity, 1);

        return 1;
    }
} // namespace 



namespace 
{
    static SEntity StaticEntity;

    int ScriptGetBindedEntity(lua_State* _pLuaState)
    {
        // -----------------------------------------------------------------------------
        // Create new full user data for a entity and set an existing entity 
        // on it.
        // -----------------------------------------------------------------------------
        SEntity* res = (SEntity *)lua_newuserdata(_pLuaState, sizeof(SEntity));
        res->m_X = StaticEntity.m_X;
        res->m_Y = StaticEntity.m_Y;

        // -----------------------------------------------------------------------------
        // Set meta table for entities on this new entity
        // -----------------------------------------------------------------------------
        luaL_getmetatable(_pLuaState, "Data.Entity");
        lua_setmetatable(_pLuaState, -2);

        return 1;
    }
} // namespace 

namespace
{
    void lua_register_Entity(lua_State* _pLuaState)
    {
        lua_register(_pLuaState, "Entity", ScriptGetBindedEntity);
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
        lua_register_entity_meta(m_pLuaState);

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