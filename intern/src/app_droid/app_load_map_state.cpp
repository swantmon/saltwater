
#include "app_droid/app_application.h"
#include "app_droid/app_load_map_state.h"

namespace App
{
    CLoadMapState& CLoadMapState::GetInstance()
    {
        static CLoadMapState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CLoadMapState::CLoadMapState()
    {

    }

    // -----------------------------------------------------------------------------

    CLoadMapState::~CLoadMapState()
    {

    }

    // -----------------------------------------------------------------------------

    void CLoadMapState::InternOnEnter()
    {

    }

    // -----------------------------------------------------------------------------

    void CLoadMapState::InternOnLeave()
    {

    }

    // -----------------------------------------------------------------------------

    void CLoadMapState::InternOnRun()
    {
        App::Application::ChangeState(CState::Play);
    }
} // namespace App