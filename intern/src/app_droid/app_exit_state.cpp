
#include "app_droid/app_precompiled.h"

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

    void CExitState::InternOnEnter()
    {
    }

    // -----------------------------------------------------------------------------

    void CExitState::InternOnLeave()
    {
    }

    // -----------------------------------------------------------------------------

    void CExitState::InternOnRun()
    {
    }
} // namespace App