//
//  gfx_rasterizer_description.h
//  graphic
//
//  Created by Tobias Schwandt on 24/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Gfx
{
    // -----------------------------------------------------------------------------
    // State                 | Default Value    |
    // -----------------------------------------------------------------------------
    // FillMode              | D3D11_FILL_SOLID |
    // CullMode              | D3D11_CULL_BACK  |
    // FrontCounterClockwise | FALSE            |
    // DepthBias             | 0                |
    // SlopeScaledDepthBias  | 0.0f             |
    // DepthBiasClamp        | 0.0f             |
    // DepthClipEnable       | TRUE             |
    // ScissorEnable         | FALSE            |
    // MultisampleEnable     | FALSE            |
    // AntialiasedLineEnable | FALSE            |
    // -----------------------------------------------------------------------------
    
    struct SRasterizerDescription
    {
        bool  CullEnable;
        int   FillMode;
        int   CullMode;
        int   FrontCounterClockwise;
        int   DepthBias;
        float DepthBiasClamp;
        float SlopeScaledDepthBias;
        bool  DepthClipEnable;
        bool  ScissorEnable;
        bool  MultisampleEnable;
        bool  AntialiasedLineEnable;
    };
} // namespace Gfx