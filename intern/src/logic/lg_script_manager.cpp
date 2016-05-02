
#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_lua_main.h"
#include "core/core_lua_state.h"
#include "core/core_script_base_vector3.h"

#include "logic/lg_script_manager.h"

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