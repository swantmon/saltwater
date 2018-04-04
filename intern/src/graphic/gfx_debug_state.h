
//
//  gfx_debug_state.h
//  graphic
//
//  Created by Tobias Schwandt on 03/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_export.h"

namespace Gfx
{
namespace Debug
{
    GFX_API void OnEnter();
    GFX_API void OnLeave();
    GFX_API void OnRun();
} // namespace Debug
} // namespace Gfx