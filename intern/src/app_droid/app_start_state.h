
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

        virtual void InternOnEnter();
        virtual void InternOnLeave();
        virtual void InternOnRun();

    };
} // namespace App