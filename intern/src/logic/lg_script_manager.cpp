
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
    struct SVector2
    {
        float m_X;
        float m_Y;
    };

    int ScriptFuncVector2New(lua_State* _pLuaState)
    {
        SVector2* pNewEntityInScript;

        float X = static_cast<float>(lua_tonumber(_pLuaState, 1));
        float Y = static_cast<float>(lua_tonumber(_pLuaState, 2));

        // -----------------------------------------------------------------------------
        // Create new full user data for a entity
        // -----------------------------------------------------------------------------
        pNewEntityInScript = static_cast<SVector2*>(lua_newuserdata(_pLuaState, sizeof(SVector2)));
        pNewEntityInScript->m_X = X;
        pNewEntityInScript->m_Y = Y;

        // -----------------------------------------------------------------------------
        // Set meta table for entities on this new entity
        // -----------------------------------------------------------------------------
        luaL_getmetatable(_pLuaState, "Data.Vector2");
        lua_setmetatable(_pLuaState, -2);

        return 1;
    }

    int ScriptFuncVector2SetX(lua_State* _pLuaState)
    {
        SVector2* pVector2 = static_cast<SVector2*>(lua_touserdata(_pLuaState, 1));

        float X = static_cast<float>(lua_tonumber(_pLuaState, 2));

        pVector2->m_X = X;

        return 0;
    }

    int ScriptFuncVector2SetY(lua_State* _pLuaState)
    {
        SVector2* pVector2 = static_cast<SVector2*>(lua_touserdata(_pLuaState, 1));

        float Y = static_cast<float>(lua_tonumber(_pLuaState, 2));

        pVector2->m_Y = Y;

        return 0;
    }

    int ScriptFuncVector2GetX(lua_State* _pLuaState)
    {
        SVector2* pVector2 = static_cast<SVector2*>(lua_touserdata(_pLuaState, 1));

        lua_pushnumber(_pLuaState, pVector2->m_X);

        return 1;
    }

    int ScriptFuncVector2GetY(lua_State* _pLuaState)
    {
        SVector2* pVector2 = static_cast<SVector2*>(lua_touserdata(_pLuaState, 1));

        lua_pushnumber(_pLuaState, pVector2->m_Y);

        return 1;
    }

    static int ScriptFuncVector2ToString(lua_State* _pLuaState)
    {
        SVector2* pVector2 = static_cast<SVector2*>(lua_touserdata(_pLuaState, 1));

        lua_pushfstring(_pLuaState, "Vector2 x=%f, y=%f", pVector2->m_X, pVector2->m_Y);

        return 1;
    }


    // -----------------------------------------------------------------------------
    // Create a table with every function that is accessible inside
    // Lua. It is an array of name and function pointer.
    // -----------------------------------------------------------------------------
    static const struct luaL_Reg s_RequireVector2FunctionTable[] =
    {
        { "new", ScriptFuncVector2New },
        { NULL, NULL }
    };


    // -----------------------------------------------------------------------------
    // That is an meta table that describes the functionality of an Vector2.
    // Combination of name and function pointer is needed.
    // -----------------------------------------------------------------------------
    static const struct luaL_Reg s_ObjectVector2FunctionTable[] =
    {
        { "__tostring", ScriptFuncVector2ToString },
        { "GetX", ScriptFuncVector2GetX },
        { "GetY", ScriptFuncVector2GetY },
        { "SetX", ScriptFuncVector2SetX },
        { "SetY", ScriptFuncVector2SetY },
        { NULL, NULL }
    };
} // namespace 

namespace
{
    int OnLuaRequireVector2(lua_State* _pLuaState)
    {
        // -----------------------------------------------------------------------------
        // Create new table
        // -----------------------------------------------------------------------------
        lua_newtable(_pLuaState);

        // -----------------------------------------------------------------------------
        // Set specific function on this table
        // -----------------------------------------------------------------------------
        luaL_setfuncs(_pLuaState, s_RequireVector2FunctionTable, 0);

        return 1;
    }

    void LuaRegisterLibraryVector2(lua_State* _pLuaState)
    {
        luaL_requiref(_pLuaState, "vector2", OnLuaRequireVector2, 1);
    }

    // -----------------------------------------------------------------------------

    void LuaRegisterVector2Object(lua_State *_pLuaState)
    {
        // -----------------------------------------------------------------------------
        // Create a new meta table for this Lua script
        // Namespace.UniqueName
        // -----------------------------------------------------------------------------
        luaL_newmetatable(_pLuaState, "Data.Vector2");

        lua_pushvalue(_pLuaState, -1);
        lua_setfield(_pLuaState, -2, "__index");

        // -----------------------------------------------------------------------------
        // Set a function that loads the function table on the meta table as soon
        // as the scripts require an entity
        // -----------------------------------------------------------------------------
        luaL_setfuncs(_pLuaState, s_ObjectVector2FunctionTable, 0);
    }
} // namespace 

namespace 
{
    struct SEntity
    {
        SVector2 m_Position;
        float    m_Speed;
    };

    int ScriptFuncEntityNew(lua_State* _pLuaState)
    {
        SEntity* pNewEntityInScript;

        // -----------------------------------------------------------------------------
        // Create new full user data for a entity
        // -----------------------------------------------------------------------------
        pNewEntityInScript = static_cast<SEntity*>(lua_newuserdata(_pLuaState, sizeof(SEntity)));
        pNewEntityInScript->m_Position.m_X = 0;
        pNewEntityInScript->m_Position.m_Y = 0;
        pNewEntityInScript->m_Speed = 0;

        // -----------------------------------------------------------------------------
        // Set meta table for entities on this new entity
        // -----------------------------------------------------------------------------
        luaL_getmetatable(_pLuaState, "Data.Entity");
        lua_setmetatable(_pLuaState, -2);

        return 1;
    }

