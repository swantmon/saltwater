
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_entity.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_transformation_facet.h"
#include "data/data_point_light_facet.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_point_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_point_light_facet.h"
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
    class CGfxPointLightRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxPointLightRenderer)
        
    public:
        CGfxPointLightRenderer();
        ~CGfxPointLightRenderer();
        
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
        
        struct SPerDrawCallConstantBuffer
        {
            Base::Float4x4 m_ModelMatrix;
        };
        
        struct SCameraProperties
        {
            Base::Float4x4 m_InverseCameraProjection;
            Base::Float4x4 m_InverseCameraView;
            Base::Float4   m_CameraPosition;
            Base::Float4   m_InvertedScreenSize;
            unsigned int   m_ExposureHistoryIndex;
        };
        
        struct SPunctualLightProperties
        {
            Base::Float4 m_LightPosition;
            Base::Float4 m_LightDirection;
            Base::Float4 m_LightColor;
            Base::Float4 m_LightSettings; // InvSqrAttenuationRadius, AngleScale, AngleOffset, WithShadow
            Base::Float4x4 m_LightViewProjection;
        };
        
        struct SRenderJob
        {
            Dt::CPointLightFacet* m_pDataLightFacet;
            Dt::CEntity*          m_pLevelEntity;
            
            Gfx::CPointLightFacet* m_pGfxLightFacet;
            CShaderPtr  m_ShadowVS;
            CShaderPtr  m_ShadowPS;
            CShaderPtr  m_LightVS;
            CShaderPtr  m_LightPS;
        };
        
    private:
        
        typedef std::vector<SRenderJob> CRenderJobs;
        
    private:
        
        CMeshPtr          m_SphereModelPtr;
        
        CBufferSetPtr     m_MainVSBufferPtr;
        CBufferSetPtr     m_PunctualLightPSBufferPtr;
        
        CInputLayoutPtr   m_LightProbeInputLayoutPtr;
        
        CShaderPtr        m_ModelVSPtr;

        CShaderPtr        m_PunctualLightShaderPSPtr;
        
        CTextureSetPtr    m_PunctualLightTextureSetPtr;
        
        CSamplerSetPtr    m_PSSamplerSetPtr;

        CRenderContextPtr m_LightRenderContextPtr;

        CRenderJobs       m_PunctualLightRenderJobs;
        
    private:
        
        void RenderDirectLight();
        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxPointLightRenderer::CGfxPointLightRenderer()
        : m_SphereModelPtr            ()
        , m_MainVSBufferPtr           ()
        , m_PunctualLightPSBufferPtr  ()
        , m_LightProbeInputLayoutPtr  ()
        , m_ModelVSPtr                ()
        , m_PunctualLightShaderPSPtr  ()
        , m_PunctualLightTextureSetPtr()
        , m_PSSamplerSetPtr           ()
        , m_PunctualLightRenderJobs   ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxPointLightRenderer::~CGfxPointLightRenderer()
    {
    
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnExit()
    {
        m_SphereModelPtr                    = 0;
        m_MainVSBufferPtr                   = 0;
        m_PunctualLightPSBufferPtr          = 0;
        m_LightProbeInputLayoutPtr          = 0;
        m_ModelVSPtr                        = 0;
        m_PunctualLightShaderPSPtr          = 0;
        m_PunctualLightTextureSetPtr        = 0;
        m_PSSamplerSetPtr                   = 0;
        m_LightRenderContextPtr             = 0;
        
        m_PunctualLightRenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnSetupShader()
    {
        m_ModelVSPtr = ShaderManager::CompileVS("vs_m_p.glsl", "main");
     
        m_PunctualLightShaderPSPtr = ShaderManager::CompilePS("fs_light_punctuallight.glsl", "main");
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor PositionInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0, 0, 12, CInputLayout::PerVertex, 0, },
        };
        
        m_LightProbeInputLayoutPtr = ShaderManager::CreateInputLayout(PositionInputLayout, 1, m_ModelVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnSetupRenderTargets()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnSetupStates()
    {
        CCameraPtr          MainCameraPtr      = ViewManager     ::GetMainCamera();

        CViewPortSetPtr     ViewPortSetPtr     = ViewManager     ::GetViewPortSet();

        CRenderStatePtr     LightStatePtr      = StateManager    ::GetRenderState(CRenderState::AdditionBlend);

        CTargetSetPtr       LightTargetSetPtr  = TargetSetManager::GetLightAccumulationTargetSet();

        // -----------------------------------------------------------------------------
        
        CRenderContextPtr LightContextPtr = ContextManager::CreateRenderContext();
        
        LightContextPtr->SetCamera(MainCameraPtr);
        LightContextPtr->SetViewPortSet(ViewPortSetPtr);
        LightContextPtr->SetTargetSet(LightTargetSetPtr);
        LightContextPtr->SetRenderState(LightStatePtr);
        
        m_LightRenderContextPtr = LightContextPtr;
        
        // -----------------------------------------------------------------------------
        
        CSamplerPtr Sampler[6];
        
        Sampler[0] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[1] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[2] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[3] = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);
        Sampler[4] = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);
        Sampler[5] = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);
        
        m_PSSamplerSetPtr = SamplerManager::CreateSamplerSet(Sampler, 6);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnSetupTextures()
    {
        CTextureBasePtr GBuffer0TexturePtr          = TargetSetManager::GetDeferredTargetSet()         ->GetRenderTarget(0);
        CTextureBasePtr GBuffer1TexturePtr          = TargetSetManager::GetDeferredTargetSet()         ->GetRenderTarget(1);
        CTextureBasePtr GBuffer2TexturePtr          = TargetSetManager::GetDeferredTargetSet()         ->GetRenderTarget(2);
        CTextureBasePtr DepthTexturePtr             = TargetSetManager::GetDeferredTargetSet()         ->GetDepthStencilTarget();
        
        m_PunctualLightTextureSetPtr = TextureManager::CreateTextureSet(GBuffer0TexturePtr, GBuffer1TexturePtr, GBuffer2TexturePtr, DepthTexturePtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnSetupBuffers()
    {
        // -----------------------------------------------------------------------------
        // Setup view buffer for post rendering
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PerDrawCallConstantBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCameraProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr CameraBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPunctualLightProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PointLightBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        CBufferPtr HistogramExposureHistoryBufferPtr = HistogramRenderer::GetExposureHistoryBuffer();
        
        // -----------------------------------------------------------------------------
        
        m_MainVSBufferPtr                   = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS(), PerDrawCallConstantBuffer);
        
        m_PunctualLightPSBufferPtr          = BufferManager::CreateBufferSet(CameraBuffer, PointLightBuffer, HistogramExposureHistoryBufferPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnSetupModels()
    {
        m_SphereModelPtr = MeshManager::CreateSphere(1.0f, 8, 8);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::Update()
    {
        // -----------------------------------------------------------------------------
        // Build render jobs
        // -----------------------------------------------------------------------------
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::Render()
    {
        Performance::BeginEvent("Punctual Lights");

        RenderDirectLight();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::RenderDirectLight()
    {
        if (m_PunctualLightRenderJobs.size() == 0) return;

        // -----------------------------------------------------------------------------
        // Iterate throw jobs (directional, point and spot)
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator CurrentRenderJob;
        CRenderJobs::const_iterator EndOfRenderJobs;
        const unsigned int          pOffset[]       = {0, 0};

        // -----------------------------------------------------------------------------
        // Upload data
        // -----------------------------------------------------------------------------
        CCameraPtr CameraPtr = ViewManager::GetMainCamera();
        
        SCameraProperties* pPSBuffer = static_cast<SCameraProperties*>(BufferManager::MapConstantBuffer(m_PunctualLightPSBufferPtr->GetBuffer(0), CBuffer::Write));
        
        assert(pPSBuffer != nullptr);
        
        Base::Float3 Position = CameraPtr->GetView()->GetPosition();
        
        pPSBuffer->m_InverseCameraProjection = CameraPtr->GetProjectionMatrix().GetInverted();
        pPSBuffer->m_InverseCameraView       = CameraPtr->GetView()->GetViewMatrix().GetInverted();
        pPSBuffer->m_CameraPosition          = Base::Float4(Position[0], Position[1], Position[2], 1.0f);
        pPSBuffer->m_InvertedScreenSize      = Base::Float4(1.0f / Main::GetActiveWindowSize()[0], 1.0f / Main::GetActiveWindowSize()[1], 0, 0);
        pPSBuffer->m_ExposureHistoryIndex    = HistogramRenderer::GetLastExposureHistoryIndex();
        
        BufferManager::UnmapConstantBuffer(m_PunctualLightPSBufferPtr->GetBuffer(0));
        
        // -----------------------------------------------------------------------------
        // Rendering of light sources point
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_LightRenderContextPtr);

        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        // -----------------------------------------------------------------------------
        // Iterate over every point light
        // -----------------------------------------------------------------------------
        CurrentRenderJob = m_PunctualLightRenderJobs.begin();
        EndOfRenderJobs  = m_PunctualLightRenderJobs.end();

        for (; CurrentRenderJob != EndOfRenderJobs; ++ CurrentRenderJob)
        {
            Dt::CPointLightFacet* pDtLightFacet    = static_cast<Dt::CPointLightFacet*>(CurrentRenderJob->m_pDataLightFacet);
            Dt::CEntity*          pEntity            = CurrentRenderJob->m_pLevelEntity;
            
            assert(pDtLightFacet    != nullptr);

            // -----------------------------------------------------------------------------
            // Upload model matrix to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBuffer* pModelBuffer = static_cast<SPerDrawCallConstantBuffer*>(BufferManager::MapConstantBuffer(m_MainVSBufferPtr->GetBuffer(1), CBuffer::Write));
            
            assert(pModelBuffer != nullptr);
            
            pModelBuffer->m_ModelMatrix = Base::Float4x4::s_Identity;
            pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetTranslation(pEntity->GetWorldPosition());
            pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetScale(pDtLightFacet->GetAttenuationRadius());
            
            BufferManager::UnmapConstantBuffer(m_MainVSBufferPtr->GetBuffer(1));
            
            // -----------------------------------------------------------------------------
            // Upload buffer data
            // -----------------------------------------------------------------------------
            SPunctualLightProperties* pLightBuffer = static_cast<SPunctualLightProperties*>(BufferManager::MapConstantBuffer(m_PunctualLightPSBufferPtr->GetBuffer(1), CBuffer::Write));
            
            assert(pLightBuffer != nullptr);
            
            float InvSqrAttenuationRadius = pDtLightFacet->GetReciprocalSquaredAttenuationRadius();
            float AngleScale              = pDtLightFacet->GetAngleScale();
            float AngleOffset             = pDtLightFacet->GetAngleOffset();
            float HasShadows              = pDtLightFacet->GetShadowType() != Dt::CPointLightFacet::NoShadows ? 1.0f : 0.0f;
            
            pLightBuffer->m_LightPosition       = Base::Float4(pEntity->GetWorldPosition(), 1.0f);
            pLightBuffer->m_LightDirection      = Base::Float4(pDtLightFacet->GetDirection(), 0.0f).Normalize();
            pLightBuffer->m_LightColor          = Base::Float4(pDtLightFacet->GetLightness(), 1.0f);
            pLightBuffer->m_LightSettings       = Base::Float4(InvSqrAttenuationRadius, AngleScale, AngleOffset, HasShadows);

            if (pDtLightFacet->GetShadowType() != Dt::CPointLightFacet::NoShadows)
            {
                assert(CurrentRenderJob->m_pGfxLightFacet->GetCamera().IsValid());

                pLightBuffer->m_LightViewProjection = CurrentRenderJob->m_pGfxLightFacet->GetCamera()->GetViewProjectionMatrix();
            }
            
            BufferManager::UnmapConstantBuffer(m_PunctualLightPSBufferPtr->GetBuffer(1));

            // -----------------------------------------------------------------------------
            // Render punctual lights
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(m_ModelVSPtr);

            ContextManager::SetShaderPS(m_PunctualLightShaderPSPtr);

            ContextManager::SetVertexBufferSet(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(m_LightProbeInputLayoutPtr);

            ContextManager::SetConstantBufferSetPS(m_PunctualLightPSBufferPtr);

            ContextManager::SetConstantBufferSetVS(m_MainVSBufferPtr);

            ContextManager::SetTextureSetPS(m_PunctualLightTextureSetPtr);
            
            // -----------------------------------------------------------------------------
            // Set shadow map
            // -----------------------------------------------------------------------------
            if (pDtLightFacet->GetShadowType() != Dt::CPointLightFacet::NoShadows)
            {
                ContextManager::SetTextureSetPS(CurrentRenderJob->m_pGfxLightFacet->GetTextureSMSet());
            }
            
            // -----------------------------------------------------------------------------
            // Draw light
            // -----------------------------------------------------------------------------
            ContextManager::DrawIndexed(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

            ContextManager::ResetTextureSetPS();

            ContextManager::ResetInputLayout();

            ContextManager::ResetConstantBufferSetPS();

            ContextManager::ResetConstantBufferSetVS();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBufferSet();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderPS();
        }
        
        // -----------------------------------------------------------------------------
        // Reset non-dynamic objects
        // -----------------------------------------------------------------------------
        ContextManager::ResetTopology();
        
        ContextManager::ResetSamplerSetPS();
        
        ContextManager::ResetRenderContext();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_PunctualLightRenderJobs.clear();
        
        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();
        
        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            if (rCurrentEntity.GetType() != Dt::SLightType::Point)
            {
                CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);

                continue;
            }
            
            Dt::CPointLightFacet*   pDataLightFacet    = static_cast<Dt::CPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
            Gfx::CPointLightFacet*  pGraphicLightFacet = static_cast<Gfx::CPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));
            
            SRenderJob NewRenderJob;
                
            NewRenderJob.m_pDataLightFacet        = pDataLightFacet;
            NewRenderJob.m_pLevelEntity           = &rCurrentEntity;
            NewRenderJob.m_pGfxLightFacet     = pGraphicLightFacet;
                
            NewRenderJob.m_LightVS = m_ModelVSPtr;
            NewRenderJob.m_LightPS = m_PunctualLightShaderPSPtr;

            m_PunctualLightRenderJobs.push_back(NewRenderJob);
            
            // -----------------------------------------------------------------------------
            // Get next light
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }
    }
} // namespace


namespace Gfx
{
namespace LightPointRenderer
{
    void OnStart()
    {
        CGfxPointLightRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxPointLightRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxPointLightRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxPointLightRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxPointLightRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxPointLightRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxPointLightRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxPointLightRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxPointLightRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxPointLightRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxPointLightRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxPointLightRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxPointLightRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxPointLightRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxPointLightRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxPointLightRenderer::GetInstance().Render();
    }
} // namespace LightPointRenderer
} // namespace Gfx
