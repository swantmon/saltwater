
#pragma once

#include "engine/engine_config.h"

#include "engine/graphic/gfx_blend_state.h"
#include "engine/graphic/gfx_depth_stencil_state.h"
#include "engine/graphic/gfx_rasterizer_state.h"
#include "engine/graphic/gfx_render_state.h"

namespace Gfx
{
namespace StateManager
{
    void OnStart();
    void OnExit();

    ENGINE_API CBlendStatePtr GetBlendState(unsigned int _Flags);
    ENGINE_API CDepthStencilStatePtr GetDepthStencilState(unsigned int _Flags);
    ENGINE_API CRasterizerStatePtr GetRasterizerState(unsigned int _Flags);
    ENGINE_API CRenderStatePtr GetRenderState(unsigned int _Flags);
} // StateManager
} // namespace Gfx

