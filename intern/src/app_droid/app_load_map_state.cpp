
#include "app_droid/app_application.h"
#include "app_droid/app_load_map_state.h"

#include "graphic/gfx_load_map_state.h"

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
        // Gfx::LoadMap::OnEnter(TextReader);
    }

    // -----------------------------------------------------------------------------

    void CLoadMapState::InternOnLeave()
    {
        Gfx::LoadMap::OnLeave();
    }

    // -----------------------------------------------------------------------------

    void CLoadMapState::InternOnRun()
    {
        Gfx::LoadMap::OnRun();

        App::Application::ChangeState(CState::Play);
    }
} // namespace App