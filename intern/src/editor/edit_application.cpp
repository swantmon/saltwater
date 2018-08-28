
#include "editor/edit_precompiled.h"

#include "base/base_exception.h"
#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

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
#include "editor/edit_material_helper.h"
#include "editor/edit_play_state.h"
#include "editor/edit_plugin_helper.h"
#include "editor/edit_start_state.h"
#include "editor/edit_unload_map_state.h"

#include "editor_gui/edit_gui.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_pipeline.h"

#include "engine/gui/gui_event_handler.h"

#include "SDL2/SDL.h"

#include <iostream>

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
        glm::vec2                m_LatestMousePosition;

        SDL_Joystick* m_pGamePad;
        int m_AnalogStickDeadZone;
        
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
        void OnWheel(Edit::CMessage& _rMessage);
        
        void OnResize(Edit::CMessage& _rMessage);

        void ProcessSDLEvents();
        void ProcessGamepadEvents(const SDL_Event& _rSDLEvent);

        Base::CInputEvent::EKey ConvertSDLKey(const SDL_Event& _rSDLEvent);
        Base::CInputEvent::EKey ConvertSDLAxis(const SDL_Event& _rSDLEvent);
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
        , m_LatestMousePosition(glm::vec2(0, 0))
        , m_pGamePad           (nullptr)
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
        // Init SDL for gamepad input
        // -----------------------------------------------------------------------------
        m_AnalogStickDeadZone = Core::CProgramParameters::GetInstance().Get("input:gamepad:deadzone", 3200);

        if (Core::CProgramParameters::GetInstance().Get("input:gamepad:enable", true))
        {
            if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
            {
                BASE_THROWM("Could not initialise SDL");
            }

            if (SDL_NumJoysticks() < 1)
            {
                ENGINE_CONSOLE_INFOV("No gamepads found");
            }
            else
            {
                SDL_JoystickEventState(SDL_ENABLE);
                m_pGamePad = SDL_JoystickOpen(0);
                if (m_pGamePad == nullptr)
                {
                    BASE_THROWM("Could not initialise controller");
                }
                ENGINE_CONSOLE_INFOV(SDL_JoystickName(m_pGamePad));
            }
        }

        // -----------------------------------------------------------------------------
        // Setup asset manager
        // -----------------------------------------------------------------------------
        Core::AssetManager::SetFilePath("..");

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
        // Register messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::Input_KeyPressed         , EDIT_RECEIVE_MESSAGE(&CApplication::OnKeyPressed));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Input_KeyReleased        , EDIT_RECEIVE_MESSAGE(&CApplication::OnKeyReleased));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Input_MouseLeftPressed   , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseLeftPressed));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Input_MouseLeftReleased  , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseLeftReleased));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Input_MouseMiddlePressed , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseMiddlePressed));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Input_MouseMiddleReleased, EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseMiddleReleased));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Input_MouseRightPressed  , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseRightPressed));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Input_MouseRightReleased , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseRightReleased));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Input_MouseMove          , EDIT_RECEIVE_MESSAGE(&CApplication::OnMouseMove));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Input_MouseWheel         , EDIT_RECEIVE_MESSAGE(&CApplication::OnWheel));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Window_Resize            , EDIT_RECEIVE_MESSAGE(&CApplication::OnResize));

        // -----------------------------------------------------------------------------
        // Helper
        // TODO: some tasks should be done inside the states
        // -----------------------------------------------------------------------------
        Edit::Helper::Actor   ::OnStart();
        Edit::Helper::Effect  ::OnStart();
        Edit::Helper::Entity  ::OnStart();
        Edit::Helper::Graphic ::OnStart();
        Edit::Helper::Light   ::OnStart();
        Edit::Helper::Material::OnStart();
        Edit::Helper::Plugin  ::OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
        SDL_JoystickClose(m_pGamePad);
        SDL_Quit();

        // -----------------------------------------------------------------------------
        // Helper
        // -----------------------------------------------------------------------------
        Edit::Helper::Actor   ::OnExit();
        Edit::Helper::Effect  ::OnExit();
        Edit::Helper::Entity  ::OnExit();
        Edit::Helper::Graphic ::OnExit();
        Edit::Helper::Light   ::OnExit();
        Edit::Helper::Material::OnExit();
        Edit::Helper::Plugin  ::OnExit();

        // -----------------------------------------------------------------------------
        // Start engine
        // -----------------------------------------------------------------------------
        Engine::Shutdown();

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
            ProcessSDLEvents();

            // -----------------------------------------------------------------------------
            // Send FPS to editor
            // -----------------------------------------------------------------------------
            Edit::CMessage NewMessage;

            NewMessage.Put(Core::Time::GetDeltaTimeLastFrame());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Graphic_FPS_Info, NewMessage);

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
        // -----------------------------------------------------------------------------
        // Do state switch
        // -----------------------------------------------------------------------------
        s_pStates[m_CurrentState]->OnLeave();
        
        s_pStates[_NewState]->OnEnter();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.Put(m_CurrentState);

        NewMessage.Put(_NewState);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::App_State_Change, NewMessage);

        // -----------------------------------------------------------------------------
        // Save new state
        // -----------------------------------------------------------------------------
        m_CurrentState = _NewState;
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnKeyPressed(Edit::CMessage& _rMessage)
    {
        unsigned int Key = _rMessage.Get<int>();
        unsigned int Mod = _rMessage.Get<int>();

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::KeyPressed, Key, Mod);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnKeyReleased(Edit::CMessage& _rMessage)
    {
        unsigned int Key = _rMessage.Get<int>();
        unsigned int Mod = _rMessage.Get<int>();

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::KeyReleased, Key, Mod);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseLeftPressed(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.Get<int>();
        int MousePositionY = _rMessage.Get<int>();

        m_LatestMousePosition[0] = static_cast<float>(MousePositionX);
        m_LatestMousePosition[1] = static_cast<float>(MousePositionY);

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseLeftPressed, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseLeftReleased(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.Get<int>();
        int MousePositionY = _rMessage.Get<int>();

        m_LatestMousePosition[0] = static_cast<float>(MousePositionX);
        m_LatestMousePosition[1] = static_cast<float>(MousePositionY);

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseLeftReleased, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseMiddlePressed(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.Get<int>();
        int MousePositionY = _rMessage.Get<int>();

        m_LatestMousePosition[0] = static_cast<float>(MousePositionX);
        m_LatestMousePosition[1] = static_cast<float>(MousePositionY);

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseMiddlePressed, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseMiddleReleased(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.Get<int>();
        int MousePositionY = _rMessage.Get<int>();

        m_LatestMousePosition[0] = static_cast<float>(MousePositionX);
        m_LatestMousePosition[1] = static_cast<float>(MousePositionY);

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseMiddleReleased, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseRightPressed(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.Get<int>();
        int MousePositionY = _rMessage.Get<int>();

        m_LatestMousePosition[0] = static_cast<float>(MousePositionX);
        m_LatestMousePosition[1] = static_cast<float>(MousePositionY);

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseRightPressed, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseRightReleased(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.Get<int>();
        int MousePositionY = _rMessage.Get<int>();

        m_LatestMousePosition[0] = static_cast<float>(MousePositionX);
        m_LatestMousePosition[1] = static_cast<float>(MousePositionY);

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseRightReleased, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnMouseMove(Edit::CMessage& _rMessage)
    {
        int MousePositionX = _rMessage.Get<int>();
        int MousePositionY = _rMessage.Get<int>();

        m_LatestMousePosition[0] = static_cast<float>(MousePositionX);
        m_LatestMousePosition[1] = static_cast<float>(MousePositionY);

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseMove, Base::CInputEvent::Mouse, m_LatestMousePosition);

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnWheel(Edit::CMessage& _rMessage)
    {
        bool IsVertically = _rMessage.Get<bool>();
        int WheelDelta    = _rMessage.Get<int>();

        (void)IsVertically;

        Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseWheel, Base::CInputEvent::Mouse, m_LatestMousePosition, static_cast<float>(WheelDelta));

        Gui::EventHandler::OnUserEvent(NewInput);
    }

    // -----------------------------------------------------------------------------

    void CApplication::OnResize(Edit::CMessage& _rMessage)
    {
        int Width  = _rMessage.Get<int>();
        int Height = _rMessage.Get<int>();

        Gfx::Pipeline::OnResize(m_EditWindowID, Width, Height);
    }

    // -----------------------------------------------------------------------------

    void CApplication::ProcessSDLEvents()
    {
        SDL_Event SDLEvent;

        while (SDL_PollEvent(&SDLEvent))
        {
            switch (SDLEvent.type)
            {
            case SDL_JOYDEVICEADDED:

                SDL_JoystickEventState(SDL_ENABLE);
                m_pGamePad = SDL_JoystickOpen(0);
                if (m_pGamePad == nullptr)
                {
                    BASE_THROWM("Could not initialise controller");
                }
                ENGINE_CONSOLE_INFOV(SDL_JoystickName(m_pGamePad));
                break;

            case SDL_JOYDEVICEREMOVED:

                ENGINE_CONSOLE_INFO("Gamepad disconnected");
                break;

            default:
                ProcessGamepadEvents(SDLEvent);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CApplication::ProcessGamepadEvents(const SDL_Event& _rSDLEvent)
    {
        using Base::CInputEvent;
        
        CInputEvent Event(CInputEvent::Input);

        switch (_rSDLEvent.type)
        {
        case SDL_JOYAXISMOTION:

            if (_rSDLEvent.jaxis.value > m_AnalogStickDeadZone || _rSDLEvent.jaxis.value < -m_AnalogStickDeadZone)
            {
                float Strength = _rSDLEvent.jaxis.value / static_cast<float>(std::numeric_limits<int16_t>::max());
                CInputEvent::EKey Key = ConvertSDLAxis(_rSDLEvent);
                unsigned int Axis = static_cast<int>(_rSDLEvent.jaxis.axis) % 2;
                
                if (Key == CInputEvent::LeftTrigger || Key == CInputEvent::RightTrigger)
                {
                    Event = CInputEvent(CInputEvent::Input, CInputEvent::GamepadTriggerMotion, Key, Strength * 0.5f + 0.5f);
                }
                else
                {
                    Event = CInputEvent(CInputEvent::Input, CInputEvent::GamepadAxisMotion, Key, Axis, Strength);
                }

                Gui::EventHandler::OnUserEvent(Event);
            }
            break;

        case SDL_JOYBUTTONDOWN:

            Event = CInputEvent(CInputEvent::Input, CInputEvent::GamepadKeyPressed, ConvertSDLKey(_rSDLEvent));
            Gui::EventHandler::OnUserEvent(Event);
            break;

        case SDL_JOYBUTTONUP:

            Event = CInputEvent(CInputEvent::Input, CInputEvent::GamepadKeyReleased, ConvertSDLKey(_rSDLEvent));
            Gui::EventHandler::OnUserEvent(Event);
            break;
        }
    }

    // -----------------------------------------------------------------------------

    Base::CInputEvent::EKey CApplication::ConvertSDLKey(const SDL_Event& _rSDLEvent)
    {
        using Base::CInputEvent;

        CInputEvent::EKey Xbox360Keys[] =
        {
            CInputEvent::Up,
            CInputEvent::Down,
            CInputEvent::Left,
            CInputEvent::Right,
            CInputEvent::Start,
            CInputEvent::Select,
            CInputEvent::LeftStick,
            CInputEvent::RightStick,
            CInputEvent::LeftBumper,
            CInputEvent::RightBumper,
            CInputEvent::Key0, // A
            CInputEvent::Key1, // B
            CInputEvent::Key2, // X
            CInputEvent::Key3, // Y
        };

        return Xbox360Keys[_rSDLEvent.jbutton.button];
    }

    // -----------------------------------------------------------------------------

    Base::CInputEvent::EKey CApplication::ConvertSDLAxis(const SDL_Event& _rSDLEvent)
    {
        using Base::CInputEvent;

        CInputEvent::EKey Xbox360Axis[] =
        {
            CInputEvent::LeftStick,
            CInputEvent::LeftStick,
            CInputEvent::RightStick,
            CInputEvent::RightStick,
            CInputEvent::LeftTrigger,
            CInputEvent::RightTrigger,
        };
        
        return Xbox360Axis[_rSDLEvent.jaxis.axis];
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
