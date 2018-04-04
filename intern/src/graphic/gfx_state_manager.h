
#pragma once

#include "graphic/gfx_blend_state.h"
#include "graphic/gfx_export.h"
#include "graphic/gfx_depth_stencil_state.h"
#include "graphic/gfx_rasterizer_state.h"
#include "graphic/gfx_render_state.h"

namespace Gfx
{
namespace StateManager
{
    GFX_API void OnStart();
    GFX_API void OnExit();

    GFX_API CBlendStatePtr GetBlendState(unsigned int _Flags);
    GFX_API CDepthStencilStatePtr GetDepthStencilState(unsigned int _Flags);
    GFX_API CRasterizerStatePtr GetRasterizerState(unsigned int _Flags);
    GFX_API CRenderStatePtr GetRenderState(unsigned int _Flags);
} // StateManager
} // namespace Gfx

