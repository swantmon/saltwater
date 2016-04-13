//
//  app_main_menu_state.h
//  app
//
//  Created by Tobias Schwandt on 26/02/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "app/app_state.h"

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
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace App