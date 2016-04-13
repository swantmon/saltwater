//
//  gfx_native_buffer_set.h
//  graphic
//
//  Created by Tobias Schwandt on 29/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_buffer_set.h"

namespace Gfx
{
    class CNativeBufferSet : public CBufferSet
    {
    public:
        
        Gfx::CNativeBufferHandle m_NativeBufferArrayHandle;
    };
} // namespace Gfx