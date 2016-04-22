
#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

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
        lua_register(m_pLuaState, "Summarize", ScriptFuncSummarize);

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
        }

        lua_getglobal(m_pLuaState, "Update");

        if (Script == 0)
        {
            Script = lua_pcall(m_pLuaState, 0, LUA_MULTRET, 0);
        }

        lua_getglobal(m_pLuaState, "OnExit");

        if (Script == 0)
        {
            Script = lua_pcall(m_pLuaState, 0, LUA_MULTRET, 0);
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