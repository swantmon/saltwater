
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

        virtual void InternOnEnter();
        virtual void InternOnLeave();
        virtual void InternOnRun();

    };
} // namespace App