
#include "app_droid/app_intro_state.h"

namespace App
{
    CIntroState& CIntroState::GetInstance()
    {
        static CIntroState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CIntroState::CIntroState()
    {

    }

    // -----------------------------------------------------------------------------

    CIntroState::~CIntroState()
    {

    }

    // -----------------------------------------------------------------------------

    CState::EStateType CIntroState::InternOnEnter()
    {
        return App::CState::Intro;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CIntroState::InternOnLeave()
    {
        return App::CState::Intro;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CIntroState::InternOnRun()
    {
        CState::EStateType NextState = CState::LoadMap;

        return NextState;
    }
} // namespace App