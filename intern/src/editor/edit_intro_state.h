
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
        
        void InternOnEnter() override;
        void InternOnLeave() override;
        CState::EStateType InternOnRun() override;
    };
} // namespace Edit