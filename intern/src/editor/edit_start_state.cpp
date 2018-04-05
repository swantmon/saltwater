
#include "editor/edit_precompiled.h"

#include "base/base_console.h"

#include "camera/cam_control_manager.h"

#include "editor/edit_start_state.h"

#include "engine/engine.h"

namespace Edit
{
    CStartState& CStartState::GetInstance()
    {
        static CStartState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CStartState::CStartState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CStartState::~CStartState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CStartState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Edit> Enter start state.");

        // -----------------------------------------------------------------------------
        // Prepare controls
        // -----------------------------------------------------------------------------
        Cam::ControlManager::CreateControl(Cam::CControl::EditorControl);
        Cam::ControlManager::CreateControl(Cam::CControl::GameControl);

        return Edit::CState::Start;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CStartState::InternOnLeave()
    {
        return Edit::CState::Start;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CStartState::InternOnRun()
    {
        return CState::Intro;
    }
} // namespace Edit