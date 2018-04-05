
//
//  lg_start_state.h
//  logic
//
//  Created by Tobias Schwandt on 28/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

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
    
    ENGINE_API int OnEnter();
    ENGINE_API int OnLeave();
    ENGINE_API int OnRun();
} // namespace Start
} // namespace Lg