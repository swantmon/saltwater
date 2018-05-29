
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_ar_renderer.h"
#include "engine/graphic/gfx_area_light_manager.h"
#include "engine/graphic/gfx_background_renderer.h"
#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_debug_renderer.h"
#include "engine/graphic/gfx_fog_renderer.h"
#include "engine/graphic/gfx_histogram_renderer.h"
#include "engine/graphic/gfx_light_area_renderer.h"
#include "engine/graphic/gfx_light_indirect_renderer.h"
#include "engine/graphic/gfx_light_point_renderer.h"
#include "engine/graphic/gfx_light_probe_manager.h"
#include "engine/graphic/gfx_light_sun_renderer.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_material_manager.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_mesh_renderer.h"
#include "engine/graphic/gfx_particle_renderer.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_pipeline.h"
#include "engine/graphic/gfx_point_light_manager.h"
#include "engine/graphic/gfx_postfx_hdr_renderer.h"
#include "engine/graphic/gfx_postfx_renderer.h"
#include "engine/graphic/gfx_reflection_renderer.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_selection_renderer.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_shadow_renderer.h"
#include "engine/graphic/gfx_sky_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_sun_manager.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_tonemapping_renderer.h"
#include "engine/graphic/gfx_view_manager.h"

using namespace Gfx;

namespace Gfx
{
namespace Pipeline
{
    void OnStart()
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
        ENGINE_CONSOLE_STREAMINFO("Gfx> Start manager...");

        StateManager    ::OnStart();
        ContextManager  ::OnStart();
        SamplerManager  ::OnStart();
        TextureManager  ::OnStart();
        BufferManager   ::OnStart();
        ShaderManager   ::OnStart();
        TargetSetManager::OnStart();

        ENGINE_CONSOLE_STREAMINFO("Gfx> Finished starting manager.");

        // -----------------------------------------------------------------------------
        // Start entity manager
        // -----------------------------------------------------------------------------
        ENGINE_CONSOLE_STREAMINFO("Gfx> Start entity/facet manager...");

        ViewManager       ::OnStart();
        SunManager        ::OnStart();
        MeshManager       ::OnStart();
        MaterialManager   ::OnStart();
        SkyManager        ::OnStart();
        LightProbeManager ::OnStart();
        PointLightManager ::OnStart();
        AreaLightManager  ::OnStart();

        ENGINE_CONSOLE_STREAMINFO("Gfx> Finished starting entity/facet manager.");

        // -----------------------------------------------------------------------------
        // Prepare general graphic things needed by renderer
        // -----------------------------------------------------------------------------
        ENGINE_CONSOLE_STREAMINFO("Gfx> Create and upload global buffer.");

        Main::CreatePerFrameConstantBuffers();
        Main::UploadPerFrameConstantBuffers();

        ENGINE_CONSOLE_STREAMINFO("Gfx> Finished create and upload global buffer.");

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
        ENGINE_CONSOLE_STREAMINFO("Gfx> Start renderer...");

        HistogramRenderer    ::OnStart();
        BackgroundRenderer   ::OnStart();
        LightAreaRenderer    ::OnStart();
        ReflectionRenderer   ::OnStart();
        LightPointRenderer   ::OnStart();
        LightSunRenderer     ::OnStart();
        LightIndirectRenderer::OnStart();
        ShadowRenderer       ::OnStart();
        FogRenderer          ::OnStart();
        ARRenderer           ::OnStart();
        MeshRenderer         ::OnStart();
        ParticleRenderer     ::OnStart();
        PostFXHDR            ::OnStart();
        PostFX               ::OnStart();
        DebugRenderer        ::OnStart();
        SelectionRenderer    ::OnStart();
        TonemappingRenderer  ::OnStart();
        
        
        // -----------------------------------------------------------------------------
        // Setup the shader of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer    ::OnSetupShader();
        BackgroundRenderer   ::OnSetupShader();
        LightAreaRenderer    ::OnSetupShader();
        ReflectionRenderer   ::OnSetupShader();
        LightPointRenderer   ::OnSetupShader();
        LightSunRenderer     ::OnSetupShader();
        LightIndirectRenderer::OnSetupShader();
        ShadowRenderer       ::OnSetupShader();
        FogRenderer          ::OnSetupShader();
        ARRenderer           ::OnSetupShader();
        MeshRenderer         ::OnSetupShader();
        ParticleRenderer     ::OnSetupShader();
        PostFXHDR            ::OnSetupShader();
        PostFX               ::OnSetupShader();
        DebugRenderer        ::OnSetupShader();
        SelectionRenderer    ::OnSetupShader();
        TonemappingRenderer  ::OnSetupShader();
        
