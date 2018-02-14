
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"

#include "data/data_component_facet.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"
#include "data/data_point_light_component.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_component_manager.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_sun_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_point_light_component.h"
#include "graphic/gfx_point_light_manager.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxLightIndirectRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxLightIndirectRenderer)
        
    public:
        CGfxLightIndirectRenderer();
        ~CGfxLightIndirectRenderer();
        
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

        struct SIndirectLightProperties
        {
            glm::vec4 m_RSMSettings;
            unsigned int m_ExposureHistoryIndex;
        };

        struct SRenderJob
        {
            Gfx::CPointLightComponent* m_pGraphicPointLightFacet;
        };

    private:

        typedef std::vector<SRenderJob> CRenderJobs;
        
    private:
        
        CMeshPtr          m_QuadModelPtr;
        CBufferPtr        m_IndirectLightPSBufferPtr;
        CInputLayoutPtr   m_P2InputLayoutPtr;
        CShaderPtr        m_RectangleShaderVSPtr;
        CShaderPtr        m_IndirectLightShaderPSPtr;
        CRenderContextPtr m_LightRenderContextPtr;
        CRenderJobs       m_RenderJobs;

    private:

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxLightIndirectRenderer::CGfxLightIndirectRenderer()
        : m_QuadModelPtr            ()
        , m_IndirectLightPSBufferPtr()
        , m_P2InputLayoutPtr        ()
        , m_IndirectLightShaderPSPtr()
        , m_RectangleShaderVSPtr    ()
        , m_LightRenderContextPtr   ()
        , m_RenderJobs		        ()
    {
        m_RenderJobs.reserve(4);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxLightIndirectRenderer::~CGfxLightIndirectRenderer()
    {
    	
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnExit()
    {
        m_QuadModelPtr             = 0;
        m_IndirectLightPSBufferPtr = 0;
        m_P2InputLayoutPtr         = 0;
        m_IndirectLightShaderPSPtr = 0;
        m_RectangleShaderVSPtr     = 0;
        m_LightRenderContextPtr    = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupShader()
    {       
        m_RectangleShaderVSPtr = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");
        
        m_IndirectLightShaderPSPtr = ShaderManager::CompilePS("fs_light_indirectlight.glsl", "main");
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor QuadInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        m_P2InputLayoutPtr = ShaderManager::CreateInputLayout(QuadInputLayout, 1, m_RectangleShaderVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupRenderTargets()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupStates()
    {
        CCameraPtr      QuadCameraPtr  = ViewManager     ::GetFullQuadCamera();
        CViewPortSetPtr ViewPortSetPtr = ViewManager     ::GetViewPortSet();
        CRenderStatePtr LightStatePtr  = StateManager    ::GetRenderState(CRenderState::AdditionBlend);
        CTargetSetPtr   TargetSetPtr   = TargetSetManager::GetLightAccumulationTargetSet();
       
        // -----------------------------------------------------------------------------

        m_LightRenderContextPtr = ContextManager::CreateRenderContext();

        m_LightRenderContextPtr->SetCamera(QuadCameraPtr);
        m_LightRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_LightRenderContextPtr->SetTargetSet(TargetSetPtr);
        m_LightRenderContextPtr->SetRenderState(LightStatePtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupTextures()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SIndirectLightProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_IndirectLightPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupModels()
    {
        m_QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnSetupEnd()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightIndirectRenderer::Render()
    {
        if (m_RenderJobs.size() == 0) return;

        Performance::BeginEvent("Indirect Lights");

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        

        ContextManager::SetRenderContext(m_LightRenderContextPtr);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_P2InputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

        ContextManager::SetShaderPS(m_IndirectLightShaderPSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_IndirectLightPSBufferPtr);

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(4, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(5, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(6, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(7, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        // -----------------------------------------------------------------------------
        // Set textures
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
        Gfx::ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
        Gfx::ContextManager::SetTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2));
        Gfx::ContextManager::SetTexture(3, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());

        // -----------------------------------------------------------------------------
        // Iterate throw every light
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator CurrentRenderJob = m_RenderJobs.begin();
        CRenderJobs::const_iterator EndOfRenderJobs  = m_RenderJobs.end();

        for (; CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            Gfx::CPointLightComponent* pGfxPointLight = CurrentRenderJob->m_pGraphicPointLightFacet;

            assert(pGfxPointLight != nullptr);

            // -----------------------------------------------------------------------------
            // Set shadow map
            // -----------------------------------------------------------------------------
            Gfx::ContextManager::SetTexture(4, pGfxPointLight->GetTextureRSMSet()->GetTexture(0));
            Gfx::ContextManager::SetTexture(5, pGfxPointLight->GetTextureRSMSet()->GetTexture(1));
            Gfx::ContextManager::SetTexture(6, pGfxPointLight->GetTextureRSMSet()->GetTexture(2));
            Gfx::ContextManager::SetTexture(7, pGfxPointLight->GetTextureRSMSet()->GetTexture(3));

            unsigned int HeightOfShadowmap = static_cast<unsigned int>(pGfxPointLight->GetShadowmapSize()) / 8;
            unsigned int WidthOfShadowmap  = static_cast<unsigned int>(pGfxPointLight->GetShadowmapSize()) / 8;

            for (unsigned int IndexOfRSMDataY = 0; IndexOfRSMDataY < HeightOfShadowmap; ++IndexOfRSMDataY)
            {
                // -----------------------------------------------------------------------------
                // Upload buffer data
                // -----------------------------------------------------------------------------
                SIndirectLightProperties IndirectLightBuffer;

                float SSWidthOfShadowmap = static_cast<float>(WidthOfShadowmap);

                IndirectLightBuffer.m_RSMSettings[0] = 1.0f / SSWidthOfShadowmap;
                IndirectLightBuffer.m_RSMSettings[1] = static_cast<float>(IndexOfRSMDataY) * 1.0f / SSWidthOfShadowmap;
                IndirectLightBuffer.m_RSMSettings[2] = SSWidthOfShadowmap;
                IndirectLightBuffer.m_RSMSettings[3] = 0.0f;
                IndirectLightBuffer.m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();

                Gfx::BufferManager::UploadBufferData(m_IndirectLightPSBufferPtr, &IndirectLightBuffer);

                // -----------------------------------------------------------------------------
                // Draw
                // -----------------------------------------------------------------------------
                Gfx::ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
            }
        }

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::ResetTexture(0);
        Gfx::ContextManager::ResetTexture(1);
        Gfx::ContextManager::ResetTexture(2);
        Gfx::ContextManager::ResetTexture(3);
        Gfx::ContextManager::ResetTexture(4);
        Gfx::ContextManager::ResetTexture(5);
        Gfx::ContextManager::ResetTexture(6);
        Gfx::ContextManager::ResetTexture(7);

        ContextManager::ResetConstantBuffer(0); 

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetResourceBuffer(0);

        Gfx::ContextManager::ResetSampler(0);
        Gfx::ContextManager::ResetSampler(1);
        Gfx::ContextManager::ResetSampler(2);
        Gfx::ContextManager::ResetSampler(3);
        Gfx::ContextManager::ResetSampler(4);
        Gfx::ContextManager::ResetSampler(5);
        Gfx::ContextManager::ResetSampler(6);
        Gfx::ContextManager::ResetSampler(7);

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

    void CGfxLightIndirectRenderer::BuildRenderJobs()
    {
        m_RenderJobs.clear();

        auto DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CPointLightComponent>();

        for (auto Component : DataComponents)
        {
            Dt::CPointLightComponent*  pDataPointFacet    = static_cast<Dt::CPointLightComponent*>(Component);
            Gfx::CPointLightComponent* pGraphicPointFacet = Gfx::CComponentManager::GetInstance().GetComponent<Gfx::CPointLightComponent>(pDataPointFacet->GetID());

            if (!(pDataPointFacet->IsActive() && pDataPointFacet->GetHostEntity() != nullptr && pDataPointFacet->GetHostEntity()->IsActive())) continue;

            if (pDataPointFacet->GetShadowType() == Dt::CPointLightComponent::GlobalIllumination)
            {
                SRenderJob NewRenderJob;

                NewRenderJob.m_pGraphicPointLightFacet = pGraphicPointFacet;

                m_RenderJobs.push_back(NewRenderJob);
            }
        }
    }
} // namespace

namespace Gfx
{
namespace LightIndirectRenderer
{
    void OnStart()
    {
        CGfxLightIndirectRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxLightIndirectRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxLightIndirectRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxLightIndirectRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxLightIndirectRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxLightIndirectRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxLightIndirectRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxLightIndirectRenderer::GetInstance().Render();
    }
} // namespace LightIndirectRenderer
} // namespace Gfx

