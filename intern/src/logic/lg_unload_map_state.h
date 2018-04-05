
//
//  lg_unload_map_state.h
//  logic
//
//  Created by Tobias Schwandt on 06/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

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
    
    ENGINE_API int OnEnter(Base::CTextWriter& _rSerializer);
    ENGINE_API int OnLeave();
    ENGINE_API int OnRun();
} // namespace UnloadMap
} // namespace Lg