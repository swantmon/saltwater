
//
//  lg_edit_state.h
//  logic
//
//  Created by Tobias Schwandt on 20/04/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

namespace Lg
{
namespace Edit
{
    struct SResult
    {
        enum Enum
        {
            Edit,
            Exit
        };;
    };
    
    ENGINE_API int OnEnter();
    ENGINE_API int OnLeave();
    ENGINE_API int OnRun();
} // namespace Edit
} // namespace Lg