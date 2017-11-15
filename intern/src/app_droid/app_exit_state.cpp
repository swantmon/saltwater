
#include "app_droid/app_exit_state.h"

#include "graphic/gfx_exit_state.h"

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
        Gfx::Exit::OnEnter();
    }

    // -----------------------------------------------------------------------------

    void CExitState::InternOnLeave()
    {
        Gfx::Exit::OnLeave();
    }

    // -----------------------------------------------------------------------------

    void CExitState::InternOnRun()
    {
        Gfx::Exit::OnRun();
    }
} // namespace App