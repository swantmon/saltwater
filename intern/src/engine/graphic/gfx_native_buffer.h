//
//  gfx_native_buffer.h
//  graphic
//
//  Created by Tobias Schwandt on 29/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/graphic/gfx_buffer.h"
#include "engine/graphic/gfx_native_types.h"

namespace Gfx
{
    class CNativeBuffer : public CBuffer
    {
    public:
        
        Gfx::CNativeBufferHandle m_NativeBuffer;     ///< Indicates the native buffer handle
        int m_NativeBinding;                         ///< Indicates the native binding
        int m_NativeUsage;                           ///< Indicates the native usage
    };
} // namespace Gfx