        // -----------------------------------------------------------------------------
        // Setup the kernels of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer    ::OnSetupKernels();
        BackgroundRenderer   ::OnSetupKernels();
        LightAreaRenderer    ::OnSetupKernels();
        ReflectionRenderer   ::OnSetupKernels();
        LightPointRenderer   ::OnSetupKernels();
        LightSunRenderer     ::OnSetupKernels();
        LightIndirectRenderer::OnSetupKernels();
        ShadowRenderer       ::OnSetupKernels();
        FogRenderer          ::OnSetupKernels();
        ARRenderer           ::OnSetupKernels();
        MeshRenderer         ::OnSetupKernels();
        ParticleRenderer     ::OnSetupKernels();
        PostFXHDR            ::OnSetupKernels();
        PostFX               ::OnSetupKernels();
        DebugRenderer        ::OnSetupKernels();
        SelectionRenderer    ::OnSetupKernels();
        TonemappingRenderer  ::OnSetupKernels();
        
        // -----------------------------------------------------------------------------
        // Setup the render targets of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer    ::OnSetupRenderTargets();
        BackgroundRenderer   ::OnSetupRenderTargets();
        LightAreaRenderer    ::OnSetupRenderTargets();
        ReflectionRenderer   ::OnSetupRenderTargets();
        LightPointRenderer   ::OnSetupRenderTargets();
        LightSunRenderer     ::OnSetupRenderTargets();
        LightIndirectRenderer::OnSetupRenderTargets();
        ShadowRenderer       ::OnSetupRenderTargets();
        FogRenderer          ::OnSetupRenderTargets();
        ARRenderer           ::OnSetupRenderTargets();
        MeshRenderer         ::OnSetupRenderTargets();
        ParticleRenderer     ::OnSetupRenderTargets();
        PostFXHDR            ::OnSetupRenderTargets();
        PostFX               ::OnSetupRenderTargets();
        DebugRenderer        ::OnSetupRenderTargets();
        SelectionRenderer    ::OnSetupRenderTargets();
        TonemappingRenderer  ::OnSetupRenderTargets();
        
        // -----------------------------------------------------------------------------
        // Setup the states of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer    ::OnSetupStates();
        BackgroundRenderer   ::OnSetupStates();
        LightAreaRenderer    ::OnSetupStates();
        ReflectionRenderer   ::OnSetupStates();
        LightPointRenderer   ::OnSetupStates();
        LightSunRenderer     ::OnSetupStates();
        LightIndirectRenderer::OnSetupStates();
        ShadowRenderer       ::OnSetupStates();
        FogRenderer          ::OnSetupStates();
        ARRenderer           ::OnSetupStates();
        MeshRenderer         ::OnSetupStates();
        ParticleRenderer     ::OnSetupStates();
        PostFXHDR            ::OnSetupStates();
        PostFX               ::OnSetupStates();
        DebugRenderer        ::OnSetupStates();
        SelectionRenderer    ::OnSetupStates();
        TonemappingRenderer  ::OnSetupStates();
        
        // -----------------------------------------------------------------------------
        // Setup the textures of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer    ::OnSetupTextures();
        BackgroundRenderer   ::OnSetupTextures();
        LightAreaRenderer    ::OnSetupTextures();
        ReflectionRenderer   ::OnSetupTextures();
        LightPointRenderer   ::OnSetupTextures();
        LightSunRenderer     ::OnSetupTextures();
        LightIndirectRenderer::OnSetupTextures();
        ShadowRenderer       ::OnSetupTextures();
        FogRenderer          ::OnSetupTextures();
        ARRenderer           ::OnSetupTextures();
        MeshRenderer         ::OnSetupTextures();
        ParticleRenderer     ::OnSetupTextures();
        PostFXHDR            ::OnSetupTextures();
        PostFX               ::OnSetupTextures();
        DebugRenderer        ::OnSetupTextures();
        SelectionRenderer    ::OnSetupTextures();
        TonemappingRenderer  ::OnSetupTextures();
        
