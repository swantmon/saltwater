
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/camera/cam_control_manager.h"

#include "engine/core/core_console.h"
#include "engine/core/core_time.h"

#include "engine/data/data_camera_component.h"
#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_map.h"
#include "engine/data/data_sky_component.h"

#include "engine/graphic/gfx_background_renderer.h"
#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_histogram_renderer.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_sky.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_sun.h"
#include "engine/graphic/gfx_sun_manager.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include <string>

using namespace Gfx;

namespace
{
    class CGfxBackgroundRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxBackgroundRenderer)
        
    public:

        CGfxBackgroundRenderer();
        ~CGfxBackgroundRenderer();
        
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

        struct SRenderContext
        {
            CShaderPtr        m_VSPtr;
            CShaderPtr        m_PSPtr;
            CBufferSetPtr     m_VSBufferSetPtr;
            CBufferSetPtr     m_PSBufferSetPtr;
            CInputLayoutPtr   m_InputLayoutPtr;
            CMeshPtr          m_MeshPtr;
        };

        struct SCameraRenderJob
        {
            Dt::CCameraComponent* m_pDtComponent;
            Gfx::CCamera* m_pCameraObject;
        };

        struct SSkyRenderJob
        {
            Dt::CSkyComponent* m_pDtComponent;
            Gfx::CSky* m_pSkyObject;
        };

        struct SSkyTextureBufferPS
        {
            float m_HDRFactor;
            float m_IsHDR;
            float m_ExposureIndex;
        };

        struct SSkyboxVSBuffer
        {
            glm::mat4 m_View;
            glm::mat4 m_Projection;
        };

        struct SSkyboxFromTextureVSBuffer
        {
            glm::mat4 m_ModelMatrix;
        };
        
        struct SSkyboxBufferPS
        {
            glm::vec4    m_InvertedScreenSize;
            unsigned int m_ExposureHistoryIndex;
        };

    private:

        using CCameraRenderJobs = std::vector<SCameraRenderJob>;
        using CSkyRenderJobs = std::vector<SSkyRenderJob>;
        
    private:

        SRenderContext m_BackgroundFromSkybox;
        SRenderContext m_BackgroundFromWebcam;

        CTexturePtr m_WebcamTexturePtr;

        CTargetSetPtr m_WebcamTargetSetPtr;

        CCameraRenderJobs m_CameraRenderJobs;
        CSkyRenderJobs m_SkyRenderJobs;
        
    private:

        void RenderBackgroundFromSkybox();

        void RenderBackgroundFromTexture();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxBackgroundRenderer::CGfxBackgroundRenderer()
        : m_BackgroundFromSkybox       ()
        , m_BackgroundFromWebcam       ()
        , m_WebcamTexturePtr           ()
        , m_WebcamTargetSetPtr         ()
        , m_CameraRenderJobs           ()
        , m_SkyRenderJobs              ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxBackgroundRenderer::~CGfxBackgroundRenderer()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnExit()
    {
        m_BackgroundFromSkybox.m_VSPtr            = nullptr;
        m_BackgroundFromSkybox.m_PSPtr            = nullptr;
        m_BackgroundFromSkybox.m_VSBufferSetPtr   = nullptr;
        m_BackgroundFromSkybox.m_PSBufferSetPtr   = nullptr;
        m_BackgroundFromSkybox.m_InputLayoutPtr   = nullptr;
        m_BackgroundFromSkybox.m_MeshPtr          = nullptr;

        m_BackgroundFromWebcam.m_VSPtr            = nullptr;
        m_BackgroundFromWebcam.m_PSPtr            = nullptr;
        m_BackgroundFromWebcam.m_VSBufferSetPtr   = nullptr;
        m_BackgroundFromWebcam.m_PSBufferSetPtr   = nullptr;
        m_BackgroundFromWebcam.m_InputLayoutPtr   = nullptr;
        m_BackgroundFromWebcam.m_MeshPtr          = nullptr;

        m_CameraRenderJobs.clear();

        m_SkyRenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupShader()
    {
        CShaderPtr SkyTextureVSPtr = ShaderManager::CompileVS("system/vs_fullscreen.glsl", "main");;

        CShaderPtr SkyTexturePSPtr = ShaderManager::CompilePS("atmosphere/fs_atmosphere_texture.glsl", "main");

        CShaderPtr SkyboxVSPtr  = ShaderManager::CompileVS("atmosphere/vs_cubemap.glsl", "main");
        
        CShaderPtr SkyboxPSPtr  = ShaderManager::CompilePS("atmosphere/fs_atmosphere_cubemap.glsl", "main");

        // -----------------------------------------------------------------------------

        const SInputElementDescriptor InputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };

        CInputLayoutPtr P2SkytextureLayoutPtr = ShaderManager::CreateInputLayout(InputLayout, 1, SkyTextureVSPtr);
        
        // -----------------------------------------------------------------------------

        m_BackgroundFromSkybox.m_VSPtr          = SkyboxVSPtr;
        m_BackgroundFromSkybox.m_PSPtr          = SkyboxPSPtr;
        m_BackgroundFromSkybox.m_InputLayoutPtr = nullptr;

        m_BackgroundFromWebcam.m_VSPtr          = SkyTextureVSPtr;
        m_BackgroundFromWebcam.m_PSPtr          = SkyTexturePSPtr;
        m_BackgroundFromWebcam.m_InputLayoutPtr = P2SkytextureLayoutPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupRenderTargets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupStates()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupTextures()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSkyboxVSBuffer);
        ConstanteBufferDesc.m_pBytes        = nullptr;
        ConstanteBufferDesc.m_pClassKey     = nullptr;
        
        CBufferPtr SkyboxVSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
                        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSkyboxBufferPS);
        ConstanteBufferDesc.m_pBytes        = nullptr;
        ConstanteBufferDesc.m_pClassKey     = nullptr;
        
        CBufferPtr SkyboxPSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSkyTextureBufferPS);
        ConstanteBufferDesc.m_pBytes        = nullptr;
        ConstanteBufferDesc.m_pClassKey     = nullptr;
        
        CBufferPtr SkyTexturePSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
       
        // -----------------------------------------------------------------------------
        
        CBufferSetPtr SkytextureVSBufferSetPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBuffer());
        CBufferSetPtr SkytexturePSBufferSetPtr = BufferManager::CreateBufferSet(SkyTexturePSBuffer);

        CBufferSetPtr SkyboxVSBufferSetPtr = BufferManager::CreateBufferSet(SkyboxVSBuffer);
        CBufferSetPtr SkyboxPSBufferSetPtr = BufferManager::CreateBufferSet(SkyboxPSBuffer);

        m_BackgroundFromSkybox.m_VSBufferSetPtr = SkyboxVSBufferSetPtr;
        m_BackgroundFromSkybox.m_PSBufferSetPtr = SkyboxPSBufferSetPtr;

        m_BackgroundFromWebcam.m_VSBufferSetPtr = SkytextureVSBufferSetPtr;
        m_BackgroundFromWebcam.m_PSBufferSetPtr = SkytexturePSBufferSetPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupModels()
    {
        CMeshPtr SkyboxBoxPtr = MeshManager::CreateBox(2.0f, 2.0f, 2.0f);
               
        // -----------------------------------------------------------------------------

        m_BackgroundFromSkybox.m_MeshPtr = SkyboxBoxPtr;

        m_BackgroundFromWebcam.m_MeshPtr = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupEnd()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::Render()
    {
        bool HasMainCamera = !m_CameraRenderJobs.empty();

        if (HasMainCamera)
        {
            SCameraRenderJob& rRenderJob = m_CameraRenderJobs[0];

            Performance::BeginEvent("Background");

            if (rRenderJob.m_pDtComponent->GetClearFlag() == Dt::CCameraComponent::Skybox)
            {
                RenderBackgroundFromSkybox();
            }
            else if (rRenderJob.m_pDtComponent->GetClearFlag() == Dt::CCameraComponent::Texture)
            {
                RenderBackgroundFromTexture();
            }
            else
            {
                // RenderBackgroundFromColor();
            }

            Performance::EndEvent();
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxBackgroundRenderer::RenderBackgroundFromSkybox()
    {
        if (m_SkyRenderJobs.empty()) return;

        CShaderPtr        VSPtr            = m_BackgroundFromSkybox.m_VSPtr;
        CShaderPtr        PSPtr            = m_BackgroundFromSkybox.m_PSPtr;
        CBufferSetPtr     VSBufferSetPtr   = m_BackgroundFromSkybox.m_VSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_BackgroundFromSkybox.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr   = m_BackgroundFromSkybox.m_InputLayoutPtr;
        CMeshPtr          MeshPtr          = m_BackgroundFromSkybox.m_MeshPtr;

        SSkyRenderJob& rCurrentJob = m_SkyRenderJobs[0];

        // -----------------------------------------------------------------------------
        // Render sky box
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Background From Skybox");

        // -----------------------------------------------------------------------------
        // Upload dynamic data
        // -----------------------------------------------------------------------------
        CCameraPtr CameraPtr = ViewManager::GetMainCamera();

        SSkyboxVSBuffer ViewBuffer;

        ViewBuffer.m_View       = CameraPtr->GetView()->GetViewMatrix();
        ViewBuffer.m_Projection = CameraPtr->GetProjectionMatrix();

        ViewBuffer.m_View[3] = glm::vec4(0.0f);

        BufferManager::UploadBufferData(VSBufferSetPtr->GetBuffer(0), &ViewBuffer);

        // -----------------------------------------------------------------------------

        SSkyboxBufferPS PSBuffer;
 
        PSBuffer.m_InvertedScreenSize   = glm::vec4(1.0f / Main::GetActiveWindowSize()[0], 1.0f / Main::GetActiveWindowSize()[1], 0, 0);
        PSBuffer.m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();
        
        BufferManager::UploadBufferData(PSBufferSetPtr->GetBuffer(0), &PSBuffer);
        
        // -----------------------------------------------------------------------------
        // Render sky box in background
        // -----------------------------------------------------------------------------
        ContextManager::SetTargetSet(TargetSetManager::GetLightAccumulationTargetSet());

        ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::LessEqualDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::NoCull));

        ContextManager::SetVertexBuffer(MeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(MeshPtr->GetLOD(0)->GetSurface()->GetShaderVS()->GetInputLayout());
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(VSPtr);
        
        ContextManager::SetShaderPS(PSPtr);
        
        ContextManager::SetConstantBuffer(0, VSBufferSetPtr->GetBuffer(0));
        
        ContextManager::SetConstantBuffer(8, PSBufferSetPtr->GetBuffer(0));

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        
        ContextManager::SetTexture(0, rCurrentJob.m_pSkyObject->GetCubemapPtr());
        ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());
        
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);

        ContextManager::ResetConstantBuffer(0);
        
        ContextManager::ResetConstantBuffer(8);

        ContextManager::ResetResourceBuffer(0);
        
        ContextManager::ResetTopology();
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBuffer();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxBackgroundRenderer::RenderBackgroundFromTexture()
    {
        // -----------------------------------------------------------------------------
        // Get HDR intensity from sky
        // -----------------------------------------------------------------------------
        float HDRIntensity = 1.0f;

        if (!m_SkyRenderJobs.empty())
        {
            SSkyRenderJob& rCurrentJob = m_SkyRenderJobs[0];

            Dt::CSkyComponent* pSkyComponent = rCurrentJob.m_pDtComponent;

            assert(pSkyComponent);

            HDRIntensity = pSkyComponent->GetIntensity();
        }

        // -----------------------------------------------------------------------------
        // Prepare value from sky / environment
        // -----------------------------------------------------------------------------
        CShaderPtr        VSPtr            = m_BackgroundFromWebcam.m_VSPtr;
        CShaderPtr        PSPtr            = m_BackgroundFromWebcam.m_PSPtr;
        CBufferSetPtr     VSBufferSetPtr   = m_BackgroundFromWebcam.m_VSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_BackgroundFromWebcam.m_PSBufferSetPtr;

        // -----------------------------------------------------------------------------
        // Render sky texture
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Background from Texture");

        // -----------------------------------------------------------------------------
        // Data
        // -----------------------------------------------------------------------------
        SSkyTextureBufferPS PSBuffer;

        PSBuffer.m_HDRFactor     = HDRIntensity;
        PSBuffer.m_IsHDR         = 0.0f;
        PSBuffer.m_ExposureIndex = static_cast<float>(HistogramRenderer::GetLastExposureHistoryIndex());

        BufferManager::UploadBufferData(PSBufferSetPtr->GetBuffer(0), &PSBuffer);

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        ContextManager::SetTargetSet(TargetSetManager::GetLightAccumulationTargetSet());

        ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoWriteDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::NoCull));

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetConstantBuffer(0, VSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(8, PSBufferSetPtr->GetBuffer(0));

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearMirror));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        ContextManager::SetTexture(0, m_CameraRenderJobs[0].m_pDtComponent->GetBackgroundTexture());
        ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());

        ContextManager::Draw(3, 0);

        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetConstantBuffer(8);

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetTopology();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxBackgroundRenderer::BuildRenderJobs()
    {
        m_CameraRenderJobs.clear();

        auto DataCameraComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CCameraComponent>();

        for (auto Component : DataCameraComponents)
        {
            auto* pDtComponent = static_cast<Dt::CCameraComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            SCameraRenderJob NewRenderJob;

            NewRenderJob.m_pDtComponent  = pDtComponent;
            NewRenderJob.m_pCameraObject = static_cast<Gfx::CCamera*>(pDtComponent->GetFacet(Dt::CCameraComponent::Graphic));

            m_CameraRenderJobs.push_back(NewRenderJob);
        }

        // -----------------------------------------------------------------------------

        m_SkyRenderJobs.clear();

        auto DataSkyComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CSkyComponent>();

        for (auto Component : DataSkyComponents)
        {
            auto* pDtComponent = static_cast<Dt::CSkyComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            SSkyRenderJob NewRenderJob;

            NewRenderJob.m_pDtComponent = pDtComponent;
            NewRenderJob.m_pSkyObject   = static_cast<Gfx::CSky*>(pDtComponent->GetFacet(Dt::CSkyComponent::Graphic));

            m_SkyRenderJobs.push_back(NewRenderJob);
        }
    }
} // namespace

namespace Gfx
{
namespace BackgroundRenderer
{
    void OnStart()
    {
        CGfxBackgroundRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxBackgroundRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxBackgroundRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxBackgroundRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxBackgroundRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxBackgroundRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxBackgroundRenderer::GetInstance().Render();
    }
} // namespace BackgroundRenderer
} // namespace Gfx