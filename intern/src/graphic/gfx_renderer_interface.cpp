
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_actor_renderer.h"
#include "graphic/gfx_ar_renderer.h"
#include "graphic/gfx_background_renderer.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_fog_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_area_renderer.h"
#include "graphic/gfx_light_indirect_renderer.h"
#include "graphic/gfx_light_point_renderer.h"
#include "graphic/gfx_light_sun_renderer.h"
#include "graphic/gfx_particle_renderer.h"
#include "graphic/gfx_postfx_hdr_renderer.h"
#include "graphic/gfx_postfx_renderer.h"
#include "graphic/gfx_reflection_renderer.h"
#include "graphic/gfx_renderer_interface.h"
#include "graphic/gfx_shadow_renderer.h"
#include "graphic/gfx_tonemapping_renderer.h"
#include "graphic/gfx_voxel_renderer.h"

namespace Gfx
{
namespace Renderer
{
    void ReloadRenderer()
    {
        ARRenderer           ::OnReload();
        ActorRenderer        ::OnReload();
        FogRenderer          ::OnReload();
        ShadowRenderer       ::OnReload();
        LightAreaRenderer    ::OnReload();
        LightPointRenderer   ::OnReload();
        LightSunRenderer     ::OnReload();
        LightIndirectRenderer::OnReload();
        ReflectionRenderer   ::OnReload();
        BackgroundRenderer   ::OnReload();
        HistogramRenderer    ::OnReload();
        TonemappingRenderer  ::OnReload();
        PostFXHDR            ::OnReload();
        PostFX               ::OnReload();
        VoxelRenderer        ::OnReload();
    }
} // namespace Renderer
} // namespace Gfx
