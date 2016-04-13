
//
//  app_exit_state.h
//  app
//
//  Created by Tobias Schwandt on 18/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "app/app_state.h"

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
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace App