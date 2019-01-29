
#pragma once

#include "editor/edit_state.h"

namespace Edit
{
    class CUnloadMapState : public CState
    {
        
    public:
        
        static CUnloadMapState& GetInstance();

    public:

        void SetNextState(CState::EStateType _NextState);

    private:

        CState::EStateType m_NextState;
        
    private:
        
        CUnloadMapState();
        ~CUnloadMapState();

    private:

        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();

    };
} // namespace Edit