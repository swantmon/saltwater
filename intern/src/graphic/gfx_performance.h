
//
//  gfx_performance.h
//  gfx
//
//  Created by Tobias Schwandt on 13/11/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_typedef.h"

namespace Gfx
{
namespace Performance
{
    void BeginEvent(const Base::Char* _pEventName);
    void EndEvent();
} // namespace Performance
} // namespace Gfx