    int ScriptFuncEntitySetPos(lua_State* _pLuaState)
    {
        SEntity*  pEntity = static_cast<SEntity*>(luaL_checkudata(_pLuaState, 1, "Data.Entity"));
        SVector2* pVector = static_cast<SVector2*>(luaL_checkudata(_pLuaState, 2, "Data.Vector2"));

        pEntity->m_Position.m_X = pVector->m_X;
        pEntity->m_Position.m_Y = pVector->m_Y;

        return 0;
    }

    int ScriptFuncEntityGetPos(lua_State* _pLuaState)
    {
        SEntity*  pEntity = static_cast<SEntity*>(luaL_checkudata(_pLuaState, 1, "Data.Entity"));

        SVector2* pNewEntityInScript = static_cast<SVector2*>(lua_newuserdata(_pLuaState, sizeof(SVector2)));
        pNewEntityInScript->m_X = pEntity->m_Position.m_X;
        pNewEntityInScript->m_Y = pEntity->m_Position.m_Y;

        // -----------------------------------------------------------------------------
        // Set meta table for entities on this new entity
        // -----------------------------------------------------------------------------
        luaL_getmetatable(_pLuaState, "Data.Vector2");
        lua_setmetatable(_pLuaState, -2);

        return 1;
    }

    static int ScriptFuncEntityToString(lua_State* _pLuaState)
    {
        SEntity* pEntity = static_cast<SEntity*>(luaL_checkudata(_pLuaState, 1, "Data.Entity"));

        lua_pushfstring(_pLuaState, "Entity at position: x=%f, y=%f and speed=%f", pEntity->m_Position.m_X, pEntity->m_Position.m_Y, pEntity->m_Speed);

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
        { "GetPosition", ScriptFuncEntityGetPos },
        { "SetPosition", ScriptFuncEntitySetPos },
        { NULL, NULL }
    };
} // namespace 

namespace 
{
    int OnLuaRequireEntity(lua_State* _pLuaState)
    {
        // -----------------------------------------------------------------------------
        // Create new table
        // -----------------------------------------------------------------------------
        lua_newtable(_pLuaState);

        // -----------------------------------------------------------------------------
        // Set specific function on this table
        // -----------------------------------------------------------------------------
        luaL_setfuncs(_pLuaState, s_RequireEntityFunctionTable, 0);

        return 1;
    }

    void LuaRegisterLibraryEntity(lua_State* _pLuaState)
    {
        luaL_requiref(_pLuaState, "entity", OnLuaRequireEntity, 1);
    }
    
    // -----------------------------------------------------------------------------

    void LuaRegisterEntityObject(lua_State *_pLuaState)
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
    }
} // namespace 



namespace 
{
    static SEntity StaticEntity;

    int OnLuaGetBindedEntity(lua_State* _pLuaState)
    {
        // -----------------------------------------------------------------------------
        // Create new full user data for a entity and set an existing entity 
        // on it.
        // -----------------------------------------------------------------------------
        SEntity* res = (SEntity *)lua_newuserdata(_pLuaState, sizeof(SEntity));
        res->m_Position.m_X = StaticEntity.m_Position.m_X;
        res->m_Position.m_Y = StaticEntity.m_Position.m_Y;
        res->m_Speed = StaticEntity.m_Speed;

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
    void LuaRegisterGlobalFunctionGetBindedEntity(lua_State* _pLuaState)
    {
        lua_register(_pLuaState, "Entity", OnLuaGetBindedEntity);
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
        StaticEntity.m_Position.m_X = 5.0f;
        StaticEntity.m_Position.m_Y = 5.0f;
        StaticEntity.m_Speed = 2.0f;
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
        LuaRegisterVector2Object(m_pLuaState);

        LuaRegisterLibraryVector2(m_pLuaState);
        
        LuaRegisterEntityObject(m_pLuaState);

        LuaRegisterLibraryEntity(m_pLuaState);

        LuaRegisterGlobalFunctionGetBindedEntity(m_pLuaState);

        // -----------------------------------------------------------------------------
        // Load a script
        // -----------------------------------------------------------------------------
        std::string PathToScript = std::string(g_PathToAssets) + "test.lua";

        luaL_loadfile(m_pLuaState, PathToScript.c_str());

        int Script = lua_pcall(m_pLuaState, 0, 0, 0);

        OnLuaErrorCallback(m_pLuaState, Script);

        // -----------------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------------
        Script = luaL_loadfile(m_pLuaState, PathToScript.c_str());

        lua_pcall(m_pLuaState, 0, 0, 0);

        lua_getglobal(m_pLuaState, "OnStart");

        if (Script == 0)
        {
            Script = lua_pcall(m_pLuaState, 0, LUA_MULTRET, 0);

            OnLuaErrorCallback(m_pLuaState, Script);
        }

        lua_getglobal(m_pLuaState, "Update");

        if (Script == 0)
        {
            Script = lua_pcall(m_pLuaState, 0, LUA_MULTRET, 0);

            OnLuaErrorCallback(m_pLuaState, Script);
        }

        lua_getglobal(m_pLuaState, "OnExit");

        if (Script == 0)
        {
            Script = lua_pcall(m_pLuaState, 0, LUA_MULTRET, 0);

            OnLuaErrorCallback(m_pLuaState, Script);
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