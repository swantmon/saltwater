
#include "app_droid/app_exit_state.h"

namespace App
{
    CExitState& CExitState::GetInstance()
    {
        static CExitState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CExitState::CExitState()
    {

    }

    // -----------------------------------------------------------------------------

    CExitState::~CExitState()
    {

    }

    // -----------------------------------------------------------------------------

    CState::EStateType CExitState::InternOnEnter()
    {
        return App::CState::Exit;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CExitState::InternOnLeave()
    {
        return App::CState::Exit;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CExitState::InternOnRun()
    {
        CState::EStateType NextState = CState::Exit;

        return NextState;
    }
} // namespace App