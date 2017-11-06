
#pragma once

#include "app_droid/app_state.h"

namespace App
{
    class CPlayState : public CState
    {

    public:

        static CPlayState& GetInstance();

    private:

        CPlayState();
        ~CPlayState();

    private:

        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();

    };
} // namespace App