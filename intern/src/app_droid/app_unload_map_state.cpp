
#include "app_droid/app_application.h"
#include "app_droid/app_unload_map_state.h"

namespace App
{
    CUnloadMapState& CUnloadMapState::GetInstance()
    {
        static CUnloadMapState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CUnloadMapState::CUnloadMapState()
    {

    }

    // -----------------------------------------------------------------------------

    CUnloadMapState::~CUnloadMapState()
    {

    }

    // -----------------------------------------------------------------------------

    void CUnloadMapState::InternOnEnter()
    {
    }

    // -----------------------------------------------------------------------------

    void CUnloadMapState::InternOnLeave()
    {
    }

    // -----------------------------------------------------------------------------

    void CUnloadMapState::InternOnRun()
    {
        App::Application::ChangeState(App::CState::Exit);
    }
} // namespace App