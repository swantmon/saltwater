
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_entity.h"
#include "data/data_fx_type.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_ssao_facet.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_sun_facet.h"
#include "graphic/gfx_sun_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_manager.h"
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
            Dt::CSSAOFXFacet* m_pDataSSAOFacet;
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

        typedef std::vector<SSSAORenderJob> CSSAORenderJobs;
        
    private:
        
       

        CMeshPtr m_QuadModelPtr;
        
        CBufferPtr  m_GaussianBlurPropertiesCSBufferPtr;
        CBufferPtr  m_SSAOPropertiesPSBufferPtr;
        
        CInputLayoutPtr m_QuadInputLayoutPtr;

        CShaderPtr m_FullquadShaderVSPtr;
        CShaderPtr m_SSAOShaderPSPtrs[NumberOfSSAOs];
        CShaderPtr m_GaussianBlurShaderCSPtr;

        CTextureBasePtr m_NoiseTexturePtr;

        CTextureSetPtr m_HalfTexturePtrs[2];
        CTextureSetPtr m_BilateralBlurHTextureSetPtr;
        CTextureSetPtr m_BilateralBlurVTextureSetPtr;

        CTargetSetPtr m_HalfRenderbufferPtr;

        CRenderContextPtr m_DeferredRenderContextPtr;
        CRenderContextPtr m_HalfContextPtr;

        Base::Float4 m_Kernel[s_SSAOKernelSize];

        CSSAORenderJobs m_SSAORenderJobs;

    private:

        void RenderSSAO();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxShadowRenderer::CGfxShadowRenderer()
        : m_QuadModelPtr                     ()
        , m_SSAOPropertiesPSBufferPtr        ()
        , m_GaussianBlurPropertiesCSBufferPtr()
        , m_QuadInputLayoutPtr               ()
        , m_FullquadShaderVSPtr              ()
        , m_GaussianBlurShaderCSPtr         ()
        , m_BilateralBlurHTextureSetPtr      ()
        , m_BilateralBlurVTextureSetPtr      ()
        , m_NoiseTexturePtr                  ()
        , m_DeferredRenderContextPtr         ()
        , m_SSAORenderJobs                   ()
    {
        m_SSAORenderJobs.reserve(1);

        // -----------------------------------------------------------------------------
        // Register for resizing events
        // -----------------------------------------------------------------------------
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxShadowRenderer::OnResize));
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
        m_GaussianBlurPropertiesCSBufferPtr = 0;
        m_SSAOPropertiesPSBufferPtr         = 0;
        m_QuadInputLayoutPtr                = 0;
        m_FullquadShaderVSPtr               = 0;
        m_GaussianBlurShaderCSPtr          = 0;
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
        m_FullquadShaderVSPtr = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");

        m_SSAOShaderPSPtrs[SSAO]      = ShaderManager::CompilePS("fs_ssao.glsl"      , "main");
        m_SSAOShaderPSPtrs[SSAOApply] = ShaderManager::CompilePS("fs_ssao_apply.glsl", "main");

        m_GaussianBlurShaderCSPtr = ShaderManager::CompileCS("cs_gaussian_blur.glsl", "main", "#define TILE_SIZE 8\n#define IMAGE_TYPE rgba8");
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor QuadInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        m_QuadInputLayoutPtr = ShaderManager::CreateInputLayout(QuadInputLayout, 1, m_FullquadShaderVSPtr);
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
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadowRenderer::OnSetupTextures()
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        Base::Int2 Size = Main::GetActiveWindowSize();
        
        Base::Int2 HalfSize(Size[0] / 2, Size[1] / 2);

        CTextureBasePtr HalfTextureOnePtr = m_HalfRenderbufferPtr->GetRenderTarget(0);

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

        m_NoiseTexturePtr = static_cast<CTextureBasePtr>(TextureManager::CreateTexture2D(NoiseTextureDescriptor));

        // -----------------------------------------------------------------------------
        
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
        m_QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
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
        Base::Int2 Size(_Width, _Height);

        Base::Int2 HalfSize   (Size[0] / 2, Size[1] / 2);
        Base::Int2 QuarterSize(Size[0] / 4, Size[1] / 4);

        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
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

        CTexture2DPtr HalfTexturePtr    = TextureManager::CreateTexture2D(RendertargetDescriptor);
        CTexture2DPtr HalfTextureTwoPtr = TextureManager::CreateTexture2D(RendertargetDescriptor);
        
        // -----------------------------------------------------------------------------
        // Create render target
        // -----------------------------------------------------------------------------
        CTextureBasePtr HalfRenderbuffer[1];

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
        CTextureBasePtr HalfTextureOnePtr  = m_HalfRenderbufferPtr->GetRenderTarget(0);
        
        // -----------------------------------------------------------------------------
        // Create texture sets
        // -----------------------------------------------------------------------------
        m_HalfTexturePtrs[0] = TextureManager::CreateTextureSet(HalfTextureOnePtr);
        m_HalfTexturePtrs[1] = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(HalfTextureTwoPtr));

        m_BilateralBlurHTextureSetPtr = TextureManager::CreateTextureSet(HalfTextureOnePtr, static_cast<CTextureBasePtr>(HalfTextureTwoPtr));
        m_BilateralBlurVTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(HalfTextureTwoPtr), HalfTextureOnePtr);
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
        Base::Int2 Size = Main::GetActiveWindowSize();
        
        Base::Int2 HalfSize   (Size[0] / 2, Size[1] / 2);

        // -----------------------------------------------------------------------------
        // Clear buffer
        // -----------------------------------------------------------------------------
        TargetSetManager::ClearTargetSet(m_HalfRenderbufferPtr, Base::Float4(1.0f));

        // -----------------------------------------------------------------------------
        // Rendering: SSAO
        // -----------------------------------------------------------------------------
        SSSAOProperties SSAOSettings;
        
        ContextManager::SetRenderContext(m_HalfContextPtr);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_FullquadShaderVSPtr);

        ContextManager::SetShaderPS(m_SSAOShaderPSPtrs[SSAO]);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_SSAOPropertiesPSBufferPtr);

        SSAOSettings.m_InverseCameraProjection = ViewManager::GetMainCamera()->GetProjectionMatrix().GetInverted();
        SSAOSettings.m_CameraProjection        = ViewManager::GetMainCamera()->GetProjectionMatrix();
        SSAOSettings.m_CameraView              = ViewManager::GetMainCamera()->GetView()->GetViewMatrix();
        SSAOSettings.m_NoiseScale              = Base::Float4(static_cast<float>(HalfSize[0]), static_cast<float>(HalfSize[1]), 0.14f, 2.0f);
        
        for (unsigned int IndexOfNoiseSeq = 0; IndexOfNoiseSeq < s_SSAOKernelSize; ++ IndexOfNoiseSeq)
        {
            SSAOSettings.m_Kernel[IndexOfNoiseSeq] = m_Kernel[IndexOfNoiseSeq];
        }

        BufferManager::UploadConstantBufferData(m_SSAOPropertiesPSBufferPtr, &SSAOSettings);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap));

        ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
        ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
        ContextManager::SetTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2));
        ContextManager::SetTexture(3, m_NoiseTexturePtr);

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

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

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

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

            BufferManager::UploadConstantBufferData(m_GaussianBlurPropertiesCSBufferPtr, &GaussianSettings);

            ContextManager::SetImageTexture(0, m_BilateralBlurHTextureSetPtr->GetTexture(0));
            ContextManager::SetImageTexture(1, m_BilateralBlurHTextureSetPtr->GetTexture(1));

            ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

            ContextManager::ResetImageTexture(0);
            ContextManager::ResetImageTexture(1);

            GaussianSettings.m_Direction[0] = 0;
            GaussianSettings.m_Direction[1] = 1;

            BufferManager::UploadConstantBufferData(m_GaussianBlurPropertiesCSBufferPtr, &GaussianSettings);

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
        
        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);
        
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
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
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
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBuffer();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxShadowRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_SSAORenderJobs.clear();

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
