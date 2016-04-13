
//
//  gui_load_map_state.h
//  gui
//
//  Created by Tobias Schwandt on 06/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Base
{
    class CTextReader;
} // namespace Base

namespace Gui
{
namespace LoadMap
{
    void OnEnter(Base::CTextReader& _rSerializer);
    void OnLeave();
    void OnRun();
} // namespace LoadMap
} // namespace Gui