
//
//  app_start_state.h
//  app
//
//  Created by Tobias Schwandt on 18/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "app/app_state.h"

namespace App
{
    class CStartState : public CState
    {
        
    public:
        
        static CStartState& GetInstance();
        
    public:
        
        void SetResolution(int _Width, int _Height);
        
    private:
        
        CStartState();
        ~CStartState();
        
    private:
        
        int   m_Width;
        int   m_Height;
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace App