
#include "editor/edit_precompiled.h"

#include "editor/edit_exit_state.h"

namespace Edit
{
    CExitState& CExitState::GetInstance()
    {
        static CExitState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CExitState::CExitState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CExitState::~CExitState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CExitState::InternOnEnter()
    {
        return Edit::CState::Exit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CExitState::InternOnLeave()
    {
        return Edit::CState::Exit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CExitState::InternOnRun()
    {
        CState::EStateType NextState = CState::Exit;

        return NextState;
    }
} // namespace Edit