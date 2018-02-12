
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
#include "graphic/gfx_start_state.h"
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
        // -----------------------------------------------------------------------------
        // Start engine
        // -----------------------------------------------------------------------------
        Main::OnStart();

        // -----------------------------------------------------------------------------
        // Start performance tools
        // -----------------------------------------------------------------------------
        Performance::OnStart();

        // -----------------------------------------------------------------------------
        // Start manager. We have to take care on a specific order because of
        // dependencies.
        // -----------------------------------------------------------------------------
        BASE_CONSOLE_STREAMINFO("Gfx> Start manager...");

        ViewManager     ::OnStart();
        StateManager    ::OnStart();
        ContextManager  ::OnStart();
        SamplerManager  ::OnStart();
        TextureManager  ::OnStart();
        BufferManager   ::OnStart();
        ShaderManager   ::OnStart();
        TargetSetManager::OnStart();

        BASE_CONSOLE_STREAMINFO("Gfx> Finished starting manager.");

        // -----------------------------------------------------------------------------
        // Prepare general graphic things needed by renderer
        // -----------------------------------------------------------------------------
        BASE_CONSOLE_STREAMINFO("Gfx> Create and upload global buffer.");

        Main::CreatePerFrameConstantBuffers();
        Main::UploadPerFrameConstantBuffers();

        BASE_CONSOLE_STREAMINFO("Gfx> Finished create and upload global buffer.");

        // -----------------------------------------------------------------------------
        // Start entity manager
        // -----------------------------------------------------------------------------
        BASE_CONSOLE_STREAMINFO("Gfx> Start entity/facet manager...");

        MeshActorManager  ::OnStart();
        CameraActorManager::OnStart();
        SunManager        ::OnStart();
        MeshManager       ::OnStart();
        MaterialManager   ::OnStart();
        SkyManager        ::OnStart();
        LightProbeManager ::OnStart();
        PointLightManager ::OnStart();
        AreaLightManager  ::OnStart();

        BASE_CONSOLE_STREAMINFO("Gfx> Finished starting entity/facet manager.");

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

        HistogramRenderer     ::OnStart();
        BackgroundRenderer    ::OnStart();
        LightAreaRenderer     ::OnStart();
        ReflectionRenderer    ::OnStart();
        LightPointRenderer    ::OnStart();
        LightSunRenderer      ::OnStart();
        LightIndirectRenderer ::OnStart();
        ShadowRenderer        ::OnStart();
        FogRenderer           ::OnStart();
        ARRenderer            ::OnStart();
        ActorRenderer         ::OnStart();
        ParticleRenderer      ::OnStart();
        PostFXHDR             ::OnStart();
        PostFX                ::OnStart();
        DebugRenderer         ::OnStart();
        SelectionRenderer     ::OnStart();
        TonemappingRenderer   ::OnStart();
        ReconstructionRenderer::OnStart();
        
        
        // -----------------------------------------------------------------------------
        // Setup the shader of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer     ::OnSetupShader();
        BackgroundRenderer    ::OnSetupShader();
        LightAreaRenderer     ::OnSetupShader();
        ReflectionRenderer    ::OnSetupShader();
        LightPointRenderer    ::OnSetupShader();
        LightSunRenderer      ::OnSetupShader();
        LightIndirectRenderer ::OnSetupShader();
        ShadowRenderer        ::OnSetupShader();
        FogRenderer           ::OnSetupShader();
        ARRenderer            ::OnSetupShader();
        ActorRenderer         ::OnSetupShader();
        ParticleRenderer      ::OnSetupShader();
        PostFXHDR             ::OnSetupShader();
        PostFX                ::OnSetupShader();
        DebugRenderer         ::OnSetupShader();
        SelectionRenderer     ::OnSetupShader();
        TonemappingRenderer   ::OnSetupShader();
        ReconstructionRenderer::OnSetupShader();
        
        // -----------------------------------------------------------------------------
        // Setup the kernels of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer     ::OnSetupKernels();
        BackgroundRenderer    ::OnSetupKernels();
        LightAreaRenderer     ::OnSetupKernels();
        ReflectionRenderer    ::OnSetupKernels();
        LightPointRenderer    ::OnSetupKernels();
        LightSunRenderer      ::OnSetupKernels();
        LightIndirectRenderer ::OnSetupKernels();
        ShadowRenderer        ::OnSetupKernels();
        FogRenderer           ::OnSetupKernels();
        ARRenderer            ::OnSetupKernels();
        ActorRenderer         ::OnSetupKernels();
        ParticleRenderer      ::OnSetupKernels();
        PostFXHDR             ::OnSetupKernels();
        PostFX                ::OnSetupKernels();
        DebugRenderer         ::OnSetupKernels();
        SelectionRenderer     ::OnSetupKernels();
        TonemappingRenderer   ::OnSetupKernels();
        ReconstructionRenderer::OnSetupKernels();
        
        // -----------------------------------------------------------------------------
        // Setup the render targets of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer     ::OnSetupRenderTargets();
        BackgroundRenderer    ::OnSetupRenderTargets();
        LightAreaRenderer     ::OnSetupRenderTargets();
        ReflectionRenderer    ::OnSetupRenderTargets();
        LightPointRenderer    ::OnSetupRenderTargets();
        LightSunRenderer      ::OnSetupRenderTargets();
        LightIndirectRenderer ::OnSetupRenderTargets();
        ShadowRenderer        ::OnSetupRenderTargets();
        FogRenderer           ::OnSetupRenderTargets();
        ARRenderer            ::OnSetupRenderTargets();
        ActorRenderer         ::OnSetupRenderTargets();
        ParticleRenderer      ::OnSetupRenderTargets();
        PostFXHDR             ::OnSetupRenderTargets();
        PostFX                ::OnSetupRenderTargets();
        DebugRenderer         ::OnSetupRenderTargets();
        SelectionRenderer     ::OnSetupRenderTargets();
        TonemappingRenderer   ::OnSetupRenderTargets();
        ReconstructionRenderer::OnSetupRenderTargets();
        
        // -----------------------------------------------------------------------------
        // Setup the states of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer     ::OnSetupStates();
        BackgroundRenderer    ::OnSetupStates();
        LightAreaRenderer     ::OnSetupStates();
        ReflectionRenderer    ::OnSetupStates();
        LightPointRenderer    ::OnSetupStates();
        LightSunRenderer      ::OnSetupStates();
        LightIndirectRenderer ::OnSetupStates();
        ShadowRenderer        ::OnSetupStates();
        FogRenderer           ::OnSetupStates();
        ARRenderer            ::OnSetupStates();
        ActorRenderer         ::OnSetupStates();
        ParticleRenderer      ::OnSetupStates();
        PostFXHDR             ::OnSetupStates();
        PostFX                ::OnSetupStates();
        DebugRenderer         ::OnSetupStates();
        SelectionRenderer     ::OnSetupStates();
        TonemappingRenderer   ::OnSetupStates();
        ReconstructionRenderer::OnSetupStates();
        
        // -----------------------------------------------------------------------------
        // Setup the textures of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer     ::OnSetupTextures();
        BackgroundRenderer    ::OnSetupTextures();
        LightAreaRenderer     ::OnSetupTextures();
        ReflectionRenderer    ::OnSetupTextures();
        LightPointRenderer    ::OnSetupTextures();
        LightSunRenderer      ::OnSetupTextures();
        LightIndirectRenderer ::OnSetupTextures();
        ShadowRenderer        ::OnSetupTextures();
        FogRenderer           ::OnSetupTextures();
        ARRenderer            ::OnSetupTextures();
        ActorRenderer         ::OnSetupTextures();
        ParticleRenderer      ::OnSetupTextures();
        PostFXHDR             ::OnSetupTextures();
        PostFX                ::OnSetupTextures();
        DebugRenderer         ::OnSetupTextures();
        SelectionRenderer     ::OnSetupTextures();
        TonemappingRenderer   ::OnSetupTextures();
        ReconstructionRenderer::OnSetupTextures();
        
        // -----------------------------------------------------------------------------
        // Setup the buffers of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer     ::OnSetupBuffers();
        BackgroundRenderer    ::OnSetupBuffers();
        LightAreaRenderer     ::OnSetupBuffers();
        ReflectionRenderer    ::OnSetupBuffers();
        LightPointRenderer    ::OnSetupBuffers();
        LightSunRenderer      ::OnSetupBuffers();
        LightIndirectRenderer ::OnSetupBuffers();
        ShadowRenderer        ::OnSetupBuffers();
        FogRenderer           ::OnSetupBuffers();
        ARRenderer            ::OnSetupBuffers();
        ActorRenderer         ::OnSetupBuffers();
        ParticleRenderer      ::OnSetupBuffers();
        PostFXHDR             ::OnSetupBuffers();
        PostFX                ::OnSetupBuffers();
        DebugRenderer         ::OnSetupBuffers();
        SelectionRenderer     ::OnSetupBuffers();
        TonemappingRenderer   ::OnSetupBuffers();
        ReconstructionRenderer::OnSetupBuffers();
        
        // -----------------------------------------------------------------------------
        // Setup the resources of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer     ::OnSetupResources();
        BackgroundRenderer    ::OnSetupResources();
        LightAreaRenderer     ::OnSetupResources();
        ReflectionRenderer    ::OnSetupResources();
        LightPointRenderer    ::OnSetupResources();
        LightSunRenderer      ::OnSetupResources();
        LightIndirectRenderer ::OnSetupResources();
        ShadowRenderer        ::OnSetupResources();
        FogRenderer           ::OnSetupResources();
        ARRenderer            ::OnSetupResources();
        ActorRenderer         ::OnSetupResources();
        ParticleRenderer      ::OnSetupResources();
        PostFXHDR             ::OnSetupResources();
        PostFX                ::OnSetupResources();
        DebugRenderer         ::OnSetupResources();
        SelectionRenderer     ::OnSetupResources();
        TonemappingRenderer   ::OnSetupResources();
        ReconstructionRenderer::OnSetupResources();
        
        // -----------------------------------------------------------------------------
        // Setup the models of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer     ::OnSetupModels();
        BackgroundRenderer    ::OnSetupModels();
        LightAreaRenderer     ::OnSetupModels();
        ReflectionRenderer    ::OnSetupModels();
        LightPointRenderer    ::OnSetupModels();
        LightSunRenderer      ::OnSetupModels();
        LightIndirectRenderer ::OnSetupModels();
        ShadowRenderer        ::OnSetupModels();
        FogRenderer           ::OnSetupModels();
        ARRenderer            ::OnSetupModels();
        ActorRenderer         ::OnSetupModels();
        ParticleRenderer      ::OnSetupModels();
        PostFXHDR             ::OnSetupModels();
        PostFX                ::OnSetupModels();
        DebugRenderer         ::OnSetupModels();
        SelectionRenderer     ::OnSetupModels();
        TonemappingRenderer   ::OnSetupModels();
        ReconstructionRenderer::OnSetupModels();
        
        // -----------------------------------------------------------------------------
        // Setup ends with a last call
        // -----------------------------------------------------------------------------
        HistogramRenderer     ::OnSetupEnd();
        BackgroundRenderer    ::OnSetupEnd();
        LightAreaRenderer     ::OnSetupEnd();
        ReflectionRenderer    ::OnSetupEnd();
        LightPointRenderer    ::OnSetupEnd();
        LightSunRenderer      ::OnSetupEnd();
        LightIndirectRenderer ::OnSetupEnd();
        ShadowRenderer        ::OnSetupEnd();
        FogRenderer           ::OnSetupEnd();
        ARRenderer            ::OnSetupEnd();
        ActorRenderer         ::OnSetupEnd();
        ParticleRenderer      ::OnSetupEnd();
        PostFXHDR             ::OnSetupEnd();
        PostFX                ::OnSetupEnd();
        DebugRenderer         ::OnSetupEnd();
        SelectionRenderer     ::OnSetupEnd();
        TonemappingRenderer   ::OnSetupEnd();
        ReconstructionRenderer::OnSetupEnd();

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