
#pragma once

#include "game/game_state.h"

namespace Game
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
} // namespace Game