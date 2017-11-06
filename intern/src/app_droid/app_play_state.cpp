
#include "app_droid/app_play_state.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "NativeLibrary", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "NativeLibrary", __VA_ARGS__))

namespace App
{
    CPlayState& CPlayState::GetInstance()
    {
        static CPlayState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CPlayState::CPlayState()
    {

    }

    // -----------------------------------------------------------------------------

    CPlayState::~CPlayState()
    {

    }

    // -----------------------------------------------------------------------------

    CState::EStateType CPlayState::InternOnEnter()
    {
        return App::CState::Play;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CPlayState::InternOnLeave()
    {
        return App::CState::Play;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CPlayState::InternOnRun()
    {
        CState::EStateType NextState = CState::Play;

        return NextState;
    }
} // namespace App