
#include "base/base_console.h"

#include "camera/cam_control_manager.h"

#include "editor/edit_edit_state.h"

#include "graphic/gfx_play_state.h"

#include "gui/gui_play_state.h"

#include "logic/lg_play_state.h"

namespace Edit
{
    CEditState& CEditState::GetInstance()
    {
        static CEditState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CEditState::CEditState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CEditState::~CEditState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Edit> Enter play state.");

//         Lg ::Play::OnEnter();
//         Gui::Play::OnEnter();
//         Gfx::Play::OnEnter();        
        
        return Edit::CState::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnLeave()
    {
//         Gfx::Play::OnLeave();
//         Gui::Play::OnLeave();
//         Lg ::Play::OnLeave();

        BASE_CONSOLE_STREAMINFO("Edit> Leave play state.");
        
        return Edit::CState::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnRun()
    {
        CState::EStateType NextState = CState::Edit;
        
//         // -----------------------------------------------------------------------------
//         // Update logic
//         // -----------------------------------------------------------------------------
//         switch(Lg::Play::OnRun())
//         {
//             case Lg::Play::SResult::Play:
//                 NextState = CState::Play;
//                 break;
//             case Lg::Play::SResult::Debug:
//                 NextState = CState::Debug;
//                 break;
//             case Lg::Play::SResult::Exit:
//                 NextState = CState::Exit;
//                 break;
//         }
//         
//         // -----------------------------------------------------------------------------
//         // Update cameras and views depending on logic and world
//         // -----------------------------------------------------------------------------
//         Cam::ControlManager::Update();
//         
//         // -----------------------------------------------------------------------------
//         // Update graphic and GUI
//         // -----------------------------------------------------------------------------
//         Gui::Play::OnRun();
//         Gfx::Play::OnRun();
        
        return NextState;
    }
} // namespace Edit