        // -----------------------------------------------------------------------------
        // Setup the buffers of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer    ::OnSetupBuffers();
        BackgroundRenderer   ::OnSetupBuffers();
        LightAreaRenderer    ::OnSetupBuffers();
        ReflectionRenderer   ::OnSetupBuffers();
        LightPointRenderer   ::OnSetupBuffers();
        LightSunRenderer     ::OnSetupBuffers();
        LightIndirectRenderer::OnSetupBuffers();
        ShadowRenderer       ::OnSetupBuffers();
        FogRenderer          ::OnSetupBuffers();
        ARRenderer           ::OnSetupBuffers();
        MeshRenderer         ::OnSetupBuffers();
        ParticleRenderer     ::OnSetupBuffers();
        PostFXHDR            ::OnSetupBuffers();
        PostFX               ::OnSetupBuffers();
        DebugRenderer        ::OnSetupBuffers();
        SelectionRenderer    ::OnSetupBuffers();
        TonemappingRenderer  ::OnSetupBuffers();
        
        // -----------------------------------------------------------------------------
        // Setup the resources of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer    ::OnSetupResources();
        BackgroundRenderer   ::OnSetupResources();
        LightAreaRenderer    ::OnSetupResources();
        ReflectionRenderer   ::OnSetupResources();
        LightPointRenderer   ::OnSetupResources();
        LightSunRenderer     ::OnSetupResources();
        LightIndirectRenderer::OnSetupResources();
        ShadowRenderer       ::OnSetupResources();
        FogRenderer          ::OnSetupResources();
        ARRenderer           ::OnSetupResources();
        MeshRenderer         ::OnSetupResources();
        ParticleRenderer     ::OnSetupResources();
        PostFXHDR            ::OnSetupResources();
        PostFX               ::OnSetupResources();
        DebugRenderer        ::OnSetupResources();
        SelectionRenderer    ::OnSetupResources();
        TonemappingRenderer  ::OnSetupResources();
        
        // -----------------------------------------------------------------------------
        // Setup the models of all renderer
        // -----------------------------------------------------------------------------
        HistogramRenderer    ::OnSetupModels();
        BackgroundRenderer   ::OnSetupModels();
        LightAreaRenderer    ::OnSetupModels();
        ReflectionRenderer   ::OnSetupModels();
        LightPointRenderer   ::OnSetupModels();
        LightSunRenderer     ::OnSetupModels();
        LightIndirectRenderer::OnSetupModels();
        ShadowRenderer       ::OnSetupModels();
        FogRenderer          ::OnSetupModels();
        ARRenderer           ::OnSetupModels();
        MeshRenderer         ::OnSetupModels();
        ParticleRenderer     ::OnSetupModels();
        PostFXHDR            ::OnSetupModels();
        PostFX               ::OnSetupModels();
        DebugRenderer        ::OnSetupModels();
        SelectionRenderer    ::OnSetupModels();
        TonemappingRenderer  ::OnSetupModels();
        
        // -----------------------------------------------------------------------------
        // Setup ends with a last call
        // -----------------------------------------------------------------------------
        HistogramRenderer    ::OnSetupEnd();
        BackgroundRenderer   ::OnSetupEnd();
        LightAreaRenderer    ::OnSetupEnd();
        ReflectionRenderer   ::OnSetupEnd();
        LightPointRenderer   ::OnSetupEnd();
        LightSunRenderer     ::OnSetupEnd();
        LightIndirectRenderer::OnSetupEnd();
        ShadowRenderer       ::OnSetupEnd();
        FogRenderer          ::OnSetupEnd();
        ARRenderer           ::OnSetupEnd();
        MeshRenderer         ::OnSetupEnd();
        ParticleRenderer     ::OnSetupEnd();
        PostFXHDR            ::OnSetupEnd();
        PostFX               ::OnSetupEnd();
        DebugRenderer        ::OnSetupEnd();
        SelectionRenderer    ::OnSetupEnd();
        TonemappingRenderer  ::OnSetupEnd();

        ENGINE_CONSOLE_STREAMINFO("Gfx> Finished renderer starting.");
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        // -----------------------------------------------------------------------------
        // Exit renderer. Now it isn't necessary to do this in a specific direction.
        // -----------------------------------------------------------------------------
        ENGINE_CONSOLE_STREAMINFO("Gfx> Exit renderer...");

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
        MeshRenderer         ::OnExit();
        ARRenderer           ::OnExit();
        ParticleRenderer     ::OnExit();
        PostFXHDR            ::OnExit();
        PostFX               ::OnExit();
        TonemappingRenderer  ::OnExit();

