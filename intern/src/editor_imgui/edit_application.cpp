
#include "editor_imgui/edit_precompiled.h"

#include "base/base_exception.h"
#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "editor_imgui/edit_application.h"
#include "editor_imgui/edit_edit_state.h"
#include "editor_imgui/edit_exit_state.h"
#include "editor_imgui/edit_intro_state.h"
#include "editor_imgui/edit_load_map_state.h"
#include "editor_imgui/edit_play_state.h"
#include "editor_imgui/edit_start_state.h"
#include "editor_imgui/edit_unload_map_state.h"
#include "editor_imgui/imgui_impl_opengl.h"
#include "editor_imgui/imgui_impl_sdl.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_pipeline.h"
#include "engine/graphic/gfx_performance.h"

#include "engine/gui/gui_event_handler.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"

#include "imgui.h"

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
        SDL_Window* m_pWindow;
        int m_AnalogStickDeadZone;
        
        bool m_EnableGamepad;

    private:
        
        void OnTranslation(Edit::CState::EStateType _NewState);

        void ProcessSDLEvents();
        void ProcessWindowEvents(const SDL_Event& _rSDLEvent);
        void ProcessKeyboardEvents(const SDL_Event& _rSDLEvent);
        void ProcessMouseEvents(const SDL_Event& _rSDLEvent);
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
        BASE_UNUSED(_rArgc);
        BASE_UNUSED(_ppArgv);

        // -----------------------------------------------------------------------------
        // Setup asset manager
        // -----------------------------------------------------------------------------
        Core::AssetManager::SetFilePath("..");

        // -----------------------------------------------------------------------------
        // Initialize GUI.
        // -----------------------------------------------------------------------------
        SDL_Init(SDL_INIT_VIDEO);

        m_pWindow = SDL_CreateWindow("Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        if (m_pWindow == NULL) 
        {
            BASE_THROWM("Could not initialize SDL window.");
        }

        // -----------------------------------------------------------------------------
        // Now we get the information of the window handle and set this to
        // the graphic part and active this window.
        // -----------------------------------------------------------------------------
        SDL_SysWMinfo WMinfo;

        SDL_VERSION(&WMinfo.version);

        SDL_GetWindowWMInfo(m_pWindow, &WMinfo);

        HWND NativeHWND = WMinfo.info.win.window;

        int VSync = Core::CProgramParameters::GetInstance().Get("graphics:vsync_interval", 0);

        m_EditWindowID = Gfx::Pipeline::RegisterWindow(NativeHWND, VSync);

        Gfx::Pipeline::ActivateWindow(m_EditWindowID);

        // -----------------------------------------------------------------------------
        // IMGUI
        // -----------------------------------------------------------------------------
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        ImGui_ImplSDL2_InitForOpenGL(m_pWindow, 0);

        ImGui_ImplOpenGL3_Init();

        auto RenderUI = [&]()
        {
            Gfx::Performance::BeginEvent("IMGUI");

            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            Gfx::Performance::EndEvent();
        };

        Engine::RegisterEventHandler(Engine::Gfx_OnRenderUI, RenderUI);

        // -----------------------------------------------------------------------------
        // From now on we can start the state engine and enter the first state
        // -----------------------------------------------------------------------------
        s_pStates[m_CurrentState]->OnEnter();

        // -----------------------------------------------------------------------------
        // Start engine
        // -----------------------------------------------------------------------------
        Engine::Startup();

        // -----------------------------------------------------------------------------
        // Init SDL for gamepad input
        // -----------------------------------------------------------------------------
        m_EnableGamepad = Core::CProgramParameters::GetInstance().Get("input:gamepad:enable", true);

        if (m_EnableGamepad)
        {
            m_AnalogStickDeadZone = Core::CProgramParameters::GetInstance().Get("input:gamepad:deadzone", 3200);

            if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
            {
                BASE_THROWM("Could not initialise SDL.");
            }

            if (SDL_NumJoysticks() > 1)
            {
                ENGINE_CONSOLE_INFOV("Found %i gamepads. Initialize first pad.", SDL_NumJoysticks());

                SDL_JoystickEventState(SDL_ENABLE);
                m_pGamePad = SDL_JoystickOpen(0);

                if (m_pGamePad == nullptr)
                {
                    BASE_THROWM("Could not initialise controller.");
                }

                ENGINE_CONSOLE_INFOV(SDL_JoystickName(m_pGamePad));
            }
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
        ImGui_ImplSDL2_Shutdown();

        ImGui_ImplOpenGL3_Shutdown();

        ImGui::DestroyContext();

        SDL_DestroyWindow(m_pWindow);

        SDL_JoystickClose(m_pGamePad);

        SDL_Quit();

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
            // Events
            // -----------------------------------------------------------------------------
            ProcessSDLEvents();

            // -----------------------------------------------------------------------------
            // Update GUI
            // -----------------------------------------------------------------------------
            ImGui_ImplOpenGL3_NewFrame();

            ImGui_ImplSDL2_NewFrame(m_pWindow);

            ImGui::NewFrame();

            bool open = true;

            ImGui::ShowDemoWindow(&open);

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
    
    // -----------------------------------------------------------------------------

    void CApplication::ProcessSDLEvents()
    {
        SDL_Event SDLEvent;

        while (SDL_PollEvent(&SDLEvent))
        {
            // -----------------------------------------------------------------------------
            // IMGUI
            // -----------------------------------------------------------------------------
            ImGui_ImplSDL2_ProcessEvent(&SDLEvent);

            // -----------------------------------------------------------------------------
            // Engine
            // -----------------------------------------------------------------------------
            ProcessWindowEvents(SDLEvent);
            ProcessKeyboardEvents(SDLEvent);
            ProcessMouseEvents(SDLEvent);
            ProcessGamepadEvents(SDLEvent);
        }
    }

    // -----------------------------------------------------------------------------

    void CApplication::ProcessWindowEvents(const SDL_Event& _rSDLEvent)
    {
        using Base::CInputEvent;

        CInputEvent Event(CInputEvent::Input);

        switch (_rSDLEvent.window.event)
        {
        case SDL_WINDOWEVENT_CLOSE:
            Event = Base::CInputEvent(Base::CInputEvent::Exit);

            Gui::EventHandler::OnUserEvent(Event);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            int WindowWidth;
            int WindowHeight;

            SDL_GetWindowSize(m_pWindow, &WindowWidth, &WindowHeight);

            Gfx::Pipeline::OnResize(m_EditWindowID, WindowWidth, WindowHeight);
            break;
        }
    }

    // -----------------------------------------------------------------------------

    void CApplication::ProcessKeyboardEvents(const SDL_Event& _rSDLEvent)
    {
        using Base::CInputEvent;

        CInputEvent Event(CInputEvent::Input);
        unsigned int Key;
        unsigned int Mod;

        switch (_rSDLEvent.type)
        {
        case SDL_KEYDOWN:
            Key = _rSDLEvent.key.keysym.sym;
            Mod = _rSDLEvent.key.keysym.mod;

            Event = CInputEvent(Base::CInputEvent::Input, Base::CInputEvent::KeyPressed, Key, Mod);

            Gui::EventHandler::OnUserEvent(Event);
            break;
        case SDL_KEYUP:
            Key = _rSDLEvent.key.keysym.sym;
            Mod = _rSDLEvent.key.keysym.mod;

            Event = CInputEvent(Base::CInputEvent::Input, Base::CInputEvent::KeyReleased, Key, Mod);

            Gui::EventHandler::OnUserEvent(Event);
            break;
        }
    }

    // -----------------------------------------------------------------------------

    void CApplication::ProcessMouseEvents(const SDL_Event& _rSDLEvent)
    {
        using Base::CInputEvent;

        CInputEvent Event(CInputEvent::Input);

        Base::CInputEvent::EAction MouseAction = Base::CInputEvent::UndefinedAction;
        int WheelData;

        switch (_rSDLEvent.type)
        {
        case SDL_MOUSEMOTION:
            int MousePositionX;
            int MousePositionY;

            SDL_GetMouseState(&MousePositionX, &MousePositionY);

            m_LatestMousePosition[0] = MousePositionX;
            m_LatestMousePosition[1] = MousePositionY;

            Event = CInputEvent(CInputEvent::Input, CInputEvent::MouseMove, CInputEvent::Mouse, m_LatestMousePosition, m_LatestMousePosition);

            Gui::EventHandler::OnUserEvent(Event);
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (_rSDLEvent.button.button == SDL_BUTTON_LEFT)
            {
                MouseAction = Base::CInputEvent::MouseLeftPressed;
            }
            else if (_rSDLEvent.button.button == SDL_BUTTON_MIDDLE)
            {
                MouseAction = Base::CInputEvent::MouseMiddlePressed;
            }
            else if (_rSDLEvent.button.button == SDL_BUTTON_RIGHT)
            {
                MouseAction = CInputEvent::MouseRightPressed;
            }

            if (MouseAction != CInputEvent::UndefinedAction)
            {
                Event = CInputEvent(CInputEvent::Input, MouseAction, CInputEvent::Mouse, m_LatestMousePosition, m_LatestMousePosition);

                Gui::EventHandler::OnUserEvent(Event);
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (_rSDLEvent.button.button == SDL_BUTTON_LEFT)
            {
                MouseAction = CInputEvent::MouseLeftReleased;
            }
            else if (_rSDLEvent.button.button == SDL_BUTTON_MIDDLE)
            {
                MouseAction = CInputEvent::MouseMiddleReleased;
            }
            else if (_rSDLEvent.button.button == SDL_BUTTON_RIGHT)
            {
                MouseAction = CInputEvent::MouseRightReleased;
            }

            if (MouseAction != Base::CInputEvent::UndefinedAction)
            {
                Event = CInputEvent(CInputEvent::Input, MouseAction, CInputEvent::Mouse, m_LatestMousePosition, m_LatestMousePosition);

                Gui::EventHandler::OnUserEvent(Event);
            }
            break;
        case SDL_MOUSEWHEEL:
            WheelData = _rSDLEvent.wheel.y;

            Event = CInputEvent(CInputEvent::Input, CInputEvent::MouseWheel, CInputEvent::Mouse, m_LatestMousePosition, WheelData);

            Gui::EventHandler::OnUserEvent(Event);
            break;
        }
    }

    // -----------------------------------------------------------------------------

    void CApplication::ProcessGamepadEvents(const SDL_Event& _rSDLEvent)
    {
        using Base::CInputEvent;
        
        CInputEvent Event(CInputEvent::Input);

        switch (_rSDLEvent.type)
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

        case SDL_JOYAXISMOTION:

            if (_rSDLEvent.jaxis.value > m_AnalogStickDeadZone || _rSDLEvent.jaxis.value < -m_AnalogStickDeadZone)
            {
                float Strength = _rSDLEvent.jaxis.value / static_cast<float>(INT16_MAX);
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
