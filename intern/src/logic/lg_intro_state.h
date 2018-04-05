
//
//  lg_intro_state.h
//  logic
//
//  Created by Tobias Schwandt on 28/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

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
    
    ENGINE_API int OnEnter();
    ENGINE_API int OnLeave();
    ENGINE_API int OnRun();
} // namespace Intro
} // namespace Lg