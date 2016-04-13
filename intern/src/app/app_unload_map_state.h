
//
//  app_unload_map_state.h
//  app
//
//  Created by Tobias Schwandt on 06/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "app/app_state.h"

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
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace App