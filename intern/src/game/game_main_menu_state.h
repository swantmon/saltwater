
#pragma once

#include "game/game_state.h"

namespace Game
{
    class CMainMenuState : public CState
    {
        
    public:
        
        static CMainMenuState& GetInstance();
        
    private:
        
        CMainMenuState();
        ~CMainMenuState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace Game