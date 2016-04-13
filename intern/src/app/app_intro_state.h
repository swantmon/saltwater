
//
//  app_intro_state.h
//  app
//
//  Created by Tobias Schwandt on 18/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "app/app_state.h"

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
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace App