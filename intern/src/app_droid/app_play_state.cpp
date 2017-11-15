
#include "app_droid/app_application.h"
#include "app_droid/app_play_state.h"

#include "graphic/gfx_play_state.h"

namespace App
{
    CPlayState& CPlayState::GetInstance()
    {
        static CPlayState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CPlayState::CPlayState()
    {

    }

    // -----------------------------------------------------------------------------

    CPlayState::~CPlayState()
    {

    }

    // -----------------------------------------------------------------------------

    void CPlayState::InternOnEnter()
    {
        Gfx::Play::OnEnter();
    }

    // -----------------------------------------------------------------------------

    void CPlayState::InternOnLeave()
    {
        Gfx::Play::OnLeave();
    }

    // -----------------------------------------------------------------------------

    void CPlayState::InternOnRun()
    {
        Gfx::Play::OnRun();
    }
} // namespace App