//
//  gfx_depth_description.h
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
    // State              | Default Value @ DX11             |
    // -----------------------------------------------------------------------------
    // DepthEnable        | TRUE                             |
    // DepthWriteMask     | D3D11_DEPTH_WRITE_MASK_ALL       |
    // DepthFunc          | D3D11_COMPARISON_LESS            |
    // StencilEnable      | FALSE                            |
    // StencilReadMask    | D3D11_DEFAULT_STENCIL_READ_MASK  |
    // StencilWriteMask   | D3D11_DEFAULT_STENCIL_WRITE_MASK |
    // StencilFunc        | D3D11_STENCIL_OP_KEEP            |
    // StencilDepthFailOp | D3D11_STENCIL_OP_KEEP            |
    // StencilPassOp      | D3D11_STENCIL_OP_KEEP            |
    // StencilFailOp      | D3D11_COMPARISON_ALWAYS          |
    // -----------------------------------------------------------------------------
    
    struct SDepthOperationDescription
    {
        int StencilFailOp;
        int StencilDepthFailOp;
        int StencilPassOp;
        int StencilFunc;
    };
    
    struct SDepthDescription
    {
        bool                       DepthEnable;
        int                        DepthWriteMask;
        int                        DepthFunc;
        bool                       StencilEnable;
        Base::U8                   StencilReadMask;
        Base::U8                   StencilWriteMask;
        SDepthOperationDescription FrontFace;
        SDepthOperationDescription BackFace;
    };
} // namespace Gfx