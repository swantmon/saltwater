
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
        };
        
    private:
        
        typedef std::vector<SRenderJob> CRenderJobs;
        
    private:
        
        CMeshPtr          m_SphereModelPtr;

        CBufferSetPtr     m_MainVSBufferPtr;

        CBufferSetPtr     m_PunctualLightPSBufferPtr;
        
        CShaderPtr        m_PunctualLightShaderPSPtr;

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
        , m_PunctualLightShaderPSPtr  ()
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
        m_PunctualLightShaderPSPtr          = 0;
        m_LightRenderContextPtr             = 0;
        
        m_PunctualLightRenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnSetupShader()
    {
        m_PunctualLightShaderPSPtr = ShaderManager::CompilePS("fs_light_punctuallight.glsl", "main");
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

        CRenderStatePtr     LightStatePtr      = StateManager    ::GetRenderState(CRenderState::AdditionBlend | CRenderState::NoCull);

        CTargetSetPtr       LightTargetSetPtr  = TargetSetManager::GetLightAccumulationTargetSet();

        // -----------------------------------------------------------------------------
        
        CRenderContextPtr LightContextPtr = ContextManager::CreateRenderContext();
        
        LightContextPtr->SetCamera(MainCameraPtr);
        LightContextPtr->SetViewPortSet(ViewPortSetPtr);
        LightContextPtr->SetTargetSet(LightTargetSetPtr);
        LightContextPtr->SetRenderState(LightStatePtr);
        
        m_LightRenderContextPtr = LightContextPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPointLightRenderer::OnSetupTextures()
    {
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
        
        m_MainVSBufferPtr                   = BufferManager::CreateBufferSet(PerDrawCallConstantBuffer);
        
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

        // -----------------------------------------------------------------------------
        // Upload data
        // -----------------------------------------------------------------------------
        CCameraPtr CameraPtr = ViewManager::GetMainCamera();
        
        SCameraProperties CameraProperties;
        
        Base::Float3 Position = CameraPtr->GetView()->GetPosition();
        
        CameraProperties.m_InverseCameraProjection = CameraPtr->GetProjectionMatrix().GetInverted();
        CameraProperties.m_InverseCameraView       = CameraPtr->GetView()->GetViewMatrix().GetInverted();
        CameraProperties.m_CameraPosition          = Base::Float4(Position[0], Position[1], Position[2], 1.0f);
        CameraProperties.m_InvertedScreenSize      = Base::Float4(1.0f / Main::GetActiveWindowSize()[0], 1.0f / Main::GetActiveWindowSize()[1], 0, 0);
        CameraProperties.m_ExposureHistoryIndex    = HistogramRenderer::GetLastExposureHistoryIndex();
        
        BufferManager::UploadBufferData(m_PunctualLightPSBufferPtr->GetBuffer(0), &CameraProperties);
        
        // -----------------------------------------------------------------------------
        // Rendering of light sources point
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_LightRenderContextPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(4, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        ContextManager::SetTopology(STopology::TriangleList);

        // -----------------------------------------------------------------------------
        // Set static stuff
        // -----------------------------------------------------------------------------
        ContextManager::SetShaderVS(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetMVPShaderVS());

        ContextManager::SetShaderPS(m_PunctualLightShaderPSPtr);

        ContextManager::SetVertexBuffer(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetMVPShaderVS()->GetInputLayout());

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_MainVSBufferPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(2, m_PunctualLightPSBufferPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(3, m_PunctualLightPSBufferPtr->GetBuffer(1));

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

        ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
        ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
        ContextManager::SetTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2));
        ContextManager::SetTexture(3, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());

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
            SPerDrawCallConstantBuffer ModelBuffer;
            
            ModelBuffer.m_ModelMatrix = Base::Float4x4::s_Identity;
            ModelBuffer.m_ModelMatrix *= Base::Float4x4().SetTranslation(pEntity->GetWorldPosition());
            ModelBuffer.m_ModelMatrix *= Base::Float4x4().SetScale(pDtLightFacet->GetAttenuationRadius());
            
            BufferManager::UploadBufferData(m_MainVSBufferPtr->GetBuffer(0), &ModelBuffer);
            
            // -----------------------------------------------------------------------------
            // Upload buffer data
            // -----------------------------------------------------------------------------
            SPunctualLightProperties LightBuffer;
            
            float InvSqrAttenuationRadius = pDtLightFacet->GetReciprocalSquaredAttenuationRadius();
            float AngleScale              = pDtLightFacet->GetAngleScale();
            float AngleOffset             = pDtLightFacet->GetAngleOffset();
            float HasShadows              = pDtLightFacet->GetShadowType() != Dt::CPointLightFacet::NoShadows ? 1.0f : 0.0f;
            
            LightBuffer.m_LightPosition  = Base::Float4(pEntity->GetWorldPosition(), 1.0f);
            LightBuffer.m_LightDirection = Base::Float4(pDtLightFacet->GetDirection(), 0.0f).Normalize();
            LightBuffer.m_LightColor     = Base::Float4(pDtLightFacet->GetLightness(), 1.0f);
            LightBuffer.m_LightSettings  = Base::Float4(InvSqrAttenuationRadius, AngleScale, AngleOffset, HasShadows);

            LightBuffer.m_LightViewProjection.SetIdentity();

            if (pDtLightFacet->GetShadowType() != Dt::CPointLightFacet::NoShadows)
            {
                assert(CurrentRenderJob->m_pGfxLightFacet->GetCamera().IsValid());

                LightBuffer.m_LightViewProjection = CurrentRenderJob->m_pGfxLightFacet->GetCamera()->GetViewProjectionMatrix();
            }
            
            BufferManager::UploadBufferData(m_PunctualLightPSBufferPtr->GetBuffer(1), &LightBuffer);

            // -----------------------------------------------------------------------------
            // Set shadow map
            // -----------------------------------------------------------------------------
            if (pDtLightFacet->GetShadowType() != Dt::CPointLightFacet::NoShadows)
            {
                ContextManager::SetTexture(4, CurrentRenderJob->m_pGfxLightFacet->GetTextureSMSet()->GetTexture(0));
            }
            
            // -----------------------------------------------------------------------------
            // Draw light
            // -----------------------------------------------------------------------------
            ContextManager::DrawIndexed(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        }

        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);
        ContextManager::ResetTexture(2);
        ContextManager::ResetTexture(3);
        ContextManager::ResetTexture(4);

        ContextManager::ResetInputLayout();

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);
        ContextManager::ResetConstantBuffer(2);
        ContextManager::ResetConstantBuffer(3);

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();
        
        ContextManager::ResetTopology();
        
        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        ContextManager::ResetSampler(2);
        ContextManager::ResetSampler(3);
        ContextManager::ResetSampler(4);
        
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
