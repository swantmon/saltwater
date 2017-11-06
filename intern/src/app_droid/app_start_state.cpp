
#include "app_droid/app_start_state.h"

namespace App
{
    CStartState& CStartState::GetInstance()
    {
        static CStartState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CStartState::CStartState()
    {

    }

    // -----------------------------------------------------------------------------

    CStartState::~CStartState()
    {

    }

    // -----------------------------------------------------------------------------

    CState::EStateType CStartState::InternOnEnter()
    {
        return App::CState::Start;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CStartState::InternOnLeave()
    {
        return App::CState::Start;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CStartState::InternOnRun()
    {
        CState::EStateType NextState = CState::Start;

        return NextState;
    }
} // namespace App