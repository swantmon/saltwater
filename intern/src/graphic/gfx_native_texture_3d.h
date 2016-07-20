//
//  gfx_native_texture3d.h
//  graphic
//
//  Created by Tobias Schwandt on 20/07/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_native_types.h"
#include "graphic/gfx_texture_3d.h"

namespace Gfx
{
    class CNativeTexture3D : public CTexture3D
    {
    public:
        
        Gfx::CNativeTextureHandle m_NativeTexture;
        int                       m_NativeUsage;
        int                       m_NativeInternalFormat;
        int                       m_NativeDimension;
    };
} // namespace Gfx
