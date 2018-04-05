//
//  lg_main_menu_state.h
//  logic
//
//  Created by Tobias Schwandt on 26/02/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

namespace Lg
{
namespace MainMenu
{
    struct SResult
    {
        enum Enum
        {
            MainMenu,
            LoadMap,
            Exit
        };;
    };
    
    ENGINE_API int OnEnter();
    ENGINE_API int OnLeave();
    ENGINE_API int OnRun();
} // namespace MainMenu
} // namespace Lg
