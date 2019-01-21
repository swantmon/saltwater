
#pragma once

#include "editor_imgui/edit_state.h"

namespace Edit
{
    class CStartState : public CState
    {
        
    public:
        
        static CStartState& GetInstance();
        
    private:
        
        CStartState();
        ~CStartState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace Edit