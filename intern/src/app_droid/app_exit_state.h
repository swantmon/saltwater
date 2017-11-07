
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

        virtual void InternOnEnter();
        virtual void InternOnLeave();
        virtual void InternOnRun();

    };
} // namespace App