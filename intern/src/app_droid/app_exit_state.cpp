
#include "app_droid/app_exit_state.h"
#include "app_droid/pch.h"

#include "graphic/gfx_exit_state.h"

#include "gui/gui_exit_state.h"

#include "logic/lg_exit_state.h"

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
        BASE_CONSOLE_STREAMINFO("Enter exit state.");

        Lg ::Exit::OnEnter();
        Gui::Exit::OnEnter();
        Gfx::Exit::OnEnter();
    }

    // -----------------------------------------------------------------------------

    void CExitState::InternOnLeave()
    {
        Gfx::Exit::OnLeave();
        Gui::Exit::OnLeave();
        Lg ::Exit::OnLeave();

        BASE_CONSOLE_STREAMINFO("Leave exit state.");
    }

    // -----------------------------------------------------------------------------

    void CExitState::InternOnRun()
    {
        Lg ::Exit::OnRun();
        Gui::Exit::OnRun();
        Gfx::Exit::OnRun();
    }
} // namespace App