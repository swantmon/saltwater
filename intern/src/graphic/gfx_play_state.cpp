
#include "graphic/gfx_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_actor_manager.h"
#include "graphic/gfx_actor_renderer.h"
#include "graphic/gfx_fog_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_area_renderer.h"
#include "graphic/gfx_light_manager.h"
#include "graphic/gfx_light_probe_manager.h"
#include "graphic/gfx_light_point_renderer.h"
#include "graphic/gfx_light_sun_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_ar_renderer.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_particle_renderer.h"
#include "graphic/gfx_play_state.h"
#include "graphic/gfx_postfx_renderer.h"
#include "graphic/gfx_postfx_hdr_renderer.h"
#include "graphic/gfx_reflection_renderer.h"
#include "graphic/gfx_shadow_renderer.h"
#include "graphic/gfx_sky_manager.h"
#include "graphic/gfx_sky_renderer.h"
#include "graphic/gfx_tonemapping_renderer.h"

using namespace Gfx;

namespace
{
    class CGfxPlayState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxPlayState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
    };
} // namespace

namespace
{
    void CGfxPlayState::OnEnter()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPlayState::OnLeave()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPlayState::OnRun()
    {
        // -----------------------------------------------------------------------------
        // Begin frame
        // -----------------------------------------------------------------------------
        Main::BeginFrame();

        // -----------------------------------------------------------------------------
        // Update graphic entities
        // -----------------------------------------------------------------------------
        ActorManager     ::Update();
        LightManager     ::Update();
        SkyManager       ::Update();
        LightProbeManager::Update();

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
        SkyRenderer        ::Update();
        HistogramRenderer  ::Update(); 
        TonemappingRenderer::Update();
        PostFXHDR          ::Update();
        PostFX             ::Update();

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
        SkyRenderer       ::Render();
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

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // End
        // -----------------------------------------------------------------------------
        Main::EndFrame();
    }
} // namespace

namespace Gfx
{
namespace Play
{
    void OnEnter()
    {
        CGfxPlayState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGfxPlayState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGfxPlayState::GetInstance().OnRun();
    }
} // namespace Play
} // namespace Gfx