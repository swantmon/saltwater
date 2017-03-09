
#pragma once

#include "editor/edit_state.h"

#include "editor_port/edit_message.h"

namespace Edit
{
    class CIntroState : public CState
    {
        
    public:
        
        static CIntroState& GetInstance();

    private:

        EStateType m_CurrentState;

    private:
        
        CIntroState();
        ~CIntroState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();

    private:

        void OnNewMap(Edit::CMessage& _rMessage);
        void OnLoadMap(Edit::CMessage& _rMessage);
    };
} // namespace Edit