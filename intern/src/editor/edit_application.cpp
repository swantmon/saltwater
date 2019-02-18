
#include "editor/edit_precompiled.h"

#include "base/base_exception.h"
#include "base/base_input_event.h"
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
#include "editor/edit_gui.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_pipeline.h"

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
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnStart(int& _rArgc, char** _ppArgv)
    {
        // -----------------------------------------------------------------------------
        // Setup asset manager
        // -----------------------------------------------------------------------------
        Core::AssetManager::SetFilePath("..");

        // -----------------------------------------------------------------------------
        // GUI
        // -----------------------------------------------------------------------------
        Edit::GUI::Create(_rArgc, _ppArgv);

        // -----------------------------------------------------------------------------
        // Engine
        // -----------------------------------------------------------------------------
        int VSync = Core::CProgramParameters::GetInstance().Get("graphics:vsync_interval", 0);

        m_EditWindowID = Gfx::Pipeline::RegisterWindow(Edit::GUI::GetEditorWindowHandle(), VSync);

        Gfx::Pipeline::ActivateWindow(m_EditWindowID);

        // -----------------------------------------------------------------------------
        // From now on we can start the state engine and enter the first state
        // -----------------------------------------------------------------------------
        s_pStates[m_CurrentState]->OnEnter();

        // -----------------------------------------------------------------------------
        // Start engine
        // -----------------------------------------------------------------------------
        Engine::Startup();

        // -----------------------------------------------------------------------------
        // Show GUI
        // -----------------------------------------------------------------------------

        auto WindowSize = Core::CProgramParameters::GetInstance().Get("application:window_size", glm::ivec2(1280, 720));
        Edit::GUI::Setup(m_EditWindowID, WindowSize.x, WindowSize.y);

        Edit::GUI::Show();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
        if (m_CurrentState == Edit::CState::Edit || m_CurrentState == Edit::CState::Play)
        {
            s_pStates[m_CurrentState]->OnLeave();

            Edit::CUnloadMapState::GetInstance().OnEnter();

            Edit::CUnloadMapState::GetInstance().OnRun();

            Edit::CUnloadMapState::GetInstance().OnLeave();
        }

        if (m_CurrentState != Edit::CState::Exit)
        {
            s_pStates[m_CurrentState]->OnLeave();

            Edit::CExitState::GetInstance().OnEnter();

            Edit::CExitState::GetInstance().OnRun();

            Edit::CExitState::GetInstance().OnLeave();
        }

        // -----------------------------------------------------------------------------
        // GUI
        // -----------------------------------------------------------------------------
        Edit::GUI::Destroy();

        // -----------------------------------------------------------------------------
        // Start engine
        // -----------------------------------------------------------------------------
        Engine::Shutdown();
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
            // GUI
            // -----------------------------------------------------------------------------
            Edit::GUI::ProcessEvents();

            // -----------------------------------------------------------------------------
            // State engine
            // -----------------------------------------------------------------------------
            Edit::CState::EStateType NextState;

            NextState = s_pStates[m_CurrentState]->OnRun();

            // -----------------------------------------------------------------------------
            // Update engine
            // -----------------------------------------------------------------------------
            Engine::Update();

            // -----------------------------------------------------------------------------
            // Check state
            // -----------------------------------------------------------------------------
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
