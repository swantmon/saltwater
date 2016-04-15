
#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_time.h"
#include "core/core_config.h"

#include "editor/edit_runtime.h"
#include "editor/edit_edit_state.h"
#include "editor/edit_exit_state.h"
#include "editor/edit_intro_state.h"
#include "editor/edit_load_map_state.h"
#include "editor/edit_play_state.h"
#include "editor/edit_start_state.h"
#include "editor/edit_unload_map_state.h"

#include "editor_gui/edit_gui.h"

#include "graphic/gfx_application_interface.h"

namespace
{
    class CApplication : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CApplication)
        
    public:
        
        CApplication();
       ~CApplication();
        
    public:
        
        void OnStart(int& _rArgc, char** _ppArgv);
        void OnExit();
        void OnRun();

        unsigned int GetEditWindowID();
        
    private:
        
        static Edit::CState* s_pStates[Edit::CState::NumberOfStateTypes];
        
    private:
        
        Edit::CState::EStateType m_CurrentState;
        unsigned int             m_EditWindowID;
        
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
        : m_CurrentState(Edit::CState::Start)
        , m_EditWindowID(0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CApplication::~CApplication()
    { }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnStart(int& _rArgc, char** _ppArgv)
    {   
        // -----------------------------------------------------------------------------
        // Initialize GUI.
        // -----------------------------------------------------------------------------
        Edit::GUI::Create(_rArgc, _ppArgv);

        // -----------------------------------------------------------------------------
        // Setup main window with some properties.
        // -----------------------------------------------------------------------------
        Edit::GUI::Setup(1280, 720);

        // -----------------------------------------------------------------------------
        // Show main window.
        // -----------------------------------------------------------------------------
        Edit::GUI::Show();

        // -----------------------------------------------------------------------------
        // Now we get the information of the window handle and set this to
        // the graphic part and active this window.
        // -----------------------------------------------------------------------------
        m_EditWindowID = Gfx::App::RegisterWindow(Edit::GUI::GetEditorWindowHandle());

        Gfx::App::ActivateWindow(m_EditWindowID);

        Gfx::App::OnResize(m_EditWindowID, 1280, 720);

        // -----------------------------------------------------------------------------
        // From now on we can start the state engine and enter the first state
        // -----------------------------------------------------------------------------
        s_pStates[m_CurrentState]->OnEnter();

        Core::Time::OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
        // -----------------------------------------------------------------------------
        // Exit the application
        // -----------------------------------------------------------------------------
        Core::Time::OnExit();

        // -----------------------------------------------------------------------------
        // Make last transition to exit
        // -----------------------------------------------------------------------------
        OnTranslation(Edit::CState::UnloadMap);
        
        s_pStates[m_CurrentState]->OnRun();
        
        s_pStates[m_CurrentState]->OnLeave();
        
        OnTranslation(Edit::CState::Exit);
        
        s_pStates[m_CurrentState]->OnRun();
        
        s_pStates[m_CurrentState]->OnLeave();

        // -----------------------------------------------------------------------------
        // At the end we have to clean our context and windows.
        // -----------------------------------------------------------------------------
        Edit::GUI::Destroy();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnRun()
    {
        // -----------------------------------------------------------------------------
        // With an window and context we initialize our application and run our game.
        // Furthermore we handle different events by the window.
        // -----------------------------------------------------------------------------
        int ApplicationMessage = 0;
        int WindowMessage      = 0;

        for (; ApplicationMessage == 0 && WindowMessage == 0; )
        {
            // -----------------------------------------------------------------------------
            // Events
            // -----------------------------------------------------------------------------
            Edit::GUI::ProcessEvents();

            // -----------------------------------------------------------------------------
            // Time
            // -----------------------------------------------------------------------------
            Core::Time::Update();

            // -----------------------------------------------------------------------------
            // State engine
            // -----------------------------------------------------------------------------
            Edit::CState::EStateType NextState;

            NextState = s_pStates[m_CurrentState]->OnRun();

            if (NextState != m_CurrentState)
            {
                OnTranslation(NextState);
            }
        }
    }

    // -----------------------------------------------------------------------------

    unsigned int CApplication::GetEditWindowID()
    {
        return m_EditWindowID;
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
    void OnStart(int& _rArgc, char** _ppArgv)
    {
        CApplication::GetInstance().OnStart(_rArgc, _ppArgv);
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CApplication::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CApplication::GetInstance().OnRun();
    }

    // -----------------------------------------------------------------------------

    unsigned int GetEditWindowID()
    {
        return CApplication::GetInstance().GetEditWindowID();
    }
} // namespace Runtime
} // namespace Edit
