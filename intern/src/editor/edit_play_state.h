
#pragma once

#include "editor/edit_state.h"

namespace Edit
{
    class CPlayState : public CState
    {
        
    public:
        
        static CPlayState& GetInstance();
        
    private:
        
        CPlayState();
        ~CPlayState();
        
    private:
        
        void InternOnEnter() override;
        void InternOnLeave() override;
        CState::EStateType InternOnRun() override;
    };
} // namespace Edit