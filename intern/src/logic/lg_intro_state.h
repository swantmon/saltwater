
//
//  lg_intro_state.h
//  logic
//
//  Created by Tobias Schwandt on 28/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Lg
{
namespace Intro
{
    struct SResult
    {
        enum Enum
        {
            Intro,
            MainMenu,
        };
    };
    
    int OnEnter();
    int OnLeave();
    int OnRun();
} // namespace Intro
} // namespace Lg