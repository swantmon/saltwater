
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_vector4.h"

#include "camera/cam_control_manager.h"

#include "data/data_entity.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_fog_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_model_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxFogRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxFogRenderer)
        
    public:
        CGfxFogRenderer();
        ~CGfxFogRenderer();
        
    public:
        void OnStart();
        void OnExit();
        
        void OnSetupShader();
        void OnSetupKernels();
        void OnSetupRenderTargets();
        void OnSetupStates();
        void OnSetupTextures();
        void OnSetupBuffers();
        void OnSetupResources();
        void OnSetupModels();
        void OnSetupEnd();
        
        void OnReload();
        void OnNewMap();
        void OnUnloadMap();
        
        void Update();
        void Render();
        
    private:
        
        CModelPtr         m_QuadModelPtr;
        CBufferSetPtr     m_FullQuadViewVSBufferPtr;
        CInputLayoutPtr   m_P2InputLayoutPtr;
        CShaderPtr        m_RectangleShaderVSPtr;
        CShaderPtr        m_ESMCSPtr;
        CShaderPtr        m_VolumeLightingCSPtr;
        CShaderPtr        m_VolumeScatteringCSPtr;
        CShaderPtr        m_ApplyPSPtr;
        CSamplerSetPtr    m_PSSamplerSetPtr;
        CRenderContextPtr m_LightRenderContextPtr;

    private:

        void RenderESM();
        void RenderVolumeLighting();
        void RenderScattering();
        void RenderApply();
    };
} // namespace

namespace
{
    CGfxFogRenderer::CGfxFogRenderer()
        : m_QuadModelPtr           ()
        , m_FullQuadViewVSBufferPtr()
        , m_P2InputLayoutPtr       ()
        , m_RectangleShaderVSPtr   ()
        , m_ESMCSPtr               ()
        , m_VolumeLightingCSPtr    ()
        , m_VolumeScatteringCSPtr  ()
        , m_ApplyPSPtr             ()
        , m_PSSamplerSetPtr        ()
        , m_LightRenderContextPtr  ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxFogRenderer::~CGfxFogRenderer()
    {
    	
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnExit()
    {
        m_QuadModelPtr            = 0;
        m_FullQuadViewVSBufferPtr = 0;
        m_P2InputLayoutPtr        = 0;
        m_RectangleShaderVSPtr    = 0;
        m_ESMCSPtr                = 0;
        m_VolumeLightingCSPtr     = 0;
        m_VolumeScatteringCSPtr   = 0;
        m_ApplyPSPtr              = 0;
        m_PSSamplerSetPtr         = 0;
        m_LightRenderContextPtr   = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupShader()
    {       
        m_RectangleShaderVSPtr  = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");
        m_ESMCSPtr              = ShaderManager::CompileCS("cs_esm.glsl", "main");  
        m_VolumeLightingCSPtr   = ShaderManager::CompileCS("cs_volume_lighting.glsl", "main");
        m_VolumeScatteringCSPtr = ShaderManager::CompileCS("cs_volume_scattering.glsl", "main");  
        m_ApplyPSPtr            = ShaderManager::CompilePS("fs_fog_apply.glsl", "main");  
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor QuadInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        m_P2InputLayoutPtr = ShaderManager::CreateInputLayout(QuadInputLayout, 1, m_RectangleShaderVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupRenderTargets()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupStates()
    {
        CCameraPtr      QuadCameraPtr  = ViewManager     ::GetFullQuadCamera();
        CViewPortSetPtr ViewPortSetPtr = ViewManager     ::GetViewPortSet();
        CRenderStatePtr LightStatePtr  = StateManager    ::GetRenderState(0);
        CTargetSetPtr   TargetSetPtr   = TargetSetManager::GetLightAccumulationTargetSet();
       
        // -----------------------------------------------------------------------------

        m_LightRenderContextPtr = ContextManager::CreateRenderContext();

        m_LightRenderContextPtr->SetCamera(QuadCameraPtr);
        m_LightRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_LightRenderContextPtr->SetTargetSet(TargetSetPtr);
        m_LightRenderContextPtr->SetRenderState(LightStatePtr);
        
        // -----------------------------------------------------------------------------
        
        CSamplerPtr Sampler[6];

        Sampler[0] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[1] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[2] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[3] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[4] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[5] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupTextures()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupBuffers()
    {
        m_FullQuadViewVSBufferPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS());
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupModels()
    {
        m_QuadModelPtr = ModelManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupEnd()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::Update()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::Render()
    {
        Performance::BeginEvent("Fog");

        RenderESM();

        RenderVolumeLighting();

        RenderScattering();

        RenderApply();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxFogRenderer::RenderESM()
    {
        Performance::BeginEvent("ESM");

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxFogRenderer::RenderVolumeLighting()
    {
        Performance::BeginEvent("Volume Lighting");

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxFogRenderer::RenderScattering()
    {
        Performance::BeginEvent("Scattering");

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxFogRenderer::RenderApply()
    {
        Performance::BeginEvent("Apply");

        Performance::EndEvent();
    }
} // namespace

namespace Gfx
{
namespace FogRenderer
{
    void OnStart()
    {
        CGfxFogRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxFogRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxFogRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxFogRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxFogRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxFogRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxFogRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxFogRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxFogRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxFogRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxFogRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxFogRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxFogRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxFogRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxFogRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxFogRenderer::GetInstance().Render();
    }
} // namespace FogRenderer
} // namespace Gfx

