
//
//  lg_edit_state.h
//  logic
//
//  Created by Tobias Schwandt on 20/04/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

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
    
    int OnEnter();
    int OnLeave();
    int OnRun();
} // namespace Edit
} // namespace Lg