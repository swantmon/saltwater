
#pragma once

#include "game/game_state.h"

namespace Game
{
    class CStartState : public CState
    {
        
    public:
        
        static CStartState& GetInstance();
        
    public:
        
        void SetResolution(int _Width, int _Height);
        
    private:
        
        CStartState();
        ~CStartState();
        
    private:
        
        int   m_Width;
        int   m_Height;
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace Game