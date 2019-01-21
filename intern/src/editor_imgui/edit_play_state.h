
#pragma once

#include "editor_imgui/edit_state.h"

namespace Edit
{
    class CPlayState : public CState
    {
        
    public:
        
        static CPlayState& GetInstance();

    private:

        CState::EStateType m_Action;
        
    private:
        
        CPlayState();
        ~CPlayState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
    };
} // namespace Edit