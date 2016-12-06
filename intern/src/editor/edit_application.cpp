
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_time.h"
#include "core/core_config.h"

#include "editor/edit_actor_helper.h"
#include "editor/edit_application.h"
#include "editor/edit_edit_state.h"
#include "editor/edit_effect_helper.h"
#include "editor/edit_entity_helper.h"
#include "editor/edit_exit_state.h"
#include "editor/edit_graphic_helper.h"
#include "editor/edit_intro_state.h"
#include "editor/edit_light_helper.h"
#include "editor/edit_load_map_state.h"
#include "editor/edit_map_helper.h"
#include "editor/edit_material_helper.h"
#include "editor/edit_play_state.h"
#include "editor/edit_plugin_helper.h"
#include "editor/edit_start_state.h"
#include "editor/edit_texture_helper.h"
#include "editor/edit_unload_map_state.h"

#include "editor_gui/edit_gui.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include "graphic/gfx_application_interface.h"

#include "gui/gui_event_handler.h"

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
        Base::Short2             m_LatestMousePosition;
        
    private:
        
        void OnTranslation(Edit::CState::EStateType _NewState);

        void OnKeyPressed(Edit::CMessage& _rMessage);
        void OnKeyReleased(Edit::CMessage& _rMessage);
        void OnMouseLeftPressed(Edit::CMessage& _rMessage);
        void OnMouseLeftReleased(Edit::CMessage& _rMessage);
        void OnMouseMiddlePressed(Edit::CMessage& _rMessage);
        void OnMouseMiddleReleased(Edit::CMessage& _rMessage);
        void OnMouseRightPressed(Edit::CMessage& _rMessage);
        void OnMouseRightReleased(Edit::CMessage& _rMessage);
        void OnMouseMove(Edit::CMessage& _rMessage);
        
        void OnTakeScreenshot(Edit::CMessage& _rMessage);
        void OnResize(Edit::CMessage& _rMessage);
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
        : m_CurrentState       (Edit::CState::Start)
        , m_EditWindowID       (0)
        , m_LatestMousePosition(Base::Short2(0, 0))
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
        Edit::MessageManager::Register(Edit::SGUIMessageType::KeyPressed         , EDIT_RECEIVE_MESSAGE(&CApplication::OnKeyPressed));
        Edit::MessageManager::Register(Edit::SGUIMessageType::KeyReleased        , EDIT_RECEIVE_MESSAGE(&CApplication::OnKeyReleased));
        Edit::MessageManager::Register(Edit::SGUIMessageType::MouseLeftPressed   , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseLeftPressed));
        Edit::MessageManager::Register(Edit::SGUIMessageType::MouseLeftReleased  , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseLeftReleased));
        Edit::MessageManager::Register(Edit::SGUIMessageType::MouseMiddlePressed , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseMiddlePressed));
        Edit::MessageManager::Register(Edit::SGUIMessageType::MouseMiddleReleased, EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseMiddleReleased));
        Edit::MessageManager::Register(Edit::SGUIMessageType::MouseRightPressed  , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseRightPressed));
        Edit::MessageManager::Register(Edit::SGUIMessageType::MouseRightReleased , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseRightReleased));
        Edit::MessageManager::Register(Edit::SGUIMessageType::MouseMove          , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseMove));
        Edit::MessageManager::Register(Edit::SGUIMessageType::TakeScreenshot     , EDIT_RECEIVE_MESSAGE(&CApplication::OnTakeScreenshot));
        Edit::MessageManager::Register(Edit::SGUIMessageType::ResizeMapEditWindow, EDIT_RECEIVE_MESSAGE(&CApplication::OnResize));

        // -----------------------------------------------------------------------------
        // Helper
        // TODO: some tasks should be done inside the states
        // -----------------------------------------------------------------------------
        Edit::Helper::Actor   ::OnStart();
        Edit::Helper::Effect  ::OnStart();
        Edit::Helper::Entity  ::OnStart();
        Edit::Helper::Graphic ::OnStart();
        Edit::Helper::Light   ::OnStart();
        Edit::Helper::Map     ::OnStart();
        Edit::Helper::Material::OnStart();
        Edit::Helper::Plugin  ::OnStart();
        Edit::Helper::Texture ::OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
        // -----------------------------------------------------------------------------
        // Exit the application
        // -----------------------------------------------------------------------------
        Core::Time::OnExit();

        // -----------------------------------------------------------------------------
        // Helper
        // -----------------------------------------------------------------------------
        Edit::Helper::Actor   ::OnExit();
        Edit::Helper::Effect  ::OnExit();
        Edit::Helper::Entity  ::OnExit();
        Edit::Helper::Graphic ::OnExit();
        Edit::Helper::Light   ::OnExit();
        Edit::Helper::Map     ::OnExit();
        Edit::Helper::Material::OnExit();
        Edit::Helper::Plugin  ::OnExit();
        Edit::Helper::Texture ::OnExit();

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
            // Send FPS to editor
            // -----------------------------------------------------------------------------
            Edit::CMessage NewMessage;

            NewMessage.PutDouble(Core::Time::GetDeltaTimeLastFrame());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::FramesPerSecond, NewMessage);

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
        unsigned int Key = _rMessage.GetInt();
        unsigned int Mod = _rMessage.GetInt();

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::KeyPressed, Key, Mod);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnKeyReleased(Edit::CMessage& _rMessage)
    {
        unsigned int Key = _rMessage.GetInt();
        unsigned int Mod = _rMessage.GetInt();

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::KeyReleased, Key, Mod);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseLeftPressed(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.GetInt();
        int MousePositionY = _rMessage.GetInt();

        m_LatestMousePosition[0] = MousePositionX;
        m_LatestMousePosition[1] = MousePositionY;

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseLeftPressed, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseLeftReleased(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.GetInt();
        int MousePositionY = _rMessage.GetInt();

        m_LatestMousePosition[0] = MousePositionX;
        m_LatestMousePosition[1] = MousePositionY;

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseLeftReleased, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseMiddlePressed(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.GetInt();
        int MousePositionY = _rMessage.GetInt();

        m_LatestMousePosition[0] = MousePositionX;
        m_LatestMousePosition[1] = MousePositionY;

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseMiddlePressed, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseMiddleReleased(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.GetInt();
        int MousePositionY = _rMessage.GetInt();

        m_LatestMousePosition[0] = MousePositionX;
        m_LatestMousePosition[1] = MousePositionY;

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseMiddleReleased, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseRightPressed(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.GetInt();
        int MousePositionY = _rMessage.GetInt();

        m_LatestMousePosition[0] = MousePositionX;
        m_LatestMousePosition[1] = MousePositionY;

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseRightPressed, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseRightReleased(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.GetInt();
        int MousePositionY = _rMessage.GetInt();

        m_LatestMousePosition[0] = MousePositionX;
        m_LatestMousePosition[1] = MousePositionY;

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseRightReleased, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseMove(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.GetInt();
        int MousePositionY = _rMessage.GetInt();

        m_LatestMousePosition[0] = MousePositionX;
        m_LatestMousePosition[1] = MousePositionY;

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseMove, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnTakeScreenshot(Edit::CMessage& _rMessage)
    {
        char pPathToSave[256];

        _rMessage.GetString(pPathToSave, 256);

        Gfx::App::TakeScreenshot(m_EditWindowID, pPathToSave);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnResize(Edit::CMessage& _rMessage)
    {
        int Width  = _rMessage.GetInt();
        int Height = _rMessage.GetInt();

        Gfx::App::OnResize(m_EditWindowID, 1280, 720);
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
