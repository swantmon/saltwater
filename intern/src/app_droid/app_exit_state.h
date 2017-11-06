
#pragma once

#include "app_droid/app_state.h"

namespace App
{
    class CExitState : public CState
    {

    public:

        static CExitState& GetInstance();

    private:

        CExitState();
        ~CExitState();

    private:

        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();

    };
} // namespace App