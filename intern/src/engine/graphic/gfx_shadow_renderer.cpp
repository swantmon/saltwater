
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"
#include "engine/core/core_time.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_map.h"
#include "engine/data/data_ssao_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_sun.h"
#include "engine/graphic/gfx_sun_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_shadow_renderer.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

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

        void OnResize(unsigned int _Width, unsigned int _Height);
        
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

        struct SSSAORenderJob
        {
            Dt::CSSAOComponent* m_pDataSSAOFacet;
        };
        
        struct SGaussianSettings
        {
            glm::ivec2 m_Direction;
            glm::ivec2 m_MaxPixelCoord;
            float      m_Weights[7];
        };

        struct SSSAOProperties
        {
            glm::mat4 m_InverseCameraProjection;
            glm::mat4 m_CameraProjection;
            glm::mat4 m_CameraView;
            glm::vec4 m_NoiseScale;
            glm::vec4 m_Kernel[s_SSAOKernelSize];
        };

    private:

        typedef std::vector<SSSAORenderJob> CSSAORenderJobs;

    private:

        CBufferPtr  m_GaussianBlurPropertiesCSBufferPtr;
        CBufferPtr  m_SSAOPropertiesPSBufferPtr;

        CShaderPtr m_FullquadShaderVSPtr;
        CShaderPtr m_SSAOShaderPSPtrs[NumberOfSSAOs];
        CShaderPtr m_GaussianBlurShaderCSPtr;

        CTexturePtr m_NoiseTexturePtr;

        CTextureSetPtr m_HalfTexturePtrs[2];
        CTextureSetPtr m_BilateralBlurHTextureSetPtr;
        CTextureSetPtr m_BilateralBlurVTextureSetPtr;

        CTargetSetPtr m_HalfRenderbufferPtr;

        CRenderContextPtr m_DeferredRenderContextPtr;
        CRenderContextPtr m_HalfContextPtr;

        glm::vec4 m_Kernel[s_SSAOKernelSize];

        CSSAORenderJobs m_SSAORenderJobs;

		Gfx::Main::CResizeDelegate::HandleType m_OnResizeDelegate;

    private:

        void RenderSSAO();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxShadowRenderer::CGfxShadowRenderer()
        : m_SSAOPropertiesPSBufferPtr        ()
        , m_GaussianBlurPropertiesCSBufferPtr()
        , m_FullquadShaderVSPtr              ()
        , m_GaussianBlurShaderCSPtr          ()
        , m_BilateralBlurHTextureSetPtr      ()
        , m_BilateralBlurVTextureSetPtr      ()
        , m_NoiseTexturePtr                  ()
        , m_DeferredRenderContextPtr         ()
        , m_SSAORenderJobs                   ()
    {
        m_SSAORenderJobs.reserve(1);

		m_OnResizeDelegate = Gfx::Main::RegisterResizeHandler(std::bind(&CGfxShadowRenderer::OnResize, this, std::placeholders::_1, std::placeholders::_2));
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
            glm::vec3 KernelElement;

            KernelElement = glm::linearRand(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

            float Scale = static_cast<float>(IndexOfNoiseSeq) / static_cast<float>(s_SSAOKernelSize);

            KernelElement += glm::lerp(0.1f, 1.0f, Scale * Scale);

            KernelElement = glm::normalize(KernelElement);

            m_Kernel[IndexOfNoiseSeq] = glm::vec4(KernelElement, 0.0f);
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnExit()
    {
        m_GaussianBlurPropertiesCSBufferPtr = 0;
        m_SSAOPropertiesPSBufferPtr         = 0;
        m_FullquadShaderVSPtr               = 0;
        m_GaussianBlurShaderCSPtr           = 0;
        m_BilateralBlurHTextureSetPtr       = 0;
        m_BilateralBlurVTextureSetPtr       = 0;
        m_HalfRenderbufferPtr               = 0;
        m_DeferredRenderContextPtr          = 0;
        m_HalfContextPtr                    = 0;
        m_NoiseTexturePtr                   = 0;

        m_SSAOShaderPSPtrs[SSAO]      = 0;
        m_SSAOShaderPSPtrs[SSAOApply] = 0;

        m_HalfTexturePtrs[0] = 0;
        m_HalfTexturePtrs[1] = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupShader()
    {
        m_FullquadShaderVSPtr = ShaderManager::CompileVS("system/vs_fullscreen.glsl", "main");

        m_SSAOShaderPSPtrs[SSAO]      = ShaderManager::CompilePS("ssao/fs_ssao.glsl"      , "main");
        m_SSAOShaderPSPtrs[SSAOApply] = ShaderManager::CompilePS("ssao/fs_ssao_apply.glsl", "main");

        m_GaussianBlurShaderCSPtr = ShaderManager::CompileCS("filter/cs_gaussian_blur.glsl", "main", "#define TILE_SIZE 8\n#define IMAGE_TYPE rgba8");
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
        glm::ivec2 Size = Main::GetActiveWindowSize();

        glm::ivec2 HalfSize   (Size[0] / 2, Size[1] / 2);

        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;

        RendertargetDescriptor.m_NumberOfPixelsU  = HalfSize[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = HalfSize[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::RenderTarget;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::R8G8B8A8_UBYTE;
        RendertargetDescriptor.m_Usage            = CTexture::GPURead;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;

        CTexturePtr HalfTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

		TextureManager::SetTextureLabel(HalfTexturePtr, "SSAO");
        
        // -----------------------------------------------------------------------------
        // Create render target
        // -----------------------------------------------------------------------------
        CTexturePtr HalfRenderbuffer[1];

        HalfRenderbuffer[0] = HalfTexturePtr;

        m_HalfRenderbufferPtr = TargetSetManager::CreateTargetSet(HalfRenderbuffer, 1);

		TargetSetManager::SetTargetSetLabel(m_HalfRenderbufferPtr, "SSAO");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupStates()
    {
        // -----------------------------------------------------------------------------
        // Get screen resolutions
        // -----------------------------------------------------------------------------
        glm::ivec2 Size = Main::GetActiveWindowSize();

        glm::ivec2 HalfSize(Size[0] / 2, Size[1] / 2);
        glm::ivec2 QuarterSize(Size[0] / 4, Size[1] / 4);

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
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupTextures()
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        glm::ivec2 Size = Main::GetActiveWindowSize();
        
        glm::ivec2 HalfSize(Size[0] / 2, Size[1] / 2);

        CTexturePtr HalfTextureOnePtr = m_HalfRenderbufferPtr->GetRenderTarget(0);

        // -----------------------------------------------------------------------------
        // Create texture for result of blurring
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = HalfSize[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = HalfSize[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::RenderTarget;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::R8G8B8A8_UBYTE;
        RendertargetDescriptor.m_Usage            = CTexture::GPURead;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        
        CTexturePtr HalfTextureTwoPtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        // -----------------------------------------------------------------------------
        // Noise textures
        // -----------------------------------------------------------------------------
        glm::tvec3<char> NoiseColor[16];

        for (unsigned int IndexOfNoiseSeq = 0; IndexOfNoiseSeq < 16; ++IndexOfNoiseSeq)
        {
            glm::vec3 NoiseNormal;

            NoiseNormal = glm::vec3(glm::linearRand(glm::vec2(-1.0f, -1.0f), glm::vec2(1.0f, 1.0f)), 0.0f);

            NoiseNormal = glm::normalize(NoiseNormal);

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
        NoiseTextureDescriptor.m_Binding          = CTexture::ShaderResource;
        NoiseTextureDescriptor.m_Access           = CTexture::CPUWrite;
        NoiseTextureDescriptor.m_Format           = CTexture::Unknown;
        NoiseTextureDescriptor.m_Usage            = CTexture::GPURead;
        NoiseTextureDescriptor.m_Semantic         = CTexture::Diffuse;
        NoiseTextureDescriptor.m_pFileName        = 0;
        NoiseTextureDescriptor.m_pPixels          = NoiseColor;
        NoiseTextureDescriptor.m_Format           = CTexture::R8G8B8_UBYTE;

        m_NoiseTexturePtr = static_cast<CTexturePtr>(TextureManager::CreateTexture2D(NoiseTextureDescriptor));

        // -----------------------------------------------------------------------------
        
        m_HalfTexturePtrs[0]     = TextureManager::CreateTextureSet(HalfTextureOnePtr);
        m_HalfTexturePtrs[1]     = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(HalfTextureTwoPtr));

        m_BilateralBlurHTextureSetPtr = TextureManager::CreateTextureSet(HalfTextureOnePtr, static_cast<CTexturePtr>(HalfTextureTwoPtr));
        m_BilateralBlurVTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(HalfTextureTwoPtr), HalfTextureOnePtr);
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
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SGaussianSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_GaussianBlurPropertiesCSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSSAOProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_SSAOPropertiesPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupResources()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupModels()
    {
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

    void CGfxShadowRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {
        STextureDescriptor RendertargetDescriptor;

        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        glm::ivec2 Size(_Width, _Height);

        glm::ivec2 HalfSize   (Size[0] / 2, Size[1] / 2);
        glm::ivec2 QuarterSize(Size[0] / 4, Size[1] / 4);

        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        RendertargetDescriptor.m_NumberOfPixelsU  = HalfSize[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = HalfSize[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::RenderTarget;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::R8G8B8A8_UBYTE;
        RendertargetDescriptor.m_Usage            = CTexture::GPURead;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;

        CTexturePtr HalfTexturePtr    = TextureManager::CreateTexture2D(RendertargetDescriptor);
        CTexturePtr HalfTextureTwoPtr = TextureManager::CreateTexture2D(RendertargetDescriptor);
        
        // -----------------------------------------------------------------------------
        // Create render target
        // -----------------------------------------------------------------------------
        CTexturePtr HalfRenderbuffer[1];

        HalfRenderbuffer[0] = HalfTexturePtr;

        m_HalfRenderbufferPtr = TargetSetManager::CreateTargetSet(HalfRenderbuffer, 1);

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
        m_HalfContextPtr->SetViewPortSet(HalfViewPortSetPtr);
        m_HalfContextPtr->SetTargetSet(m_HalfRenderbufferPtr);

        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        CTexturePtr HalfTextureOnePtr  = m_HalfRenderbufferPtr->GetRenderTarget(0);
        
        // -----------------------------------------------------------------------------
        // Create texture sets
        // -----------------------------------------------------------------------------
        m_HalfTexturePtrs[0] = TextureManager::CreateTextureSet(HalfTextureOnePtr);
        m_HalfTexturePtrs[1] = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(HalfTextureTwoPtr));

        m_BilateralBlurHTextureSetPtr = TextureManager::CreateTextureSet(HalfTextureOnePtr, static_cast<CTexturePtr>(HalfTextureTwoPtr));
        m_BilateralBlurVTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(HalfTextureTwoPtr), HalfTextureOnePtr);
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

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::RenderSSAO()
    {
        if (m_SSAORenderJobs.size() == 0) return;

        Performance::BeginEvent("SSAO");

        // -----------------------------------------------------------------------------
        // Get screen resolutions
        // -----------------------------------------------------------------------------
        glm::ivec2 Size = Main::GetActiveWindowSize();
        
        glm::ivec2 HalfSize   (Size[0] / 2, Size[1] / 2);

        // -----------------------------------------------------------------------------
        // Clear buffer
        // -----------------------------------------------------------------------------
        TargetSetManager::ClearTargetSet(m_HalfRenderbufferPtr, glm::vec4(1.0f));

        // -----------------------------------------------------------------------------
        // Rendering: SSAO
        // -----------------------------------------------------------------------------
        SSSAOProperties SSAOSettings;
        
        ContextManager::SetRenderContext(m_HalfContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_FullquadShaderVSPtr);

        ContextManager::SetShaderPS(m_SSAOShaderPSPtrs[SSAO]);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_SSAOPropertiesPSBufferPtr);

        SSAOSettings.m_InverseCameraProjection = glm::inverse(ViewManager::GetMainCamera()->GetProjectionMatrix());
        SSAOSettings.m_CameraProjection        = ViewManager::GetMainCamera()->GetProjectionMatrix();
        SSAOSettings.m_CameraView              = ViewManager::GetMainCamera()->GetView()->GetViewMatrix();
        SSAOSettings.m_NoiseScale              = glm::vec4(static_cast<float>(HalfSize[0]), static_cast<float>(HalfSize[1]), 0.14f, 2.0f);
        
        for (unsigned int IndexOfNoiseSeq = 0; IndexOfNoiseSeq < s_SSAOKernelSize; ++ IndexOfNoiseSeq)
        {
            SSAOSettings.m_Kernel[IndexOfNoiseSeq] = m_Kernel[IndexOfNoiseSeq];
        }

        BufferManager::UploadBufferData(m_SSAOPropertiesPSBufferPtr, &SSAOSettings);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap));

        ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
        ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
        ContextManager::SetTexture(2, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());
        ContextManager::SetTexture(3, m_NoiseTexturePtr);

        ContextManager::Draw(3, 0);

        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);
        ContextManager::ResetTexture(2);
        ContextManager::ResetTexture(3);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        ContextManager::ResetSampler(2);
        ContextManager::ResetSampler(3);

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetTopology();

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

            SGaussianSettings GaussianSettings;

            GaussianSettings.m_MaxPixelCoord[0] = HalfSize[0];
            GaussianSettings.m_MaxPixelCoord[1] = HalfSize[1];
            GaussianSettings.m_Weights[0] = 0.036262f;
            GaussianSettings.m_Weights[1] = 0.051046f;
            GaussianSettings.m_Weights[2] = 0.067526f;
            GaussianSettings.m_Weights[3] = 0.083942f;
            GaussianSettings.m_Weights[4] = 0.098059f;
            GaussianSettings.m_Weights[5] = 0.107644f;
            GaussianSettings.m_Weights[6] = 0.111043f;

            ContextManager::SetShaderCS(m_GaussianBlurShaderCSPtr);

            ContextManager::SetResourceBuffer(0, m_GaussianBlurPropertiesCSBufferPtr);

            GaussianSettings.m_Direction[0] = 1;
            GaussianSettings.m_Direction[1] = 0;

            BufferManager::UploadBufferData(m_GaussianBlurPropertiesCSBufferPtr, &GaussianSettings);

            ContextManager::SetImageTexture(0, m_BilateralBlurHTextureSetPtr->GetTexture(0));
            ContextManager::SetImageTexture(1, m_BilateralBlurHTextureSetPtr->GetTexture(1));

            ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

            ContextManager::ResetImageTexture(0);
            ContextManager::ResetImageTexture(1);

            GaussianSettings.m_Direction[0] = 0;
            GaussianSettings.m_Direction[1] = 1;

            BufferManager::UploadBufferData(m_GaussianBlurPropertiesCSBufferPtr, &GaussianSettings);

            ContextManager::SetImageTexture(0, m_BilateralBlurVTextureSetPtr->GetTexture(0));
            ContextManager::SetImageTexture(1, m_BilateralBlurVTextureSetPtr->GetTexture(1));

            ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

            ContextManager::ResetImageTexture(0);
            ContextManager::ResetImageTexture(1);

            // -----------------------------------------------------------------------------

            ContextManager::ResetResourceBuffer(0);

            ContextManager::ResetShaderCS();
        }

        // -----------------------------------------------------------------------------
        // Apply SSAO on deferred buffer
        // -----------------------------------------------------------------------------        
        ContextManager::SetRenderContext(m_DeferredRenderContextPtr);
                
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_FullquadShaderVSPtr);
        
        ContextManager::SetShaderPS(m_SSAOShaderPSPtrs[SSAOApply]);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
        ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
        ContextManager::SetTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2));
        ContextManager::SetTexture(3, m_HalfTexturePtrs[0]->GetTexture(0));
        
        ContextManager::Draw(3, 0);
        
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);
        ContextManager::ResetTexture(2);
        ContextManager::ResetTexture(3);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        ContextManager::ResetSampler(2);
        ContextManager::ResetSampler(3);
        
        ContextManager::ResetConstantBuffer(0);
        
        ContextManager::ResetTopology();

        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxShadowRenderer::BuildRenderJobs()
    {
        m_SSAORenderJobs.clear();

        auto DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CSSAOComponent>();

        for (auto Component : DataComponents)
        {
            Dt::CSSAOComponent* pDtComponent = static_cast<Dt::CSSAOComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            SSSAORenderJob NewRenderJob;

            NewRenderJob.m_pDataSSAOFacet = pDtComponent;

            m_SSAORenderJobs.push_back(NewRenderJob);
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
