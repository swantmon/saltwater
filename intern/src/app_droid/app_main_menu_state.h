
#pragma once

#include "app_droid/app_state.h"

namespace App
{
    class CMainMenuState : public CState
    {

    public:

        static CMainMenuState& GetInstance();

    private:

        CMainMenuState();
        ~CMainMenuState();

    private:

        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();

    };
} // namespace App