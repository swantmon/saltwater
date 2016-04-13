
//
//  lg_debug_state.h
//  logic
//
//  Created by Tobias Schwandt on 03/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Lg
{
namespace Debug
{
    struct SResult
    {
        enum Enum
        {
            Debug,
            Play
        };;
    };
    
    int OnEnter();
    int OnLeave();
    int OnRun();
} // namespace Debug
} // namespace Lg