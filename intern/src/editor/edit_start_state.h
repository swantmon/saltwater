
#pragma once

#include "editor/edit_state.h"

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
        
        void InternOnEnter() override;
        void InternOnLeave() override;
        CState::EStateType InternOnRun() override;
        
    };
} // namespace Edit