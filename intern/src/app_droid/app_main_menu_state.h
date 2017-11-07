
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

        virtual void InternOnEnter();
        virtual void InternOnLeave();
        virtual void InternOnRun();

    };
} // namespace App