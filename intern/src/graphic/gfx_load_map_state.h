
//
//  gfx_load_map_state.h
//  graphic
//
//  Created by Tobias Schwandt on 06/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_export.h"

namespace Base
{
    class CTextReader;
} // namespace Base

namespace Gfx
{
namespace LoadMap
{
    GFX_API void OnEnter(Base::CTextReader& _rSerializer);
    GFX_API void OnLeave();
    GFX_API void OnRun();
} // namespace LoadMap
} // namespace Gfx