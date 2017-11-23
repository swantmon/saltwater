
#include "app_droid/app_application.h"
#include "app_droid/app_load_map_state.h"

#include "graphic/gfx_load_map_state.h"

#include "gui/gui_load_map_state.h"

#include "logic/lg_load_map_state.h"

namespace App
{
    CLoadMapState& CLoadMapState::GetInstance()
    {
        static CLoadMapState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace Base
{
    class CTextReader
    {
    };
} // namespace Base

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
        BASE_CONSOLE_STREAMINFO("Enter load level state.");
        Base::CTextReader TextReader;

        Lg ::LoadMap::OnEnter(TextReader);
        Gui::LoadMap::OnEnter(TextReader);
        Gfx::LoadMap::OnEnter(TextReader);
    }

    // -----------------------------------------------------------------------------

    void CLoadMapState::InternOnLeave()
    {
        Gfx::LoadMap::OnLeave();
        Gui::LoadMap::OnLeave();
        Lg ::LoadMap::OnLeave();        

        BASE_CONSOLE_STREAMINFO("Leave load level state.");
    }

    // -----------------------------------------------------------------------------

    void CLoadMapState::InternOnRun()
    {
        CState::EStateType NextState = CState::LoadMap;

        switch (Lg::LoadMap::OnRun())
        {
        case Lg::LoadMap::SResult::LoadMap:
            NextState = CState::LoadMap;
            break;
        case Lg::LoadMap::SResult::Play:
            NextState = CState::Play;
            break;
        }

        Gui::LoadMap::OnRun();
        Gfx::LoadMap::OnRun();

        App::Application::ChangeState(NextState);
    }
} // namespace App