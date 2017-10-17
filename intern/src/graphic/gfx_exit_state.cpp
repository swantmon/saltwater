
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_actor_renderer.h"
#include "graphic/gfx_ar_renderer.h"
#include "graphic/gfx_area_light_manager.h"
#include "graphic/gfx_background_renderer.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_camera_actor_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_exit_state.h"
#include "graphic/gfx_fog_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_area_renderer.h"
#include "graphic/gfx_light_indirect_renderer.h"
#include "graphic/gfx_light_point_renderer.h"
#include "graphic/gfx_light_probe_manager.h"
#include "graphic/gfx_light_sun_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_mesh_actor_manager.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_particle_renderer.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_point_light_manager.h"
#include "graphic/gfx_postfx_hdr_renderer.h"
#include "graphic/gfx_postfx_renderer.h"
#include "graphic/gfx_reflection_renderer.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_selection_renderer.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_shadow_renderer.h"
#include "graphic/gfx_sky_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sun_manager.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_tonemapping_renderer.h"
#include "graphic/gfx_view_manager.h"
#include "graphic/gfx_reconstruction_renderer.h"

using namespace Gfx;

namespace
{
    class CGfxExitState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxExitState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGfxExitState::OnEnter()
    {
        // -----------------------------------------------------------------------------
        // Exit renderer. Now it isn't necessary to do this in a specific direction.
        // -----------------------------------------------------------------------------
        BASE_CONSOLE_STREAMINFO("Gfx> Exit renderer...");

        BackgroundRenderer   ::OnExit();
        DebugRenderer        ::OnExit();
        SelectionRenderer    ::OnExit();
        LightAreaRenderer    ::OnExit();
        ReflectionRenderer   ::OnExit();
        LightPointRenderer   ::OnExit();
        LightSunRenderer     ::OnExit();
        LightIndirectRenderer::OnExit();
        ShadowRenderer       ::OnExit();
        FogRenderer          ::OnExit();
        HistogramRenderer    ::OnExit();
        ActorRenderer        ::OnExit();
        ARRenderer           ::OnExit();
        ParticleRenderer     ::OnExit();
        PostFXHDR            ::OnExit();
        PostFX               ::OnExit();
        TonemappingRenderer  ::OnExit();
        ReconstructionRenderer        ::OnExit();

        BASE_CONSOLE_STREAMINFO("Gfx> Finished exit of renderer.");
        
        // -----------------------------------------------------------------------------
        // Destroy main graphic data
        // -----------------------------------------------------------------------------
        BASE_CONSOLE_STREAMINFO("Gfx> Destroy global constant buffer.");

        Main::DestroyPerFrameConstantBuffers();   

        BASE_CONSOLE_STREAMINFO("Gfx> Global constant buffer destroyed.");
        
        // -----------------------------------------------------------------------------
        // Exit manager
        // -----------------------------------------------------------------------------
        BASE_CONSOLE_STREAMINFO("Gfx> Exit manager...");

        PointLightManager ::OnExit();
        AreaLightManager  ::OnExit();
        LightProbeManager ::OnExit();
        SkyManager        ::OnExit();
        MeshActorManager  ::OnExit();
        CameraActorManager::OnExit();
        SunManager        ::OnExit();
        MeshManager       ::OnExit();
        MaterialManager   ::OnExit();

        TargetSetManager::OnExit();
        ContextManager  ::OnExit();
        StateManager    ::OnExit();
        ShaderManager   ::OnExit();
        BufferManager   ::OnExit();
        TextureManager  ::OnExit();
        SamplerManager  ::OnExit();
        ViewManager     ::OnExit();

        BASE_CONSOLE_STREAMINFO("Gfx> Finished exiting manager.");

        Performance::OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxExitState::OnLeave()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxExitState::OnRun()
    {
        
    }
} // namespace

namespace Gfx
{
namespace Exit
{
    void OnEnter()
    {
        CGfxExitState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGfxExitState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGfxExitState::GetInstance().OnRun();
    }
} // namespace Exit
} // namespace Gfx