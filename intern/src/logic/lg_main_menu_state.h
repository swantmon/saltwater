//
//  lg_main_menu_state.h
//  logic
//
//  Created by Tobias Schwandt on 26/02/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

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
    
    int OnEnter();
    int OnLeave();
    int OnRun();
} // namespace MainMenu
} // namespace Lg
