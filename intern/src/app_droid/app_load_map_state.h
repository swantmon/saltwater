
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

        virtual void InternOnEnter();
        virtual void InternOnLeave();
        virtual void InternOnRun();

    };
} // namespace App