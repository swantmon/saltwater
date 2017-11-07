
#include "app_droid/app_application.h"
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

    void CStartState::InternOnEnter()
    {
    }

    // -----------------------------------------------------------------------------

    void CStartState::InternOnLeave()
    {
    }

    // -----------------------------------------------------------------------------

    void CStartState::InternOnRun()
    {
        App::Application::ChangeState(CState::Start);
    }
} // namespace App