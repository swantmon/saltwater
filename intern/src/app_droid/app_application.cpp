
#include "app_droid/app_precompiled.h"

#include "app_droid/app_application.h"
#include "app_droid/app_exit_state.h"
#include "app_droid/app_init_state.h"
#include "app_droid/app_intro_state.h"
#include "app_droid/app_load_map_state.h"
#include "app_droid/app_main_menu_state.h"
#include "app_droid/app_play_state.h"
#include "app_droid/app_start_state.h"
#include "app_droid/app_unload_map_state.h"

#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"
#include "base/base_include_glm.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_jni_interface.h"
#include "engine/core/core_time.h"
#include "engine/core/core_plugin_manager.h"
#include "engine/core/core_program_parameters.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_pipeline.h"

#include "engine/gui/gui_event_handler.h"

namespace
{
    class CApplication : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CApplication)
        
    public:
        
        CApplication();
       ~CApplication();
        
    public:
        
        void OnStart(android_app* _pAndroidApp);
        void OnExit();
        void OnRun();

        void ChangeState(unsigned int _State);

        android_app* GetAndroidApp();
        
    private:
        
        static App::CState* s_pStates[App::CState::NumberOfStateTypes];

    private:

        struct SApplicationSetup 
        {
            struct android_app* m_pAndroidApp;

            ASensorManager* m_SensorManager;
            const ASensor* m_AccelerometerSensor;
            ASensorEventQueue* m_SensorEventQueue;

            unsigned int m_WindowID;
            std::string m_ParameterFile;
            bool m_IsStarted;
            int m_Running;
        };
        
    private:
        
        App::CState::EStateType m_CurrentState;
        App::CState::EStateType m_RequestState;
        SApplicationSetup       m_AppSetup;
        
    private:
        
        void OnTranslation(App::CState::EStateType _NewState);

        static int32_t HandleInputs(struct android_app* _AndroidApp, AInputEvent* _pEvent);
        static void HandleEvents(struct android_app* _AndroidApp, int32_t _Command);
    };
} // namespace

namespace
{
    App::CState* CApplication::s_pStates[] =
    {
        &App::CInitState     ::GetInstance(),
        &App::CStartState    ::GetInstance(),
        &App::CIntroState    ::GetInstance(),
        &App::CLoadMapState  ::GetInstance(),
        &App::CMainMenuState ::GetInstance(),
        &App::CPlayState     ::GetInstance(),
        &App::CUnloadMapState::GetInstance(),
        &App::CExitState     ::GetInstance(),
    };
} // namespace

namespace
{
    CApplication::CApplication()
        : m_CurrentState (App::CState::Init)
        , m_RequestState (App::CState::Init)
    {
        Base::CMemory::Zero(&m_AppSetup, sizeof(m_AppSetup));
    }
    
    // -----------------------------------------------------------------------------
    
