
//
//  gfx_play_state.h
//  graphic
//
//  Created by Tobias Schwandt on 18/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

namespace Gfx
{
namespace Play
{
    ENGINE_API void OnEnter();
    ENGINE_API void OnLeave();
    ENGINE_API void OnRun();
} // namespace Play
} // namespace Gfx