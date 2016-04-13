
//
//  lg_load_map_state.h
//  logic
//
//  Created by Tobias Schwandt on 06/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Base
{
    class CTextReader;
} // namespace Base

namespace Lg
{
namespace LoadMap
{
    struct SResult
    {
        enum Enum
        {
            LoadMap,
            Play,
        };
    };
    
    int OnEnter(Base::CTextReader& _rSerializer);
    int OnLeave();
    int OnRun();
} // namespace LoadMap
} // namespace Lg