
#include "app_droid/app_application.h"
#include "app_droid/app_exit_state.h"
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

#include "core/core_time.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

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
        
    private:
        
        static App::CState* s_pStates[App::CState::NumberOfStateTypes];

    private:

        struct SApplicationSetup 
        {
            struct android_app* m_AndroidApp;

            ASensorManager* m_SensorManager;
            const ASensor* m_AccelerometerSensor;
            ASensorEventQueue* m_SensorEventQueue;

            bool m_IsStarted;
            bool m_TerminateRequested;
        };
        
    private:
        
        App::CState::EStateType m_CurrentState;
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
        : m_CurrentState(App::CState::Start)
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
		// Set base
		// -----------------------------------------------------------------------------
		// Base::SetInternalStoragePath(_pAndroidApp->activity->internalDataPath);

        // -----------------------------------------------------------------------------
        // Set engine
        // -----------------------------------------------------------------------------
        m_AppSetup.m_AndroidApp = _pAndroidApp;

        // -----------------------------------------------------------------------------
        // Prepare to monitor accelerometer
        // -----------------------------------------------------------------------------
        m_AppSetup.m_SensorManager       = ASensorManager_getInstance();
        m_AppSetup.m_AccelerometerSensor = ASensorManager_getDefaultSensor(m_AppSetup.m_SensorManager, ASENSOR_TYPE_ACCELEROMETER);
        m_AppSetup.m_SensorEventQueue    = ASensorManager_createEventQueue(m_AppSetup.m_SensorManager, _pAndroidApp->looper, LOOPER_ID_USER, NULL, NULL);

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

            while ((Identifcation = ALooper_pollAll(0, NULL, &Events, (void**)&AndroidPollSource)) >= 0) 
            {
                if (AndroidPollSource != NULL) 
                {
                    AndroidPollSource->process(m_AppSetup.m_AndroidApp, AndroidPollSource);
                }

                if (m_AppSetup.m_AndroidApp->destroyRequested != 0 || m_AppSetup.m_TerminateRequested != 0) 
                {
                    ApplicationMessage = 1;
                }
            }

            // -----------------------------------------------------------------------------
            // Time
            // -----------------------------------------------------------------------------
            Core::Time::Update();

            // -----------------------------------------------------------------------------
            // States
            // -----------------------------------------------------------------------------
            App::CState::EStateType NextState;

            NextState = s_pStates[m_CurrentState]->OnRun();

            if (NextState != m_CurrentState)
            {
                OnTranslation(NextState);
            }
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnTranslation(App::CState::EStateType _NewState)
    {
        s_pStates[m_CurrentState]->OnLeave();
        
        s_pStates[_NewState]->OnEnter();
        
        m_CurrentState = _NewState;
    }

    // -----------------------------------------------------------------------------

    int32_t CApplication::HandleInputs(struct android_app* app, AInputEvent* event)
    {
        CApplication::SApplicationSetup* AppSetup = static_cast<CApplication::SApplicationSetup*>(app->userData);

        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) 
        {
            return 1;
        }

        return 0;
    }

    // -----------------------------------------------------------------------------

    void CApplication::HandleEvents(struct android_app* app, int32_t cmd)
    {
        CApplication::SApplicationSetup* AppSetup = static_cast<CApplication::SApplicationSetup*>(app->userData);

        switch (cmd) 
        {
        case APP_CMD_SAVE_STATE:
            break;

        case APP_CMD_INIT_WINDOW:
            // -----------------------------------------------------------------------------
            // The window is being shown, get it ready.
            // -----------------------------------------------------------------------------
            if (AppSetup->m_AndroidApp->window != NULL) 
            {
                // Gfx::Main::RegisterWindow(AppSetup->m_AndroidApp->window);

                AppSetup->m_IsStarted = true;
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
} // namespace Application
} // namespace App