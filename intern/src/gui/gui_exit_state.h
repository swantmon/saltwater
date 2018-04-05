
//
//  gui_exit_state.h
//  gui
//
//  Created by Tobias Schwandt on 28/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

namespace Gui
{
namespace Exit
{
    ENGINE_API void OnEnter();
    ENGINE_API void OnLeave();
    ENGINE_API void OnRun();
} // namespace Exit
} // namespace Gui