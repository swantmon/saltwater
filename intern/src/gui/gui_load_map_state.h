
//
//  gui_load_map_state.h
//  gui
//
//  Created by Tobias Schwandt on 06/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/engine_config.h"

namespace Base
{
    class CTextReader;
} // namespace Base

namespace Gui
{
namespace LoadMap
{
    ENGINE_API void OnEnter(Base::CTextReader& _rSerializer);
    ENGINE_API void OnLeave();
    ENGINE_API void OnRun();
} // namespace LoadMap
} // namespace Gui