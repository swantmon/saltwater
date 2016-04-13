
#include "app/app_application.h"

#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "game/game_application.h"
#include "game/game_debug_state.h"
#include "game/game_exit_state.h"
#include "game/game_intro_state.h"
#include "game/game_load_map_state.h"
#include "game/game_main_menu_state.h"
#include "game/game_play_state.h"
#include "game/game_start_state.h"
#include "game/game_unload_map_state.h"

namespace
{
    class CApplication : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CApplication)
        
    public:
        
        CApplication();
       ~CApplication();
        
    public:
        
        void OnStart(int _Width, int _Height);
        void OnExit();
        int OnRun();
        
    private:
        
        static Game::CState* s_pStates[Game::CState::NumberOfStateTypes];
        
    private:
        
        Game::CState::EStateType m_CurrentState;
        
    private:
        
        void OnTranslation(Game::CState::EStateType _NewState);
        
    };
} // namespace

namespace
{
    Game::CState* CApplication::s_pStates[] =
    {
        &Game::CStartState    ::GetInstance(),
        &Game::CIntroState    ::GetInstance(),
        &Game::CLoadMapState  ::GetInstance(),
        &Game::CMainMenuState ::GetInstance(),
        &Game::CPlayState     ::GetInstance(),
        &Game::CDebugState    ::GetInstance(),
        &Game::CUnloadMapState::GetInstance(),
        &Game::CExitState     ::GetInstance(),
    };
} // namespace

namespace
{
    CApplication::CApplication()
        : m_CurrentState   (Game::CState::Start)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CApplication::~CApplication()
    { }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnStart(int _Width, int _Height)
    {        
        Game::CStartState::GetInstance().SetResolution(_Width, _Height);
        
        s_pStates[m_CurrentState]->OnEnter();

        App::Application::OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
        App::Application::OnExit();

        OnTranslation(Game::CState::UnloadMap);
        
        s_pStates[m_CurrentState]->OnRun();
        
        s_pStates[m_CurrentState]->OnLeave();
        
        OnTranslation(Game::CState::Exit);
        
        s_pStates[m_CurrentState]->OnRun();
        
        s_pStates[m_CurrentState]->OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int CApplication::OnRun()
    {
        App::Application::Update();

        Game::CState::EStateType NextState;
        
        NextState = s_pStates[m_CurrentState]->OnRun();
        
        if (NextState != m_CurrentState)
        {
            OnTranslation(NextState);
        }

        return 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnTranslation(Game::CState::EStateType _NewState)
    {
        s_pStates[m_CurrentState]->OnLeave();
        
        s_pStates[_NewState]->OnEnter();
        
        m_CurrentState = _NewState;
    }
}

namespace Game
{
namespace Runtime
{
    void OnStart(int _Width, int _Height)
    {
        CApplication::GetInstance().OnStart(_Width, _Height);
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CApplication::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnRun()
    {
        return CApplication::GetInstance().OnRun();
    }
} // namespace Runtime
} // namespace App
