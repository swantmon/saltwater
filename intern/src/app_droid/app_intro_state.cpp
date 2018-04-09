
#include "app_droid/app_precompiled.h"

#include "app_droid/app_application.h"
#include "app_droid/app_intro_state.h"

#include "graphic/gfx_intro_state.h"

#include "gui/gui_intro_state.h"

#include "logic/lg_intro_state.h"

namespace App
{
    CIntroState& CIntroState::GetInstance()
    {
        static CIntroState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CIntroState::CIntroState()
    {

    }

    // -----------------------------------------------------------------------------

    CIntroState::~CIntroState()
    {

    }

    // -----------------------------------------------------------------------------

    void CIntroState::InternOnEnter()
    {

    }

    // -----------------------------------------------------------------------------

    void CIntroState::InternOnLeave()
    {

    }

    // -----------------------------------------------------------------------------

    void CIntroState::InternOnRun()
    {
        App::Application::ChangeState(CState::Intro);
    }
} // namespace App