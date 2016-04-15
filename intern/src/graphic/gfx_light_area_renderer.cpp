
#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_entity.h"
#include "data/data_light_facet.h"
#include "data/data_map.h"

#include "graphic/gfx_light_area_renderer.h"
#include "graphic/gfx_sky_renderer.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_facet.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_model.h"
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
    class CGfxAreaLightRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxAreaLightRenderer)
        
    public:
        CGfxAreaLightRenderer();
        ~CGfxAreaLightRenderer();
        
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
        void RenderBulbs();
        
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
        
        struct SSphereLightProperties
        {
            Base::Float4 m_LightPosition;
            Base::Float4 m_LightColor;
            Base::Float4 m_LightSettings; // Range, Radius
        };

        struct SDiskLightProperties
        {
            Base::Float4 m_LightPosition;
            Base::Float4 m_LightDirection;
            Base::Float4 m_LightColor;
            Base::Float4 m_LightSettings; // Range, Radius, AngleScale, AngleOffset
        };
        
        struct SRenderJob
        {
//             Dt::CLight*  m_pLightFacet;
//             Dt::CEntity* m_pLevelEntity;
//             
//             CLightPtr   m_LightPtr;
//             CShaderPtr  m_ShadowVS;
//             CShaderPtr  m_ShadowPS;
//             CShaderPtr  m_LightVS;
//             CShaderPtr  m_LightPS;
        };
        
    private:
        
        typedef std::vector<SRenderJob> CRenderJobs;
        
    private:
        
        CModelPtr         m_SphereModelPtr;
        CModelPtr         m_LightBulbModelPtr;
        
        CBufferSetPtr     m_MainVSBufferPtr;
        CBufferSetPtr     m_SphereLightPSBufferPtr;
        CBufferSetPtr     m_DiskLightPSBufferPtr;
        CBufferSetPtr     m_ColorPSBufferPtr;
        
        CInputLayoutPtr   m_QuadInputLayoutPtr;
        CInputLayoutPtr   m_LightProbeInputLayoutPtr;
        
        CShaderPtr        m_ModelVSPtr;
        CShaderPtr        m_Model2DVSPtr;

        CShaderPtr        m_SphereLightShaderPSPtr;

        CShaderPtr        m_DiskLightShaderPSPtr;

        CShaderPtr        m_ColorSpherePSPtr;
        
        CTextureSetPtr    m_AreaLightTextureSetPtr;
        
        CSamplerSetPtr    m_PSSamplerSetPtr;

        CRenderContextPtr m_DefaultRenderContextPtr;
        CRenderContextPtr m_LightRenderContextPtr;

        CRenderJobs            m_SphereLightRenderJobs;
        CRenderJobs            m_DiskLightRenderJobs;
        
    private:
        
        void RenderDirectLight();
        void RenderLightbulbs();
        
        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxAreaLightRenderer::CGfxAreaLightRenderer()
        : m_SphereModelPtr          ()
        , m_LightBulbModelPtr       ()
        , m_MainVSBufferPtr         ()
        , m_SphereLightPSBufferPtr  ()
        , m_DiskLightPSBufferPtr    ()
        , m_ColorPSBufferPtr        ()
        , m_QuadInputLayoutPtr      ()
        , m_LightProbeInputLayoutPtr()
        , m_ModelVSPtr              ()
        , m_Model2DVSPtr            ()
        , m_SphereLightShaderPSPtr  ()
        , m_DiskLightShaderPSPtr    ()
        , m_ColorSpherePSPtr        ()
        , m_AreaLightTextureSetPtr  ()
        , m_PSSamplerSetPtr         ()
        , m_DefaultRenderContextPtr ()
        , m_LightRenderContextPtr   ()
        , m_SphereLightRenderJobs   ()
        , m_DiskLightRenderJobs     ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxAreaLightRenderer::~CGfxAreaLightRenderer()
    {
    
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnExit()
    {
        m_SphereModelPtr           = 0;
        m_LightBulbModelPtr        = 0;
        m_MainVSBufferPtr          = 0;
        m_SphereLightPSBufferPtr   = 0;
        m_DiskLightPSBufferPtr     = 0;
        m_ColorPSBufferPtr         = 0;
        m_QuadInputLayoutPtr       = 0;
        m_LightProbeInputLayoutPtr = 0;
        m_ModelVSPtr               = 0;
        m_Model2DVSPtr             = 0;
        m_SphereLightShaderPSPtr   = 0;
        m_DiskLightShaderPSPtr     = 0;
        m_ColorSpherePSPtr         = 0;
        m_AreaLightTextureSetPtr   = 0;
        m_PSSamplerSetPtr          = 0;
        m_DefaultRenderContextPtr  = 0;
        m_LightRenderContextPtr    = 0;

        m_SphereLightRenderJobs.clear();
        m_DiskLightRenderJobs  .clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupShader()
    {
        CShaderPtr LightVSPtr     = ShaderManager::CompileVS("vs_m_p.glsl"      , "main");
        CShaderPtr LightBulbVSPtr = ShaderManager::CompileVS("vs_vm_p_quad.glsl", "main");

        CShaderPtr SpherePSPtr = ShaderManager::CompilePS("fs_light_spherelight.glsl" , "main");
        CShaderPtr DiskPSPtr   = ShaderManager::CompilePS("fs_light_disklight.glsl"   , "main");
        CShaderPtr ColorPSPtr  = ShaderManager::CompilePS("fs_color_sphere_alpha.glsl", "main");
        
        m_ModelVSPtr   = LightVSPtr;
        m_Model2DVSPtr = LightBulbVSPtr;
        
        m_SphereLightShaderPSPtr = SpherePSPtr;

        m_DiskLightShaderPSPtr = DiskPSPtr;
        
        m_ColorSpherePSPtr = ColorPSPtr;
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor QuadInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        const SInputElementDescriptor PositionInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0, 0, 12, CInputLayout::PerVertex, 0, },
        };
        
        m_QuadInputLayoutPtr       = ShaderManager::CreateInputLayout(QuadInputLayout, 1, m_Model2DVSPtr);
        
        m_LightProbeInputLayoutPtr = ShaderManager::CreateInputLayout(PositionInputLayout, 1, m_ModelVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupRenderTargets()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupStates()
    {
        CCameraPtr          MainCameraPtr      = ViewManager     ::GetMainCamera();

        CViewPortSetPtr     ViewPortSetPtr     = ViewManager     ::GetViewPortSet();

        CTargetSetPtr       LightTargetSetPtr   = TargetSetManager::GetLightAccumulationTargetSet();
        CTargetSetPtr       DefaultTargetSetPtr = TargetSetManager::GetDefaultTargetSet();

        CRenderStatePtr     DefaultStatePtr = StateManager::GetRenderState(0);
        CRenderStatePtr     LightStatePtr   = StateManager::GetRenderState(CRenderState::AdditionBlend);

        // -----------------------------------------------------------------------------
        
        CRenderContextPtr DefaultContextPtr = ContextManager::CreateRenderContext();
        
        DefaultContextPtr->SetCamera(MainCameraPtr);
        DefaultContextPtr->SetViewPortSet(ViewPortSetPtr);
        DefaultContextPtr->SetTargetSet(DefaultTargetSetPtr);
        DefaultContextPtr->SetRenderState(DefaultStatePtr);
        
        m_DefaultRenderContextPtr = DefaultContextPtr;
        
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

        m_PSSamplerSetPtr = SamplerManager::CreateSamplerSet(Sampler, 5);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupTextures()
    {
        CTextureBasePtr GBuffer0TexturePtr          = TargetSetManager::GetDeferredTargetSet()         ->GetRenderTarget(0);
        CTextureBasePtr GBuffer1TexturePtr          = TargetSetManager::GetDeferredTargetSet()         ->GetRenderTarget(1);
        CTextureBasePtr GBuffer2TexturePtr          = TargetSetManager::GetDeferredTargetSet()         ->GetRenderTarget(2);
        CTextureBasePtr DepthTexturePtr             = TargetSetManager::GetDeferredTargetSet()         ->GetDepthStencilTarget();
        CTextureBasePtr LightAccumulationTexturePtr = TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0);

        m_AreaLightTextureSetPtr = TextureManager::CreateTextureSet(GBuffer0TexturePtr, GBuffer1TexturePtr, GBuffer2TexturePtr, DepthTexturePtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupBuffers()
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
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSphereLightProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SphereLightBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SDiskLightProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr DiskLightBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(Base::Float4);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr ColorBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        CBufferPtr HistogramExposureHistoryBufferPtr = HistogramRenderer::GetExposureHistoryBuffer();
        
        // -----------------------------------------------------------------------------
        
        m_MainVSBufferPtr                   = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS(), PerDrawCallConstantBuffer);
        
        m_SphereLightPSBufferPtr            = BufferManager::CreateBufferSet(CameraBuffer, SphereLightBuffer, HistogramExposureHistoryBufferPtr);

        m_DiskLightPSBufferPtr              = BufferManager::CreateBufferSet(CameraBuffer, DiskLightBuffer, HistogramExposureHistoryBufferPtr);
        
        m_ColorPSBufferPtr                  = BufferManager::CreateBufferSet(ColorBuffer);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupModels()
    {
        m_SphereModelPtr    = ModelManager::CreateSphere(1.0f, 8, 8);

        m_LightBulbModelPtr = ModelManager::CreateRectangle(-1.0f, -1.0f, 2.0f, 2.0f);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::Update()
    {
        // -----------------------------------------------------------------------------
        // Build render jobs
        // -----------------------------------------------------------------------------
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::Render()
    {
        Performance::BeginEvent("Area Lights");

        RenderDirectLight();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxAreaLightRenderer::RenderBulbs()
    {
        Performance::BeginEvent("Area Lights Bulbs");

        RenderLightbulbs();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::RenderDirectLight()
    {
        // -----------------------------------------------------------------------------
        // Iterate throw jobs
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator CurrentRenderJob;
        CRenderJobs::const_iterator EndOfRenderJobs;
        const unsigned int          pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Prepare
        // -----------------------------------------------------------------------------
        CurrentRenderJob = m_SphereLightRenderJobs.begin();
        EndOfRenderJobs  = m_SphereLightRenderJobs.end();

        if (CurrentRenderJob == EndOfRenderJobs) return;

        // -----------------------------------------------------------------------------
        // Upload buffer data
        // -----------------------------------------------------------------------------
        CCameraPtr CameraPtr = ViewManager::GetMainCamera();

        SCameraProperties* pPSBuffer = static_cast<SCameraProperties*>(BufferManager::MapConstantBuffer(m_SphereLightPSBufferPtr->GetBuffer(0)));

        assert(pPSBuffer != nullptr);

        Base::Float3 Position = CameraPtr->GetView()->GetPosition();
        Base::Float3 ViewDirection = CameraPtr->GetView()->GetViewDirection();

        pPSBuffer->m_InverseCameraProjection = CameraPtr->GetProjectionMatrix().GetInverted();
        pPSBuffer->m_InverseCameraView       = CameraPtr->GetView()->GetViewMatrix().GetInverted();
        pPSBuffer->m_CameraPosition          = Base::Float4(Position[0], Position[1], Position[2], 1.0f);
        pPSBuffer->m_InvertedScreenSize      = Base::Float4(1.0f / Main::GetActiveWindowSize()[0], 1.0f / Main::GetActiveWindowSize()[1], 0, 0);
        pPSBuffer->m_ExposureHistoryIndex    = HistogramRenderer::GetLastExposureHistoryIndex();

        BufferManager::UnmapConstantBuffer(m_SphereLightPSBufferPtr->GetBuffer(0));
        
        // -----------------------------------------------------------------------------
        // Rendering of light probes
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_LightRenderContextPtr);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        // -----------------------------------------------------------------------------
        // Sphere lights
        // -----------------------------------------------------------------------------
        ContextManager::SetShaderVS(m_ModelVSPtr);
        
        ContextManager::SetShaderPS(m_SphereLightShaderPSPtr);
        
        ContextManager::SetVertexBufferSet(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_LightProbeInputLayoutPtr);
        
        ContextManager::SetConstantBufferSetPS(m_SphereLightPSBufferPtr);
        
        ContextManager::SetConstantBufferSetVS(m_MainVSBufferPtr);
        
        ContextManager::SetTextureSetPS(m_AreaLightTextureSetPtr);
        
        for (; CurrentRenderJob != EndOfRenderJobs; ++ CurrentRenderJob)
        {
//             Dt::CSphereLight* pDataLightFacet = static_cast<Dt::CSphereLight*>(CurrentRenderJob->m_pLightFacet);
//             Dt::CEntity*      pEntity         = CurrentRenderJob->m_pLevelEntity;
//             
//             assert(pDataLightFacet    != nullptr);
//             
//             ContextManager::SetTextureSetPS(m_AreaLightTextureSetPtr);
//             
//             // -----------------------------------------------------------------------------
//             // Upload model matrix to buffer
//             // -----------------------------------------------------------------------------
//             SPerDrawCallConstantBuffer* pModelBuffer = static_cast<SPerDrawCallConstantBuffer*>(BufferManager::MapConstantBuffer(m_MainVSBufferPtr->GetBuffer(1)));
//             
//             assert(pModelBuffer != nullptr);
//             
//             pModelBuffer->m_ModelMatrix = Base::Float4x4::s_Identity;
//             pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetTranslation(pEntity->m_Position);
//             pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetScale(pDataLightFacet->m_Range);
//             
//             BufferManager::UnmapConstantBuffer(m_MainVSBufferPtr->GetBuffer(1));
//             
//             // -----------------------------------------------------------------------------
//             // Upload buffer data
//             // -----------------------------------------------------------------------------
//             SSphereLightProperties* pLightBuffer = static_cast<SSphereLightProperties*>(BufferManager::MapConstantBuffer(m_SphereLightPSBufferPtr->GetBuffer(1)));
//             
//             assert(pLightBuffer != nullptr);
// 
//             float InvSqrAttenuationRadius = 1.0f / (pDataLightFacet->m_Range * pDataLightFacet->m_Range);
//             float Radius                  = pDataLightFacet->m_Radius;
//             
//             float LuminanceIntensity      = pDataLightFacet->m_LuminousPower / (4.0f * Radius * Radius * Base::SConstants<float>::s_Pi * Base::SConstants<float>::s_Pi);
//             
//             pLightBuffer->m_LightPosition  = Base::Float4(pEntity->m_Position[0], pEntity->m_Position[1], pEntity->m_Position[2], 1.0f);
//             pLightBuffer->m_LightColor     = Base::Float4(pDataLightFacet->m_Color[0] * LuminanceIntensity, pDataLightFacet->m_Color[1] * LuminanceIntensity, pDataLightFacet->m_Color[2] * LuminanceIntensity, 1.0f);
//             pLightBuffer->m_LightSettings  = Base::Float4(InvSqrAttenuationRadius, Radius);
//             
//             BufferManager::UnmapConstantBuffer(m_SphereLightPSBufferPtr->GetBuffer(1));
//             
//             ContextManager::DrawIndexed(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        }
        
        ContextManager::ResetTextureSetPS();
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetConstantBufferSetPS();
        
        ContextManager::ResetConstantBufferSetVS();
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBufferSet();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();

        // -----------------------------------------------------------------------------
        // Disk lights
        // -----------------------------------------------------------------------------
        EndOfRenderJobs = m_DiskLightRenderJobs.end();

        ContextManager::SetShaderVS(m_ModelVSPtr);

        ContextManager::SetShaderPS(m_DiskLightShaderPSPtr);

        ContextManager::SetVertexBufferSet(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_LightProbeInputLayoutPtr);

        ContextManager::SetConstantBufferSetPS(m_DiskLightPSBufferPtr);

        ContextManager::SetConstantBufferSetVS(m_MainVSBufferPtr);

        ContextManager::SetTextureSetPS(m_AreaLightTextureSetPtr);

        for (CRenderJobs::const_iterator CurrentRenderJob = m_DiskLightRenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
//             Dt::CDiskLight* pDataLightFacet = static_cast<Dt::CDiskLight*>(CurrentRenderJob->m_pLightFacet);
//             Dt::CEntity*    pEntity         = CurrentRenderJob->m_pLevelEntity;
// 
//             assert(pDataLightFacet != nullptr);
// 
//             ContextManager::SetTextureSetPS(m_AreaLightTextureSetPtr);
// 
//             // -----------------------------------------------------------------------------
//             // Upload model matrix to buffer
//             // -----------------------------------------------------------------------------
//             SPerDrawCallConstantBuffer* pModelBuffer = static_cast<SPerDrawCallConstantBuffer*>(BufferManager::MapConstantBuffer(m_MainVSBufferPtr->GetBuffer(1)));
// 
//             assert(pModelBuffer != nullptr);
// 
//             pModelBuffer->m_ModelMatrix = Base::Float4x4::s_Identity;
//             pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetTranslation(pEntity->m_Position);
//             pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetScale(pDataLightFacet->m_Range);
// 
//             BufferManager::UnmapConstantBuffer(m_MainVSBufferPtr->GetBuffer(1));
// 
//             // -----------------------------------------------------------------------------
//             // Upload buffer data
//             // -----------------------------------------------------------------------------
//             SDiskLightProperties* pLightBuffer = static_cast<SDiskLightProperties*>(BufferManager::MapConstantBuffer(m_DiskLightPSBufferPtr->GetBuffer(1)));
// 
//             assert(pLightBuffer != nullptr);
// 
//             float InvSqrAttenuationRadius = 1.0f / (pDataLightFacet->m_Range * pDataLightFacet->m_Range);
//             float AngleScale              = 1.0f / Base::Max(0.001f, (Base::Cos(Base::DegreesToRadians(pDataLightFacet->m_InnerAngle)) - Base::Cos(Base::DegreesToRadians(pDataLightFacet->m_OuterAngle))));
//             float AngleOffset             = -Base::Cos(Base::DegreesToRadians(pDataLightFacet->m_OuterAngle)) * AngleScale;
//             float Radius                  = pDataLightFacet->m_Radius;
//             
//             float LuminanceIntensity      = pDataLightFacet->m_LuminousPower / (Radius * Radius * Base::SConstants<float>::s_Pi * Base::SConstants<float>::s_Pi);
// 
//             pLightBuffer->m_LightPosition  = Base::Float4(pEntity->m_Position[0], pEntity->m_Position[1], pEntity->m_Position[2], 1.0f);
//             pLightBuffer->m_LightDirection = Base::Float4(-pDataLightFacet->m_Direction[0], -pDataLightFacet->m_Direction[1], -pDataLightFacet->m_Direction[2], 0.0f);
//             pLightBuffer->m_LightColor     = Base::Float4(pDataLightFacet->m_Color[0] * LuminanceIntensity, pDataLightFacet->m_Color[1] * LuminanceIntensity, pDataLightFacet->m_Color[2] * LuminanceIntensity, 1.0f);
//             pLightBuffer->m_LightSettings  = Base::Float4(InvSqrAttenuationRadius, Radius, AngleScale, AngleOffset);
// 
//             BufferManager::UnmapConstantBuffer(m_DiskLightPSBufferPtr->GetBuffer(1));
// 
//             ContextManager::DrawIndexed(m_SphereModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        }

        ContextManager::ResetTextureSetPS();

        ContextManager::ResetInputLayout();

        ContextManager::ResetConstantBufferSetPS();

        ContextManager::ResetConstantBufferSetVS();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();
        
        // -----------------------------------------------------------------------------
        // Reset non-dynamic objects
        // -----------------------------------------------------------------------------
        ContextManager::ResetTopology();
        
        ContextManager::ResetSamplerSetPS();
        
        ContextManager::ResetRenderContext();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::RenderLightbulbs()
    {
        // -----------------------------------------------------------------------------
        // Render light bulbs of area lights
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator EndOfRenderJobs;
        const unsigned int          pOffset[]       = {0, 0};
        
        ContextManager::SetRenderContext(m_DefaultRenderContextPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        // -----------------------------------------------------------------------------
        // Sphere and disk lights
        // -----------------------------------------------------------------------------
        EndOfRenderJobs = m_SphereLightRenderJobs.end();
        
        ContextManager::SetShaderVS(m_Model2DVSPtr);
        
        ContextManager::SetShaderPS(m_ColorSpherePSPtr);
        
        ContextManager::SetVertexBufferSet(m_LightBulbModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_LightBulbModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);
        
        ContextManager::SetConstantBufferSetPS(m_ColorPSBufferPtr);
        
        ContextManager::SetConstantBufferSetVS(m_MainVSBufferPtr);
        
        for (CRenderJobs::const_iterator CurrentRenderJob = m_SphereLightRenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++ CurrentRenderJob)
        {
//             Dt::CSphereLight* pDataLightFacet = static_cast<Dt::CSphereLight*>(CurrentRenderJob->m_pLightFacet);
//             Dt::CEntity*      pEntity         = CurrentRenderJob->m_pLevelEntity;
//             
//             assert(pDataLightFacet    != nullptr);
//             
//             // -----------------------------------------------------------------------------
//             // Upload model matrix to buffer
//             // -----------------------------------------------------------------------------
//             SPerDrawCallConstantBuffer* pModelBuffer = static_cast<SPerDrawCallConstantBuffer*>(BufferManager::MapConstantBuffer(m_MainVSBufferPtr->GetBuffer(1)));
//             
//             assert(pModelBuffer != nullptr);
//             
//             pModelBuffer->m_ModelMatrix  = Base::Float4x4::s_Identity;
//             pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetTranslation(pEntity->m_Position);
//             pModelBuffer->m_ModelMatrix *= ViewManager::GetMainCamera()->GetView()->GetRotationMatrix().GetTransposed();
//             pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetRotation(Base::DegreesToRadians(-180.0f), 0.0f, 0.0f);
//             pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetScale(pDataLightFacet->m_Radius);
//             
//             BufferManager::UnmapConstantBuffer(m_MainVSBufferPtr->GetBuffer(1));
//             
//             // -----------------------------------------------------------------------------
//             // Upload buffer data
//             // -----------------------------------------------------------------------------
//             Base::Float4* pColor = static_cast<Base::Float4*>(BufferManager::MapConstantBuffer(m_ColorPSBufferPtr->GetBuffer(0)));
//             
//             assert(pColor != nullptr);
//             
//             (*pColor)[0] = pDataLightFacet->m_Color[0];
//             (*pColor)[1] = pDataLightFacet->m_Color[1];
//             (*pColor)[2] = pDataLightFacet->m_Color[2];
//             (*pColor)[3] = 1.0f;
//             
//             BufferManager::UnmapConstantBuffer(m_ColorPSBufferPtr->GetBuffer(0));
//             
//             ContextManager::DrawIndexed(m_LightBulbModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        }

        EndOfRenderJobs = m_DiskLightRenderJobs.end();

        for (CRenderJobs::const_iterator CurrentRenderJob = m_DiskLightRenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
//             Dt::CDiskLight* pDataLightFacet = static_cast<Dt::CDiskLight*>(CurrentRenderJob->m_pLightFacet);
//             Dt::CEntity*    pEntity = CurrentRenderJob->m_pLevelEntity;
// 
//             assert(pDataLightFacet != nullptr);
// 
//             // -----------------------------------------------------------------------------
//             // Upload model matrix to buffer
//             // -----------------------------------------------------------------------------
//             SPerDrawCallConstantBuffer* pModelBuffer = static_cast<SPerDrawCallConstantBuffer*>(BufferManager::MapConstantBuffer(m_MainVSBufferPtr->GetBuffer(1)));
// 
//             assert(pModelBuffer != nullptr);
// 
//             pModelBuffer->m_ModelMatrix = Base::Float4x4::s_Identity;
//             pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetTranslation(pEntity->m_Position);
//             pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetRotation(pDataLightFacet->m_Direction[0], pDataLightFacet->m_Direction[1], pDataLightFacet->m_Direction[2]);
//             pModelBuffer->m_ModelMatrix *= Base::Float4x4().SetScale(pDataLightFacet->m_Radius);
// 
//             BufferManager::UnmapConstantBuffer(m_MainVSBufferPtr->GetBuffer(1));
// 
//             // -----------------------------------------------------------------------------
//             // Upload buffer data
//             // -----------------------------------------------------------------------------
//             Base::Float4* pColor = static_cast<Base::Float4*>(BufferManager::MapConstantBuffer(m_ColorPSBufferPtr->GetBuffer(0)));
// 
//             assert(pColor != nullptr);
// 
//             (*pColor)[0] = pDataLightFacet->m_Color[0];
//             (*pColor)[1] = pDataLightFacet->m_Color[1];
//             (*pColor)[2] = pDataLightFacet->m_Color[2];
//             (*pColor)[3] = 1.0f;
// 
//             BufferManager::UnmapConstantBuffer(m_ColorPSBufferPtr->GetBuffer(0));
// 
//             ContextManager::DrawIndexed(m_LightBulbModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        }
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetConstantBufferSetPS();
        
        ContextManager::ResetConstantBufferSetVS();
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBufferSet();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();

        // -----------------------------------------------------------------------------
        // Reset non-dynamic objects
        // -----------------------------------------------------------------------------
        ContextManager::ResetTopology();
        
        ContextManager::ResetSamplerSetPS();
        
        ContextManager::ResetRenderContext();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxAreaLightRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_SphereLightRenderJobs  .clear();
        m_DiskLightRenderJobs    .clear();
        
        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();
        
        for (; CurrentEntity != EndOfEntities; )
        {
//             Dt::CEntity& rCurrentEntity = *CurrentEntity;
//             
//             Dt::CLight* pLightFacet  = static_cast<Dt::CLight*>(rCurrentEntity.m_Facets.GetFacet(Dt::CEntity::Light)  );
//             CEntity*    pLightEntity = static_cast<CEntity*   >(rCurrentEntity.m_Facets.GetFacet(Dt::CEntity::Graphic));
//             
//             if (pLightFacet == 0 || pLightEntity == 0 || pLightEntity->m_Facets.HasFacet(CEntity::Light) == false)
//             {
//                 CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
// 
//                 continue;
//             }
//             
//             SRenderJob NewRenderJob;
//             
//             NewRenderJob.m_pLightFacet  = pLightFacet;
//             NewRenderJob.m_pLevelEntity = &rCurrentEntity;
//             NewRenderJob.m_LightPtr     = static_cast<CLightFacet*>(pLightEntity->m_Facets.GetFacet(CEntity::Light))->m_LightPtr;
//             
//             if (pLightFacet->m_Category == Dt::SLightCategory::Sphere)
//             {
//                 NewRenderJob.m_LightVS = m_ModelVSPtr;
//                 NewRenderJob.m_LightPS = 0;
//                 
//                 m_SphereLightRenderJobs.push_back(NewRenderJob);
//             }
//             else if (pLightFacet->m_Category == Dt::SLightCategory::Disk)
//             {
//                 NewRenderJob.m_LightVS = m_ModelVSPtr;
//                 NewRenderJob.m_LightPS = 0;
// 
//                 m_DiskLightRenderJobs.push_back(NewRenderJob);
//             }
            
            // -----------------------------------------------------------------------------
            // Get next light
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }
    }
} // namespace


namespace Gfx
{
namespace LightAreaRenderer
{
    void OnStart()
    {
        CGfxAreaLightRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxAreaLightRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxAreaLightRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxAreaLightRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxAreaLightRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxAreaLightRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxAreaLightRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxAreaLightRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void RenderBulbs()
    {
        CGfxAreaLightRenderer::GetInstance().RenderBulbs();
    }
} // namespace LightAreaRenderer
} // namespace Gfx
