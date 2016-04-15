
#pragma once

#include "editor/edit_state.h"

namespace Edit
{
    class CExitState : public CState
    {
        
    public:
        
        static CExitState& GetInstance();
        
    private:
        
        CExitState();
        ~CExitState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace Edit