        ENGINE_CONSOLE_STREAMINFO("Gfx> Finished exit of renderer.");
        
        // -----------------------------------------------------------------------------
        // Destroy main graphic data
        // -----------------------------------------------------------------------------
        ENGINE_CONSOLE_STREAMINFO("Gfx> Destroy global constant buffer.");

        Main::DestroyPerFrameConstantBuffers();   

        ENGINE_CONSOLE_STREAMINFO("Gfx> Global constant buffer destroyed.");
        
        // -----------------------------------------------------------------------------
        // Exit manager
        // -----------------------------------------------------------------------------
        ENGINE_CONSOLE_STREAMINFO("Gfx> Exit manager...");

        PointLightManager ::OnExit();
        AreaLightManager  ::OnExit();
        LightProbeManager ::OnExit();
        SkyManager        ::OnExit();
        SunManager        ::OnExit();
        MeshManager       ::OnExit();
        MaterialManager   ::OnExit();
        ViewManager       ::OnExit();

        ENGINE_CONSOLE_STREAMINFO("Gfx> Finished exiting manager.");

        // -----------------------------------------------------------------------------
        // Exit graphic resources
        // -----------------------------------------------------------------------------
        ENGINE_CONSOLE_STREAMINFO("Gfx> Exit resource manager.");

        TargetSetManager::OnExit();
        ContextManager  ::OnExit();
        StateManager    ::OnExit();
        ShaderManager   ::OnExit();
        BufferManager   ::OnExit();
        TextureManager  ::OnExit();
        SamplerManager  ::OnExit();

        ENGINE_CONSOLE_STREAMINFO("Gfx> Finished exiting resource manager.");

        // -----------------------------------------------------------------------------
        // Exit performance tools
        // -----------------------------------------------------------------------------
        Performance::OnExit();

        // -----------------------------------------------------------------------------
        // Exit engine
        // -----------------------------------------------------------------------------
        Main::OnExit();
    }

    // -----------------------------------------------------------------------------

    void Render()
    {
        Performance::Update();

        // -----------------------------------------------------------------------------
        // Update graphic
        // -----------------------------------------------------------------------------
        Main::UploadPerFrameConstantBuffers();

        // -----------------------------------------------------------------------------
        // Begin frame
        // -----------------------------------------------------------------------------
        Main::BeginFrame();

        // -----------------------------------------------------------------------------
        // Update graphic entities
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Update Pass");

        SunManager        ::Update();
        SkyManager        ::Update();
        LightProbeManager ::Update();
        PointLightManager ::Update();
        AreaLightManager  ::Update();

        // -----------------------------------------------------------------------------
        // Update renderer to prepare for rendering
        // -----------------------------------------------------------------------------
        ARRenderer           ::Update();
        MeshRenderer         ::Update();
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
        SelectionRenderer    ::Update();

        Engine::RaiseEvent(Engine::Gfx_OnUpdate);

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Creation Pass
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Creation Pass");

        ARRenderer  ::Render();
        MeshRenderer::Render();

        Engine::RaiseEvent(Engine::Gfx_OnRenderGBuffer);

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

        Engine::RaiseEvent(Engine::Gfx_OnRenderLighting);

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Forward Pass
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Forward Pass");

        Engine::RaiseEvent(Engine::Gfx_OnRenderForward);

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Shading Pass
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Shading Pass");

        HistogramRenderer::Render();

        PostFXHDR::Render();

        TonemappingRenderer::Render();

        LightAreaRenderer::RenderBulbs();

        SelectionRenderer::Render();

        PostFX::Render();

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // UI Pass
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("UI Pass");

        Engine::RaiseEvent(Engine::Gfx_OnRenderUI);

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // End
        // -----------------------------------------------------------------------------
        Main::EndFrame();
    }

    // -----------------------------------------------------------------------------

    unsigned int RegisterWindow(void* _pWindow, int _VSync)
    {
        assert(_pWindow != 0);

        return Main::RegisterWindow(_pWindow, _VSync);
    }

    // -----------------------------------------------------------------------------

    void ActivateWindow(unsigned int _WindowID)
    {
        Main::ActivateWindow(_WindowID);
    }

    // -----------------------------------------------------------------------------

    void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height)
    {
        Main::OnResize(_WindowID, _Width, _Height);
    }
} // namespace Pipeline
} // namespace Gfx