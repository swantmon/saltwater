
#include "logic/lg_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_lua_export.h"
#include "core/core_lua_main.h"
#include "core/core_lua_state.h"
#include "core/core_script_interface.h"

#include "data/data_script_interface.h"
#include "data/data_entity_manager.h"
#include "data/data_script_manager.h"

#include "gui/gui_script_interface.h"

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

        Dt::CEntity* GetActiveEntity() const;

    private:

        struct SInternScript
        {
            Dt::CEntity*      m_pEntity;
            Dt::CScriptFacet* m_pScriptFacet;
        };

    private:

        typedef std::vector<SInternScript> CInternScripts;

    private:

        CInternScripts m_InternScripts;
        SInternScript* m_pActiveScript;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace 

namespace 
{
    CLgScriptManager::CLgScriptManager()
        : m_InternScripts()
        , m_pActiveScript(0)
    {
    }

    // -----------------------------------------------------------------------------

    CLgScriptManager::~CLgScriptManager()
    {
        m_InternScripts.clear();
    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::OnStart()
    {
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CLgScriptManager::OnDirtyEntity));
    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::Clear()
    {
        m_InternScripts.clear();
    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::Update()
    {
        // -----------------------------------------------------------------------------
        // Create state
        // -----------------------------------------------------------------------------
        Core::Lua::BState LuaState = Core::Lua::Main::GetMainState();

        CInternScripts::iterator CurrentInternScript = m_InternScripts.begin();
        CInternScripts::iterator EndOfInternScripts = m_InternScripts.end();

        for (; CurrentInternScript != EndOfInternScripts; ++ CurrentInternScript)
        {
            // -----------------------------------------------------------------------------
            // Set active entity
            // -----------------------------------------------------------------------------
            m_pActiveScript = &(*CurrentInternScript);

            // -----------------------------------------------------------------------------
            // Load a script
            // -----------------------------------------------------------------------------
            std::string PathToScript = std::string(g_PathToAssets) + CurrentInternScript->m_pScriptFacet->GetScriptFile();

            Core::Lua::State::LoadScript(LuaState, PathToScript.c_str(), 0);

            Core::Lua::State::CallFunction(LuaState, "OnStart", 0);
            Core::Lua::State::CallFunction(LuaState, "Update", 0);
            Core::Lua::State::CallFunction(LuaState, "OnExit", 0);
        }
    }

    // -----------------------------------------------------------------------------

    Dt::CEntity* CLgScriptManager::GetActiveEntity() const
    {
        return m_pActiveScript->m_pEntity;
    }

    // -----------------------------------------------------------------------------

    void CLgScriptManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        unsigned int DirtyFlags;

        DirtyFlags = _pEntity->GetDirtyFlags();

        // -----------------------------------------------------------------------------
        // Check if it is a new actor
        // -----------------------------------------------------------------------------
        if ((DirtyFlags & Dt::CEntity::DirtyCreate) != 0)
        {
            Dt::CScriptFacet* pScriptFacet = static_cast<Dt::CScriptFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Script));

            if (pScriptFacet)
            {
                // -----------------------------------------------------------------------------
                // TODO:
                // Check if entity is already inside; is this possible?
                // -----------------------------------------------------------------------------
                SInternScript NewScript;

                NewScript.m_pScriptFacet = pScriptFacet;
                NewScript.m_pEntity      = _pEntity;

                m_InternScripts.push_back(NewScript);
            }
        }
    }
} // namespace 

namespace 
{
	LUA_REGISTER_OBJECT(Core::Lua::Main::GetMainState(), BaseFloat3ObjFuncs, Base_Vector3)
    LUA_REGISTER_OBJECT(Core::Lua::Main::GetMainState(), BaseFloat4ObjFuncs, Base_Vector4)
    LUA_REGISTER_OBJECT(Core::Lua::Main::GetMainState(), DataEntityObjFuncs, Data_Entity)

    LUA_REGISTER_LIBRARY(Core::Lua::Main::GetMainState(), CoreTimeLibFuncs, Time)
    LUA_REGISTER_LIBRARY(Core::Lua::Main::GetMainState(), BaseFloat3LibFuncs, Vector3)
    LUA_REGISTER_LIBRARY(Core::Lua::Main::GetMainState(), BaseFloat4LibFuncs, Vector4)
    LUA_REGISTER_LIBRARY(Core::Lua::Main::GetMainState(), GuiInputLibFuncs, Input)

    // -----------------------------------------------------------------------------

    Core::Lua::CStaticFunctionList ScriptFunctions;

    LUA_DEFINE_FUNCTION(ScriptFunctions, GetEntity)
    {
        Dt::CEntity* pActiveEntity = CLgScriptManager::GetInstance().GetActiveEntity();

        if (pActiveEntity != 0)
        {
            CLuaEntity& rResult = *static_cast<CLuaEntity*>(Core::Lua::State::PushUserData(_State, sizeof(CLuaEntity), "Data_Entity"));

            rResult.m_pEntity = pActiveEntity;

            return 1;
        }

        return 0;
    }

    LUA_REGISTER_FUNCTIONS(Core::Lua::Main::GetMainState(), ScriptFunctions)
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