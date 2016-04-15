
#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_time.h"
#include "core/core_config.h"

#include "game/game_runtime.h"
#include "game/game_debug_state.h"
#include "game/game_exit_state.h"
#include "game/game_intro_state.h"
#include "game/game_load_map_state.h"
#include "game/game_main_menu_state.h"
#include "game/game_play_state.h"
#include "game/game_start_state.h"
#include "game/game_unload_map_state.h"

#include "graphic/gfx_application_interface.h"

#include "gui/gui_event_handler.h"

#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL.h>


#define APP_NAME       "Game"
#define APP_POSITION_X SDL_WINDOWPOS_UNDEFINED
#define APP_POSITION_Y SDL_WINDOWPOS_UNDEFINED
#define APP_STYLE      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE


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
        void OnRun();
        
    private:
        
        static Game::CState* s_pStates[Game::CState::NumberOfStateTypes];
        
    private:
        
        Game::CState::EStateType m_CurrentState;
        Base::Short2             m_LatestMousePosition;
        SDL_Window*              m_pWindow;
        unsigned int             m_GameWindowID;
        
    private:
        
        void OnTranslation(Game::CState::EStateType _NewState);

        bool HandleEvents(const SDL_Event& _rEvent);
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
        : m_CurrentState       (Game::CState::Start)
        , m_LatestMousePosition(Base::Short2(0, 0))
        , m_pWindow            (nullptr)
        , m_GameWindowID       (0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CApplication::~CApplication()
    { }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnStart(int _Width, int _Height)
    {        
        HWND          NativeWindowHandle;
        SDL_GLContext ContextHandle = 0;
        SDL_SysWMinfo WindowInfo;

        // -----------------------------------------------------------------------------
        // Initialize SDL2 library.
        // We initialize everything like input events, video context, mouse, keyboard,
        // sound, ...
        // -----------------------------------------------------------------------------
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            BASE_THROWM("Can't initialize SDL2.");
        }

        // -----------------------------------------------------------------------------
        // Creating a window to show the game
        // -----------------------------------------------------------------------------
        m_pWindow = SDL_CreateWindow(APP_NAME, APP_POSITION_X, APP_POSITION_Y, _Width, _Height, APP_STYLE);

        if (m_pWindow == nullptr)
        {
            SDL_Quit();

            BASE_THROWM("Can't create an SDL window.");
        }

        // -----------------------------------------------------------------------------
        // Now we get the information of the window handle and set this to
        // the graphic part and active this window.
        // -----------------------------------------------------------------------------
        SDL_VERSION(&WindowInfo.version);

        SDL_GetWindowWMInfo(m_pWindow, &WindowInfo);

        NativeWindowHandle = WindowInfo.info.win.window;

        m_GameWindowID = Gfx::App::RegisterWindow(NativeWindowHandle);

        // -----------------------------------------------------------------------------
        // In game mode we can directly activate this window. It will be active the
        // whole runtime.
        // -----------------------------------------------------------------------------
        Gfx::App::ActivateWindow(m_GameWindowID);

        Gfx::App::OnResize(m_GameWindowID, _Width, _Height);

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
        OnTranslation(Game::CState::UnloadMap);
        
        s_pStates[m_CurrentState]->OnRun();
        
        s_pStates[m_CurrentState]->OnLeave();
        
        OnTranslation(Game::CState::Exit);
        
        s_pStates[m_CurrentState]->OnRun();
        
        s_pStates[m_CurrentState]->OnLeave();

        // -----------------------------------------------------------------------------
        // At the end we have to clean our context and window.
        // -----------------------------------------------------------------------------
        SDL_DestroyWindow(m_pWindow);
        SDL_Quit();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnRun()
    {
        // -----------------------------------------------------------------------------
        // With an window and context we initialize our application and run our game.
        // Furthermore we handle different events by the window.
        // -----------------------------------------------------------------------------
        int       ApplicationMessage = 0;
        int       WindowMessage      = 0;
        SDL_Event WindowEvent;

        for (; ApplicationMessage == 0 && WindowMessage == 0; )
        {
            // -----------------------------------------------------------------------------
            // Events
            // -----------------------------------------------------------------------------
            for (; SDL_PollEvent(&WindowEvent); )
            {
                if (!HandleEvents(WindowEvent))
                {
                    WindowMessage = 1;

                    break;
                }
            }

            // -----------------------------------------------------------------------------
            // Time
            // -----------------------------------------------------------------------------
            Core::Time::Update();

            // -----------------------------------------------------------------------------
            // State engine
            // -----------------------------------------------------------------------------
            Game::CState::EStateType NextState;

            NextState = s_pStates[m_CurrentState]->OnRun();

            if (NextState != m_CurrentState)
            {
                OnTranslation(NextState);
            }
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnTranslation(Game::CState::EStateType _NewState)
    {
        s_pStates[m_CurrentState]->OnLeave();
        
        s_pStates[_NewState]->OnEnter();
        
        m_CurrentState = _NewState;
    }

    // -----------------------------------------------------------------------------

    bool CApplication::HandleEvents(const SDL_Event& _rEvent)
    {
        switch (_rEvent.window.event)
        {
        case SDL_WINDOWEVENT_RESIZED:
        {
            int WindowWidth;
            int WindowHeight;

            SDL_GetWindowSize(m_pWindow, &WindowWidth, &WindowHeight);

            Gfx::App::OnResize(m_GameWindowID, WindowWidth, WindowHeight);
        }
        break;

        case SDL_WINDOWEVENT_CLOSE:
        {
            return false;
        }
        break;
        };

        switch (_rEvent.type)
        {
        case SDL_QUIT:
        {
            return false;
        }
        break;

        case SDL_KEYDOWN:
        {
            unsigned int Key = _rEvent.key.keysym.sym;
            unsigned int Mod = _rEvent.key.keysym.mod;

            Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::KeyPressed, Key, Mod);

            Gui::EventHandler::OnUserEvent(NewInput);
        }
        break;

        case SDL_KEYUP:
        {
            unsigned int Key = _rEvent.key.keysym.sym;
            unsigned int Mod = _rEvent.key.keysym.mod;

            Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::KeyReleased, Key, Mod);

            Gui::EventHandler::OnUserEvent(NewInput);
        }
        break;

        case SDL_MOUSEBUTTONDOWN:
        {
            Base::CInputEvent::EAction MouseAction = Base::CInputEvent::UndefinedAction;

            if (SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                MouseAction = Base::CInputEvent::MouseLeftPressed;
            }
            else if (SDL_BUTTON(SDL_BUTTON_RIGHT))
            {
                MouseAction = Base::CInputEvent::MouseMiddlePressed;
            }
            else if (SDL_BUTTON(SDL_BUTTON_RIGHT))
            {
                MouseAction = Base::CInputEvent::MouseRightPressed;
            }

            if (MouseAction != Base::CInputEvent::UndefinedAction)
            {
                Base::CInputEvent NewInput(Base::CInputEvent::Input, MouseAction, Base::CInputEvent::Mouse, m_LatestMousePosition);

                Gui::EventHandler::OnUserEvent(NewInput);
            }
        }
        break;

        case SDL_MOUSEBUTTONUP:
        {
            Base::CInputEvent::EAction MouseAction = Base::CInputEvent::UndefinedAction;

            if (SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                MouseAction = Base::CInputEvent::MouseLeftReleased;
            }
            else if (SDL_BUTTON(SDL_BUTTON_MIDDLE))
            {
                MouseAction = Base::CInputEvent::MouseMiddleReleased;
            }
            else if (SDL_BUTTON(SDL_BUTTON_RIGHT))
            {
                MouseAction = Base::CInputEvent::MouseRightReleased;
            }

            if (MouseAction != Base::CInputEvent::UndefinedAction)
            {
                Base::CInputEvent NewInput(Base::CInputEvent::Input, MouseAction, Base::CInputEvent::Mouse, m_LatestMousePosition);

                Gui::EventHandler::OnUserEvent(NewInput);
            }
        }
        break;

        case SDL_MOUSEWHEEL:
        {
            int WheelData = _rEvent.wheel.y;

            Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseWheel, Base::CInputEvent::Mouse, m_LatestMousePosition, WheelData);

            Gui::EventHandler::OnUserEvent(NewInput);
        }
        break;

        case SDL_MOUSEMOTION:
        {
            int MousePositionX;
            int MousePositionY;

            SDL_GetMouseState(&MousePositionX, &MousePositionY);

            m_LatestMousePosition[0] = MousePositionX;
            m_LatestMousePosition[1] = MousePositionY;

            Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseMove, Base::CInputEvent::Mouse, m_LatestMousePosition);

            Gui::EventHandler::OnUserEvent(NewInput);
        }
        break;
        }

        return true;
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
    
    void OnRun()
    {
        CApplication::GetInstance().OnRun();
    }
} // namespace Runtime
} // namespace Game
