
//
//  lg_start_state.h
//  logic
//
//  Created by Tobias Schwandt on 28/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Lg
{
namespace Start
{
    struct SResult
    {
        enum Enum
        {
            Intro,
            Start
        };
    };
    
    int OnEnter();
    int OnLeave();
    int OnRun();
} // namespace Start
} // namespace Lg