
//
//  lg_unload_map_state.h
//  logic
//
//  Created by Tobias Schwandt on 06/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Base
{
    class CTextWriter;
} // namespace Base

namespace Lg
{
namespace UnloadMap
{
    struct SResult
    {
        enum Enum
        {
            UnloadMap,
            Exit
        };
    };
    
    int OnEnter(Base::CTextWriter& _rSerializer);
    int OnLeave();
    int OnRun();
} // namespace UnloadMap
} // namespace Lg