    CApplication::~CApplication()
    { }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnStart(android_app* _pAndroidApp)
    {
        // -----------------------------------------------------------------------------
        // Assets
        // -----------------------------------------------------------------------------
        Core::AssetManager::SetFilePath(_pAndroidApp->activity->externalDataPath);

        // -----------------------------------------------------------------------------
        // Set commands and input
        // -----------------------------------------------------------------------------
        _pAndroidApp->userData     = &m_AppSetup;
        _pAndroidApp->onAppCmd     = CApplication::HandleEvents;
        _pAndroidApp->onInputEvent = CApplication::HandleInputs;

        // -----------------------------------------------------------------------------
        // Set engine
        // -----------------------------------------------------------------------------
        m_AppSetup.m_pAndroidApp   = _pAndroidApp;
        m_AppSetup.m_Running       = 0;
        m_AppSetup.m_ParameterFile = Core::AssetManager::GetPathToData() + "/android.config";

        // -----------------------------------------------------------------------------
        // Prepare to monitor accelerometer
        // -----------------------------------------------------------------------------
        //m_AppSetup.m_SensorManager       = ASensorManager_getInstance();
        //m_AppSetup.m_AccelerometerSensor = ASensorManager_getDefaultSensor(m_AppSetup.m_SensorManager, ASENSOR_TYPE_ACCELEROMETER);
        //m_AppSetup.m_SensorEventQueue    = ASensorManager_createEventQueue(m_AppSetup.m_SensorManager, _pAndroidApp->looper, LOOPER_ID_USER, NULL, NULL);

        // -----------------------------------------------------------------------------
        // Load configuration file
        // -----------------------------------------------------------------------------
        const std::string VerbosityNameString = "console:verbose"; // TODO: this should be somewhere else - available for every application

        int VerbosityLevel = 0;

        Core::CProgramParameters::GetInstance().ParseFile(m_AppSetup.m_ParameterFile);

        VerbosityLevel = Core::CProgramParameters::GetInstance().Get<int>(VerbosityNameString, 3);

        Core::CConsole::GetInstance().SetVerbosityLevel(VerbosityLevel);

        // -----------------------------------------------------------------------------
        // From now on we can start the state engine and enter the first state
        // -----------------------------------------------------------------------------
        m_CurrentState = App::CState::Init;
        m_RequestState = App::CState::Init;

        App::CInitState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
        // -----------------------------------------------------------------------------
        // Save configuration
        // -----------------------------------------------------------------------------
        Core::CProgramParameters::GetInstance().WriteFile(m_AppSetup.m_ParameterFile);

        // -----------------------------------------------------------------------------
        // Shutdown engine
        // -----------------------------------------------------------------------------
        Engine::Shutdown();

        // -----------------------------------------------------------------------------
        // Exit app
        // -----------------------------------------------------------------------------
        ANativeActivity_finish(m_AppSetup.m_pAndroidApp->activity);
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
            // Inputs
            // -----------------------------------------------------------------------------
            int Identifcation;
            int Events;
            struct android_poll_source* AndroidPollSource;

            while ((Identifcation = ALooper_pollAll(m_AppSetup.m_Running ? 0 : -1, NULL, &Events, (void**)&AndroidPollSource)) >= 0)
            {
                if (AndroidPollSource != NULL) 
                {
                    AndroidPollSource->process(m_AppSetup.m_pAndroidApp, AndroidPollSource);
                }

                /*
                if (Identifcation == LOOPER_ID_USER) 
                {
                    if (m_AppSetup.m_AccelerometerSensor != NULL)
                    {
                        ASensorEvent SensorEvent;

                        while (ASensorEventQueue_getEvents(m_AppSetup.m_SensorEventQueue, &SensorEvent, 1) > 0)
                        {
                            BASE_CONSOLE_INFOV("Accelerometer: x=%f y=%f z=%f", SensorEvent.acceleration.x, SensorEvent.acceleration.y, SensorEvent.acceleration.z);
                        }
                    }
                }
                 */
            }

            // -----------------------------------------------------------------------------
            // Process
            // -----------------------------------------------------------------------------
            s_pStates[m_CurrentState]->OnRun();

            if (m_AppSetup.m_Running) Engine::Update();

            // -----------------------------------------------------------------------------
            // Check state
            // -----------------------------------------------------------------------------
            if (m_RequestState != m_CurrentState)
            {
                OnTranslation(m_RequestState);
            }

            if (m_RequestState == App::CState::Exit)
            {
                App::CExitState::GetInstance().OnRun();

                App::CExitState::GetInstance().OnLeave();

                break;
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CApplication::ChangeState(unsigned int _State)
    {
        m_RequestState = static_cast<App::CState::EStateType>(_State);
    }

    // -----------------------------------------------------------------------------

    android_app* CApplication::GetAndroidApp()
    {
        return m_AppSetup.m_pAndroidApp;
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnTranslation(App::CState::EStateType _NewState)
    {
        s_pStates[m_CurrentState]->OnLeave();
        
        s_pStates[_NewState]->OnEnter();
        
        m_CurrentState = _NewState;
    }

    // -----------------------------------------------------------------------------

    int32_t CApplication::HandleInputs(struct android_app* _pAndroidApp, AInputEvent* _pEvent)
    {
        CApplication::SApplicationSetup* AppSetup = static_cast<CApplication::SApplicationSetup*>(_pAndroidApp->userData);

        BASE_UNUSED(AppSetup);

        if (AInputEvent_getType(_pEvent) == AINPUT_EVENT_TYPE_MOTION) 
        {
            int32_t Action = AMotionEvent_getAction(_pEvent);
            int32_t Source = AInputEvent_getSource(_pEvent);

            switch (Source)
            {
                case AINPUT_SOURCE_TOUCHSCREEN:
                    {
                        int NumberOfPointer = 0;
                        Base::CInputEvent::EAction InputAction;

                        switch( Action & AMOTION_EVENT_ACTION_MASK )
                        {
                            case AMOTION_EVENT_ACTION_DOWN:
                                InputAction = Base::CInputEvent::TouchPressed;
                                NumberOfPointer = 1;
                                break;
                            case AMOTION_EVENT_ACTION_UP:
                                InputAction = Base::CInputEvent::TouchReleased;
                                NumberOfPointer = 1;
                                break;
                            case AMOTION_EVENT_ACTION_MOVE:
                                InputAction = Base::CInputEvent::TouchMove;
                                NumberOfPointer = AMotionEvent_getPointerCount(_pEvent);
                                break;
                        }

                        int IndexOfPointer = Action >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

                        for( int i = 0; i < NumberOfPointer; ++i, ++IndexOfPointer )
                        {
                            float PointerX = AMotionEvent_getRawX(_pEvent, IndexOfPointer);
                            float PointerY = AMotionEvent_getRawY(_pEvent, IndexOfPointer);

                            Base::CInputEvent Input(Base::CInputEvent::Input, InputAction, Base::CInputEvent::Pointer + IndexOfPointer, glm::vec2(PointerX, PointerY), glm::vec2(PointerX, PointerY));

                            Gui::EventHandler::OnEvent(Input);
                        }
                    }
                    break;
            }

            return 1;
        }

        return 0;
    }

    // -----------------------------------------------------------------------------

    void CApplication::HandleEvents(struct android_app* _pAndroidApp, int32_t _Command)
    {
        CApplication::SApplicationSetup* AppSetup = static_cast<CApplication::SApplicationSetup*>(_pAndroidApp->userData);

        switch (_Command)
        {
            case APP_CMD_SAVE_STATE:
                {
                    Core::CProgramParameters::GetInstance().WriteFile(AppSetup->m_ParameterFile);
                }
                break;

            case APP_CMD_INIT_WINDOW:
                {
                    // -----------------------------------------------------------------------------
                    // The window is being shown, get it ready.
                    // -----------------------------------------------------------------------------
                    if (AppSetup->m_pAndroidApp->window != NULL)
                    {
                        if (AppSetup->m_IsStarted == false)
                        {
                            // -----------------------------------------------------------------------------
                            // Do JNI stuff
                            // -----------------------------------------------------------------------------
                            Core::PluginManager::SetLibraryPath(Core::JNI::GetLibraryPath());

                            // -----------------------------------------------------------------------------
                            // Register new native window in engine as a new window
                            // -----------------------------------------------------------------------------
                            unsigned int WindowID = Gfx::Pipeline::RegisterWindow(AppSetup->m_pAndroidApp->window, 0, true);

                            Gfx::Pipeline::ActivateWindow(WindowID);

                            AppSetup->m_WindowID = WindowID;

                            // -----------------------------------------------------------------------------
                            // Start engine because this is the first start
                            // -----------------------------------------------------------------------------
                            Engine::Startup();

                            // -----------------------------------------------------------------------------
                            // Change state
                            // -----------------------------------------------------------------------------
                            App::Application::ChangeState(App::CState::Start);

                            AppSetup->m_IsStarted = true;
                        }
                        else
                        {
                            // -----------------------------------------------------------------------------
                            // Reinitialize window because a new native window has been created
                            // -----------------------------------------------------------------------------
                            Gfx::Pipeline::ReinitializeWindow(AppSetup->m_WindowID, AppSetup->m_pAndroidApp->window);
                        }

                        AppSetup->m_Running = 1;
                    }
                }
                break;

            case APP_CMD_TERM_WINDOW:
                {
                    AppSetup->m_Running = 0;
                }
                break;

            case APP_CMD_DESTROY:
                {
                    Core::CProgramParameters::GetInstance().WriteFile(AppSetup->m_ParameterFile);

                    Engine::Shutdown();

                    ANativeActivity_finish(AppSetup->m_pAndroidApp->activity);

                    exit(0);
                }
                break;

            case APP_CMD_START:
                {
                    if (AppSetup->m_IsStarted)
                    {
                        AppSetup->m_Running = 1;
                    }
                }
                break;

            case APP_CMD_STOP:
                {
                     AppSetup->m_Running = 0;
                }
                break;

            case APP_CMD_PAUSE:
                {
                    if (AppSetup->m_IsStarted)
                    {
                        Engine::Pause();

                        AppSetup->m_Running = 0;
                    }
                }
                break;
            case APP_CMD_RESUME:
                {
                    if (AppSetup->m_IsStarted)
                    {
                        Engine::Resume();

                        AppSetup->m_Running = 1;
                    }
                }
                break;

            case APP_CMD_CONTENT_RECT_CHANGED:
                {
                    // -----------------------------------------------------------------------------
                    // The window has changed the rectangle
                    // -----------------------------------------------------------------------------
                    ARect Rectangle = AppSetup->m_pAndroidApp->pendingContentRect;

                    int Width  = glm::abs(Rectangle.left   - Rectangle.right);
                    int Height = glm::abs(Rectangle.bottom - Rectangle.top);

                    // -----------------------------------------------------------------------------
                    // Inform all libs
                    // -----------------------------------------------------------------------------
                    Gfx::Pipeline::OnResize(AppSetup->m_WindowID, Width, Height);
                }
                break;

            case APP_CMD_GAINED_FOCUS:
                {
                    /*
                    if (AppSetup->m_AccelerometerSensor != NULL)
                    {
                        ASensorEventQueue_enableSensor(AppSetup->m_SensorEventQueue, AppSetup->m_AccelerometerSensor);

                        // -----------------------------------------------------------------------------
                        // We'd like to get 60 events per second (in us).
                        // -----------------------------------------------------------------------------
                        ASensorEventQueue_setEventRate(AppSetup->m_SensorEventQueue, AppSetup->m_AccelerometerSensor, (1000L / 60) * 1000);
                    }
                    */
                }
                break;

            case APP_CMD_LOST_FOCUS:
                {
                    /*
                    if (AppSetup->m_AccelerometerSensor != NULL)
                    {
                        ASensorEventQueue_disableSensor(AppSetup->m_SensorEventQueue, AppSetup->m_AccelerometerSensor);
                    }
                    */
                }
                break;
        }
    }
}

namespace App
{
namespace Application
{
    void OnStart(android_app* _pAndroidApp)
    {
        CApplication::GetInstance().OnStart(_pAndroidApp);
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

    void ChangeState(unsigned int _State)
    {
        CApplication::GetInstance().ChangeState(_State);
    }
} // namespace Application
} // namespace App