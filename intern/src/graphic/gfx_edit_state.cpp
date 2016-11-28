
#include "graphic/gfx_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_actor_renderer.h"
#include "graphic/gfx_ar_mesh_facet_manager.h"
#include "graphic/gfx_ar_renderer.h"
#include "graphic/gfx_background_renderer.h"
#include "graphic/gfx_camera_facet_manager.h"
#include "graphic/gfx_edit_state.h"
#include "graphic/gfx_fog_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_area_renderer.h"
#include "graphic/gfx_light_point_renderer.h"
#include "graphic/gfx_light_probe_manager.h"
#include "graphic/gfx_light_sun_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_facet_manager.h"
#include "graphic/gfx_particle_renderer.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_point_light_manager.h"
#include "graphic/gfx_postfx_hdr_renderer.h"
#include "graphic/gfx_postfx_renderer.h"
#include "graphic/gfx_reflection_renderer.h"
#include "graphic/gfx_selection_renderer.h"
#include "graphic/gfx_shadow_renderer.h"
#include "graphic/gfx_sky_manager.h"
#include "graphic/gfx_sun_manager.h"
#include "graphic/gfx_tonemapping_renderer.h"

using namespace Gfx;

namespace
{
    class CGfxEditState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxEditState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
    };
} // namespace

namespace
{
    void CGfxEditState::OnEnter()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEditState::OnLeave()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEditState::OnRun()
    {
        // -----------------------------------------------------------------------------
        // Begin frame
        // -----------------------------------------------------------------------------
        Main::BeginFrame();

        // -----------------------------------------------------------------------------
        // Update graphic entities
        // -----------------------------------------------------------------------------
        ARMeshFacetManager::Update();
        MeshFacetManager  ::Update();
        CameraFacetManager::Update();
        SunManager        ::Update();
        SkyManager        ::Update();
        LightProbeManager ::Update();
        PointLightManager ::Update();

        // -----------------------------------------------------------------------------
        // Update graphic
        // -----------------------------------------------------------------------------
        Main::UploadPerFrameConstantBuffers();
        
        // -----------------------------------------------------------------------------
        // Update renderer to prepare for rendering
        // -----------------------------------------------------------------------------
        ARRenderer         ::Update();
        ActorRenderer      ::Update();
        FogRenderer        ::Update();
        ShadowRenderer     ::Update();
        LightAreaRenderer  ::Update();
        LightPointRenderer ::Update();   
        LightSunRenderer   ::Update();   
        ReflectionRenderer ::Update();    
        BackgroundRenderer ::Update();
        HistogramRenderer  ::Update(); 
        TonemappingRenderer::Update();
        PostFXHDR          ::Update();
        PostFX             ::Update();
        SelectionRenderer  ::Update();

        // -----------------------------------------------------------------------------
        // Creation Pass
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Creation Pass");

        ARRenderer   ::Render();
        ActorRenderer::Render();

        Performance::EndEvent();
        
        // -----------------------------------------------------------------------------
        // Lighting Pass
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Lighting Pass");

        ShadowRenderer    ::Render();
        LightSunRenderer  ::Render();
        LightAreaRenderer ::Render();
        LightPointRenderer::Render();
        BackgroundRenderer::Render();
        ReflectionRenderer::Render();
        FogRenderer       ::Render();

        HistogramRenderer::Render();

        PostFXHDR::Render();

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Shading Pass
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Shading Pass");
        
        TonemappingRenderer::Render();

        PostFX::Render();

        SelectionRenderer::Render();

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // End
        // -----------------------------------------------------------------------------
        Main::EndFrame();
    }
} // namespace

namespace Gfx
{
namespace Edit
{
    void OnEnter()
    {
        CGfxEditState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGfxEditState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGfxEditState::GetInstance().OnRun();
    }
} // namespace Edit
} // namespace Gfx