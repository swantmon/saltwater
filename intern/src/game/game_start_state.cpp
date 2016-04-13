
#include "base/base_console.h"

#include "camera/cam_control_manager.h"

#include "game/game_start_state.h"

#include "graphic/gfx_main.h"
#include "graphic/gfx_start_state.h"

#include "gui/gui_start_state.h"

#include "logic/lg_start_state.h"

namespace Game
{
    CStartState& CStartState::GetInstance()
    {
        static CStartState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Game

namespace Game
{
    CStartState::CStartState()
        : m_Width       (0)
        , m_Height      (0)
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CStartState::~CStartState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CStartState::SetResolution(int _Width, int _Height)
    {
        BASE_CONSOLE_STREAMINFO("Game> Enter set resolution to w=" << _Width << " and h=" << _Height);

        m_Width  = _Width;
        m_Height = _Height;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CStartState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Game> Enter start state.");

        // -----------------------------------------------------------------------------
        // Start engine
        // -----------------------------------------------------------------------------
        Cam::ControlManager::CreateControl(Cam::CControl::FlyControl);
        Cam::ControlManager::CreateControl(Cam::CControl::ARControl);
         
        Gfx::Main::OnStart(m_Width, m_Height);
        
        // -----------------------------------------------------------------------------
        // Start normal states
        // -----------------------------------------------------------------------------
        Lg ::Start::OnEnter();
        Gfx::Start::OnEnter();
        Gui::Start::OnEnter();
        
        return Game::CState::Start;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CStartState::InternOnLeave()
    {
        Gui::Start::OnLeave();
        Gfx::Start::OnLeave();
        Lg ::Start::OnLeave();

        BASE_CONSOLE_STREAMINFO("Game> Leave start state.");
        
        return Game::CState::Start;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CStartState::InternOnRun()
    {
        CState::EStateType NextState = CState::Intro;
        
        switch(Lg::Start::OnRun())
        {
            case Lg::Start::SResult::Start:
            NextState = CState::Start;
            break;
            case Lg::Start::SResult::Intro:
            NextState = CState::Intro;
            break;
        }
        
        Gfx::Start::OnRun();
        Gui::Start::OnRun();
        
        return NextState;
    }
} // namespace Game