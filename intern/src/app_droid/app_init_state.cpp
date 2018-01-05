
#include "app_droid/app_application.h"
#include "app_droid/app_init_state.h"

namespace App
{
    CInitState& CInitState::GetInstance()
    {
        static CInitState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CInitState::CInitState()
    {

    }

    // -----------------------------------------------------------------------------

    CInitState::~CInitState()
    {

    }

    // -----------------------------------------------------------------------------

    void CInitState::InternOnEnter()
    {
    }

    // -----------------------------------------------------------------------------

    void CInitState::InternOnLeave()
    {
    }

    // -----------------------------------------------------------------------------

    void CInitState::InternOnRun()
    {
    }
} // namespace App