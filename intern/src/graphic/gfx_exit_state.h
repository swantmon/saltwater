
//
//  gfx_exit_state.h
//  graphic
//
//  Created by Tobias Schwandt on 18/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

namespace Gfx
{
namespace Exit
{
    ENGINE_API void OnEnter();
    ENGINE_API void OnLeave();
    ENGINE_API void OnRun();
} // namespace Exit
} // namespace Gfx