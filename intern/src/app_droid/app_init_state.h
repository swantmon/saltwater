
#pragma once

#include "app_droid/app_state.h"

namespace App
{
    class CInitState : public CState
    {
    public:

        static CInitState& GetInstance();

    private:

        CInitState();
        ~CInitState();

    private:

        virtual void InternOnEnter();
        virtual void InternOnLeave();
        virtual void InternOnRun();

    };
} // namespace App