
#pragma once

#include "game/game_state.h"

namespace Game
{
    class CPlayState : public CState
    {
        
    public:
        
        static CPlayState& GetInstance();
        
    private:
        
        CPlayState();
        ~CPlayState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace Game