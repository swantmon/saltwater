
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_light_probe_facet.h"
#include "data/data_light_type.h"
#include "data/data_entity.h"
#include "data/data_fx_type.h"
#include "data/data_map.h"
#include "data/data_ssr_facet.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_reflection_renderer.h"
#include "graphic/gfx_light_probe_facet.h"
#include "graphic/gfx_light_probe_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sun_facet.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include <vector>

using namespace Gfx;

namespace
{
    class CGfxReflectionRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxReflectionRenderer)
        
    public:
        CGfxReflectionRenderer();
        ~CGfxReflectionRenderer();
        
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

        struct SLightProbeRenderJob
        {
            Dt::CLightProbeFacet*  m_pDataLightProbe;
            Gfx::CLightProbeFacet* m_pGraphicLightProbe;
        };

        struct SSSRRenderJob
        {
            Dt::CSSRFXFacet* m_pDataSSRFacet;
        };
        
        struct SPerDrawCallConstantBuffer
        {
            Base::Float4x4 m_ModelMatrix;
        };

        struct SSSRProperties
        {
            float m_SSRIntesity;
            float m_SSRRougnessMaskScale;
            float m_SSRDistance;
            float m_PreviousFrame;
        };

        struct SHCBProperties
        {
            Base::Float4 m_UVBoundaries;
            Base::Float2 m_InverseTextureSize;
            float        m_MipmapLevel;
        };
        
        struct SIBLSettings
        {
            float m_NumberOfMiplevelsSpecularIBL;
            float m_ExposureHistoryIndex;
        };

    private:

        typedef std::vector<SLightProbeRenderJob> CLightProbeRenderJobs;
        typedef std::vector<SSSRRenderJob>        CSSRRenderJobs;
        
    private:
        
        CMeshPtr         m_QuadModelPtr;
        
        CBufferSetPtr     m_QuadVSBufferSetPtr;
        
        CBufferSetPtr     m_ImageLightPSBufferSetPtr;

        CBufferSetPtr     m_SSRLightPSBufferSetPtr;

        CBufferSetPtr     m_HCBPSBufferSetPtr;
        
        CInputLayoutPtr   m_QuadInputLayoutPtr;
        
        CShaderPtr        m_RectangleShaderVSPtr;
        
        CShaderPtr        m_ImageLightShaderPSPtr;

        CShaderPtr        m_SSRShaderPSPtr;

        CShaderPtr        m_SSRApplyShaderPSPtr;

        CShaderPtr        m_HCBShaderPSPtr;

        CShaderPtr        m_BRDFShaderPtr;
        
        CTexture2DPtr     m_BRDFTexture2DPtr;

        CTexture2DPtr     m_HCBTexture2DPtr;
        
        CTextureSetPtr    m_ImageLightTextureSetPtr;

        CTextureSetPtr    m_SSRTextureSetPtr;

        CTextureSetPtr    m_SSRApplyTextureSetPtr;        

        CTextureSetPtr    m_BRDFTextureSetPtr;
        
        CSamplerSetPtr    m_PSSamplerSetPtr;

        CTargetSetPtr     m_SSRTargetSetPtr;        
        
        CRenderContextPtr m_LightAccumulationRenderContextPtr;

        CRenderContextPtr m_SSRRenderContextPtr;

        CRenderContextPtr m_HCBRenderContextPtr;


        std::vector<CTextureSetPtr>    m_HCBTextureSetPtrs;

        std::vector<CTargetSetPtr>     m_HCBTargetSetPtrs;

        std::vector<CViewPortSetPtr>   m_HCBViewPortSetPtrs;

        CLightProbeRenderJobs m_LightProbeRenderJobs;
        CSSRRenderJobs         m_SSRRenderJobs;
        
    private:
        
        void RenderIBL();
        void RenderHCB();
        void RenderScreenSpaceReflections();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxReflectionRenderer::CGfxReflectionRenderer()
        : m_QuadModelPtr                     ()
        , m_QuadVSBufferSetPtr               ()
        , m_ImageLightPSBufferSetPtr         ()
        , m_SSRLightPSBufferSetPtr           ()
        , m_HCBPSBufferSetPtr                ()
        , m_QuadInputLayoutPtr               ()
        , m_RectangleShaderVSPtr             ()
        , m_ImageLightShaderPSPtr            ()
        , m_SSRShaderPSPtr                   ()
        , m_SSRApplyShaderPSPtr              ()
        , m_HCBShaderPSPtr                   ()
        , m_BRDFShaderPtr                    ()
        , m_BRDFTexture2DPtr                 ()
        , m_HCBTexture2DPtr                  ()
        , m_ImageLightTextureSetPtr          ()
        , m_SSRTextureSetPtr                 ()
        , m_SSRApplyTextureSetPtr            ()
        , m_BRDFTextureSetPtr                ()
        , m_PSSamplerSetPtr                  ()
        , m_SSRTargetSetPtr                  ()
        , m_LightAccumulationRenderContextPtr()
        , m_SSRRenderContextPtr              ()
        , m_HCBRenderContextPtr              ()
        , m_LightProbeRenderJobs             ()
        , m_SSRRenderJobs                    ()
    {
        m_LightProbeRenderJobs.reserve(1);
        m_SSRRenderJobs       .reserve(1);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxReflectionRenderer::~CGfxReflectionRenderer()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnExit()
    {
        m_QuadModelPtr                      = 0;
        m_QuadVSBufferSetPtr                = 0;
        m_ImageLightPSBufferSetPtr          = 0;
        m_SSRLightPSBufferSetPtr            = 0;
        m_HCBPSBufferSetPtr                 = 0;
        m_QuadInputLayoutPtr                = 0;
        m_RectangleShaderVSPtr              = 0;
        m_ImageLightShaderPSPtr             = 0;
        m_SSRShaderPSPtr                    = 0;
        m_SSRApplyShaderPSPtr               = 0;
        m_HCBShaderPSPtr                    = 0;
        m_BRDFShaderPtr                     = 0;
        m_BRDFTexture2DPtr                  = 0;
        m_HCBTexture2DPtr                   = 0;
        m_ImageLightTextureSetPtr           = 0;
        m_SSRTextureSetPtr                  = 0;
        m_SSRApplyTextureSetPtr             = 0;
        m_BRDFTextureSetPtr                 = 0;
        m_PSSamplerSetPtr                   = 0;
        m_SSRTargetSetPtr                   = 0;
        m_LightAccumulationRenderContextPtr = 0;
        m_SSRRenderContextPtr               = 0;
        m_HCBRenderContextPtr               = 0;

        for (unsigned int IndexOfElement = 0; IndexOfElement < m_HCBTextureSetPtrs.size(); ++IndexOfElement)
        {
            m_HCBTextureSetPtrs [IndexOfElement] = 0;
            m_HCBTargetSetPtrs  [IndexOfElement] = 0;
            m_HCBViewPortSetPtrs[IndexOfElement] = 0;
        }

        m_HCBTextureSetPtrs .clear();
        m_HCBTargetSetPtrs  .clear();
        m_HCBViewPortSetPtrs.clear();

        m_LightProbeRenderJobs.clear();
        m_SSRRenderJobs        .clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnSetupShader()
    {
        m_RectangleShaderVSPtr  = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");
        
        m_ImageLightShaderPSPtr = ShaderManager::CompilePS("fs_light_imagelight.glsl" , "main");

        m_SSRShaderPSPtr        = ShaderManager::CompilePS("fs_ssr.glsl", "main");

        m_SSRApplyShaderPSPtr   = ShaderManager::CompilePS("fs_texture.glsl", "main");

        m_BRDFShaderPtr         = ShaderManager::CompileCS("cs_brdf.glsl", "main");

        m_HCBShaderPSPtr        = ShaderManager::CompilePS("fs_hcb_generation.glsl", "main");
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor QuadInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        m_QuadInputLayoutPtr = ShaderManager::CreateInputLayout(QuadInputLayout, 1, m_RectangleShaderVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnSetupKernels()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnSetupRenderTargets()
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        Base::Int2 Size = Main::GetActiveWindowSize();

        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::RenderTarget | CTextureBase::ShaderResource;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::Unknown;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        RendertargetDescriptor.m_Format           = CTextureBase::R16G16B16A16_FLOAT;
        
        CTexture2DPtr SSRTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // SSR Temp Color

        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::RenderTarget | CTextureBase::ShaderResource;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::Unknown;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        RendertargetDescriptor.m_Format           = CTextureBase::R16G16B16A16_FLOAT;
        
        m_HCBTexture2DPtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // HCB

        TextureManager::UpdateMipmap(m_HCBTexture2DPtr);

        // -----------------------------------------------------------------------------
        // Create target set
        // -----------------------------------------------------------------------------
        m_SSRTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(SSRTexturePtr));

        // -----------------------------------------------------------------------------

        SViewPortDescriptor ViewPortDesc;

        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;

        for (unsigned int IndexOfMipmap = 0; IndexOfMipmap < m_HCBTexture2DPtr->GetNumberOfMipLevels(); ++IndexOfMipmap)
        {
            // -----------------------------------------------------------------------------
            // Target set
            // -----------------------------------------------------------------------------
            CTexture2DPtr MipmapTexture = TextureManager::GetMipmapFromTexture2D(m_HCBTexture2DPtr, IndexOfMipmap);

            CTargetSetPtr MipmapTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(MipmapTexture));

            // -----------------------------------------------------------------------------
            // View port
            // -----------------------------------------------------------------------------
            ViewPortDesc.m_Width = static_cast<float>(MipmapTexture->GetNumberOfPixelsU());
            ViewPortDesc.m_Height = static_cast<float>(MipmapTexture->GetNumberOfPixelsV());

            CViewPortPtr MipMapViewPort = ViewManager::CreateViewPort(ViewPortDesc);

            CViewPortSetPtr ViewPortSetPtr = ViewManager::CreateViewPortSet(MipMapViewPort);

            // -----------------------------------------------------------------------------
            // Put into class
            // -----------------------------------------------------------------------------
            m_HCBTargetSetPtrs  .push_back(MipmapTargetSetPtr);
            m_HCBViewPortSetPtrs.push_back(ViewPortSetPtr);
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnSetupStates()
    {
        CCameraPtr          MainCameraPtr      = ViewManager     ::GetFullQuadCamera();
        
        CViewPortSetPtr     ViewPortSetPtr     = ViewManager     ::GetViewPortSet();
        
        CTargetSetPtr       LightTargetSetPtr  = TargetSetManager::GetLightAccumulationTargetSet();
        
        // TODO by tschwandt
        // Addional Blend work as well; Why destination blend?
        CRenderStatePtr     LightStatePtr      = StateManager::GetRenderState(CRenderState::DestinationBlend);

        CRenderStatePtr     DefaultStatePtr    = StateManager::GetRenderState(0);

        // -----------------------------------------------------------------------------
        
        m_LightAccumulationRenderContextPtr = ContextManager::CreateRenderContext();
        
        m_LightAccumulationRenderContextPtr->SetCamera(MainCameraPtr);
        m_LightAccumulationRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_LightAccumulationRenderContextPtr->SetTargetSet(LightTargetSetPtr);
        m_LightAccumulationRenderContextPtr->SetRenderState(LightStatePtr);

        // -----------------------------------------------------------------------------

        m_SSRRenderContextPtr = ContextManager::CreateRenderContext();

        m_SSRRenderContextPtr->SetCamera(MainCameraPtr);
        m_SSRRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_SSRRenderContextPtr->SetTargetSet(m_SSRTargetSetPtr);
        m_SSRRenderContextPtr->SetRenderState(DefaultStatePtr);

        // -----------------------------------------------------------------------------

        m_HCBRenderContextPtr = ContextManager::CreateRenderContext();

        m_HCBRenderContextPtr->SetCamera(MainCameraPtr);
        m_HCBRenderContextPtr->SetRenderState(DefaultStatePtr);
        
        // -----------------------------------------------------------------------------
        
        CSamplerPtr Sampler[8];
        
        Sampler[0] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[1] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[2] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[3] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[4] = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);
        Sampler[5] = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);
        Sampler[6] = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);
        Sampler[7] = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);
        
        m_PSSamplerSetPtr = SamplerManager::CreateSamplerSet(Sampler, 8);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnSetupTextures()
    {
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = 512;
        RendertargetDescriptor.m_NumberOfPixelsV  = 512;
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::ShaderResource;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::R32G32B32A32_FLOAT;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPUReadWrite;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        
        m_BRDFTexture2DPtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        m_BRDFTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_BRDFTexture2DPtr));
        
        // -----------------------------------------------------------------------------
        
        CTextureBasePtr GBuffer0TexturePtr          = TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0);
        CTextureBasePtr GBuffer1TexturePtr          = TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1);
        CTextureBasePtr GBuffer2TexturePtr          = TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2);
        CTextureBasePtr DepthTexturePtr             = TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget();
        CTextureBasePtr LightAccumuationPtr         = TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0);
        
        // -----------------------------------------------------------------------------

        CTextureBasePtr ImageLightTexturePtrs[] = {GBuffer0TexturePtr, GBuffer1TexturePtr, GBuffer2TexturePtr, DepthTexturePtr, static_cast<CTextureBasePtr>(m_BRDFTexture2DPtr) };
        
        m_ImageLightTextureSetPtr = TextureManager::CreateTextureSet(ImageLightTexturePtrs, 5);

        // -----------------------------------------------------------------------------
        
        CTextureBasePtr SSRTexturePtrs[] = { GBuffer0TexturePtr, GBuffer1TexturePtr, GBuffer2TexturePtr, DepthTexturePtr, static_cast<CTextureBasePtr>(m_HCBTexture2DPtr), static_cast<CTextureBasePtr>(m_BRDFTexture2DPtr) };

        m_SSRTextureSetPtr = TextureManager::CreateTextureSet(SSRTexturePtrs, 6);

        // -----------------------------------------------------------------------------

        m_SSRApplyTextureSetPtr = TextureManager::CreateTextureSet(m_SSRTargetSetPtr->GetRenderTarget(0));

        // -----------------------------------------------------------------------------

        m_HCBTextureSetPtrs.push_back(TextureManager::CreateTextureSet(LightAccumuationPtr));

        for (unsigned int IndexOfMipmap = 1; IndexOfMipmap < m_HCBTexture2DPtr->GetNumberOfMipLevels(); ++IndexOfMipmap)
        {
            m_HCBTextureSetPtrs.push_back(TextureManager::CreateTextureSet(m_HCBTargetSetPtrs[IndexOfMipmap - 1]->GetRenderTarget(0)));
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnSetupBuffers()
    {
        // -----------------------------------------------------------------------------
        // Setup view buffer for post rendering
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSSRProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SSRBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SHCBProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr HCBBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        CBufferPtr HistogramExposureHistoryBufferPtr = HistogramRenderer::GetExposureHistoryBuffer();
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SIBLSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr IBLSettingsPSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        m_QuadVSBufferSetPtr       = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS());
        
        m_ImageLightPSBufferSetPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), IBLSettingsPSBuffer, HistogramExposureHistoryBufferPtr);

        m_SSRLightPSBufferSetPtr   = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), SSRBuffer);

        m_HCBPSBufferSetPtr        = BufferManager::CreateBufferSet(HCBBuffer);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnSetupResources()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnSetupModels()
    {
        m_QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnSetupEnd()
    {
        ContextManager::SetShaderCS(m_BRDFShaderPtr);

        ContextManager::SetTextureSetCS(m_BRDFTextureSetPtr);

        ContextManager::Dispatch(512, 512, 1);

        ContextManager::ResetTextureSetCS();

        ContextManager::ResetShaderCS();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnUnloadMap()
    {
        
    } 
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::Render()
    {
        Performance::BeginEvent("Reflections");

        RenderScreenSpaceReflections();
        
        RenderIBL();

        RenderHCB();

        Performance::EndEvent();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::RenderIBL()
    {
        if (m_LightProbeRenderJobs.size() == 0) return;

        // TODO:
        // activate multiple global probes
        SLightProbeRenderJob& rRenderJob = m_LightProbeRenderJobs[0];

        Gfx::CLightProbeFacet* pGraphicProbeFacet = rRenderJob.m_pGraphicLightProbe;

        Performance::BeginEvent("IBL");

        const unsigned int pOffset[] = {0, 0};
                
        // -----------------------------------------------------------------------------
        // Distance light probes
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_LightAccumulationRenderContextPtr);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);
        
        ContextManager::SetShaderPS(m_ImageLightShaderPSPtr);
        
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);
        
        ContextManager::SetConstantBufferSetPS(m_ImageLightPSBufferSetPtr);
        
        ContextManager::SetConstantBufferSetVS(m_QuadVSBufferSetPtr);
        
        ContextManager::SetTextureSetPS(m_ImageLightTextureSetPtr);
                    
        // -----------------------------------------------------------------------------
        // IBL data
        // -----------------------------------------------------------------------------
        SIBLSettings* pIBLSettings = static_cast<SIBLSettings*>(BufferManager::MapConstantBuffer(m_ImageLightPSBufferSetPtr->GetBuffer(1), CBuffer::Write));
            
        pIBLSettings->m_NumberOfMiplevelsSpecularIBL = static_cast<float>(pGraphicProbeFacet->GetFilteredSetPtr()->GetTexture(0)->GetNumberOfMipLevels() - 1);
        pIBLSettings->m_ExposureHistoryIndex         = static_cast<float>(HistogramRenderer::GetLastExposureHistoryIndex());
            
        BufferManager::UnmapConstantBuffer(m_ImageLightPSBufferSetPtr->GetBuffer(1));
            
        // -----------------------------------------------------------------------------
        // IBL textures
        // -----------------------------------------------------------------------------            
        ContextManager::SetTextureSetPS(pGraphicProbeFacet->GetFilteredSetPtr());
            
        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTextureSetPS();
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetConstantBufferSetPS();
        
        ContextManager::ResetConstantBufferSetVS();
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBufferSet();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetTopology();
        
        ContextManager::ResetSamplerSetPS();
        
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReflectionRenderer::RenderHCB()
    {
        Performance::BeginEvent("HCB");

        const unsigned int pOffset[] = { 0, 0 };

        for (unsigned int IndexOfMipmap = 0; IndexOfMipmap < m_HCBTexture2DPtr->GetNumberOfMipLevels(); ++ IndexOfMipmap)
        {
            SHCBProperties* pPSBuffer = static_cast<SHCBProperties*>(BufferManager::MapConstantBuffer(m_HCBPSBufferSetPtr->GetBuffer(0), CBuffer::Write));

            assert(pPSBuffer != nullptr);

            pPSBuffer->m_UVBoundaries       = Base::Float4(0.0f, 0.0f, 1.0f, 1.0f);
            pPSBuffer->m_InverseTextureSize = Base::Float2(1.0f, 1.0f);
            pPSBuffer->m_MipmapLevel        = static_cast<float>(IndexOfMipmap);

            if (IndexOfMipmap > 0)
            {
                pPSBuffer->m_InverseTextureSize = Base::Float2(1.0f / m_HCBViewPortSetPtrs[IndexOfMipmap - 1]->GetViewPorts()[0]->GetWidth(), 1.0f / m_HCBViewPortSetPtrs[IndexOfMipmap - 1]->GetViewPorts()[0]->GetHeight());
            }

            BufferManager::UnmapConstantBuffer(m_HCBPSBufferSetPtr->GetBuffer(0));

            // -----------------------------------------------------------------------------

            m_HCBRenderContextPtr->SetTargetSet  (m_HCBTargetSetPtrs[IndexOfMipmap]);
            m_HCBRenderContextPtr->SetViewPortSet(m_HCBViewPortSetPtrs[IndexOfMipmap]);

            ContextManager::SetRenderContext(m_HCBRenderContextPtr);

            ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

            ContextManager::SetShaderPS(m_HCBShaderPSPtr);

            ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(m_QuadInputLayoutPtr);

            ContextManager::SetConstantBufferSetVS(m_QuadVSBufferSetPtr);

            ContextManager::SetConstantBufferSetPS(m_HCBPSBufferSetPtr);

            ContextManager::SetTextureSetPS(m_HCBTextureSetPtrs[0]);

            ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

            ContextManager::ResetTextureSetPS();

            ContextManager::ResetInputLayout();

            ContextManager::ResetConstantBufferSetPS();

            ContextManager::ResetConstantBufferSetVS();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBufferSet();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetTopology();

            ContextManager::ResetSamplerSetPS();

            ContextManager::ResetRenderContext();
        }        

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReflectionRenderer::RenderScreenSpaceReflections()
    {
        if (m_SSRRenderJobs.size() == 0) return;

        Performance::BeginEvent("SSR");

        // TODO: What happens if more then one DOF effect is available?
        Dt::CSSRFXFacet* pDataSSRFacet = m_SSRRenderJobs[0].m_pDataSSRFacet;

        assert(pDataSSRFacet != 0);

        // -----------------------------------------------------------------------------
        // Upload dynamic data
        // -----------------------------------------------------------------------------
        CCameraPtr CameraPtr = ViewManager::GetMainCamera();

        SSSRProperties* pPSBuffer = static_cast<SSSRProperties*>(BufferManager::MapConstantBuffer(m_SSRLightPSBufferSetPtr->GetBuffer(1), CBuffer::Write));

        assert(pPSBuffer != nullptr);

        Base::Float3 Position = CameraPtr->GetView()->GetPosition();

        pPSBuffer->m_SSRIntesity          = pDataSSRFacet->GetIntensity();
        pPSBuffer->m_SSRRougnessMaskScale = pDataSSRFacet->GetRoughnessMask();
        pPSBuffer->m_SSRDistance          = pDataSSRFacet->GetDistance();
        pPSBuffer->m_PreviousFrame        = pDataSSRFacet->GetUseLastFrame() ? 1.0f : 0.0f;

        BufferManager::UnmapConstantBuffer(m_SSRLightPSBufferSetPtr->GetBuffer(1));

        // -----------------------------------------------------------------------------
        // Screen Space Reflections
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = { 0, 0 };

        ContextManager::SetRenderContext(m_SSRRenderContextPtr);

        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

        ContextManager::SetShaderPS(m_SSRShaderPSPtr);

        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);

        ContextManager::SetConstantBufferSetVS(m_QuadVSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(m_SSRLightPSBufferSetPtr);

        ContextManager::SetTextureSetPS(m_SSRTextureSetPtr);

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTextureSetPS();

        ContextManager::ResetConstantBufferSetPS();

        ContextManager::ResetConstantBufferSetVS();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetSamplerSetPS();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Apply
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_LightAccumulationRenderContextPtr);

        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

        ContextManager::SetShaderPS(m_SSRApplyShaderPSPtr);

        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);

        ContextManager::SetConstantBufferSetVS(m_QuadVSBufferSetPtr);

        ContextManager::SetTextureSetPS(m_SSRApplyTextureSetPtr);

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTextureSetPS();

        ContextManager::ResetInputLayout();

        ContextManager::ResetConstantBufferSetPS();

        ContextManager::ResetConstantBufferSetVS();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetSamplerSetPS();

        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReflectionRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_LightProbeRenderJobs.clear();
        m_SSRRenderJobs        .clear();

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentLightEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfLightEntities = Dt::Map::EntitiesEnd();

        for (; CurrentLightEntity != EndOfLightEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentLightEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() == Dt::SLightType::LightProbe)
            {
                Dt::CLightProbeFacet*  pDataLightProbeFacet = static_cast<Dt::CLightProbeFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                Gfx::CLightProbeFacet* pGraphicLightProbeFacet = static_cast<Gfx::CLightProbeFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                assert(pDataLightProbeFacet != 0 && pGraphicLightProbeFacet != 0);

                // -----------------------------------------------------------------------------
                // Set sun into a new render job
                // -----------------------------------------------------------------------------
                SLightProbeRenderJob NewRenderJob;

                NewRenderJob.m_pDataLightProbe = pDataLightProbeFacet;
                NewRenderJob.m_pGraphicLightProbe = pGraphicLightProbeFacet;

                m_LightProbeRenderJobs.push_back(NewRenderJob);
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentLightEntity = CurrentLightEntity.Next(Dt::SEntityCategory::Light);
        }

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEffectEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::FX);
        Dt::Map::CEntityIterator EndOfEffectEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEffectEntity != EndOfEffectEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEffectEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() == Dt::SFXType::SSR)
            {
                Dt::CSSRFXFacet* pDataSSRFacet = static_cast<Dt::CSSRFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

                assert(pDataSSRFacet != 0);

                // -----------------------------------------------------------------------------
                // Set sun into a new render job
                // -----------------------------------------------------------------------------
                SSSRRenderJob NewRenderJob;

                NewRenderJob.m_pDataSSRFacet = pDataSSRFacet;

                m_SSRRenderJobs.push_back(NewRenderJob);
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEffectEntity = CurrentEffectEntity.Next(Dt::SEntityCategory::FX);
        }
    }
} // namespace


namespace Gfx
{
namespace ReflectionRenderer
{
    void OnStart()
    {
        CGfxReflectionRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxReflectionRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxReflectionRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxReflectionRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxReflectionRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxReflectionRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxReflectionRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxReflectionRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxReflectionRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxReflectionRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxReflectionRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxReflectionRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxReflectionRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxReflectionRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxReflectionRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxReflectionRenderer::GetInstance().Render();
    }
} // namespace ReflectionRenderer
} // namespace Gfx
