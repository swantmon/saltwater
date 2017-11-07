
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

        virtual void InternOnEnter();
        virtual void InternOnLeave();
        virtual void InternOnRun();

    };
} // namespace App