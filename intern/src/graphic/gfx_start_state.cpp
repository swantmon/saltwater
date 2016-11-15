
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_actor_manager.h"
#include "graphic/gfx_actor_renderer.h"
#include "graphic/gfx_ar_renderer.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_fog_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_manager.h"
#include "graphic/gfx_light_area_renderer.h"
#include "graphic/gfx_light_probe_renderer.h"
#include "graphic/gfx_light_point_renderer.h"
#include "graphic/gfx_light_sun_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_particle_renderer.h"
#include "graphic/gfx_postfx_renderer.h"
#include "graphic/gfx_postfx_hdr_renderer.h"
#include "graphic/gfx_reflection_renderer.h"
#include "graphic/gfx_start_state.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_tonemapping_renderer.h"
#include "graphic/gfx_shadow_renderer.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_sky_renderer.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxStartState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxStartState)
        
    public:
        
        void OnEnter();
        void OnLeave();
        void OnRun();
        
    };
} // namespace

namespace
{
    void CGfxStartState::OnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Gfx> Start manager...");

        // -----------------------------------------------------------------------------
        // Start manager. We have to take care on a specific order because of
        // dependencies.
        // -----------------------------------------------------------------------------
        ViewManager     ::OnStart();
        StateManager    ::OnStart();
        ContextManager  ::OnStart();
        SamplerManager  ::OnStart();
        TextureManager  ::OnStart();
        BufferManager   ::OnStart();
        ShaderManager   ::OnStart();
        TargetSetManager::OnStart();

        ActorManager    ::OnStart();
        LightManager    ::OnStart();
        MeshManager     ::OnStart();
        MaterialManager ::OnStart();

        BASE_CONSOLE_STREAMINFO("Gfx> Finished starting manager.");

        // -----------------------------------------------------------------------------
        // Prepare general graphic things needed by renderer
        // -----------------------------------------------------------------------------
        BASE_CONSOLE_STREAMINFO("Gfx> Create and upload global buffer.");

        Main::CreatePerFrameConstantBuffers();
        Main::UploadPerFrameConstantBuffers();

        BASE_CONSOLE_STREAMINFO("Gfx> Finished create and upload global buffer.");

        // -----------------------------------------------------------------------------
        // Start renderer. It is not possible to setup all the data in the 'OnStart'
        // methods of the renderer at once. This has two reasons:
        //
        // 1. Shader and render targets have the highest priority and should always be
        //    in GPU memory. We ensure this be allocating them first.
        //
        // 2. There are dependencies between resources and renderer. For example all the
        //    shader should exist before loading the material library and the materials
        //    should exist before loading a model.
        // -----------------------------------------------------------------------------
        BASE_CONSOLE_STREAMINFO("Gfx> Start renderer...");

        HistogramRenderer  ::OnStart();
        SkyRenderer        ::OnStart();
        LightAreaRenderer  ::OnStart();
        ReflectionRenderer ::OnStart();
        LightPointRenderer ::OnStart();
        LightProbeRenderer ::OnStart();
        LightSunRenderer   ::OnStart();
        ShadowRenderer     ::OnStart();
        FogRenderer        ::OnStart();
        ARRenderer         ::OnStart();
        ActorRenderer      ::OnStart();
        ParticleRenderer   ::OnStart();
        PostFXHDR          ::OnStart();
        PostFX             ::OnStart();
        DebugRenderer      ::OnStart();
        TonemappingRenderer::OnStart();
        
        
        // -----------------------------------------------------------------------------
        // Setup the shader of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer  ::OnSetupShader();
        SkyRenderer        ::OnSetupShader();
        LightAreaRenderer  ::OnSetupShader();
        ReflectionRenderer ::OnSetupShader();
        LightPointRenderer ::OnSetupShader();
        LightProbeRenderer ::OnSetupShader();
        LightSunRenderer   ::OnSetupShader();
        ShadowRenderer     ::OnSetupShader();
        FogRenderer        ::OnSetupShader();
        ARRenderer         ::OnSetupShader();
        ActorRenderer      ::OnSetupShader();
        ParticleRenderer   ::OnSetupShader();
        PostFXHDR          ::OnSetupShader();
        PostFX             ::OnSetupShader();
        DebugRenderer      ::OnSetupShader();
        TonemappingRenderer::OnSetupShader();
        
