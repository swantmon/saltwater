
#pragma once

#include "editor/edit_state.h"

#include "editor_port/edit_message.h"

namespace Edit
{
    class CEditState : public CState
    {
        
    public:
        
        static CEditState& GetInstance();

    private:

        CState::EStateType m_Action;

    private:
        
        CEditState();
        ~CEditState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    private:

        void OnExit(Edit::CMessage& _rMessage);
    };
} // namespace Edit