
#pragma once

#include "editor/edit_state.h"

#include "editor_port/edit_message.h"

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

    private:

        void OnExit(Edit::CMessage& _rMessage);
        void OnEdit(Edit::CMessage& _rMessage);
        
    };
} // namespace Edit