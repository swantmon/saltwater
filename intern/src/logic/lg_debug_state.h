
//
//  lg_debug_state.h
//  logic
//
//  Created by Tobias Schwandt on 03/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

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
    
    ENGINE_API int OnEnter();
    ENGINE_API int OnLeave();
    ENGINE_API int OnRun();
} // namespace Debug
} // namespace Lg