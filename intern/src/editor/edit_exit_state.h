
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
        
        void InternOnEnter() override;
        void InternOnLeave() override;
        CState::EStateType InternOnRun() override;
        
    };
} // namespace Edit