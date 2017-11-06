
#pragma once

#include "app_droid/app_state.h"

namespace App
{
    class CStartState : public CState
    {

    public:

        static CStartState& GetInstance();

    private:

        CStartState();
        ~CStartState();

    private:

        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();

    };
} // namespace App