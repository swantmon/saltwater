
#pragma once

#include "game/game_state.h"

namespace Game
{
    class CLoadMapState : public CState
    {
        
    public:
        
        static CLoadMapState& GetInstance();
        
    public:
        
        void SetMapfile(const char* _pFilename);
        const char* GetMapfile() const;
        
    private:
        
        const char* m_pMapfile;
        
    private:
        
        CLoadMapState();
        ~CLoadMapState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace Game