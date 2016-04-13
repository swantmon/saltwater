
//
//  lg_play_state.h
//  logic
//
//  Created by Tobias Schwandt on 28/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Lg
{
namespace Play
{
    struct SResult
    {
        enum Enum
        {
            Play,
            Debug,
            Exit
        };;
    };
    
    int OnEnter();
    int OnLeave();
    int OnRun();
} // namespace Play
} // namespace Lg