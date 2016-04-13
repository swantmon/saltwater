
#include "app/app_application.h"
#include "app/app_debug_state.h"
#include "app/app_exit_state.h"
#include "app/app_intro_state.h"
#include "app/app_load_map_state.h"
#include "app/app_main_menu_state.h"
#include "app/app_play_state.h"
#include "app/app_start_state.h"
#include "app/app_unload_map_state.h"

#include "base/base_clock.h"
#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "gui/gui_event_handler.h"

#include <vector>

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
        
        void OnInputEvent(Base::CInputEvent& _rInputEvent);
        
        void OnResize(int _Width, int _Height);
        
        void RegisterResizeHandler(App::Application::CResizeDelegate _NewDelgate);
        
        double GetDeltaTimeLastFrame();
        
        Base::U64 GetNumberOfFrame() const;
        
    private:
        
        static App::CState* s_pStates[App::CState::NumberOfStateTypes];
        
    private:
        
        typedef std::vector<App::Application::CResizeDelegate> CResizeDelagates;
        typedef CResizeDelagates::iterator                     CResizeDelegateIterator;
        
    private:
        
        App::CState::EStateType m_CurrentState;
        CResizeDelagates        m_ResizeDelegates;
        Base::CDefaultClock     m_Clock;
        Base::U64               m_NumberOfFrame;
        
    private:
        
        void OnTranslation(App::CState::EStateType _NewState);
        
    };
} // namespace

namespace
{
    App::CState* CApplication::s_pStates[] =
    {
        &App::CStartState    ::GetInstance(),
        &App::CIntroState    ::GetInstance(),
        &App::CLoadMapState  ::GetInstance(),
        &App::CMainMenuState ::GetInstance(),
        &App::CPlayState     ::GetInstance(),
        &App::CDebugState    ::GetInstance(),
        &App::CUnloadMapState::GetInstance(),
        &App::CExitState     ::GetInstance(),
    };
} // namespace

namespace
{
    CApplication::CApplication()
        : m_CurrentState   (App::CState::Start)
        , m_ResizeDelegates()
        , m_Clock          ()
        , m_NumberOfFrame  (0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CApplication::~CApplication()
    { }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnStart(int _Width, int _Height)
    {
        BASE_CONSOLE_STREAMINFO("App> Start application");
        
        App::CStartState::GetInstance().SetResolution(_Width, _Height);
        
        s_pStates[m_CurrentState]->OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
        OnTranslation(App::CState::UnloadMap);
        
        s_pStates[m_CurrentState]->OnRun();
        
        s_pStates[m_CurrentState]->OnLeave();
        
        OnTranslation(App::CState::Exit);
        
        s_pStates[m_CurrentState]->OnRun();
        
        s_pStates[m_CurrentState]->OnLeave();

        BASE_CONSOLE_STREAMINFO("App> Exit application");
    }
    
    // -----------------------------------------------------------------------------
    
    int CApplication::OnRun()
    {
        m_Clock.OnFrame();
        
        App::CState::EStateType NextState;
        
        NextState = s_pStates[m_CurrentState]->OnRun();
        
        ++ m_NumberOfFrame;
        
        if (NextState != m_CurrentState)
        {
            OnTranslation(NextState);
        }

        return 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnInputEvent(Base::CInputEvent& _rInputEvent)
    {
        Gui::EventHandler::OnUserEvent(_rInputEvent);
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnResize(int _Width, int _Height)
    {
        CResizeDelegateIterator EndOfDelagates = m_ResizeDelegates.end();
        
        for (CResizeDelegateIterator CurrentDelagate = m_ResizeDelegates.begin(); CurrentDelagate != EndOfDelagates; ++CurrentDelagate)
        {
            (*CurrentDelagate)(_Width, _Height);
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::RegisterResizeHandler(App::Application::CResizeDelegate _NewDelgate)
    {
        m_ResizeDelegates.push_back(_NewDelgate);
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnTranslation(App::CState::EStateType _NewState)
    {
        s_pStates[m_CurrentState]->OnLeave();
        
        s_pStates[_NewState]->OnEnter();
        
        m_CurrentState = _NewState;
    }
    
    // -----------------------------------------------------------------------------
    
    double CApplication::GetDeltaTimeLastFrame()
    {
        return m_Clock.GetDurationOfFrame();
    }
    
    // -----------------------------------------------------------------------------
    
    Base::U64 CApplication::GetNumberOfFrame() const
    {
        return m_NumberOfFrame;
    }
}

namespace App
{
namespace Application
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
    
    // -----------------------------------------------------------------------------
    
    void OnInputEvent(Base::CInputEvent& _rInputEvent)
    {
        CApplication::GetInstance().OnInputEvent(_rInputEvent);
    }
    
    // -----------------------------------------------------------------------------
    
    void OnResize(int _Width, int _Height)
    {
        CApplication::GetInstance().OnResize(_Width, _Height);
    }
    
    // -----------------------------------------------------------------------------
    
    void RegisterResizeHandler(CResizeDelegate _NewDelgate)
    {
        CApplication::GetInstance().RegisterResizeHandler(_NewDelgate);
    }
    
    // -----------------------------------------------------------------------------
    
    double GetDeltaTimeLastFrame()
    {
        return CApplication::GetInstance().GetDeltaTimeLastFrame();
    }
    
    // -----------------------------------------------------------------------------
    
    Base::U64 GetNumberOfFrame()
    {
        return CApplication::GetInstance().GetNumberOfFrame();
    }
} // namespace Application
} // namespace App
