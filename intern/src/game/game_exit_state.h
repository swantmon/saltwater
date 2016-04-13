
#pragma once

#include "game/game_state.h"

namespace Game
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
} // namespace Game