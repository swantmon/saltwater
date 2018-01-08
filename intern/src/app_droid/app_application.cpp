
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

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_asset_manager.h"

#include "core/core_time.h"

#include "graphic/gfx_application_interface.h"

#include "mr/mr_control_manager.h"

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
            bool m_TerminateRequested;
            int m_Animating;
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
        : m_CurrentState(App::CState::Init)
        , m_RequestState(App::CState::Init)
    {
        memset(&m_AppSetup, 0, sizeof(m_AppSetup));
    }
    
    // -----------------------------------------------------------------------------
    
    CApplication::~CApplication()
    { }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnStart(android_app* _pAndroidApp)
    {        
        // -----------------------------------------------------------------------------
        // Set commands and input
        // -----------------------------------------------------------------------------
        _pAndroidApp->userData     = &m_AppSetup;
        _pAndroidApp->onAppCmd     = CApplication::HandleEvents;
        _pAndroidApp->onInputEvent = CApplication::HandleInputs;

        // -----------------------------------------------------------------------------
        // Set engine
        // -----------------------------------------------------------------------------
        m_AppSetup.m_pAndroidApp = _pAndroidApp;
        m_AppSetup.m_Animating   = 1;

        // -----------------------------------------------------------------------------
        // Prepare to monitor accelerometer
        // -----------------------------------------------------------------------------
        m_AppSetup.m_SensorManager       = ASensorManager_getInstance();
        m_AppSetup.m_AccelerometerSensor = ASensorManager_getDefaultSensor(m_AppSetup.m_SensorManager, ASENSOR_TYPE_ACCELEROMETER);
        m_AppSetup.m_SensorEventQueue    = ASensorManager_createEventQueue(m_AppSetup.m_SensorManager, _pAndroidApp->looper, LOOPER_ID_USER, NULL, NULL);

        // -----------------------------------------------------------------------------
        // Setup asset manager
        // -----------------------------------------------------------------------------
        Core::AssetManager::SetFilePath(_pAndroidApp->activity->externalDataPath);

        // -----------------------------------------------------------------------------
        // Setup mixed reality
        // -----------------------------------------------------------------------------
        MR::ControlManager::SConfiguration Config;

        Config.m_pEnv     = _pAndroidApp->activity->env;
        Config.m_pContext = 0;

        MR::ControlManager::OnStart(Config);

        // -----------------------------------------------------------------------------
        // Start timing
        // -----------------------------------------------------------------------------
        Core::Time::OnStart();

        // -----------------------------------------------------------------------------
        // From now on we can start the state engine and enter the first state
        // -----------------------------------------------------------------------------        
        s_pStates[m_CurrentState]->OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
        // -----------------------------------------------------------------------------
        // Make last transition to exit
        // -----------------------------------------------------------------------------
        OnTranslation(App::CState::UnloadMap);

        s_pStates[m_CurrentState]->OnRun();

        s_pStates[m_CurrentState]->OnLeave();

        OnTranslation(App::CState::Exit);

        s_pStates[m_CurrentState]->OnRun();

        s_pStates[m_CurrentState]->OnLeave();

        // -----------------------------------------------------------------------------
        // Stop timing
        // -----------------------------------------------------------------------------
        Core::Time::OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnRun()
    {
        // -----------------------------------------------------------------------------
        // With an window and context we initialize our application and run our game.
        // Furthermore we handle different events by the window.
        // -----------------------------------------------------------------------------
        int ApplicationMessage = 0;

        for (; ApplicationMessage == 0 ; )
        {
            // -----------------------------------------------------------------------------
            // Events and inputs
            // -----------------------------------------------------------------------------
            int Identifcation;
            int Events;
            struct android_poll_source* AndroidPollSource;

            while ((Identifcation = ALooper_pollAll(m_AppSetup.m_Animating ? 0 : -1, NULL, &Events, (void**)&AndroidPollSource)) >= 0) 
            {
                if (AndroidPollSource != NULL) 
                {
                    AndroidPollSource->process(m_AppSetup.m_pAndroidApp, AndroidPollSource);
                }

                if (Identifcation == LOOPER_ID_USER) 
                {
                    if (m_AppSetup.m_AccelerometerSensor != NULL)
                    {
                        ASensorEvent SensorEvent;

                        while (ASensorEventQueue_getEvents(m_AppSetup.m_SensorEventQueue, &SensorEvent, 1) > 0)
                        {
                            // BASE_CONSOLE_INFOV("Accelerometer: x=%f y=%f z=%f", SensorEvent.acceleration.x, SensorEvent.acceleration.y, SensorEvent.acceleration.z);
                        }
                    }
                }

                if (m_AppSetup.m_pAndroidApp->destroyRequested != 0 || m_AppSetup.m_TerminateRequested != 0) 
                {
                    ApplicationMessage = 1;
                }
            }

            if (m_AppSetup.m_Animating)
            {
                // -----------------------------------------------------------------------------
                // Time
                // -----------------------------------------------------------------------------
                Core::Time::Update();

                // -----------------------------------------------------------------------------
                // States
                // -----------------------------------------------------------------------------
                s_pStates[m_CurrentState]->OnRun();

                if (m_RequestState != m_CurrentState)
                {
                    OnTranslation(m_RequestState);
                }
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CApplication::ChangeState(unsigned int _State)
    {
        m_RequestState = static_cast<App::CState::EStateType>(_State);
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
            break;

        case APP_CMD_INIT_WINDOW:
            // -----------------------------------------------------------------------------
            // The window is being shown, get it ready.
            // -----------------------------------------------------------------------------
            if (AppSetup->m_pAndroidApp->window != NULL) 
            {
                unsigned int WindowID = Gfx::App::RegisterWindow(AppSetup->m_pAndroidApp->window);

                Gfx::App::ActivateWindow(WindowID);

                AppSetup->m_WindowID = WindowID;

                App::Application::ChangeState(App::CState::Start);
            }
            break;

        case APP_CMD_TERM_WINDOW:
            // -----------------------------------------------------------------------------
            // The window is being hidden or closed, clean it up.
            // -----------------------------------------------------------------------------
            AppSetup->m_TerminateRequested = true;
            break;

        case APP_CMD_GAINED_FOCUS:
            // -----------------------------------------------------------------------------
            // When our app gains focus, we start monitoring the accelerometer.
            // -----------------------------------------------------------------------------
            if (AppSetup->m_AccelerometerSensor != NULL) 
            {
                ASensorEventQueue_enableSensor(AppSetup->m_SensorEventQueue, AppSetup->m_AccelerometerSensor);

                // -----------------------------------------------------------------------------
                // We'd like to get 60 events per second (in us).
                // -----------------------------------------------------------------------------
                ASensorEventQueue_setEventRate(AppSetup->m_SensorEventQueue, AppSetup->m_AccelerometerSensor, (1000L / 60) * 1000);
            }
            break;

        case APP_CMD_LOST_FOCUS:
            // -----------------------------------------------------------------------------
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            // -----------------------------------------------------------------------------
            if (AppSetup->m_AccelerometerSensor != NULL) 
            {
                ASensorEventQueue_disableSensor(AppSetup->m_SensorEventQueue, AppSetup->m_AccelerometerSensor);
            }

            AppSetup->m_Animating = 0;
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