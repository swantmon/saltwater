
#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_time.h"
#include "core/core_config.h"

#include "editor/edit_application.h"
#include "editor/edit_edit_state.h"
#include "editor/edit_exit_state.h"
#include "editor/edit_intro_state.h"
#include "editor/edit_load_map_state.h"
#include "editor/edit_play_state.h"
#include "editor/edit_start_state.h"
#include "editor/edit_unload_map_state.h"

#include "editor_gui/edit_gui.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

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

        void OnKeyPressed(Edit::CMessage& _rMessage);
        
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
    {
    }
    
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

        // -----------------------------------------------------------------------------
        // Register messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::KeyPressed, EDIT_RECEIVE_MESSAGE(&CApplication::OnKeyPressed));
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
        // -----------------------------------------------------------------------------
        // Exit the application
        // -----------------------------------------------------------------------------
        Core::Time::OnExit();

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
        for (;;)
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

            if (NextState == Edit::CState::Exit)
            {
                Edit::CExitState::GetInstance().OnRun();

                Edit::CExitState::GetInstance().OnLeave();

                break;
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

    // -----------------------------------------------------------------------------

    void CApplication::OnKeyPressed(Edit::CMessage& _rMessage)
    {
        int Test = _rMessage.GetInt();

        BASE_CONSOLE_INFOV("Got key pressed: %i", Test);
    }
}

namespace Edit
{
namespace Application
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
} // namespace Application
} // namespace Edit
