
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
        : CState(Exit)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CExitState::~CExitState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CExitState::InternOnEnter()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CExitState::InternOnLeave()
    {
        m_NextState = CState::Exit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CExitState::InternOnRun()
    {
        return m_NextState;
    }
} // namespace Edit