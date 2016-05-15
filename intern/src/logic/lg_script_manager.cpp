
#include "logic/lg_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_lua_export.h"
#include "core/core_lua_main.h"
#include "core/core_lua_state.h"
#include "core/core_script_base_vector3.h"
#include "core/core_script_base_vector4.h"
#include "core/core_script_core_time.h"

#include "data/data_script_entity.h"

#include "gui/gui_script_input.h"

#include "logic/lg_script_manager.h"

LUA_REQUIRE_LIBRARY_FUNC(CoreTimeLibFuncs, Time)
LUA_REQUIRE_LIBRARY_FUNC(BaseFloat3LibFuncs, Vector3)
LUA_REQUIRE_LIBRARY_FUNC(BaseFloat4LibFuncs, Vector4)
LUA_REQUIRE_LIBRARY_FUNC(GuiInputLibFuncs, Input)

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
    };
} // namespace 

namespace 
{
    CLgScriptManager::CLgScriptManager()
    {
    }

    // -----------------------------------------------------------------------------

    CLgScriptManager::~CLgScriptManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::OnStart()
    {
        LUA_REGISTER_OBJECT(Core::Lua::Main::GetMainState(), BaseFloat3ObjFuncs, Base_Vector3);
        LUA_REGISTER_OBJECT(Core::Lua::Main::GetMainState(), BaseFloat4ObjFuncs, Base_Vector4);
        LUA_REGISTER_OBJECT(Core::Lua::Main::GetMainState(), DataEntityObjFuncs, Data_Entity);

        LUA_REGISTER_LIBRARY(Core::Lua::Main::GetMainState(), CoreTimeLibFuncs, Time);
        LUA_REGISTER_LIBRARY(Core::Lua::Main::GetMainState(), BaseFloat3LibFuncs, Vector3);
        LUA_REGISTER_LIBRARY(Core::Lua::Main::GetMainState(), BaseFloat4LibFuncs, Vector4);
        LUA_REGISTER_LIBRARY(Core::Lua::Main::GetMainState(), GuiInputLibFuncs, Input);
    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::Clear()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::Update()
    {
        // -----------------------------------------------------------------------------
        // Create state
        // -----------------------------------------------------------------------------
        Core::Lua::BState LuaState = Core::Lua::Main::GetMainState();

        // -----------------------------------------------------------------------------
        // Load a script
        // -----------------------------------------------------------------------------
        std::string PathToScript = std::string(g_PathToAssets) + "test.lua";

        Core::Lua::State::LoadScript(LuaState, PathToScript.c_str(), 0);

        Core::Lua::State::CallFunction(LuaState, "OnStart", 0);
        Core::Lua::State::CallFunction(LuaState, "Update", 0);
        Core::Lua::State::CallFunction(LuaState, "OnExit", 0);
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