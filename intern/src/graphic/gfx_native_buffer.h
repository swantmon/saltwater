//
//  gfx_native_buffer.h
//  graphic
//
//  Created by Tobias Schwandt on 29/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_native_types.h"

namespace Gfx
{
    class CNativeBuffer : public CBuffer
    {
    public:
        
        Gfx::CNativeBufferHandle m_NativeBuffer;     ///< Indicates the current native buffer handle
        int m_NativeBinding;                         ///< Indicates the current native binding
        int m_NativeUsage;                           ///< Indicates the current native usage
    };
} // namespace Gfx