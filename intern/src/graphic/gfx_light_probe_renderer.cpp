
#include "app/app_application.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_entity.h"
#include "data/data_light_facet.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_light_facet.h"
#include "graphic/gfx_light_probe_renderer.h"
#include "graphic/gfx_model_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_sky_renderer.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxLightProbeRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxLightProbeRenderer)
        
    public:
        
        CGfxLightProbeRenderer();
        ~CGfxLightProbeRenderer();
        
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

        struct SGlobalProbeRenderJob
        {
            Dt::CGlobalProbeLightFacet*  m_pDataGlobalProbe;
            Gfx::CGlobalProbeLightFacet* m_pGraphicGlobalProbe;
            CTextureSetPtr               m_CubemapTextureSetPtr;
        };

        struct SViewBuffer
        {
            Base::Float4x4 m_View;
            Base::Float4x4 m_Projection;
        };
        
        struct SConstantBufferGS
        {
            Base::Float4x4 m_CubeProjectionMatrix;
            Base::Float4x4 m_CubeViewMatrix[6];
        };

        struct SConstantBufferPS
        {
            float m_HDRFactor;
            float m_IsHDR;
        };
        
        struct SSpecularCubemapSettings
        {
            float m_LinearRoughness;
            float m_NumberOfMiplevels;
        };

    private:

        typedef std::vector<SGlobalProbeRenderJob> CGlobalProbeRenderJobs;
        
    private:
        
        CModelPtr m_EnvironmentSpherePtr;
        
        CShaderPtr m_FilteringVSPtr;
        CShaderPtr m_FilteringGSPtr;
        CShaderPtr m_FilteringDiffusePSPtr;
        CShaderPtr m_FilteringSpecularPSPtr;

        CShaderPtr m_CustomVSPtr;
        CShaderPtr m_CustomGSPtr;
        CShaderPtr m_CustomPSPtr;
        
        CBufferSetPtr m_CubemapGSBufferSetPtr;
        CBufferSetPtr m_FilteringPSBufferSetPtr;
        CBufferSetPtr m_CustomVSBufferSetPtr;
        CBufferSetPtr m_CustomPSBufferSetPtr;
        
        CInputLayoutPtr m_PositionInputLayoutPtr;
        
        CSamplerSetPtr m_PSSamplerSetPtr;
        
        CRenderContextPtr m_CubemapRenderContextPtr;

        CGlobalProbeRenderJobs m_GlobalProbeRenderJobs;       

    private:

        void RenderEnvironment();

        void RenderFiltering();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxLightProbeRenderer::CGfxLightProbeRenderer()
        : m_EnvironmentSpherePtr   ()
        , m_FilteringVSPtr         ()
        , m_FilteringGSPtr         ()
        , m_FilteringDiffusePSPtr  ()
        , m_FilteringSpecularPSPtr ()
        , m_CustomVSPtr            ()
        , m_CustomGSPtr            ()
        , m_CustomPSPtr            ()
        , m_CubemapGSBufferSetPtr  ()
        , m_FilteringPSBufferSetPtr()
        , m_CustomVSBufferSetPtr   ()
        , m_CustomPSBufferSetPtr   ()
        , m_PositionInputLayoutPtr ()
        , m_PSSamplerSetPtr        ()
        , m_CubemapRenderContextPtr()
        , m_GlobalProbeRenderJobs  ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxLightProbeRenderer::~CGfxLightProbeRenderer()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightProbeRenderer::OnStart()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxLightProbeRenderer::OnExit()
    {
        m_EnvironmentSpherePtr = 0;
        
        m_FilteringVSPtr         = 0;
        m_FilteringGSPtr         = 0;
        m_FilteringDiffusePSPtr  = 0;
        m_FilteringSpecularPSPtr = 0;

        m_CustomVSPtr = 0;
        m_CustomGSPtr = 0;
        m_CustomPSPtr = 0;
        
        m_CubemapGSBufferSetPtr   = 0;
        m_FilteringPSBufferSetPtr = 0;

        m_CustomVSBufferSetPtr = 0;
        m_CustomPSBufferSetPtr = 0;
        
        m_PositionInputLayoutPtr = 0;
        
        m_PSSamplerSetPtr = 0;
        
        m_CubemapRenderContextPtr = 0;

        m_GlobalProbeRenderJobs.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnSetupShader()
    {
        m_FilteringVSPtr = ShaderManager::CompileVS("vs_lightprobe_sampling.glsl", "main");

        m_FilteringGSPtr = ShaderManager::CompileGS("gs_lightprobe_sampling.glsl", "main");

        m_FilteringDiffusePSPtr = ShaderManager::CompilePS("fs_lightprobe_diffuse_sampling.glsl", "main");

        m_FilteringSpecularPSPtr = ShaderManager::CompilePS("fs_lightprobe_specular_sampling.glsl", "main");

        m_CustomVSPtr = ShaderManager::CompileVS("vs_spherical_env_cubemap_generation.glsl", "main");

        m_CustomGSPtr = ShaderManager::CompileGS("gs_spherical_env_cubemap_generation.glsl", "main");

        m_CustomPSPtr = ShaderManager::CompilePS("fs_cubemap_env_cubemap_generation.glsl", "main");

        // -----------------------------------------------------------------------------

        const SInputElementDescriptor PositionInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0, 0 , 32, CInputLayout::PerVertex, 0 },
            { "NORMAL"  , 0, CInputLayout::Float3Format, 0, 12, 32, CInputLayout::PerVertex, 0 },
            { "TEXCOORD", 0, CInputLayout::Float2Format, 0, 24, 32, CInputLayout::PerVertex, 0 },
        };

        m_PositionInputLayoutPtr = ShaderManager::CreateInputLayout(PositionInputLayout, 3, m_FilteringVSPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnSetupKernels()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnSetupRenderTargets()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnSetupStates()
    {
        CCameraPtr      CameraPtr       = ViewManager::GetMainCamera();
        CRenderStatePtr NoDepthStatePtr = StateManager::GetRenderState(CRenderState::NoDepth);

        m_CubemapRenderContextPtr = ContextManager::CreateRenderContext();

        m_CubemapRenderContextPtr->SetCamera(CameraPtr);
        m_CubemapRenderContextPtr->SetRenderState(NoDepthStatePtr);

        CSamplerPtr LinearFilter = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);

        m_PSSamplerSetPtr = SamplerManager::CreateSamplerSet(LinearFilter, LinearFilter, LinearFilter);
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnSetupTextures()
    {
       
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SViewBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr ViewBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSpecularCubemapSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SpecularPSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        Base::Float3 EyePosition = Base::Float3(0.0f);
        Base::Float3 UpDirection;
        Base::Float3 LookDirection;
        
        float lookAt =  1.5f;
        
        SConstantBufferGS DefaultGSValues;
        
        DefaultGSValues.m_CubeProjectionMatrix.SetRHFieldOfView(Base::RadiansToDegree(Base::SConstants<float>::s_Pi * 0.5f), 1.0f, 1.0f, 20000.0f);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(-lookAt, 0.0f, 0.0f);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[0].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(lookAt, 0.0f, 0.0f);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[1].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, lookAt, 0.0f);
        UpDirection = Base::Float3(0.0f, 0.0f, -1.0f);
        
        DefaultGSValues.m_CubeViewMatrix[3].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, -lookAt, 0.0f);
        UpDirection = Base::Float3(0.0f, 0.0f, 1.0f);
        
        DefaultGSValues.m_CubeViewMatrix[2].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, 0.0f, -lookAt);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[4].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, 0.0f, lookAt);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[5].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SConstantBufferGS);
        ConstanteBufferDesc.m_pBytes        = &DefaultGSValues;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr GSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SConstantBufferPS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        m_CustomVSBufferSetPtr    = BufferManager::CreateBufferSet(ViewBuffer);

        m_CubemapGSBufferSetPtr   = BufferManager::CreateBufferSet(GSBuffer);

        m_CustomPSBufferSetPtr    = BufferManager::CreateBufferSet(PSBuffer);
        
        m_FilteringPSBufferSetPtr = BufferManager::CreateBufferSet(SpecularPSBuffer);       
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnSetupResources()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnSetupModels()
    {
        Dt::SModelFileDescriptor ModelFileDesc;

        ModelFileDesc.m_pFileName = "envsphere.obj";
        ModelFileDesc.m_GenFlag = Dt::SGeneratorFlag::Nothing;

        Dt::CModel& rSphereModel = Dt::ModelManager::CreateModel(ModelFileDesc);

        SModelDescriptor ModelDesc;

        ModelDesc.m_pModel = &rSphereModel;

        m_EnvironmentSpherePtr = ModelManager::CreateModel(ModelDesc);
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnSetupEnd()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnReload()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnNewMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::OnUnloadMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::Update()
    {
        BuildRenderJobs();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::Render()
    {
        if (m_GlobalProbeRenderJobs.size() == 0) return;

        Performance::BeginEvent("Global Probe");

        RenderEnvironment();

        RenderFiltering();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::RenderEnvironment()
    {
        // TODO:
        // activate multiple global probes
        SGlobalProbeRenderJob& rRenderJob = m_GlobalProbeRenderJobs[0];

        // -----------------------------------------------------------------------------
        // Get environment of global light probe
        // -----------------------------------------------------------------------------
        if (rRenderJob.m_pDataGlobalProbe->GetType() == Dt::CGlobalProbeLightFacet::Sky)
        {
            rRenderJob.m_CubemapTextureSetPtr = SkyRenderer::GetCubemap();
        }
        else if (rRenderJob.m_pDataGlobalProbe->GetType() == Dt::CGlobalProbeLightFacet::Custom)
        {
            Performance::BeginEvent("Prepare custom cubemap");

            // -----------------------------------------------------------------------------
            // Setup constant buffer
            // -----------------------------------------------------------------------------
            SConstantBufferPS* pPSBuffer = static_cast<SConstantBufferPS*>(BufferManager::MapConstantBuffer(m_CustomPSBufferSetPtr->GetBuffer(0)));

            pPSBuffer->m_HDRFactor = rRenderJob.m_pDataGlobalProbe->GetIntensity();
            pPSBuffer->m_IsHDR     = rRenderJob.m_pDataGlobalProbe->GetCubemap()->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

            BufferManager::UnmapConstantBuffer(m_CustomPSBufferSetPtr->GetBuffer(0));

            // -----------------------------------------------------------------------------
            // Update render context
            // -----------------------------------------------------------------------------
            m_CubemapRenderContextPtr->SetViewPortSet(rRenderJob.m_pGraphicGlobalProbe->GetCustomViewPortSet());
            m_CubemapRenderContextPtr->SetTargetSet  (rRenderJob.m_pGraphicGlobalProbe->GetCustomHDRTargetSet());

            TargetSetManager::ClearTargetSet(rRenderJob.m_pGraphicGlobalProbe->GetCustomHDRTargetSet());

            // -----------------------------------------------------------------------------
            // Environment to cube map
            // -----------------------------------------------------------------------------           
            const unsigned int pOffset[] = { 0, 0 };

            // -----------------------------------------------------------------------------
            // Setup
            // -----------------------------------------------------------------------------
            ContextManager::SetRenderContext(m_CubemapRenderContextPtr);

            ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(m_CustomVSPtr);

            ContextManager::SetShaderGS(m_CustomGSPtr);

            ContextManager::SetShaderPS(m_CustomPSPtr);

            ContextManager::SetVertexBufferSet(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(m_PositionInputLayoutPtr);

            ContextManager::SetConstantBufferSetGS(m_CubemapGSBufferSetPtr);

            ContextManager::SetConstantBufferSetPS(m_CustomPSBufferSetPtr);

            ContextManager::SetTextureSetPS(rRenderJob.m_pGraphicGlobalProbe->GetCustomTextureSet());

            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            ContextManager::DrawIndexed(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

            // -----------------------------------------------------------------------------
            // Reset
            // -----------------------------------------------------------------------------
            ContextManager::ResetTextureSetPS();

            ContextManager::ResetConstantBufferSetPS();

            ContextManager::ResetConstantBufferSetGS();

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBufferSet();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderGS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetTopology();

            ContextManager::ResetSamplerSetPS();

            ContextManager::ResetRenderContext();

            // -----------------------------------------------------------------------------
            // Update mip maps
            // -----------------------------------------------------------------------------
            TextureManager::UpdateMipmap(rRenderJob.m_pGraphicGlobalProbe->GetCustomHDRTexture2D());

            Performance::EndEvent();

            rRenderJob.m_CubemapTextureSetPtr = rRenderJob.m_pGraphicGlobalProbe->GetCustomHDRTextureSet();
        }
    }

    void CGfxLightProbeRenderer::RenderFiltering()
    {
        SGlobalProbeRenderJob& rRenderJob = m_GlobalProbeRenderJobs[0];

        // -----------------------------------------------------------------------------
        // Get global probe facet
        // -----------------------------------------------------------------------------
        Gfx::CGlobalProbeLightFacet* pGraphicProbeFacet = rRenderJob.m_pGraphicGlobalProbe;

        // -----------------------------------------------------------------------------
        // Start updating/filtering
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Filter Distance Light Probe");

        unsigned int IndexOfMipmap = 0;

        // -----------------------------------------------------------------------------
        // Refine HDR specular from HDR cube map
        // -----------------------------------------------------------------------------
        CGlobalProbeLightFacet::CTargetSets&   rSpecularTargetSets   = pGraphicProbeFacet->GetSpecularHDRTargetSets();
        CGlobalProbeLightFacet::CViewPortSets& rSpecularViewPortSets = pGraphicProbeFacet->GetSpecularViewPortSets();

        CGlobalProbeLightFacet::CTargetSets::iterator CurrentOfSpecularMipmap = rSpecularTargetSets.begin();
        CGlobalProbeLightFacet::CTargetSets::iterator EndOfSpecularMipmaps    = rSpecularTargetSets.end();

        IndexOfMipmap = 0;

        float NumberOfMiplevels = static_cast<float>(pGraphicProbeFacet->GetSpecularHDRTargetSets().size());

        float MipmapRoughness      = 0.0f;
        float MipmapRoughnessDelta = 1.0f / NumberOfMiplevels;

        for (; CurrentOfSpecularMipmap != EndOfSpecularMipmaps; ++CurrentOfSpecularMipmap)
        {
            // -----------------------------------------------------------------------------
            // Prepare render target for environment cube map generation per mipmap
            // -----------------------------------------------------------------------------
            m_CubemapRenderContextPtr->SetViewPortSet(rSpecularViewPortSets[IndexOfMipmap]);
            m_CubemapRenderContextPtr->SetTargetSet  (rSpecularTargetSets[IndexOfMipmap]);

            TargetSetManager::ClearTargetSet(rSpecularTargetSets[IndexOfMipmap]);

            // -----------------------------------------------------------------------------
            // Upload per mipmap changing data
            // -----------------------------------------------------------------------------
            SSpecularCubemapSettings& rSpecularCubemapSettings = *static_cast<SSpecularCubemapSettings*>(BufferManager::MapConstantBuffer(m_FilteringPSBufferSetPtr->GetBuffer(0)));

            rSpecularCubemapSettings.m_LinearRoughness   = MipmapRoughness;
            rSpecularCubemapSettings.m_NumberOfMiplevels = NumberOfMiplevels - 1.0f;

            BufferManager::UnmapConstantBuffer(m_FilteringPSBufferSetPtr->GetBuffer(0));

            // -----------------------------------------------------------------------------

            const unsigned int pOffset[] = { 0, 0 };

            // -----------------------------------------------------------------------------
            // Setup
            // -----------------------------------------------------------------------------
            ContextManager::SetRenderContext(m_CubemapRenderContextPtr);

            ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(m_FilteringVSPtr);

            ContextManager::SetShaderGS(m_FilteringGSPtr);

            ContextManager::SetShaderPS(m_FilteringSpecularPSPtr);

            ContextManager::SetVertexBufferSet(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(m_PositionInputLayoutPtr);

            ContextManager::SetConstantBufferSetGS(m_CubemapGSBufferSetPtr);

            ContextManager::SetConstantBufferSetPS(m_FilteringPSBufferSetPtr);

            ContextManager::SetTextureSetPS(rRenderJob.m_CubemapTextureSetPtr);

            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            ContextManager::DrawIndexed(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

            // -----------------------------------------------------------------------------
            // Reset
            // -----------------------------------------------------------------------------
            ContextManager::ResetTextureSetPS();

            ContextManager::ResetConstantBufferSetPS();

            ContextManager::ResetConstantBufferSetGS();

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBufferSet();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderGS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetTopology();

            ContextManager::ResetSamplerSetPS();

            ContextManager::ResetRenderContext();

            MipmapRoughness += MipmapRoughnessDelta;

            ++IndexOfMipmap;
        }

        // -----------------------------------------------------------------------------
        // Refine HDR diffuse from HDR cubemap
        // -----------------------------------------------------------------------------
        {
            // -----------------------------------------------------------------------------
            // Prepare render target for environment cubemap generation per mipmap
            // -----------------------------------------------------------------------------
            m_CubemapRenderContextPtr->SetViewPortSet(pGraphicProbeFacet->GetDiffuseViewPortSet());
            m_CubemapRenderContextPtr->SetTargetSet  (pGraphicProbeFacet->GetDiffuseHDRTargetSet());

            TargetSetManager::ClearTargetSet(pGraphicProbeFacet->GetDiffuseHDRTargetSet());

            // -----------------------------------------------------------------------------

            const unsigned int pOffset[] = { 0, 0 };

            // -----------------------------------------------------------------------------
            // Setup
            // -----------------------------------------------------------------------------
            ContextManager::SetRenderContext(m_CubemapRenderContextPtr);

            ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(m_FilteringVSPtr);

            ContextManager::SetShaderGS(m_FilteringGSPtr);

            ContextManager::SetShaderPS(m_FilteringDiffusePSPtr);

            ContextManager::SetVertexBufferSet(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(m_PositionInputLayoutPtr);

            ContextManager::SetConstantBufferSetGS(m_CubemapGSBufferSetPtr);

            ContextManager::SetTextureSetPS(rRenderJob.m_CubemapTextureSetPtr);

            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            ContextManager::DrawIndexed(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

            // -----------------------------------------------------------------------------
            // Reset
            // -----------------------------------------------------------------------------
            ContextManager::ResetTextureSetPS();

            ContextManager::ResetConstantBufferSetGS();

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBufferSet();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderGS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetTopology();

            ContextManager::ResetSamplerSetPS();

            ContextManager::ResetRenderContext();
        }

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_GlobalProbeRenderJobs.clear();

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() == Dt::SLightType::GlobalProbe)
            {
                Dt::CGlobalProbeLightFacet*  pDataGlobalProbeFacet    = static_cast<Dt::CGlobalProbeLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                Gfx::CGlobalProbeLightFacet* pGraphicGlobalProbeFacet = static_cast<Gfx::CGlobalProbeLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                // -----------------------------------------------------------------------------
                // Check update needs
                // -----------------------------------------------------------------------------
                Base::U64 FrameTime = App::Application::GetNumberOfFrame();

                if (pGraphicGlobalProbeFacet->GetTimeStamp() == FrameTime)
                {
                    // -----------------------------------------------------------------------------
                    // Set probe into a new render job
                    // -----------------------------------------------------------------------------
                    SGlobalProbeRenderJob NewRenderJob;

                    NewRenderJob.m_pDataGlobalProbe    = pDataGlobalProbeFacet;
                    NewRenderJob.m_pGraphicGlobalProbe = pGraphicGlobalProbeFacet;

                    m_GlobalProbeRenderJobs.push_back(NewRenderJob);
                }
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }
    }
} // namespace

namespace Gfx
{
namespace LightProbeRenderer
{
    void OnStart()
    {
        CGfxLightProbeRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxLightProbeRenderer::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void OnSetupShader()
    {
        CGfxLightProbeRenderer::GetInstance().OnSetupShader();
    }

    // -----------------------------------------------------------------------------

    void OnSetupKernels()
    {
        CGfxLightProbeRenderer::GetInstance().OnSetupKernels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupRenderTargets()
    {
        CGfxLightProbeRenderer::GetInstance().OnSetupRenderTargets();
    }

    // -----------------------------------------------------------------------------

    void OnSetupStates()
    {
        CGfxLightProbeRenderer::GetInstance().OnSetupStates();
    }

    // -----------------------------------------------------------------------------

    void OnSetupTextures()
    {
        CGfxLightProbeRenderer::GetInstance().OnSetupTextures();
    }

    // -----------------------------------------------------------------------------

    void OnSetupBuffers()
    {
        CGfxLightProbeRenderer::GetInstance().OnSetupBuffers();
    }

    // -----------------------------------------------------------------------------

    void OnSetupResources()
    {
        CGfxLightProbeRenderer::GetInstance().OnSetupResources();
    }

    // -----------------------------------------------------------------------------

    void OnSetupModels()
    {
        CGfxLightProbeRenderer::GetInstance().OnSetupModels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupEnd()
    {
        CGfxLightProbeRenderer::GetInstance().OnSetupEnd();
    }

    // -----------------------------------------------------------------------------

    void OnReload()
    {
        CGfxLightProbeRenderer::GetInstance().OnReload();
    }

    // -----------------------------------------------------------------------------

    void OnNewMap()
    {
        CGfxLightProbeRenderer::GetInstance().OnNewMap();
    }

    // -----------------------------------------------------------------------------

    void OnUnloadMap()
    {
        CGfxLightProbeRenderer::GetInstance().OnUnloadMap();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxLightProbeRenderer::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void Render()
    {
        CGfxLightProbeRenderer::GetInstance().Render();
    }
} // namespace LightProbeRenderer
} // namespace Gfx
