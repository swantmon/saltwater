//
//  gui_main_menu_state.h
//  gui
//
//  Created by Tobias Schwandt on 26/02/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

namespace Gui
{
    namespace MainMenu
    {
        ENGINE_API void OnEnter();
        ENGINE_API void OnLeave();
        ENGINE_API void OnRun();
    } // namespace MainMenu
} // namespace Gui
