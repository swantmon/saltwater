
#pragma once

#include "game/game_state.h"

namespace Game
{
    class CIntroState : public CState
    {
        
    public:
        
        static CIntroState& GetInstance();
        
    private:
        
        CIntroState();
        ~CIntroState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace Game