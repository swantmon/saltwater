
#include "app/app_start_state.h"

#include "base/base_console.h"

#include "camera/cam_control_manager.h"

#include "graphic/gfx_main.h"
#include "graphic/gfx_start_state.h"

#include "gui/gui_start_state.h"

#include "logic/lg_start_state.h"

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
        BASE_CONSOLE_STREAMINFO("App> Enter set resolution to w=" << _Width << " and h=" << _Height);

        m_Width  = _Width;
        m_Height = _Height;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CStartState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("App> Enter start state.");

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
        
        return App::CState::Start;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CStartState::InternOnLeave()
    {
        Gui::Start::OnLeave();
        Gfx::Start::OnLeave();
        Lg ::Start::OnLeave();

        BASE_CONSOLE_STREAMINFO("App> Leave start state.");
        
        return App::CState::Start;
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
} // namespace App