        // -----------------------------------------------------------------------------
        // Setup the kernels of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer  ::OnSetupKernels();
        SkyRenderer        ::OnSetupKernels();
        LightAreaRenderer  ::OnSetupKernels();
        ReflectionRenderer ::OnSetupKernels();
        LightPointRenderer ::OnSetupKernels();
        LightProbeRenderer ::OnSetupKernels();
        LightSunRenderer   ::OnSetupKernels();
        ShadowRenderer     ::OnSetupKernels();
        FogRenderer        ::OnSetupKernels();
        ARRenderer         ::OnSetupKernels();
        ActorRenderer      ::OnSetupKernels();
        ParticleRenderer   ::OnSetupKernels();
        PostFXHDR          ::OnSetupKernels();
        PostFX             ::OnSetupKernels();
        DebugRenderer      ::OnSetupKernels();
        TonemappingRenderer::OnSetupKernels();
        
        // -----------------------------------------------------------------------------
        // Setup the render targets of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer  ::OnSetupRenderTargets();
        SkyRenderer        ::OnSetupRenderTargets();
        LightAreaRenderer  ::OnSetupRenderTargets();
        ReflectionRenderer ::OnSetupRenderTargets();
        LightPointRenderer ::OnSetupRenderTargets();
        LightProbeRenderer ::OnSetupRenderTargets();
        LightSunRenderer   ::OnSetupRenderTargets();
        ShadowRenderer     ::OnSetupRenderTargets();
        FogRenderer        ::OnSetupRenderTargets();
        ARRenderer         ::OnSetupRenderTargets();
        ActorRenderer      ::OnSetupRenderTargets();
        ParticleRenderer   ::OnSetupRenderTargets();
        PostFXHDR          ::OnSetupRenderTargets();
        PostFX             ::OnSetupRenderTargets();
        DebugRenderer      ::OnSetupRenderTargets();
        TonemappingRenderer::OnSetupRenderTargets();
        
        // -----------------------------------------------------------------------------
        // Setup the states of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer  ::OnSetupStates();
        SkyRenderer        ::OnSetupStates();
        LightAreaRenderer  ::OnSetupStates();
        ReflectionRenderer ::OnSetupStates();
        LightPointRenderer ::OnSetupStates();
        LightProbeRenderer ::OnSetupStates();
        LightSunRenderer   ::OnSetupStates();
        ShadowRenderer     ::OnSetupStates();
        FogRenderer        ::OnSetupStates();
        ARRenderer         ::OnSetupStates();
        ActorRenderer      ::OnSetupStates();
        ParticleRenderer   ::OnSetupStates();
        PostFXHDR          ::OnSetupStates();
        PostFX             ::OnSetupStates();
        DebugRenderer      ::OnSetupStates();
        TonemappingRenderer::OnSetupStates();
        
        // -----------------------------------------------------------------------------
        // Setup the textures of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer  ::OnSetupTextures();
        SkyRenderer        ::OnSetupTextures();
        LightAreaRenderer  ::OnSetupTextures();
        ReflectionRenderer ::OnSetupTextures();
        LightPointRenderer ::OnSetupTextures();
        LightProbeRenderer ::OnSetupTextures();
        LightSunRenderer   ::OnSetupTextures();
        ShadowRenderer     ::OnSetupTextures();
        FogRenderer        ::OnSetupTextures();
        ARRenderer         ::OnSetupTextures();
        ActorRenderer      ::OnSetupTextures();
        ParticleRenderer   ::OnSetupTextures();
        PostFXHDR          ::OnSetupTextures();
        PostFX             ::OnSetupTextures();
        DebugRenderer      ::OnSetupTextures();
        TonemappingRenderer::OnSetupTextures();
        
