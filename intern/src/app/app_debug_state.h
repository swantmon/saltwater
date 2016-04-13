
//
//  app_debug_state.h
//  app
//
//  Created by Tobias Schwandt on 03/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "app/app_state.h"

namespace App
{
    class CDebugState : public CState
    {
        
    public:
        
        static CDebugState& GetInstance();
        
    private:
        
        CDebugState();
        ~CDebugState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace App