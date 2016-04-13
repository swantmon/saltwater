//
//  gfx_native_target_set.h
//  graphic
//
//  Created by Tobias Schwandt on 15/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_native_types.h"
#include "graphic/gfx_target_set.h"

namespace Gfx
{
    class CNativeTargetSet : public CTargetSet
    {
    public:
        
        Gfx::CNativeTargetSetHandle m_NativeTargetSet;
    };
} // namespace Gfx
