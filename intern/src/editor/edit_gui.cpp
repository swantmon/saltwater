
#include "editor/edit_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_input_event.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "editor/edit_console_panel.h"
#include "editor/edit_gui.h"
#include "editor/edit_infos_panel.h"
#include "editor/edit_inspector_panel.h"
#include "editor/edit_scene_graph_panel.h"

#include "editor/imgui/imgui.h"
#include "editor/imgui/imgui_impl_opengl.h"
#include "editor/imgui/imgui_impl_sdl.h"
#include "editor/imgui/imgui_internal.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/data/data_entity_manager.h"
#include "engine/data/data_map.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_pipeline.h"
#include "engine/graphic/gfx_performance.h"

#include "engine/gui/gui_event_handler.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"

#include <array>

using namespace Edit;

namespace 
{
    class CEditorGui : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CEditorGui)

    public:

        CEditorGui();
        ~CEditorGui();

    public:

        void Create(int& _rArgc, char** _ppArgv);
        void Destroy();

        void Setup(int _WindowID, int _Width, int _Height);

        void Show();

        void ProcessEvents();

        void* GetEditorWindowHandle();

    private:

        enum EPanels
        {
            Inspector,
            SceneGraph,
            Infos,
            Console,
            NumberOfPanels,
        };

        const std::array<Edit::GUI::IPanel*, NumberOfPanels> m_Panels = 
        {
            &GUI::CInspectorPanel::GetInstance(),
            &GUI::CSceneGraphPanel::GetInstance(),
            &GUI::CInfosPanel::GetInstance(),
            &GUI::CConsolePanel::GetInstance()
        };

    private:

        unsigned int m_EditWindowID;

        SDL_Joystick* m_pGamePad;
        SDL_Window* m_pWindow;
        int m_AnalogStickDeadZone;

        bool m_EnableGamepad;
        bool m_CloseWindow;

        bool m_ShowGUI;

        Engine::CEventDelegates::HandleType m_GfxOnRenderGUIDelegate;

    private:

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
    CEditorGui::CEditorGui()
        : m_EditWindowID(0)
        , m_pGamePad(nullptr)
        , m_CloseWindow(false)
    {
    }
    
    // -----------------------------------------------------------------------------

    CEditorGui::~CEditorGui()
    {
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::Create(int& _rArgc, char** _ppArgv)
    {
        BASE_UNUSED(_rArgc);
        BASE_UNUSED(_ppArgv);

        // -----------------------------------------------------------------------------
        // SDL
        // -----------------------------------------------------------------------------
        SDL_Init(SDL_INIT_VIDEO);

        SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "0", SDL_HINT_OVERRIDE);

        auto WindowSize = Core::CProgramParameters::GetInstance().Get("application:window_size", glm::ivec2(1280, 720));

        m_pWindow = SDL_CreateWindow("Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WindowSize.x, WindowSize.y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        if (m_pWindow == nullptr)
        {
            BASE_THROWM("Could not initialize SDL window.");
        }

        // -----------------------------------------------------------------------------
        // IMGUI
        // -----------------------------------------------------------------------------
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // -----------------------------------------------------------------------------
        // Style
        // -----------------------------------------------------------------------------
        ImGuiStyle& rGUIStyle = ImGui::GetStyle();

        ImVec4* GUIColors = rGUIStyle.Colors;

        GUIColors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        GUIColors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        GUIColors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
        GUIColors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        GUIColors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        GUIColors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        GUIColors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        GUIColors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
        GUIColors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        GUIColors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        GUIColors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        GUIColors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
        GUIColors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        GUIColors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        GUIColors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        GUIColors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        GUIColors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        GUIColors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        GUIColors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        GUIColors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        GUIColors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        GUIColors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        GUIColors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        GUIColors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        GUIColors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        GUIColors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        GUIColors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        GUIColors[ImGuiCol_Separator] = GUIColors[ImGuiCol_Border];
        GUIColors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        GUIColors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        GUIColors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        GUIColors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        GUIColors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        GUIColors[ImGuiCol_Tab] = ImLerp(GUIColors[ImGuiCol_Header], GUIColors[ImGuiCol_TitleBgActive], 0.80f);
        GUIColors[ImGuiCol_TabHovered] = GUIColors[ImGuiCol_HeaderHovered];
        GUIColors[ImGuiCol_TabActive] = ImLerp(GUIColors[ImGuiCol_HeaderActive], GUIColors[ImGuiCol_TitleBgActive], 0.60f);
        GUIColors[ImGuiCol_TabUnfocused] = ImLerp(GUIColors[ImGuiCol_Tab], GUIColors[ImGuiCol_TitleBg], 0.80f);
        GUIColors[ImGuiCol_TabUnfocusedActive] = ImLerp(GUIColors[ImGuiCol_TabActive], GUIColors[ImGuiCol_TitleBg], 0.40f);
        GUIColors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        GUIColors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        GUIColors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        GUIColors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        GUIColors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        GUIColors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        GUIColors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        GUIColors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        GUIColors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        GUIColors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

        rGUIStyle.WindowRounding = 0.0f;
        rGUIStyle.ChildRounding = 0.0f;
        rGUIStyle.FrameRounding = 0.0f;
        rGUIStyle.ScrollbarRounding = 0.0f;
        rGUIStyle.GrabRounding = 0.0f;
        rGUIStyle.TabRounding = 0.0f;

        // -----------------------------------------------------------------------------
        // Init
        // -----------------------------------------------------------------------------
        ImGui_ImplSDL2_InitForOpenGL(m_pWindow, nullptr);

        ImGui_ImplOpenGL3_Init();

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        auto RenderUI = []()
        {
            Gfx::Performance::BeginEvent("IMGUI");

            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            Gfx::Performance::EndEvent();
        };

        m_GfxOnRenderGUIDelegate = Engine::RegisterEventHandler(Engine::EEvent::Gfx_OnRenderUI, RenderUI);

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

        m_ShowGUI = Core::CProgramParameters::GetInstance().Get("application:show_gui", true);
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::Destroy()
    {
        ImGui_ImplSDL2_Shutdown();

        ImGui_ImplOpenGL3_Shutdown();

        ImGui::DestroyContext();

        SDL_DestroyWindow(m_pWindow);

        SDL_JoystickClose(m_pGamePad);

        SDL_Quit();
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::Setup(int _WindowID, int _Width, int _Height)
    {
        m_EditWindowID = _WindowID;

        SDL_SetWindowSize(m_pWindow, _Width, _Height);

        Gfx::Pipeline::OnResize(m_EditWindowID, _Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::Show()
    {
        SDL_ShowWindow(m_pWindow);
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::ProcessEvents()
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

        if (!m_ShowGUI)
        {
            return;
        }

        // -----------------------------------------------------------------------------
        // Menu
        // -----------------------------------------------------------------------------
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("Exit", "ALT+F4", &m_CloseWindow);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Entity"))
            {
                if (ImGui::MenuItem("Add empty entity"))
                {
                    Dt::SEntityDescriptor EntityDesc;

                    EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
                    EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

                    Dt::CEntity& rNewEntity = Dt::EntityManager::CreateEntity(EntityDesc);

                    rNewEntity.SetName("New entity");

                    Dt::EntityManager::MarkEntityAsDirty(rNewEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Windows"))
            {
                for (auto pPanel : m_Panels)
                {
                    if (ImGui::MenuItem(pPanel->GetName()))
                    {
                        pPanel->Show(!pPanel->IsVisible());
                    }
                }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // -----------------------------------------------------------------------------
        // Panels
        // -----------------------------------------------------------------------------
        for (auto pPanel : m_Panels)
        {
            if (pPanel->IsVisible()) pPanel->Render();
        }
    }

    // -----------------------------------------------------------------------------

    void* CEditorGui::GetEditorWindowHandle()
    {
        // -----------------------------------------------------------------------------
        // Now we get the information of the window handle and set this to
        // the graphic part and active this window.
        // -----------------------------------------------------------------------------
        SDL_SysWMinfo WMinfo;

        SDL_VERSION(&WMinfo.version);

        SDL_GetWindowWMInfo(m_pWindow, &WMinfo);

        return WMinfo.info.win.window;
    }
    
    // -----------------------------------------------------------------------------

    void CEditorGui::ProcessSDLEvents()
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

            // -----------------------------------------------------------------------------
            // Special
            // -----------------------------------------------------------------------------
            if (SDLEvent.type == SDL_QUIT || m_CloseWindow)
            {
                Base::CInputEvent Event = Base::CInputEvent(Base::CInputEvent::Exit);

                Gui::EventHandler::OnEvent(Event);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::ProcessWindowEvents(const SDL_Event& _rSDLEvent)
    {
        using Base::CInputEvent;

        CInputEvent Event(CInputEvent::Input);

        switch (_rSDLEvent.window.event)
        {
        case SDL_WINDOWEVENT_CLOSE:
            Event = Base::CInputEvent(Base::CInputEvent::Exit);

            Gui::EventHandler::OnEvent(Event);
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

    void CEditorGui::ProcessKeyboardEvents(const SDL_Event& _rSDLEvent)
    {
        if (ImGui::GetIO().WantCaptureKeyboard) return;

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

            Gui::EventHandler::OnEvent(Event);
            break;
        case SDL_KEYUP:
            Key = _rSDLEvent.key.keysym.sym;
            Mod = _rSDLEvent.key.keysym.mod;

            Event = CInputEvent(Base::CInputEvent::Input, Base::CInputEvent::KeyReleased, Key, Mod);

            Gui::EventHandler::OnEvent(Event);
            break;
        }
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::ProcessMouseEvents(const SDL_Event& _rSDLEvent)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        using Base::CInputEvent;

        CInputEvent Event(CInputEvent::Input);

        Base::CInputEvent::EAction MouseAction = Base::CInputEvent::UndefinedAction;
        int WheelData;
        glm::ivec2 LocalMousePosition, GlobalMousePosition;

        SDL_GetMouseState(&LocalMousePosition.x, &LocalMousePosition.y);
        SDL_GetGlobalMouseState(&GlobalMousePosition.x, &GlobalMousePosition.y);

        WheelData = _rSDLEvent.wheel.y;

        switch (_rSDLEvent.type)
        {
        case SDL_MOUSEMOTION:
            Event = CInputEvent(CInputEvent::Input, CInputEvent::MouseMove, CInputEvent::Mouse, GlobalMousePosition, LocalMousePosition);

            Gui::EventHandler::OnEvent(Event);
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
                Event = CInputEvent(CInputEvent::Input, MouseAction, CInputEvent::Mouse, GlobalMousePosition, LocalMousePosition);

                Gui::EventHandler::OnEvent(Event);
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
                Event = CInputEvent(CInputEvent::Input, MouseAction, CInputEvent::Mouse, GlobalMousePosition, LocalMousePosition);

                Gui::EventHandler::OnEvent(Event);
            }
            break;
        case SDL_MOUSEWHEEL:
            WheelData = _rSDLEvent.wheel.y;

            Event = CInputEvent(CInputEvent::Input, CInputEvent::MouseWheel, CInputEvent::Mouse, GlobalMousePosition, LocalMousePosition, static_cast<float>(WheelData));

            Gui::EventHandler::OnEvent(Event);
            break;
        }
    }

    // -----------------------------------------------------------------------------

    void CEditorGui::ProcessGamepadEvents(const SDL_Event& _rSDLEvent)
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

                Gui::EventHandler::OnEvent(Event);
            }
            break;

        case SDL_JOYBUTTONDOWN:

            Event = CInputEvent(CInputEvent::Input, CInputEvent::GamepadKeyPressed, ConvertSDLKey(_rSDLEvent));
            Gui::EventHandler::OnEvent(Event);
            break;

        case SDL_JOYBUTTONUP:

            Event = CInputEvent(CInputEvent::Input, CInputEvent::GamepadKeyReleased, ConvertSDLKey(_rSDLEvent));
            Gui::EventHandler::OnEvent(Event);
            break;
        }
    }

    // -----------------------------------------------------------------------------

    Base::CInputEvent::EKey CEditorGui::ConvertSDLKey(const SDL_Event& _rSDLEvent)
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

    Base::CInputEvent::EKey CEditorGui::ConvertSDLAxis(const SDL_Event& _rSDLEvent)
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
} // namespace 

namespace Edit
{
namespace GUI
{
    void Create(int& _rArgc, char** _ppArgv)
    {
        CEditorGui::GetInstance().Create(_rArgc, _ppArgv);
    }
    
    // -----------------------------------------------------------------------------

    void Destroy()
    {
        CEditorGui::GetInstance().Destroy();
    }

    // -----------------------------------------------------------------------------

    void Setup(int _WindowID, int _Width, int _Height)
    {
        CEditorGui::GetInstance().Setup(_WindowID, _Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void Show()
    {
        CEditorGui::GetInstance().Show();
    }

    // -----------------------------------------------------------------------------

    void ProcessEvents()
    {
        CEditorGui::GetInstance().ProcessEvents();
    }

    // -----------------------------------------------------------------------------

    void* GetEditorWindowHandle()
    {
        return CEditorGui::GetInstance().GetEditorWindowHandle();
    }
} // namespace GUI
} // namespace Edit