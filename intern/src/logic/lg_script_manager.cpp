
#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_lua_export.h"
#include "core/core_lua_main.h"
#include "core/core_lua_state.h"

#include "logic/lg_script_manager.h"

#include <assert.h>
#include <string>

namespace
{
    struct SVector2
    {
        float m_X;
        float m_Y;
    };

    LUA::CStaticFunctionList Vector2LibraryFunctions;
    LUA::CStaticFunctionList Vector2ObjectFunctions;

    LUA_DEFINE_FUNCTION(Vector2LibraryFunctions, New)
    {
        SVector2* pNewEntityInScript;

        float X = LUA::State::GetFloat(_State, 1);
        float Y = LUA::State::GetFloat(_State, 2);

        // -----------------------------------------------------------------------------
        // Create new full user data for a entity
        // -----------------------------------------------------------------------------
        pNewEntityInScript = static_cast<SVector2*>(LUA::State::PushUserData(_State, sizeof(SVector2), "Data_Vector2"));

        pNewEntityInScript->m_X = X;
        pNewEntityInScript->m_Y = Y;

        return 1;
    }


    LUA_DEFINE_FUNCTION(Vector2ObjectFunctions, SetX)
    {
        SVector2* pVector2 = static_cast<SVector2*>(LUA::State::GetUserData(_State, 1));

        float X = static_cast<float>(LUA::State::GetFloat(_State, 2));

        pVector2->m_X = X;

        return 0;
    }

    LUA_DEFINE_FUNCTION(Vector2ObjectFunctions, SetY)
    {
        SVector2* pVector2 = static_cast<SVector2*>(LUA::State::GetUserData(_State, 1));

        float Y = static_cast<float>(LUA::State::GetFloat(_State, 2));

        pVector2->m_Y = Y;

        return 0;
    }

    LUA_DEFINE_FUNCTION(Vector2ObjectFunctions, GetX)
    {
        SVector2* pVector2 = static_cast<SVector2*>(LUA::State::GetUserData(_State, 1));

        LUA::State::PushFloat(_State, pVector2->m_X);

        return 1;
    }

    LUA_DEFINE_FUNCTION(Vector2ObjectFunctions, GetY)
    {
        SVector2* pVector2 = static_cast<SVector2*>(LUA::State::GetUserData(_State, 1));

        LUA::State::PushFloat(_State, pVector2->m_Y);

        return 1;
    }

    LUA_DEFINE_FUNCTION(Vector2ObjectFunctions, __tostring)
    {
        SVector2* pVector2 = static_cast<SVector2*>(LUA::State::GetUserData(_State, 1));

        std::string String;

        String = "Vector2 x=" + std::to_string(pVector2->m_X) + ", y=" + std::to_string(pVector2->m_Y);

        LUA::State::PushString(_State, String.c_str());

        return 1;
    }
} // namespace 

LUA_REGISTER_LIBRARY(LUA::Main::GetMainState(), Vector2LibraryFunctions, vector2)

LUA_REGISTER_OBJECT(LUA::Main::GetMainState(), Vector2ObjectFunctions, Data_Vector2)


namespace 
{
    struct SEntity
    {
        SVector2 m_Position;
        float    m_Speed;
    };

    LUA::CStaticFunctionList EntityLibraryFunctions;
    LUA::CStaticFunctionList EntityObjectFunctions;

    LUA_DEFINE_FUNCTION(EntityLibraryFunctions, New)
    {
        SEntity* pNewEntityInScript;

        // -----------------------------------------------------------------------------
        // Create new full user data for a entity
        // -----------------------------------------------------------------------------
        pNewEntityInScript = static_cast<SEntity*>(LUA::State::PushUserData(_State, sizeof(SEntity), "Data_Entity"));
        pNewEntityInScript->m_Position.m_X = 0;
        pNewEntityInScript->m_Position.m_Y = 0;
        pNewEntityInScript->m_Speed = 0;

        return 1;
    }

    LUA_DEFINE_FUNCTION(EntityObjectFunctions, SetPosition)
    {
        SEntity*  pEntity = static_cast<SEntity*>(LUA::State::GetUserData(_State, 1));
        SVector2* pVector = static_cast<SVector2*>(LUA::State::GetUserData(_State, 2));

        pEntity->m_Position.m_X = pVector->m_X;
        pEntity->m_Position.m_Y = pVector->m_Y;

        return 0;
    }

    LUA_DEFINE_FUNCTION(EntityObjectFunctions, GetPosition)
    {
        SEntity*  pEntity = static_cast<SEntity*>(LUA::State::GetUserData(_State, 1));

        SVector2* pNewEntityInScript = static_cast<SVector2*>(LUA::State::PushUserData(_State, sizeof(SVector2), "Data_Vector2"));
        pNewEntityInScript->m_X = pEntity->m_Position.m_X;
        pNewEntityInScript->m_Y = pEntity->m_Position.m_Y;

        return 1;
    }

    LUA_DEFINE_FUNCTION(EntityObjectFunctions, __tostring)
    {
        SEntity* pEntity = static_cast<SEntity*>(LUA::State::GetUserData(_State, 1));

        std::string String;

        String = "Entity at position: x=" + std::to_string(pEntity->m_Position.m_X) + ", y=" + std::to_string(pEntity->m_Position.m_Y);

        LUA::State::PushString(_State, String.c_str());

        return 1;
    }
} // namespace 

LUA_REGISTER_LIBRARY(LUA::Main::GetMainState(), EntityLibraryFunctions, entity)

LUA_REGISTER_OBJECT(LUA::Main::GetMainState(), EntityObjectFunctions, Data_Entity)

namespace 
{
    static SEntity StaticEntity;

    LUA::CStaticFunctionList ScriptFunctions;

    LUA_DEFINE_FUNCTION(ScriptFunctions, Entity)
    {
        // -----------------------------------------------------------------------------
        // Create new full user data for a entity and set an existing entity 
        // on it.
        // -----------------------------------------------------------------------------
        SEntity* pNewEntityInScript = static_cast<SEntity*>(LUA::State::PushUserData(_State, sizeof(SEntity), "Data_Entity"));
        pNewEntityInScript->m_Position.m_X = StaticEntity.m_Position.m_X;
        pNewEntityInScript->m_Position.m_Y = StaticEntity.m_Position.m_Y;
        pNewEntityInScript->m_Speed = StaticEntity.m_Speed;

        return 1;
    }
} // namespace 

LUA_REGISTER_FUNCTIONS(LUA::Main::GetMainState(), ScriptFunctions)


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
        LUA::BState LuaState = LUA::Main::GetMainState();

        // -----------------------------------------------------------------------------
        // Load a script
        // -----------------------------------------------------------------------------
        std::string PathToScript = std::string(g_PathToAssets) + "test.lua";

        LUA::State::LoadScript(LuaState, PathToScript.c_str(), 0);

        LUA::State::CallFunction(LuaState, "OnStart", 0);
        LUA::State::CallFunction(LuaState, "Update", 0);
        LUA::State::CallFunction(LuaState, "OnExit", 0);
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