//
//  app_state.h
//  app
//
//  Created by Tobias Schwandt on 18/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace App
{
    class CState
    {
        
    public:
        
        enum EStateType
        {
            Start,
            Intro,
            LoadMap,
            MainMenu,
            Play,
            Debug,
            UnloadMap,
            Exit,
            NumberOfStateTypes,
            UndefinedStateType = -1
        };
        
    public:
        
        inline CState::EStateType OnEnter();
        inline CState::EStateType OnLeave();
        inline CState::EStateType OnRun();

    private:
        
        virtual CState::EStateType InternOnEnter() = 0;
        virtual CState::EStateType InternOnLeave() = 0;
        virtual CState::EStateType InternOnRun()   = 0;
        
    };
} // namespace App

namespace App
{
    CState::EStateType CState::OnEnter()
    {
        return InternOnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CState::OnLeave()
    {
        return InternOnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CState::OnRun()
    {
        return InternOnRun();
    }
} // namespace App