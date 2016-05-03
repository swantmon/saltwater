
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_fx_facet.h"
#include "data/data_light_facet.h"
#include "data/data_map.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_actor_facet.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_light_facet.h"
#include "graphic/gfx_light_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_model.h"
#include "graphic/gfx_model_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_shadow_renderer.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxShadowRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxShadowRenderer)
        
    public:
        CGfxShadowRenderer();
        ~CGfxShadowRenderer();
        
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

        static const unsigned int s_SSAOKernelSize = 16;
        static const unsigned int s_BlurTileSize   = 8;

    private:

        enum ESSAOShader
        {
            SSAO,
            SSAOApply,
            NumberOfSSAOs,
            UndefinedSSAO = -1
        };
        
    private:

        struct SSunLightRenderJob
        {
            Gfx::CSunLightFacet* m_pGraphicSunLightFacet;
        };

        struct SPointLightRenderJob
        {
            Gfx::CPointLightFacet* m_pGraphicPointLightFacet;
        };

        struct SSSAORenderJob
        {
            Dt::CSSAOFXFacet* m_pDataSSAOFacet;
        };
        
        struct SPerLightConstantBuffer
        {
            Base::Float4x4 vs_ViewProjectionMatrix;
        };
        
        struct SPerDrawCallConstantBuffer
        {
            Base::Float4x4 m_ModelMatrix;
        };

        struct SGaussianSettings
        {
            Base::Int2 m_Direction;
            Base::Int2 m_MaxPixelCoord;
            float      m_Weights[7];
        };

        struct SSSAOProperties
        {
            Base::Float4x4 m_InverseCameraProjection;
            Base::Float4x4 m_CameraProjection;
            Base::Float4x4 m_CameraView;
            Base::Float4   m_NoiseScale;
            Base::Float4   m_Kernel[s_SSAOKernelSize];
        };

    private:

        typedef std::vector<SSunLightRenderJob>   CSunLightRenderJobs;
        typedef std::vector<SPointLightRenderJob> CPointLightRenderJobs;
        typedef std::vector<SSSAORenderJob>       CSSAORenderJobs;
        
    private:
        
        CModelPtr         m_QuadModelPtr;
        
        CBufferSetPtr     m_LightCameraVSBufferPtr;
        CBufferSetPtr     m_MainVSBufferPtr;
        CBufferSetPtr     m_QuadVSBufferPtr;
        CBufferSetPtr     m_GaussianBlurPropertiesCSBufferPtr;
        CBufferSetPtr     m_SSAOPropertiesPSBufferPtr;
        
        CInputLayoutPtr   m_QuadInputLayoutPtr;
        
        CShaderPtr        m_FullquadShaderVSPtr;
        CShaderPtr        m_ShadowShaderVSPtr;
        
        CShaderPtr        m_SSAOShaderPSPtrs[NumberOfSSAOs];
        
        CShaderPtr        m_ShadowSMShaderPSPtr;

        CShaderPtr        m_BilateralBlurShaderCSPtr;
        
        CTextureSetPtr    m_SSAOTextureSets[NumberOfSSAOs];
        
        CSamplerSetPtr    m_PSSamplerSetPtr;
        
        CTargetSetPtr     m_HalfRenderbufferPtr;

        CRenderContextPtr m_DeferredRenderContextPtr;
        CRenderContextPtr m_HalfContextPtr;
        
        CTextureSetPtr    m_HalfTexturePtrs[2];

        CTextureSetPtr    m_BilateralBlurHTextureSetPtr;
        CTextureSetPtr    m_BilateralBlurVTextureSetPtr;

        Base::Float4 m_Kernel[s_SSAOKernelSize];

        CSunLightRenderJobs   m_SunLightRenderJobs;
        CPointLightRenderJobs m_PointLightRenderJobs;
        CSSAORenderJobs       m_SSAORenderJobs;

    private:
        
        void RenderShadowFromLightsources();
        void RenderCascadedShadowFromSun();
        void RenderSSAO();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxShadowRenderer::CGfxShadowRenderer()
        : m_QuadModelPtr                     ()
        , m_LightCameraVSBufferPtr           ()
        , m_MainVSBufferPtr                  ()
        , m_QuadVSBufferPtr                  ()
        , m_SSAOPropertiesPSBufferPtr        ()
        , m_GaussianBlurPropertiesCSBufferPtr()
        , m_QuadInputLayoutPtr               ()
        , m_FullquadShaderVSPtr              ()
        , m_ShadowShaderVSPtr                ()
        , m_ShadowSMShaderPSPtr              ()
        , m_BilateralBlurShaderCSPtr         ()
        , m_BilateralBlurHTextureSetPtr      ()
        , m_BilateralBlurVTextureSetPtr      ()
        , m_SSAOTextureSets                  ()
        , m_PSSamplerSetPtr                  ()
        , m_DeferredRenderContextPtr         ()
        , m_SunLightRenderJobs               ()
        , m_PointLightRenderJobs             ()
        , m_SSAORenderJobs                   ()
    {
        m_SunLightRenderJobs  .reserve(1);
        m_PointLightRenderJobs.reserve(16);
        m_SSAORenderJobs      .reserve(1);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxShadowRenderer::~CGfxShadowRenderer()
    {
    
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Create SSAO randomized kernel
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfNoiseSeq = 0; IndexOfNoiseSeq < s_SSAOKernelSize; ++IndexOfNoiseSeq)
        {
            Base::Float3 KernelElement;

            float fRandomX = (Base::Random() * 2.0f - 1.0f);
            float fRandomY = (Base::Random() * 2.0f - 1.0f);
            float fRandomZ = Base::Random();

            KernelElement = Base::Float3(fRandomX, fRandomY, fRandomZ);

            float Scale = static_cast<float>(IndexOfNoiseSeq) / static_cast<float>(s_SSAOKernelSize);

            KernelElement += Base::Lerp(0.1f, 1.0f, Scale * Scale);

            KernelElement = KernelElement.Normalize();

            m_Kernel[IndexOfNoiseSeq] = Base::Float4(KernelElement, 0.0f);
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnExit()
    {
        m_QuadModelPtr                      = 0;
        m_LightCameraVSBufferPtr            = 0;
        m_MainVSBufferPtr                   = 0;
        m_QuadVSBufferPtr                   = 0;
        m_GaussianBlurPropertiesCSBufferPtr = 0;
        m_SSAOPropertiesPSBufferPtr         = 0;
        m_QuadInputLayoutPtr                = 0;
        m_FullquadShaderVSPtr               = 0;
        m_ShadowShaderVSPtr                 = 0;
        m_ShadowSMShaderPSPtr               = 0;
        m_BilateralBlurShaderCSPtr          = 0;
        m_BilateralBlurHTextureSetPtr       = 0;
        m_BilateralBlurVTextureSetPtr       = 0;
        m_PSSamplerSetPtr                   = 0;
        m_HalfRenderbufferPtr               = 0;
        m_DeferredRenderContextPtr          = 0;
        m_HalfContextPtr                    = 0;
               
        m_SSAOTextureSets[SSAO]      = 0;
        m_SSAOTextureSets[SSAOApply] = 0;

        m_SSAOShaderPSPtrs[SSAO]      = 0;
        m_SSAOShaderPSPtrs[SSAOApply] = 0;
        
        m_HalfTexturePtrs[0] = 0;
        m_HalfTexturePtrs[1] = 0;

        m_SunLightRenderJobs  .clear();
        m_PointLightRenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupShader()
    {
        m_FullquadShaderVSPtr = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");
        m_ShadowShaderVSPtr   = ShaderManager::CompileVS("vs_vm_pnx0.glsl", "main");

        m_ShadowSMShaderPSPtr         = ShaderManager::CompilePS("fs_shadow.glsl"    , "main");
        m_SSAOShaderPSPtrs[SSAO]      = ShaderManager::CompilePS("fs_ssao.glsl"      , "main");
        m_SSAOShaderPSPtrs[SSAOApply] = ShaderManager::CompilePS("fs_ssao_apply.glsl", "main");

        m_BilateralBlurShaderCSPtr    = ShaderManager::CompileCS("cs_gaussian_blur_rgba8.glsl", "main");
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor QuadInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        m_QuadInputLayoutPtr       = ShaderManager::CreateInputLayout(QuadInputLayout, 1, m_FullquadShaderVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupKernels()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupRenderTargets()
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        Base::Int2 Size = Main::GetActiveWindowSize();

        Base::Int2 HalfSize   (Size[0] / 2, Size[1] / 2);

        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;

        RendertargetDescriptor.m_NumberOfPixelsU  = HalfSize[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = HalfSize[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::R8G8B8A8_UBYTE;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;

        CTexture2DPtr HalfTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_NumberOfPixelsU  = 2048;
        RendertargetDescriptor.m_NumberOfPixelsV  = 2048;
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::R8G8B8A8_UBYTE;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        
        CTexture2DPtr SunShadowmapTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        // -----------------------------------------------------------------------------
        // Create render target
        // -----------------------------------------------------------------------------
        CTextureBasePtr HalfRenderbuffer[1];

        HalfRenderbuffer[0] = HalfTexturePtr;

        m_HalfRenderbufferPtr = TargetSetManager::CreateTargetSet(HalfRenderbuffer, 1);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupStates()
    {
        // -----------------------------------------------------------------------------
        // Get screen resolutions
        // -----------------------------------------------------------------------------
        Base::Int2 Size = Main::GetActiveWindowSize();

        Base::Int2 HalfSize(Size[0] / 2, Size[1] / 2);
        Base::Int2 QuarterSize(Size[0] / 4, Size[1] / 4);

        // -----------------------------------------------------------------------------
        // Build view ports
        // -----------------------------------------------------------------------------
        SViewPortDescriptor ViewPortDesc;

        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_Width    = static_cast<float>(HalfSize[0]);
        ViewPortDesc.m_Height   = static_cast<float>(HalfSize[1]);
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;
        
        CViewPortPtr HalfViewPort = ViewManager::CreateViewPort(ViewPortDesc);

        CViewPortSetPtr HalfViewPortSetPtr = ViewManager::CreateViewPortSet(HalfViewPort);
        
        // -----------------------------------------------------------------------------
        // Setup states
        // -----------------------------------------------------------------------------
        CCameraPtr      QuadCameraPtr        = ViewManager     ::GetFullQuadCamera();

        CViewPortSetPtr ViewPortSetPtr       = ViewManager     ::GetViewPortSet();

        CRenderStatePtr NoDepthStatePtr      = StateManager    ::GetRenderState(CRenderState::NoDepth);

        CTargetSetPtr   DeferredTargetSetPtr = TargetSetManager::GetDeferredTargetSet();
        
        // -----------------------------------------------------------------------------
        
        CRenderContextPtr DeferredContextPtr = ContextManager::CreateRenderContext();
        
        DeferredContextPtr->SetCamera(QuadCameraPtr);
        DeferredContextPtr->SetViewPortSet(ViewPortSetPtr);
        DeferredContextPtr->SetTargetSet(DeferredTargetSetPtr);
        DeferredContextPtr->SetRenderState(NoDepthStatePtr);
        
        m_DeferredRenderContextPtr = DeferredContextPtr;

        // -----------------------------------------------------------------------------

        CRenderContextPtr HalfContextPtr = ContextManager::CreateRenderContext();

        HalfContextPtr->SetCamera(QuadCameraPtr);
        HalfContextPtr->SetViewPortSet(HalfViewPortSetPtr);
        HalfContextPtr->SetTargetSet(m_HalfRenderbufferPtr);
        HalfContextPtr->SetRenderState(NoDepthStatePtr);

        m_HalfContextPtr = HalfContextPtr;
        
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
    
    void CGfxShadowRenderer::OnSetupTextures()
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        Base::Int2 Size = Main::GetActiveWindowSize();
        
        Base::Int2 HalfSize(Size[0] / 2, Size[1] / 2);
        
        CTextureBasePtr GBuffer0TexturePtr = TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0);
        CTextureBasePtr GBuffer1TexturePtr = TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1);
        CTextureBasePtr GBuffer2TexturePtr = TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2);
        CTextureBasePtr DepthTexturePtr    = TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget();
        CTextureBasePtr HalfTextureOnePtr  = m_HalfRenderbufferPtr->GetRenderTarget(0);
        
        // -----------------------------------------------------------------------------
        // Create texture for result of blurring
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = HalfSize[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = HalfSize[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::R8G8B8A8_UBYTE;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        
        CTexture2DPtr HalfTextureTwoPtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        // -----------------------------------------------------------------------------
        // Noise textures
        // -----------------------------------------------------------------------------
        Base::Byte3 NoiseColor[16];

        for (unsigned int IndexOfNoiseSeq = 0; IndexOfNoiseSeq < 16; ++IndexOfNoiseSeq)
        {
            Base::Float3 NoiseNormal;

            float fRandomX = (Base::Random() * 2.0f - 1.0f);
            float fRandomY = (Base::Random() * 2.0f - 1.0f);

            NoiseNormal = Base::Float3(fRandomX, fRandomY, 0.0f);

            NoiseNormal = NoiseNormal.Normalize();

            NoiseColor[IndexOfNoiseSeq][0] = static_cast<unsigned char>((NoiseNormal[0] * 0.5f + 0.5f) * 255.0f);
            NoiseColor[IndexOfNoiseSeq][1] = static_cast<unsigned char>((NoiseNormal[1] * 0.5f + 0.5f) * 255.0f);
            NoiseColor[IndexOfNoiseSeq][2] = static_cast<unsigned char>(0.0f);
        }

        STextureDescriptor NoiseTextureDescriptor;

        NoiseTextureDescriptor.m_NumberOfPixelsU  = 4;
        NoiseTextureDescriptor.m_NumberOfPixelsV  = 4;
        NoiseTextureDescriptor.m_NumberOfPixelsW  = 1;
        NoiseTextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        NoiseTextureDescriptor.m_NumberOfTextures = 1;
        NoiseTextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        NoiseTextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        NoiseTextureDescriptor.m_Format           = CTextureBase::Unknown;
        NoiseTextureDescriptor.m_Usage            = CTextureBase::GPURead;
        NoiseTextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        NoiseTextureDescriptor.m_pFileName        = 0;
        NoiseTextureDescriptor.m_pPixels          = NoiseColor;
        NoiseTextureDescriptor.m_Format           = CTextureBase::R8G8B8_UBYTE;

        CTextureBasePtr NoiseTexturePtr = static_cast<CTextureBasePtr>(TextureManager::CreateTexture2D(NoiseTextureDescriptor));

        // -----------------------------------------------------------------------------

        m_SSAOTextureSets[SSAO]      = TextureManager::CreateTextureSet(GBuffer0TexturePtr, GBuffer1TexturePtr, DepthTexturePtr, NoiseTexturePtr);
        m_SSAOTextureSets[SSAOApply] = TextureManager::CreateTextureSet(GBuffer0TexturePtr, GBuffer1TexturePtr, GBuffer2TexturePtr);
        
        m_HalfTexturePtrs[0]     = TextureManager::CreateTextureSet(HalfTextureOnePtr);
        m_HalfTexturePtrs[1]     = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(HalfTextureTwoPtr));

        m_BilateralBlurHTextureSetPtr = TextureManager::CreateTextureSet(HalfTextureOnePtr, static_cast<CTextureBasePtr>(HalfTextureTwoPtr));
        m_BilateralBlurVTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(HalfTextureTwoPtr), HalfTextureOnePtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupBuffers()
    {
        // -----------------------------------------------------------------------------
        // Setup view buffer for post rendering
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerLightConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PerLightConstantBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PerDrawCallConstantBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SGaussianSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr GaussianSettingsBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSSAOProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SSAOBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        m_LightCameraVSBufferPtr            = BufferManager::CreateBufferSet(PerLightConstantBuffer, PerDrawCallConstantBuffer);
        
        m_MainVSBufferPtr                   = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS(), PerDrawCallConstantBuffer);
        
        m_QuadVSBufferPtr                   = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS());

        m_GaussianBlurPropertiesCSBufferPtr = BufferManager::CreateBufferSet(GaussianSettingsBuffer);

        m_SSAOPropertiesPSBufferPtr         = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), SSAOBuffer);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupResources()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupModels()
    {
        m_QuadModelPtr = ModelManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::Render()
    {
        Performance::BeginEvent("Shadows");

        RenderSSAO();

        RenderShadowFromLightsources();

        RenderCascadedShadowFromSun();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::RenderSSAO()
    {
        if (m_SSAORenderJobs.size() == 0) return;

        Performance::BeginEvent("SSAO");

        const unsigned int pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Get screen resolutions
        // -----------------------------------------------------------------------------
        Base::Int2 Size = Main::GetActiveWindowSize();
        
        Base::Int2 HalfSize   (Size[0] / 2, Size[1] / 2);

        // -----------------------------------------------------------------------------
        // Rendering: SSAO
        // -----------------------------------------------------------------------------
        SSSAOProperties* pSSAOSettings;
        
        ContextManager::SetRenderContext(m_HalfContextPtr);

        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_FullquadShaderVSPtr);

        ContextManager::SetShaderPS(m_SSAOShaderPSPtrs[SSAO]);

        ContextManager::SetConstantBufferSetVS(m_QuadVSBufferPtr);

        ContextManager::SetConstantBufferSetPS(m_SSAOPropertiesPSBufferPtr);

        pSSAOSettings = static_cast<SSSAOProperties*>(BufferManager::MapConstantBuffer(m_SSAOPropertiesPSBufferPtr->GetBuffer(1)));
        
        assert(pSSAOSettings != 0);

        pSSAOSettings->m_InverseCameraProjection = ViewManager::GetMainCamera()->GetProjectionMatrix().GetInverted();
        pSSAOSettings->m_CameraProjection        = ViewManager::GetMainCamera()->GetProjectionMatrix();
        pSSAOSettings->m_CameraView              = ViewManager::GetMainCamera()->GetView()->GetViewMatrix();
        pSSAOSettings->m_NoiseScale              = Base::Float4(static_cast<float>(HalfSize[0]), static_cast<float>(HalfSize[1]), 0.08f, 0.0f);
        
        for (unsigned int IndexOfNoiseSeq = 0; IndexOfNoiseSeq < s_SSAOKernelSize; ++ IndexOfNoiseSeq)
        {
            pSSAOSettings->m_Kernel[IndexOfNoiseSeq] = m_Kernel[IndexOfNoiseSeq];
        }

        BufferManager::UnmapConstantBuffer(m_SSAOPropertiesPSBufferPtr->GetBuffer(1));

        ContextManager::SetTextureSetPS(m_SSAOTextureSets[SSAO]);

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTextureSetPS();

        ContextManager::ResetConstantBufferSetPS();

        ContextManager::ResetConstantBufferSetVS();

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

        ContextManager::ResetSamplerSetPS();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Rendering: Do gaussian blur
        // -----------------------------------------------------------------------------
        {
            unsigned int NumberOfThreadGroupsX;
            unsigned int NumberOfThreadGroupsY;

            NumberOfThreadGroupsX = (HalfSize[0] + s_BlurTileSize - 1) / (s_BlurTileSize);
            NumberOfThreadGroupsY = (HalfSize[1] + s_BlurTileSize - 1) / (s_BlurTileSize);

            ContextManager::SetShaderCS(m_BilateralBlurShaderCSPtr);

            ContextManager::SetConstantBufferSetCS(m_GaussianBlurPropertiesCSBufferPtr);

            // -----------------------------------------------------------------------------

            SGaussianSettings* pGaussianSettings = static_cast<SGaussianSettings*>(BufferManager::MapConstantBuffer(m_GaussianBlurPropertiesCSBufferPtr->GetBuffer(0)));

            pGaussianSettings->m_Direction[0]     = 1;
            pGaussianSettings->m_Direction[1]     = 0;
            pGaussianSettings->m_MaxPixelCoord[0] = HalfSize[0];
            pGaussianSettings->m_MaxPixelCoord[1] = HalfSize[1];
            pGaussianSettings->m_Weights[0]       = 0.036262f;
            pGaussianSettings->m_Weights[1]       = 0.051046f;
            pGaussianSettings->m_Weights[2]       = 0.067526f;
            pGaussianSettings->m_Weights[3]       = 0.083942f;
            pGaussianSettings->m_Weights[4]       = 0.098059f;
            pGaussianSettings->m_Weights[5]       = 0.107644f;
            pGaussianSettings->m_Weights[6]       = 0.111043f;

            BufferManager::UnmapConstantBuffer(m_GaussianBlurPropertiesCSBufferPtr->GetBuffer(0));

            ContextManager::SetTextureSetCS(m_BilateralBlurHTextureSetPtr);

            ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

            ContextManager::ResetTextureSetCS();

            // -----------------------------------------------------------------------------

            pGaussianSettings = static_cast<SGaussianSettings*>(BufferManager::MapConstantBuffer(m_GaussianBlurPropertiesCSBufferPtr->GetBuffer(0)));

            pGaussianSettings->m_Direction[0]     = 0;
            pGaussianSettings->m_Direction[1]     = 1;
            pGaussianSettings->m_MaxPixelCoord[0] = HalfSize[0];
            pGaussianSettings->m_MaxPixelCoord[1] = HalfSize[1];
            pGaussianSettings->m_Weights[0]       = 0.036262f;
            pGaussianSettings->m_Weights[1]       = 0.051046f;
            pGaussianSettings->m_Weights[2]       = 0.067526f;
            pGaussianSettings->m_Weights[3]       = 0.083942f;
            pGaussianSettings->m_Weights[4]       = 0.098059f;
            pGaussianSettings->m_Weights[5]       = 0.107644f;
            pGaussianSettings->m_Weights[6]       = 0.111043f;

            BufferManager::UnmapConstantBuffer(m_GaussianBlurPropertiesCSBufferPtr->GetBuffer(0));

            ContextManager::SetTextureSetCS(m_BilateralBlurVTextureSetPtr);

            ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

            ContextManager::ResetTextureSetCS();

            // -----------------------------------------------------------------------------

            ContextManager::ResetConstantBufferSetCS();

            ContextManager::ResetShaderCS();
        }

        // -----------------------------------------------------------------------------
        // Apply SSAO on deferred buffer
        // -----------------------------------------------------------------------------        
        ContextManager::SetRenderContext(m_DeferredRenderContextPtr);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_FullquadShaderVSPtr);
        
        ContextManager::SetShaderPS(m_SSAOShaderPSPtrs[SSAOApply]);
        
        ContextManager::SetConstantBufferSetVS(m_QuadVSBufferPtr);
        
        ContextManager::SetTextureSetPS(m_SSAOTextureSets[SSAOApply]);

        ContextManager::SetTextureSetPS(m_HalfTexturePtrs[0]);
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTextureSetPS();
        
        ContextManager::ResetConstantBufferSetPS();
        
        ContextManager::ResetConstantBufferSetVS();
        
        ContextManager::ResetTopology();
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBufferSet();
        
        ContextManager::ResetSamplerSetPS();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::RenderShadowFromLightsources()
    {
        if (m_PointLightRenderJobs.size() == 0) return;

        Performance::BeginEvent("Point Light Shadows");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = {0, 0};
        
        CPointLightRenderJobs::const_iterator CurrentRenderJob = m_PointLightRenderJobs.begin();
        CPointLightRenderJobs::const_iterator EndOfRenderJobs  = m_PointLightRenderJobs.end();

        for (; CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            Gfx::CPointLightFacet* pGraphicPointFacet = CurrentRenderJob->m_pGraphicPointLightFacet;

            // -----------------------------------------------------------------------------
            // Prepare shadow
            // -----------------------------------------------------------------------------
            TargetSetManager::ClearTargetSet(pGraphicPointFacet->GetRenderContext()->GetTargetSet());
            
            // -----------------------------------------------------------------------------
            // Set light as render target
            // -----------------------------------------------------------------------------
            ContextManager::SetRenderContext(pGraphicPointFacet->GetRenderContext());
            
            // -----------------------------------------------------------------------------
            // Set shader
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(m_ShadowShaderVSPtr);
            
            ContextManager::SetShaderPS(m_ShadowSMShaderPSPtr);
            
            // -----------------------------------------------------------------------------
            // Set constant buffer
            // -----------------------------------------------------------------------------
            ContextManager::SetConstantBufferSetVS(m_LightCameraVSBufferPtr);
            
            // -----------------------------------------------------------------------------
            // Upload data light view projection matrix
            // -----------------------------------------------------------------------------
            SPerLightConstantBuffer* pViewBuffer = static_cast<SPerLightConstantBuffer*>(BufferManager::MapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(0)));
            
            assert(pViewBuffer != nullptr);
            
            pViewBuffer->vs_ViewProjectionMatrix = pGraphicPointFacet->GetRenderContext()->GetCamera()->GetViewProjectionMatrix();
            
            BufferManager::UnmapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(0));
            
            // -----------------------------------------------------------------------------
            // Iterate throw every entity inside this map
            // -----------------------------------------------------------------------------
            Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Actor);
            Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();
            
            for (; CurrentEntity != EndOfEntities; )
            {
                Dt::CEntity& rCurrentEntity = *CurrentEntity;
                
                // -----------------------------------------------------------------------------
                // Get graphic facet
                // -----------------------------------------------------------------------------
                if (rCurrentEntity.GetType() != Dt::SActorType::Model)
                {
                    CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);

                    continue;
                }
                
                // -----------------------------------------------------------------------------
                // Set other graphic data of this entity
                // -----------------------------------------------------------------------------
                CModelActorFacet* pActorModelFacet = static_cast<CModelActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                CModelPtr ModelPtr = pActorModelFacet->GetModel();
                
                // -----------------------------------------------------------------------------
                // Upload model matrix to buffer
                // -----------------------------------------------------------------------------
                SPerDrawCallConstantBuffer* pModelBuffer = static_cast<SPerDrawCallConstantBuffer*>(BufferManager::MapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(1)));
                
                assert(pModelBuffer != nullptr);
                
                pModelBuffer->m_ModelMatrix = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();
                
                BufferManager::UnmapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(1));
                
                // -----------------------------------------------------------------------------
                // Render every surface of this entity
                // -----------------------------------------------------------------------------
                unsigned int NumberOfSurfaces = ModelPtr->GetLOD(0)->GetNumberOfSurfaces();
                
                for (unsigned int IndexOfSurface = 0; IndexOfSurface < NumberOfSurfaces; ++ IndexOfSurface)
                {
                    CSurfacePtr SurfacePtr = ModelPtr->GetLOD(0)->GetSurface(IndexOfSurface);
                    
                    if (SurfacePtr == nullptr)
                    {
                        continue;
                    }
                    
                    // -----------------------------------------------------------------------------
                    // Set material
                    // -----------------------------------------------------------------------------
                    CMaterialPtr MaterialPtr = SurfacePtr->GetMaterial();
                    
                    // -----------------------------------------------------------------------------
                    // Get input layout from optimal shader
                    // -----------------------------------------------------------------------------
                    assert(SurfacePtr->GetKey().m_HasPosition);
                    
                    CInputLayoutPtr LayoutPtr = SurfacePtr->GetMaterial()->GetShaderVS()->GetInputLayout();
                    
                    // -----------------------------------------------------------------------------
                    // Set items to context manager
                    // -----------------------------------------------------------------------------
                    ContextManager::SetVertexBufferSet(SurfacePtr->GetVertexBuffer(), pOffset);
                    
                    ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);
                    
                    ContextManager::SetInputLayout(LayoutPtr);
                    
                    ContextManager::SetTopology(STopology::TriangleList);
                    
                    ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
                    
                    ContextManager::ResetTopology();
                    
                    ContextManager::ResetInputLayout();
                    
                    ContextManager::ResetIndexBuffer();
                    
                    ContextManager::ResetVertexBufferSet();
                }
                
                // -----------------------------------------------------------------------------
                // Next entity
                // -----------------------------------------------------------------------------
                CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);
            }
            
            ContextManager::ResetConstantBufferSetVS();

            ContextManager::ResetShaderVS();
            
            ContextManager::ResetShaderPS();
            
            ContextManager::ResetRenderContext();
        }

        Performance::EndEvent();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::RenderCascadedShadowFromSun()
    {
        if (m_SunLightRenderJobs.size() == 0) return;

        Performance::BeginEvent("Sun Shadows");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = {0, 0};
        
        CSunLightRenderJobs::const_iterator CurrentRenderJob = m_SunLightRenderJobs.begin();
        CSunLightRenderJobs::const_iterator EndOfRenderJobs  = m_SunLightRenderJobs.end();

        for (; CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            Gfx::CSunLightFacet* pGraphicSunFacet = CurrentRenderJob->m_pGraphicSunLightFacet;

            // -----------------------------------------------------------------------------
            // Prepare shadow
            // -----------------------------------------------------------------------------
            TargetSetManager::ClearTargetSet(pGraphicSunFacet->GetRenderContext()->GetTargetSet());
            
            // -----------------------------------------------------------------------------
            // Set light as render target
            // -----------------------------------------------------------------------------
            ContextManager::SetRenderContext(pGraphicSunFacet->GetRenderContext());
            
            // -----------------------------------------------------------------------------
            // Set shader
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(m_ShadowShaderVSPtr);
            
            ContextManager::SetShaderPS(m_ShadowSMShaderPSPtr);
            
            // -----------------------------------------------------------------------------
            // Set constant buffer
            // -----------------------------------------------------------------------------
            ContextManager::SetConstantBufferSetVS(m_LightCameraVSBufferPtr);
            
            // -----------------------------------------------------------------------------
            // Upload data light view projection matrix
            // -----------------------------------------------------------------------------
            SPerLightConstantBuffer* pViewBuffer = static_cast<SPerLightConstantBuffer*>(BufferManager::MapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(0)));
            
            assert(pViewBuffer != nullptr);
            
            pViewBuffer->vs_ViewProjectionMatrix = pGraphicSunFacet->GetRenderContext()->GetCamera()->GetViewProjectionMatrix();
            
            BufferManager::UnmapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(0));
            
            // -----------------------------------------------------------------------------
            // Iterate throw every entity inside this map
            // -----------------------------------------------------------------------------
            Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Actor);
            Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();
            
            for (; CurrentEntity != EndOfEntities; )
            {
                Dt::CEntity& rCurrentEntity = *CurrentEntity;
                
                // -----------------------------------------------------------------------------
                // Get graphic facet
                // -----------------------------------------------------------------------------
                if (rCurrentEntity.GetType() != Dt::SActorType::Model)
                {
                    CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);

                    continue;
                }
                
                // -----------------------------------------------------------------------------
                // Set other graphic data of this entity
                // -----------------------------------------------------------------------------
                CModelActorFacet* pActorModelFacet = static_cast<CModelActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                CModelPtr ModelPtr = pActorModelFacet->GetModel();
                
                // -----------------------------------------------------------------------------
                // Upload model matrix to buffer
                // -----------------------------------------------------------------------------
                SPerDrawCallConstantBuffer* pModelBuffer = static_cast<SPerDrawCallConstantBuffer*>(BufferManager::MapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(1)));
                
                assert(pModelBuffer != nullptr);
                
                pModelBuffer->m_ModelMatrix = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();
                
                BufferManager::UnmapConstantBuffer(m_LightCameraVSBufferPtr->GetBuffer(1));
                
                // -----------------------------------------------------------------------------
                // Render every surface of this entity
                // -----------------------------------------------------------------------------
                unsigned int NumberOfSurfaces = ModelPtr->GetLOD(0)->GetNumberOfSurfaces();
                
                for (unsigned int IndexOfSurface = 0; IndexOfSurface < NumberOfSurfaces; ++ IndexOfSurface)
                {
                    CSurfacePtr SurfacePtr = ModelPtr->GetLOD(0)->GetSurface(IndexOfSurface);
                    
                    if (SurfacePtr == nullptr)
                    {
                        continue;
                    }
                    
                    // -----------------------------------------------------------------------------
                    // Set material
                    // -----------------------------------------------------------------------------
                    CMaterialPtr MaterialPtr = SurfacePtr->GetMaterial();
                    
                    // -----------------------------------------------------------------------------
                    // Get input layout from optimal shader
                    // -----------------------------------------------------------------------------
                    assert(SurfacePtr->GetKey().m_HasPosition);
                    
                    CInputLayoutPtr LayoutPtr = SurfacePtr->GetMaterial()->GetShaderVS()->GetInputLayout();
                    
                    // -----------------------------------------------------------------------------
                    // Set items to context manager
                    // -----------------------------------------------------------------------------
                    ContextManager::SetVertexBufferSet(SurfacePtr->GetVertexBuffer(), pOffset);
                    
                    ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);
                    
                    ContextManager::SetInputLayout(LayoutPtr);
                    
                    ContextManager::SetTopology(STopology::TriangleList);
                    
                    ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
                    
                    ContextManager::ResetTopology();
                    
                    ContextManager::ResetInputLayout();
                    
                    ContextManager::ResetIndexBuffer();
                    
                    ContextManager::ResetVertexBufferSet();
                }
                
                // -----------------------------------------------------------------------------
                // Next entity
                // -----------------------------------------------------------------------------
                CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);
            }
            
            ContextManager::ResetConstantBufferSetVS();

            ContextManager::ResetShaderVS();
            
            ContextManager::ResetShaderPS();
            
            ContextManager::ResetRenderContext();
        }

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxShadowRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_SunLightRenderJobs  .clear();
        m_PointLightRenderJobs.clear();

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            if (rCurrentEntity.GetType() == Dt::SLightType::Sun)
            {
                Dt::CSunLightFacet*  pDataSunFacet    = static_cast<Dt::CSunLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                Gfx::CSunLightFacet* pGraphicSunFacet = static_cast<Gfx::CSunLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                // -----------------------------------------------------------------------------
                // If update is needed
                // -----------------------------------------------------------------------------
                Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

                if (pGraphicSunFacet->GetTimeStamp() == FrameTime || pDataSunFacet->GetRefreshMode() == Dt::CSunLightFacet::Dynamic)
                {
                    // -----------------------------------------------------------------------------
                    // Set sun into a new render job
                    // -----------------------------------------------------------------------------
                    SSunLightRenderJob NewRenderJob;

                    NewRenderJob.m_pGraphicSunLightFacet = pGraphicSunFacet;

                    m_SunLightRenderJobs.push_back(NewRenderJob);
                }
            }
            else if (rCurrentEntity.GetType() == Dt::SLightType::Point)
            {
                Dt::CPointLightFacet*  pDataPointFacet    = static_cast<Dt::CPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                Gfx::CPointLightFacet* pGraphicPointFacet = static_cast<Gfx::CPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                // -----------------------------------------------------------------------------
                // If update is needed
                // -----------------------------------------------------------------------------
                Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

                if (pGraphicPointFacet->GetTimeStamp() == FrameTime || pDataPointFacet->GetRefreshMode() == Dt::CPointLightFacet::Dynamic)
                {
                    // -----------------------------------------------------------------------------
                    // Set sun into a new render job
                    // -----------------------------------------------------------------------------
                    SPointLightRenderJob NewRenderJob;

                    NewRenderJob.m_pGraphicPointLightFacet = pGraphicPointFacet;

                    m_PointLightRenderJobs.push_back(NewRenderJob);
                }
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
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
            if (rCurrentEntity.GetType() == Dt::SFXType::SSAO)
            {
                Dt::CSSAOFXFacet* pDataSSAOFacet = static_cast<Dt::CSSAOFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

                assert(pDataSSAOFacet != 0);

                // -----------------------------------------------------------------------------
                // Set sun into a new render job
                // -----------------------------------------------------------------------------
                SSSAORenderJob NewRenderJob;

                NewRenderJob.m_pDataSSAOFacet = pDataSSAOFacet;

                m_SSAORenderJobs.push_back(NewRenderJob);
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
namespace ShadowRenderer
{
    void OnStart()
    {
        CGfxShadowRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxShadowRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxShadowRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxShadowRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxShadowRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxShadowRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxShadowRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxShadowRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxShadowRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxShadowRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxShadowRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxShadowRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxShadowRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxShadowRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxShadowRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxShadowRenderer::GetInstance().Render();
    }
} // namespace ShadowRenderer
} // namespace Gfx
