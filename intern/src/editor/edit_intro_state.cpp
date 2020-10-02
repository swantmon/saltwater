
#include "editor/edit_precompiled.h"

#include "editor/edit_intro_state.h"

namespace Edit
{
    CIntroState& CIntroState::GetInstance()
    {
        static CIntroState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CIntroState::CIntroState()
        : CState(Intro)
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
        m_NextState = CState::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnRun()
    {
        m_NextState = CState::LoadMap;

        return m_NextState;
    }
} // namespace Edit