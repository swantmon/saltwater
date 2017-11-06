
#pragma once

#include "app_droid/app_state.h"

namespace App
{
    class CIntroState : public CState
    {

    public:

        static CIntroState& GetInstance();

    private:

        CIntroState();
        ~CIntroState();

    private:

        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();

    };
} // namespace App