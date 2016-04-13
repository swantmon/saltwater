
//
//  lg_exit_state.h
//  logic
//
//  Created by Tobias Schwandt on 28/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Lg
{
namespace Exit
{
    struct SResult
    {
        enum Enum
        {
            Exit,
        };
    };
    
    int OnEnter();
    int OnLeave();
    int OnRun();
} // namespace Exit
} // namespace Lg