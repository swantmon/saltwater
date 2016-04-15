
#pragma once

#include "editor/edit_state.h"

namespace Edit
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
} // namespace Edit