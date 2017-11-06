
#pragma once

#include "app_droid/app_state.h"

namespace App
{
    class CUnloadMapState : public CState
    {

    public:

        static CUnloadMapState& GetInstance();

    private:

        CUnloadMapState();
        ~CUnloadMapState();

    private:

        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();

    };
} // namespace App