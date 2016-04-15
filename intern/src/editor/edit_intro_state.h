
#pragma once

#include "editor/edit_state.h"

namespace Edit
{
    class CIntroState : public CState
    {
        
    public:
        
        static CIntroState& GetInstance();
        
    private:
        
        CIntroState();
        ~CIntroState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace Edit