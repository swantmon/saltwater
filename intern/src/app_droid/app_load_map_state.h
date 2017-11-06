
#pragma once

#include "app_droid/app_state.h"

namespace App
{
    class CLoadMapState : public CState
    {

    public:

        static CLoadMapState& GetInstance();

    private:

        CLoadMapState();
        ~CLoadMapState();

    private:

        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();

    };
} // namespace App