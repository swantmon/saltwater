
#include "app_droid/app_application.h"
#include "app_droid/app_start_state.h"

#include "graphic/gfx_start_state.h"

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

    void CStartState::InternOnEnter()
    {
        Gfx::Start::OnEnter();
    }

    // -----------------------------------------------------------------------------

    void CStartState::InternOnLeave()
    {
        Gfx::Start::OnLeave();
    }

    // -----------------------------------------------------------------------------

    void CStartState::InternOnRun()
    {
        Gfx::Start::OnRun();

        App::Application::ChangeState(CState::Start);
    }
} // namespace App