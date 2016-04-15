
#include "app/app_application.h"

#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "editor/edit_application.h"
#include "editor/edit_edit_state.h"
#include "editor/edit_exit_state.h"
#include "editor/edit_intro_state.h"
#include "editor/edit_load_map_state.h"
#include "editor/edit_play_state.h"
#include "editor/edit_start_state.h"
#include "editor/edit_unload_map_state.h"

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
        
        static Edit::CState* s_pStates[Edit::CState::NumberOfStateTypes];
        
    private:
        
        Edit::CState::EStateType m_CurrentState;
        
    private:
        
        void OnTranslation(Edit::CState::EStateType _NewState);
        
    };
} // namespace

namespace
{
    Edit::CState* CApplication::s_pStates[] =
    {
        &Edit::CStartState    ::GetInstance(),
        &Edit::CIntroState    ::GetInstance(),
        &Edit::CLoadMapState  ::GetInstance(),
        &Edit::CEditState     ::GetInstance(),
        &Edit::CPlayState     ::GetInstance(),
        &Edit::CUnloadMapState::GetInstance(),
        &Edit::CExitState     ::GetInstance(),
    };
} // namespace

namespace
{
    CApplication::CApplication()
        : m_CurrentState   (Edit::CState::Start)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CApplication::~CApplication()
    { }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnStart(int _Width, int _Height)
    {        
        Edit::CStartState::GetInstance().SetResolution(_Width, _Height);
        
        s_pStates[m_CurrentState]->OnEnter();

        App::Application::OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
        App::Application::OnExit();

        OnTranslation(Edit::CState::UnloadMap);
        
        s_pStates[m_CurrentState]->OnRun();
        
        s_pStates[m_CurrentState]->OnLeave();
        
        OnTranslation(Edit::CState::Exit);
        
        s_pStates[m_CurrentState]->OnRun();
        
        s_pStates[m_CurrentState]->OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int CApplication::OnRun()
    {
        App::Application::Update();

        Edit::CState::EStateType NextState;
        
        NextState = s_pStates[m_CurrentState]->OnRun();
        
        if (NextState != m_CurrentState)
        {
            OnTranslation(NextState);
        }

        return 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnTranslation(Edit::CState::EStateType _NewState)
    {
        s_pStates[m_CurrentState]->OnLeave();
        
        s_pStates[_NewState]->OnEnter();
        
        m_CurrentState = _NewState;
    }
}

namespace Edit
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
