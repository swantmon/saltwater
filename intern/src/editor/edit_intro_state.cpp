
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
    {

    }
    
    // -----------------------------------------------------------------------------
    
    CIntroState::~CIntroState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnEnter()
    {
        return Edit::CState::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnLeave()
    {
        return Edit::CState::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnRun()
    {
        return Edit::CState::LoadMap;
    }
} // namespace Edit