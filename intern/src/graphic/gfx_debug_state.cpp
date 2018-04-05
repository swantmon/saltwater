
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_entity.h"
#include "data/data_map.h"

#include "graphic/gfx_ar_renderer.h"
#include "graphic/gfx_area_light_manager.h"
#include "graphic/gfx_background_renderer.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_debug_state.h"
#include "graphic/gfx_fog_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_area_renderer.h"
#include "graphic/gfx_light_indirect_renderer.h"
#include "graphic/gfx_light_point_renderer.h"
#include "graphic/gfx_light_probe_manager.h"
#include "graphic/gfx_light_sun_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_renderer.h"
#include "graphic/gfx_particle_renderer.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_point_light_manager.h"
#include "graphic/gfx_postfx_hdr_renderer.h"
#include "graphic/gfx_postfx_renderer.h"
#include "graphic/gfx_reflection_renderer.h"
#include "graphic/gfx_render_context.h"
#include "graphic/gfx_shadow_renderer.h"
#include "graphic/gfx_sky_manager.h"
#include "graphic/gfx_sun_manager.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_tonemapping_renderer.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxDebugState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxDebugState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGfxDebugState::OnEnter()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugState::OnLeave()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugState::OnRun()
    {
        Main::BeginFrame();

        SunManager        ::Update();
        SkyManager        ::Update();
        LightProbeManager ::Update();
        PointLightManager ::Update();
        AreaLightManager  ::Update();

        Main::UploadPerFrameConstantBuffers();
        
        // -----------------------------------------------------------------------------
        // Update renderer to prepare for rendering
        // -----------------------------------------------------------------------------
        ARRenderer           ::Update();
        MeshRenderer        ::Update();   
        FogRenderer          ::Update();
        ShadowRenderer       ::Update();
        LightAreaRenderer    ::Update();
        LightPointRenderer   ::Update();  
        LightSunRenderer     ::Update();   
        LightIndirectRenderer::Update();
        ReflectionRenderer   ::Update();    
        BackgroundRenderer   ::Update();
        HistogramRenderer    ::Update(); 
        TonemappingRenderer  ::Update();
        PostFXHDR            ::Update();
        PostFX               ::Update();

        // -----------------------------------------------------------------------------
        // Add debug outputs
        // -----------------------------------------------------------------------------
        DebugRenderer::DrawText("DEBUGGING!", glm::vec2(0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 20);

        Gfx::DebugRenderer::DrawGizmo(true);
        
        // -----------------------------------------------------------------------------
        // Creation Pass
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Creation Pass");

        MeshRenderer::Render(); 
        ARRenderer   ::Render();

        Performance::EndEvent();
        
        // -----------------------------------------------------------------------------
        // Lighting Pass
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Lighting Pass");

        ShadowRenderer       ::Render();
        LightSunRenderer     ::Render();
        LightAreaRenderer    ::Render();
        LightPointRenderer   ::Render();
        LightIndirectRenderer::Render();
        ReflectionRenderer   ::Render();
        BackgroundRenderer   ::Render();
        FogRenderer          ::Render();

        HistogramRenderer::Render();

        PostFXHDR::Render();

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Shading Pass
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Shading Pass");
        
        TonemappingRenderer::Render();

        LightAreaRenderer::RenderBulbs();

        Performance::BeginEvent("Debugging Pass");

        DebugRenderer::Render();

        Performance::EndEvent();

        PostFX::Render();

        Performance::EndEvent();

        Main::EndFrame();
    }
} // namespace

namespace Gfx
{
namespace Debug
{
    void OnEnter()
    {
        CGfxDebugState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGfxDebugState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGfxDebugState::GetInstance().OnRun();
    }
} // namespace Debug
} // namespace Gfx