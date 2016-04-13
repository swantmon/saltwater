
#pragma once

#include "game/game_state.h"

namespace Game
{
    class CUnloadMapState : public CState
    {
        
    public:
        
        static CUnloadMapState& GetInstance();
        
    private:
        
        CUnloadMapState();
        ~CUnloadMapState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace Game