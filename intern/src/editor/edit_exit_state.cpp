
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
        m_NextState = CState::Exit;
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
        return m_NextState;
    }
} // namespace Edit