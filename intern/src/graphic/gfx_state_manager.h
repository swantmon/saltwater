
#pragma once

#include "graphic/gfx_blend_state.h"
#include "graphic/gfx_depth_stencil_state.h"
#include "graphic/gfx_rasterizer_state.h"
#include "graphic/gfx_render_state.h"

namespace Gfx
{
namespace StateManager
{
    void OnStart();
    void OnExit();

    CBlendStatePtr GetBlendState(unsigned int _Flags);
    CDepthStencilStatePtr GetDepthStencilState(unsigned int _Flags);
    CRasterizerStatePtr GetRasterizerState(unsigned int _Flags);
    CRenderStatePtr GetRenderState(unsigned int _Flags);
} // StateManager
} // namespace Gfx

