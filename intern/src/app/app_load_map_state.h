
//
//  app_load_map_state.h
//  app
//
//  Created by Tobias Schwandt on 06/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "app/app_state.h"

namespace App
{
    class CLoadMapState : public CState
    {
        
    public:
        
        static CLoadMapState& GetInstance();
        
    public:
        
        void SetMapfile(const char* _pFilename);
        const char* GetMapfile() const;
        
    private:
        
        const char* m_pMapfile;
        
    private:
        
        CLoadMapState();
        ~CLoadMapState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace App