
//
//  gui_debug_state.h
//  gui
//
//  Created by Tobias Schwandt on 03/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

namespace Gui
{
namespace Debug
{
    ENGINE_API void OnEnter();
    ENGINE_API void OnLeave();
    ENGINE_API void OnRun();
} // namespace Debug
} // namespace Gui