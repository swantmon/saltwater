
//
//  gui_unload_map_state.h
//  gui
//
//  Created by Tobias Schwandt on 06/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Base
{
    class CTextWriter;
} // namespace Base


namespace Gui
{
namespace UnloadMap
{
    void OnEnter(Base::CTextWriter& _rSerializer);
    void OnLeave();
    void OnRun();
} // namespace UnloadMap
} // namespace Gui