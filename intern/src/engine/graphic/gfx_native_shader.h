//
//  gfx_native_shader.h
//  graphic
//
//  Created by Tobias Schwandt on 23/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "engine/graphic/gfx_buffer_set.h"
#include "engine/graphic/gfx_native_types.h"
#include "engine/graphic/gfx_shader.h"

namespace Gfx
{
    class CNativeShader : public CShader
    {
    public:
        
        Gfx::CNativeShaderHandle m_NativeShader;
    };
} // namespace Gfx