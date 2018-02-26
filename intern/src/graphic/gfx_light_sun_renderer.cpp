
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"

#include "data/data_component_facet.h"
#include "data/data_component.h"
#include "data/data_entity.h"
#include "data/data_map.h"
#include "data/data_mesh_helper.h"
#include "data/data_sun_component.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_component.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_sun_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sun_component.h"
#include "graphic/gfx_sun_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxLightSunRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxLightSunRenderer)
        
    public:
        CGfxLightSunRenderer();
        ~CGfxLightSunRenderer();
        
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

        struct SSunLightProperties
        {
            glm::mat4 m_LightViewProjection;
            glm::vec4   m_LightDirection;
            glm::vec4   m_LightColor;
            float          m_SunAngularRadius;
            unsigned int   m_ExposureHistoryIndex;
        };

        struct SRenderJob
        {
            Dt::CSunComponent*  m_pDataSunLightFacet;
            Gfx::CSunComponent* m_pGraphicSunLightFacet;
        };

    private:

        typedef std::vector<SRenderJob> CRenderJobs;
        
    private:
        
        CMeshPtr          m_QuadModelPtr;
        CBufferPtr        m_SunLightPSBufferPtr;
        CInputLayoutPtr   m_P2InputLayoutPtr;
        CShaderPtr        m_RectangleShaderVSPtr;
        CShaderPtr        m_SunLightShaderPSPtr;
        CRenderContextPtr m_LightRenderContextPtr;
        CRenderJobs       m_RenderJobs;

    private:

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxLightSunRenderer::CGfxLightSunRenderer()
        : m_QuadModelPtr           ()
        , m_SunLightPSBufferPtr    ()
        , m_P2InputLayoutPtr       ()
        , m_SunLightShaderPSPtr    ()
        , m_RectangleShaderVSPtr   ()
        , m_LightRenderContextPtr  ()
        , m_RenderJobs		       ()
    {
        m_RenderJobs.reserve(4);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxLightSunRenderer::~CGfxLightSunRenderer()
    {
    	
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnExit()
    {
        m_QuadModelPtr          = 0;
        m_SunLightPSBufferPtr   = 0;
        m_P2InputLayoutPtr      = 0;
        m_SunLightShaderPSPtr   = 0;
        m_RectangleShaderVSPtr  = 0;
        m_LightRenderContextPtr = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnSetupShader()
    {       
        m_RectangleShaderVSPtr = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");
        
        m_SunLightShaderPSPtr  = ShaderManager::CompilePS("fs_light_sunlight.glsl", "main");
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor QuadInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        m_P2InputLayoutPtr = ShaderManager::CreateInputLayout(QuadInputLayout, 1, m_RectangleShaderVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnSetupRenderTargets()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnSetupStates()
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
    
    void CGfxLightSunRenderer::OnSetupTextures()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSunLightProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_SunLightPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnSetupModels()
    {
        m_QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnSetupEnd()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightSunRenderer::Render()
    {
        if (m_RenderJobs.size() == 0) return;

        Performance::BeginEvent("Sun");

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        

        ContextManager::SetRenderContext(m_LightRenderContextPtr);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_P2InputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

        ContextManager::SetShaderPS(m_SunLightShaderPSPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(4, SamplerManager::GetSampler(CSampler::PCF));

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_SunLightPSBufferPtr);

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

        ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
        ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
        ContextManager::SetTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2));
        ContextManager::SetTexture(3, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());

        // -----------------------------------------------------------------------------
        // Render every sun
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator CurrentRenderJob = m_RenderJobs.begin();
        CRenderJobs::const_iterator EndOfRenderJobs  = m_RenderJobs.end();

        for (; CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
        	Dt::CSunComponent* pDataSunFacet     = CurrentRenderJob->m_pDataSunLightFacet;
        	Gfx::CSunComponent* pGraphicSunFacet = CurrentRenderJob->m_pGraphicSunLightFacet;

        	// -----------------------------------------------------------------------------
            // Upload buffer data
            // -----------------------------------------------------------------------------
            SSunLightProperties LightBuffer;
    
            LightBuffer.m_LightViewProjection  = pGraphicSunFacet->GetCamera()->GetViewProjectionMatrix();
            LightBuffer.m_LightDirection       = glm::normalize(glm::vec4(pDataSunFacet->GetDirection(), 0.0f));
            LightBuffer.m_LightColor           = glm::vec4(pDataSunFacet->GetLightness(), 1.0f);
            LightBuffer.m_SunAngularRadius     = 0.27f * glm::pi<float>() / 180.0f;
            LightBuffer.m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();
    
            BufferManager::UploadBufferData(m_SunLightPSBufferPtr, &LightBuffer);
    
            // -----------------------------------------------------------------------------
            // Prepare last context
            // -----------------------------------------------------------------------------
            ContextManager::SetTexture(4, pGraphicSunFacet->GetShadowMapPtr());
    
            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);
        }

        // -----------------------------------------------------------------------------
        // Reset everything
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);
        ContextManager::ResetTexture(2);
        ContextManager::ResetTexture(3);
        ContextManager::ResetTexture(4);

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        ContextManager::ResetSampler(2);
        ContextManager::ResetSampler(3);
        ContextManager::ResetSampler(4);

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

    void CGfxLightSunRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_RenderJobs.clear();

        auto DataSunComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CSunComponent>();

        for (auto DataComponent : DataSunComponents)
        {
            Dt::CSunComponent*  pDtComponent  = static_cast<Dt::CSunComponent*>(DataComponent);
            Gfx::CSunComponent* pGfxComponent = CComponentManager::GetInstance().GetComponent<Gfx::CSunComponent>(pDtComponent->GetID());

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            // -----------------------------------------------------------------------------
            // Set sun into a new render job
            // -----------------------------------------------------------------------------
            SRenderJob NewRenderJob;

            NewRenderJob.m_pDataSunLightFacet = pDtComponent;
            NewRenderJob.m_pGraphicSunLightFacet = pGfxComponent;

            m_RenderJobs.push_back(NewRenderJob);
        }
    }
} // namespace

namespace Gfx
{
namespace LightSunRenderer
{
    void OnStart()
    {
        CGfxLightSunRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxLightSunRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxLightSunRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxLightSunRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxLightSunRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxLightSunRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxLightSunRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxLightSunRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxLightSunRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxLightSunRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxLightSunRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxLightSunRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxLightSunRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxLightSunRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxLightSunRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxLightSunRenderer::GetInstance().Render();
    }
} // namespace LightSunRenderer
} // namespace Gfx

