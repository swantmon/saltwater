
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

        virtual void InternOnEnter();
        virtual void InternOnLeave();
        virtual void InternOnRun();

    };
} // namespace App