        // -----------------------------------------------------------------------------
        // Setup the buffers of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer  ::OnSetupBuffers();
        SkyRenderer        ::OnSetupBuffers();
        LightAreaRenderer  ::OnSetupBuffers();
        ReflectionRenderer ::OnSetupBuffers();
        LightPointRenderer ::OnSetupBuffers();
        LightProbeRenderer ::OnSetupBuffers();
        LightSunRenderer   ::OnSetupBuffers();
        ShadowRenderer     ::OnSetupBuffers();
        FogRenderer        ::OnSetupBuffers();
        ARRenderer         ::OnSetupBuffers();
        ActorRenderer      ::OnSetupBuffers();
        ParticleRenderer   ::OnSetupBuffers();
        PostFXHDR          ::OnSetupBuffers();
        PostFX             ::OnSetupBuffers();
        DebugRenderer      ::OnSetupBuffers();
        TonemappingRenderer::OnSetupBuffers();
        
        // -----------------------------------------------------------------------------
        // Setup the resources of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer  ::OnSetupResources();
        SkyRenderer        ::OnSetupResources();
        LightAreaRenderer  ::OnSetupResources();
        ReflectionRenderer ::OnSetupResources();
        LightPointRenderer ::OnSetupResources();
        LightProbeRenderer ::OnSetupResources();
        LightSunRenderer   ::OnSetupResources();
        ShadowRenderer     ::OnSetupResources();
        FogRenderer        ::OnSetupResources();
        ARRenderer         ::OnSetupResources();
        ActorRenderer      ::OnSetupResources();
        ParticleRenderer   ::OnSetupResources();
        PostFXHDR          ::OnSetupResources();
        PostFX             ::OnSetupResources();
        DebugRenderer      ::OnSetupResources();
        TonemappingRenderer::OnSetupResources();
        
        // -----------------------------------------------------------------------------
        // Setup the models of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer  ::OnSetupModels();
        SkyRenderer        ::OnSetupModels();
        LightAreaRenderer  ::OnSetupModels();
        ReflectionRenderer ::OnSetupModels();
        LightPointRenderer ::OnSetupModels();
        LightProbeRenderer ::OnSetupModels();
        LightSunRenderer   ::OnSetupModels();
        ShadowRenderer     ::OnSetupModels();
        FogRenderer        ::OnSetupModels();
        ARRenderer         ::OnSetupModels();
        ActorRenderer      ::OnSetupModels();
        ParticleRenderer   ::OnSetupModels();
        PostFXHDR          ::OnSetupModels();
        PostFX             ::OnSetupModels();
        DebugRenderer      ::OnSetupModels();
        TonemappingRenderer::OnSetupModels();
        
        // -----------------------------------------------------------------------------
        // Setup ends with a last call
        // -----------------------------------------------------------------------------
        HistogramRenderer  ::OnSetupEnd();
        SkyRenderer        ::OnSetupEnd();
        LightAreaRenderer  ::OnSetupEnd();
        ReflectionRenderer ::OnSetupEnd();
        LightPointRenderer ::OnSetupEnd();
        LightProbeRenderer ::OnSetupEnd();
        LightSunRenderer   ::OnSetupEnd();
        ShadowRenderer     ::OnSetupEnd();
        FogRenderer        ::OnSetupEnd();
        ARRenderer         ::OnSetupEnd();
        ActorRenderer      ::OnSetupEnd();
        ParticleRenderer   ::OnSetupEnd();
        PostFXHDR          ::OnSetupEnd();
        PostFX             ::OnSetupEnd();
        DebugRenderer      ::OnSetupEnd();
        TonemappingRenderer::OnSetupEnd();

        BASE_CONSOLE_STREAMINFO("Gfx> Finished renderer starting.");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxStartState::OnLeave()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxStartState::OnRun()
    {
        
    }
} // namespace

namespace Gfx
{
namespace Start
{
    void OnEnter()
    {
        CGfxStartState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnLeave()
    {
        CGfxStartState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnRun()
    {
        CGfxStartState::GetInstance().OnRun();
    }
} // namespace Start
} // namespace Gfx