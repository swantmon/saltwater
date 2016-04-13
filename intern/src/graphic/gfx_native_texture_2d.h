//
//  gfx_native_texture2d.h
//  graphic
//
//  Created by Tobias Schwandt on 26/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_native_types.h"
#include "graphic/gfx_texture_2d.h"

namespace Gfx
{
    class CNativeTexture2D : public CTexture2D
    {
    public:
        
        Gfx::CNativeTextureHandle m_NativeTexture;
        int                       m_NativeUsage;
        int                       m_NativeInternalFormat;
        int                       m_NativeDimension;
    };
} // namespace Gfx
