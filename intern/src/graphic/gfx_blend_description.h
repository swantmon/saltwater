//
//  gfx_blend_description.h
//  graphic
//
//  Created by Tobias Schwandt on 24/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_typedef.h"

namespace Gfx
{
    // -----------------------------------------------------------------------------
    // State                                 | Default Value @ DX11         |
    // -----------------------------------------------------------------------------
    // AlphaToCoverageEnable                 | FALSE                        |
    // IndependentBlendEnable                | FALSE                        |
    // RenderTarget[0].BlendEnable           | FALSE                        |
    // RenderTarget[0].SrcBlend              | D3D11_BLEND_ONE              |
    // RenderTarget[0].DestBlend             | D3D11_BLEND_AxisZERO             |
    // RenderTarget[0].BlendOp               | D3D11_BLEND_OP_ADD           |
    // RenderTarget[0].SrcBlendAlpha         | D3D11_BLEND_ONE              |
    // RenderTarget[0].DestBlendAlpha        | D3D11_BLEND_AxisZERO             |
    // RenderTarget[0].BlendOpAlpha          | D3D11_BLEND_OP_ADD           |
    // RenderTarget[0].RenderTargetWriteMask | D3D11_COLOR_WRITE_ENABLE_ALL |
    // -----------------------------------------------------------------------------
    
    static const unsigned int s_MaxNumberOfRendertargets = 8;
    
    struct SRenderTargetBlendDescription
    {
        bool     BlendEnable;
        int      SrcBlend;
        int      DestBlend;
        int      BlendOp;
        int      SrcBlendAlpha;
        int      DestBlendAlpha;
        int      BlendOpAlpha;
        Base::U8 RenderTargetWriteMask;
    };
    
    struct SBlendDescription
    {
        bool                          AlphaToCoverageEnable;
        bool                          IndependentBlendEnable;
        SRenderTargetBlendDescription RenderTarget[s_MaxNumberOfRendertargets];
    };
} // namespace Gfx
