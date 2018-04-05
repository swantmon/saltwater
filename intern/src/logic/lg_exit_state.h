
//
//  lg_exit_state.h
//  logic
//
//  Created by Tobias Schwandt on 28/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

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
    
    ENGINE_API int OnEnter();
    ENGINE_API int OnLeave();
    ENGINE_API int OnRun();
} // namespace Exit
} // namespace Lg