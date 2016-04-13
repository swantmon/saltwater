
#pragma once

#include "game/game_state.h"

namespace Game
{
    class CDebugState : public CState
    {
        
    public:
        
        static CDebugState& GetInstance();
        
    private:
        
        CDebugState();
        ~CDebugState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace Game