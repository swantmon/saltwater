
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/data/data_bloom_component.h"
#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_map.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_postfx_hdr_renderer.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxPostFXHDRRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxPostFXHDRRenderer)
        
    public:
        CGfxPostFXHDRRenderer();
        ~CGfxPostFXHDRRenderer();
        
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

        static const unsigned int s_NumberOfBlurStages = 5;
        static const unsigned int s_BlurTileSize       = 8;
        
    private:
        
        enum EPostEffectShader
        {
            Bloom,
            NumberOfPostEffects,
            UndefinedPostEffect = -1
        };

        enum EDownSamples
        {
            DownSample1,
            DownSample2,
            DownSample3,
            DownSample4,
            DownSample5,
            NumberOfDownSamples,
        };

    private:

        struct SBloomRenderJob
        {
            Dt::CBloomComponent* m_pDataBloomFacet;
        };

        struct SGaussianShaderProperties
        {
            glm::ivec2 m_Direction;
            glm::ivec2 m_MaxPixelCoord;
            float      m_Weights[7];
        };

        struct SBloomShaderProperties
        {
            glm::vec4 m_BloomThresholdValue;
            glm::vec4 m_BloomTintIntensity;
        };

        struct SDownSampleShaderProperties
        {
            glm::vec4 m_InvertTexturesize;
        };

    private:

        typedef std::vector<SBloomRenderJob> CBloomRenderJobs;
        
    private: 

        CShaderPtr m_RectangleShaderVSPtr;
        CShaderPtr m_DownSampleShaderPSPtr;
        CShaderPtr m_GaussianBlurShaderPtr;
        CShaderPtr m_BloomShaderPSPtr;
        CShaderPtr m_PassThroughPSPtr;

        CBufferPtr m_GaussianBlurPropertiesBufferPtr;
        CBufferPtr m_DownSamplePropertiesBufferPtr;
        CBufferPtr m_BloomPropertiesBufferPtr;

        CTextureSetPtr m_BlurStagesTextureSetPtrs[s_NumberOfBlurStages * 2];
        CTextureSetPtr m_BlurStageFinalTextureSetPtrs[s_NumberOfBlurStages];

        CTextureSetPtr    m_SwapTexturePtrs[2];
        CRenderContextPtr m_SwapContextPtrs[2];
        CTargetSetPtr     m_SwapRenderTargetPtrs[2];

        CTextureSetPtr    m_DownSampleTextureSetPtrs[NumberOfDownSamples];
        CRenderContextPtr m_DownSampleRenderContextPtrs[NumberOfDownSamples];
        CTargetSetPtr     m_DownSampleTargetSetPtrs[NumberOfDownSamples];
        glm::ivec2        m_DownSampleSizes[NumberOfDownSamples];

        CBloomRenderJobs m_BloomRenderJobs;
        
        unsigned int m_SwapCounter;
        
    private:
        
        void RenderBloom();
        void PassThrough();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxPostFXHDRRenderer::CGfxPostFXHDRRenderer()
        : m_RectangleShaderVSPtr           ()
        , m_DownSampleShaderPSPtr          ()
        , m_GaussianBlurShaderPtr          ()
        , m_BloomShaderPSPtr               ()
        , m_PassThroughPSPtr               ()
        , m_GaussianBlurPropertiesBufferPtr()
        , m_DownSamplePropertiesBufferPtr  ()
        , m_BloomPropertiesBufferPtr       ()
        , m_BlurStagesTextureSetPtrs       ()
        , m_BlurStageFinalTextureSetPtrs   ()
        , m_SwapTexturePtrs                ()
        , m_SwapContextPtrs                ()
        , m_SwapRenderTargetPtrs           ()
        , m_DownSampleTextureSetPtrs       ()
        , m_DownSampleRenderContextPtrs    ()
        , m_DownSampleTargetSetPtrs        ()
        , m_DownSampleSizes                ()
        , m_BloomRenderJobs                ()
        , m_SwapCounter                    (0)
    {
        m_BloomRenderJobs.reserve(2);

        // -----------------------------------------------------------------------------
        // Register for resizing events
        // -----------------------------------------------------------------------------
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxPostFXHDRRenderer::OnResize));
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxPostFXHDRRenderer::~CGfxPostFXHDRRenderer()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Initiate target set sizes of down samples
        // -----------------------------------------------------------------------------
        glm::ivec2 Size = Main::GetActiveWindowSize();

        for (int IndexOfBlur = 0; IndexOfBlur < s_NumberOfBlurStages; ++ IndexOfBlur)
        {
            int Devisor = static_cast<int>(glm::pow(2, (IndexOfBlur + 1)));

            m_DownSampleSizes[IndexOfBlur] = glm::ivec2(Size[0] / Devisor, Size[1] / Devisor);
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnExit()
    {
        m_RectangleShaderVSPtr  = 0;
        m_DownSampleShaderPSPtr = 0;
        m_GaussianBlurShaderPtr = 0;
        m_BloomShaderPSPtr      = 0;
        m_PassThroughPSPtr      = 0;

        m_GaussianBlurPropertiesBufferPtr = 0;
        m_DownSamplePropertiesBufferPtr   = 0;
        m_BloomPropertiesBufferPtr        = 0;

        m_SwapTexturePtrs[0]      = 0;
        m_SwapTexturePtrs[1]      = 0;
        m_SwapContextPtrs[0]      = 0;
        m_SwapContextPtrs[1]      = 0;
        m_SwapRenderTargetPtrs[0] = 0;
        m_SwapRenderTargetPtrs[1] = 0;


        for (unsigned int IndexOfBlurStage = 0; IndexOfBlurStage < s_NumberOfBlurStages; ++IndexOfBlurStage)
        {
            m_BlurStageFinalTextureSetPtrs[IndexOfBlurStage] = 0;
        }

        for (unsigned int IndexOfBlurStage = 0; IndexOfBlurStage < s_NumberOfBlurStages * 2; ++ IndexOfBlurStage)
        {
            m_BlurStagesTextureSetPtrs[IndexOfBlurStage] = 0;
        }

        for (unsigned int IndexOfDownSample = 0; IndexOfDownSample < NumberOfDownSamples; ++IndexOfDownSample)
        {
            m_DownSampleTextureSetPtrs[IndexOfDownSample] = 0;
            m_DownSampleRenderContextPtrs[IndexOfDownSample] = 0;
            m_DownSampleTargetSetPtrs[IndexOfDownSample] = 0;
        }

        m_BloomRenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnSetupShader()
    {   
        m_RectangleShaderVSPtr  = ShaderManager::CompileVS("system/vs_fullscreen.glsl" , "main");

        m_DownSampleShaderPSPtr = ShaderManager::CompilePS("filter/fs_down_sample.glsl", "main");

        m_GaussianBlurShaderPtr = ShaderManager::CompileCS("filter/cs_gaussian_blur.glsl", "main", "#define TILE_SIZE 8\n#define IMAGE_TYPE rgba16f");

        m_BloomShaderPSPtr      = ShaderManager::CompilePS("bloom/fs_bloom.glsl", "main");

        m_PassThroughPSPtr      = ShaderManager::CompilePS("system/fs_pass_through.glsl", "main");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnSetupRenderTargets()
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        glm::ivec2 Size = Main::GetActiveWindowSize();
        
        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::RenderTarget | CTexture::ShaderResource;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::Unknown;
        RendertargetDescriptor.m_Usage            = CTexture::GPURead;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        RendertargetDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;
        
        CTexturePtr ColorTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Swap Color

		TextureManager::SetTextureLabel(ColorTexturePtr, "PostFX (HDR) Swap Target");

        // -----------------------------------------------------------------------------
        // Create down sample target sets
        // -----------------------------------------------------------------------------
        for (unsigned int IndexofDownSample = 0; IndexofDownSample < NumberOfDownSamples; ++ IndexofDownSample)
        {
            RendertargetDescriptor.m_NumberOfPixelsU = m_DownSampleSizes[IndexofDownSample][0];
            RendertargetDescriptor.m_NumberOfPixelsV = m_DownSampleSizes[IndexofDownSample][1];

            CTexturePtr DownSampleRenderTargetTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Down Samples

			TextureManager::SetTextureLabel(DownSampleRenderTargetTexturePtr, "PostFX (HDR) Down Sample Target");

            m_DownSampleTargetSetPtrs[IndexofDownSample] = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(DownSampleRenderTargetTexturePtr));
        }
        
        // -----------------------------------------------------------------------------
        // Create swap buffer target set
        // -----------------------------------------------------------------------------
        m_SwapRenderTargetPtrs[0] = TargetSetManager::CreateTargetSet(TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0));
        m_SwapRenderTargetPtrs[1] = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(ColorTexturePtr));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnSetupStates()
    {
        // -----------------------------------------------------------------------------
        // Get screen resolutions
        // -----------------------------------------------------------------------------
        glm::ivec2 Size = Main::GetActiveWindowSize();
        
        // -----------------------------------------------------------------------------
        // Setup states
        // -----------------------------------------------------------------------------
        CCameraPtr          CameraPtr          = ViewManager     ::GetFullQuadCamera();
        CViewPortSetPtr     ViewPortSetPtr     = ViewManager     ::GetViewPortSet();
        CRenderStatePtr     RenderStatePtr     = StateManager    ::GetRenderState(CRenderState::NoDepth);
        CTargetSetPtr       TargetSetPtr       = TargetSetManager::GetSystemTargetSet();
        
        // -----------------------------------------------------------------------------
        
        CRenderContextPtr SwapOneContextPtr = ContextManager::CreateRenderContext();
        
        SwapOneContextPtr->SetCamera(CameraPtr);
        SwapOneContextPtr->SetViewPortSet(ViewPortSetPtr);
        SwapOneContextPtr->SetTargetSet(m_SwapRenderTargetPtrs[0]);
        SwapOneContextPtr->SetRenderState(RenderStatePtr);
        
        m_SwapContextPtrs[0] = SwapOneContextPtr;
        
        // -----------------------------------------------------------------------------
        
        CRenderContextPtr SwapTwoContextPtr = ContextManager::CreateRenderContext();
        
        SwapTwoContextPtr->SetCamera(CameraPtr);
        SwapTwoContextPtr->SetViewPortSet(ViewPortSetPtr);
        SwapTwoContextPtr->SetTargetSet(m_SwapRenderTargetPtrs[1]);
        SwapTwoContextPtr->SetRenderState(RenderStatePtr);
        
        m_SwapContextPtrs[1] = SwapTwoContextPtr;

        // -----------------------------------------------------------------------------

        for (unsigned int IndexOfDownSample = 0; IndexOfDownSample < NumberOfDownSamples; ++IndexOfDownSample)
        {
            SViewPortDescriptor ViewPortDesc;

            ViewPortDesc.m_TopLeftX = 0.0f;
            ViewPortDesc.m_TopLeftY = 0.0f;
            ViewPortDesc.m_Width    = static_cast<float>(m_DownSampleSizes[IndexOfDownSample][0]);
            ViewPortDesc.m_Height   = static_cast<float>(m_DownSampleSizes[IndexOfDownSample][1]);
            ViewPortDesc.m_MinDepth = 0.0f;
            ViewPortDesc.m_MaxDepth = 1.0f;
        
            CViewPortPtr DownSampleViewPortPtr = ViewManager::CreateViewPort(ViewPortDesc);

            CViewPortSetPtr DownSampleViewPortSetPtr = ViewManager::CreateViewPortSet(DownSampleViewPortPtr);

            // -----------------------------------------------------------------------------

            CRenderContextPtr DownSampleRenderContextPtr = ContextManager::CreateRenderContext();

            DownSampleRenderContextPtr->SetCamera(CameraPtr);
            DownSampleRenderContextPtr->SetViewPortSet(DownSampleViewPortSetPtr);
            DownSampleRenderContextPtr->SetTargetSet(m_DownSampleTargetSetPtrs[IndexOfDownSample]);
            DownSampleRenderContextPtr->SetRenderState(RenderStatePtr);

            m_DownSampleRenderContextPtrs[IndexOfDownSample] = DownSampleRenderContextPtr;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnSetupTextures()
    {
        CTexturePtr ColorOneTexturePtr          = m_SwapRenderTargetPtrs[0]->GetRenderTarget(0);
        CTexturePtr ColorTwoTexturePtr          = m_SwapRenderTargetPtrs[1]->GetRenderTarget(0);
        CTexturePtr LightAccumulationTexturePtr = TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0);
        CTexturePtr DepthTexturePtr             = TargetSetManager::GetDeferredTargetSet()         ->GetDepthStencilTarget();

        // -----------------------------------------------------------------------------

        STextureDescriptor TextureDescriptor;
        
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;
        TextureDescriptor.m_Usage            = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;

        assert(s_NumberOfBlurStages == NumberOfDownSamples);

        for (unsigned int IndexOfBlurStage = 0; IndexOfBlurStage < s_NumberOfBlurStages; ++IndexOfBlurStage)
        {
            TextureDescriptor.m_NumberOfPixelsU = m_DownSampleSizes[IndexOfBlurStage][0];
            TextureDescriptor.m_NumberOfPixelsV = m_DownSampleSizes[IndexOfBlurStage][1];

            CTexturePtr   TempTexturePtr       = TextureManager::CreateTexture2D(TextureDescriptor);
            CTexturePtr   ResultTexturePtr     = TextureManager::CreateTexture2D(TextureDescriptor);
            CTexturePtr DownSampleTexturePtr = m_DownSampleTargetSetPtrs[IndexOfBlurStage]->GetRenderTarget(0);

            m_BlurStagesTextureSetPtrs[IndexOfBlurStage * 2 + 0] = TextureManager::CreateTextureSet(DownSampleTexturePtr						, static_cast<CTexturePtr>(TempTexturePtr));
            m_BlurStagesTextureSetPtrs[IndexOfBlurStage * 2 + 1] = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(TempTexturePtr), static_cast<CTexturePtr>(ResultTexturePtr));

			TextureManager::SetTextureLabel(ResultTexturePtr, "Blur Stage Result");
			TextureManager::SetTextureLabel(TempTexturePtr  , "Blur Temp Stage Result");
        }

        // -----------------------------------------------------------------------------

        m_DownSampleTextureSetPtrs[DownSample1] = TextureManager::CreateTextureSet(LightAccumulationTexturePtr);
        m_DownSampleTextureSetPtrs[DownSample2] = TextureManager::CreateTextureSet(m_DownSampleTargetSetPtrs[0]->GetRenderTarget(0));
        m_DownSampleTextureSetPtrs[DownSample3] = TextureManager::CreateTextureSet(m_DownSampleTargetSetPtrs[1]->GetRenderTarget(0));
        m_DownSampleTextureSetPtrs[DownSample4] = TextureManager::CreateTextureSet(m_DownSampleTargetSetPtrs[2]->GetRenderTarget(0));
        m_DownSampleTextureSetPtrs[DownSample5] = TextureManager::CreateTextureSet(m_DownSampleTargetSetPtrs[3]->GetRenderTarget(0));

        // -----------------------------------------------------------------------------
        
        for (unsigned int IndexOfBlurStage = 0; IndexOfBlurStage < s_NumberOfBlurStages; ++IndexOfBlurStage)
        {
            unsigned int BlurStageIndex = IndexOfBlurStage * 2 + 1;

            m_BlurStageFinalTextureSetPtrs[IndexOfBlurStage] = TextureManager::CreateTextureSet(m_BlurStagesTextureSetPtrs[BlurStageIndex]->GetTexture(1));
        }

        // -----------------------------------------------------------------------------

        m_SwapTexturePtrs[0] = TextureManager::CreateTextureSet(ColorOneTexturePtr, DepthTexturePtr);
        m_SwapTexturePtrs[1] = TextureManager::CreateTextureSet(ColorTwoTexturePtr, DepthTexturePtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnSetupBuffers()
    {        
        // -----------------------------------------------------------------------------
        // Setup view buffer for post rendering
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SGaussianShaderProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_GaussianBlurPropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SBloomShaderProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_BloomPropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SDownSampleShaderProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_DownSamplePropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnSetupModels()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnSetupEnd()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::OnUnloadMap()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CGfxPostFXHDRRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {
        m_SwapRenderTargetPtrs[0] = 0;
        m_SwapRenderTargetPtrs[1] = 0;

        for (unsigned int IndexofDownSample = 0; IndexofDownSample < NumberOfDownSamples; ++IndexofDownSample)
        {
            m_DownSampleTargetSetPtrs[IndexofDownSample] = 0;
        }

        // -----------------------------------------------------------------------------
        // Initiate target set sizes of down samples
        // -----------------------------------------------------------------------------
        glm::ivec2 Size(_Width, _Height);

        for (int IndexOfBlur = 0; IndexOfBlur < s_NumberOfBlurStages; ++IndexOfBlur)
        {
            int Devisor = static_cast<int>(glm::pow(2, (IndexOfBlur + 1)));

            m_DownSampleSizes[IndexOfBlur] = glm::ivec2(Size[0] / Devisor, Size[1] / Devisor);
        }

        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::RenderTarget | CTexture::ShaderResource;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::Unknown;
        RendertargetDescriptor.m_Usage            = CTexture::GPURead;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        RendertargetDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;
        
        CTexturePtr ColorTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Swap Color

		TextureManager::SetTextureLabel(ColorTexturePtr, "PostFX (HDR) Swap Target");

        // -----------------------------------------------------------------------------
        // Create down sample target sets
        // -----------------------------------------------------------------------------
        for (unsigned int IndexofDownSample = 0; IndexofDownSample < NumberOfDownSamples; ++ IndexofDownSample)
        {
            RendertargetDescriptor.m_NumberOfPixelsU = m_DownSampleSizes[IndexofDownSample][0];
            RendertargetDescriptor.m_NumberOfPixelsV = m_DownSampleSizes[IndexofDownSample][1];

            CTexturePtr DownSampleRenderTargetTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Down Samples

			TextureManager::SetTextureLabel(DownSampleRenderTargetTexturePtr, "PostFX (HDR) Down Sample Target");

            m_DownSampleTargetSetPtrs[IndexofDownSample] = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(DownSampleRenderTargetTexturePtr));
        }
        
        // -----------------------------------------------------------------------------
        // Create swap buffer target set
        // -----------------------------------------------------------------------------
        m_SwapRenderTargetPtrs[0] = TargetSetManager::CreateTargetSet(TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0));
        m_SwapRenderTargetPtrs[1] = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(ColorTexturePtr));

        m_SwapContextPtrs[0]->SetTargetSet(m_SwapRenderTargetPtrs[0]);
        m_SwapContextPtrs[1]->SetTargetSet(m_SwapRenderTargetPtrs[1]);

        // -----------------------------------------------------------------------------

        for (unsigned int IndexOfDownSample = 0; IndexOfDownSample < NumberOfDownSamples; ++IndexOfDownSample)
        {
            SViewPortDescriptor ViewPortDesc;

            ViewPortDesc.m_TopLeftX = 0.0f;
            ViewPortDesc.m_TopLeftY = 0.0f;
            ViewPortDesc.m_Width    = static_cast<float>(m_DownSampleSizes[IndexOfDownSample][0]);
            ViewPortDesc.m_Height   = static_cast<float>(m_DownSampleSizes[IndexOfDownSample][1]);
            ViewPortDesc.m_MinDepth = 0.0f;
            ViewPortDesc.m_MaxDepth = 1.0f;
        
            CViewPortPtr DownSampleViewPortPtr = ViewManager::CreateViewPort(ViewPortDesc);

            CViewPortSetPtr DownSampleViewPortSetPtr = ViewManager::CreateViewPortSet(DownSampleViewPortPtr);

            // -----------------------------------------------------------------------------

            m_DownSampleRenderContextPtrs[IndexOfDownSample]->SetViewPortSet(DownSampleViewPortSetPtr);
            m_DownSampleRenderContextPtrs[IndexOfDownSample]->SetTargetSet(m_DownSampleTargetSetPtrs[IndexOfDownSample]);
        }

        // -----------------------------------------------------------------------------

        CTexturePtr ColorOneTexturePtr          = m_SwapRenderTargetPtrs[0]->GetRenderTarget(0);
        CTexturePtr ColorTwoTexturePtr          = m_SwapRenderTargetPtrs[1]->GetRenderTarget(0);
        CTexturePtr LightAccumulationTexturePtr = TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0);
        CTexturePtr DepthTexturePtr             = TargetSetManager::GetDeferredTargetSet()         ->GetDepthStencilTarget();

        // -----------------------------------------------------------------------------

        STextureDescriptor TextureDescriptor;
        
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;
        TextureDescriptor.m_Usage            = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;

        assert(s_NumberOfBlurStages == NumberOfDownSamples);

        for (unsigned int IndexOfBlurStage = 0; IndexOfBlurStage < s_NumberOfBlurStages; ++IndexOfBlurStage)
        {
            TextureDescriptor.m_NumberOfPixelsU = m_DownSampleSizes[IndexOfBlurStage][0];
            TextureDescriptor.m_NumberOfPixelsV = m_DownSampleSizes[IndexOfBlurStage][1];

            CTexturePtr   TempTexturePtr       = TextureManager::CreateTexture2D(TextureDescriptor);
            CTexturePtr   ResultTexturePtr     = TextureManager::CreateTexture2D(TextureDescriptor);
            CTexturePtr DownSampleTexturePtr = m_DownSampleTargetSetPtrs[IndexOfBlurStage]->GetRenderTarget(0);

            m_BlurStagesTextureSetPtrs[IndexOfBlurStage * 2 + 0] = TextureManager::CreateTextureSet(DownSampleTexturePtr						, static_cast<CTexturePtr>(TempTexturePtr));
            m_BlurStagesTextureSetPtrs[IndexOfBlurStage * 2 + 1] = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(TempTexturePtr), static_cast<CTexturePtr>(ResultTexturePtr));

            TextureManager::SetTextureLabel(ResultTexturePtr, "Blur Stage Result");
            TextureManager::SetTextureLabel(TempTexturePtr  , "Blur Temp Stage Result");
        }

        // -----------------------------------------------------------------------------

        m_DownSampleTextureSetPtrs[DownSample1] = TextureManager::CreateTextureSet(LightAccumulationTexturePtr);
        m_DownSampleTextureSetPtrs[DownSample2] = TextureManager::CreateTextureSet(m_DownSampleTargetSetPtrs[0]->GetRenderTarget(0));
        m_DownSampleTextureSetPtrs[DownSample3] = TextureManager::CreateTextureSet(m_DownSampleTargetSetPtrs[1]->GetRenderTarget(0));
        m_DownSampleTextureSetPtrs[DownSample4] = TextureManager::CreateTextureSet(m_DownSampleTargetSetPtrs[2]->GetRenderTarget(0));
        m_DownSampleTextureSetPtrs[DownSample5] = TextureManager::CreateTextureSet(m_DownSampleTargetSetPtrs[3]->GetRenderTarget(0));

        // -----------------------------------------------------------------------------
        
        for (unsigned int IndexOfBlurStage = 0; IndexOfBlurStage < s_NumberOfBlurStages; ++IndexOfBlurStage)
        {
            unsigned int BlurStageIndex = IndexOfBlurStage * 2 + 1;

            m_BlurStageFinalTextureSetPtrs[IndexOfBlurStage] = TextureManager::CreateTextureSet(m_BlurStagesTextureSetPtrs[BlurStageIndex]->GetTexture(1));
        }

        // -----------------------------------------------------------------------------

        m_SwapTexturePtrs[0] = TextureManager::CreateTextureSet(ColorOneTexturePtr, DepthTexturePtr);
        m_SwapTexturePtrs[1] = TextureManager::CreateTextureSet(ColorTwoTexturePtr, DepthTexturePtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXHDRRenderer::Render()
    {
        Performance::BeginEvent("HDR Post Process");

        m_SwapCounter = 0;

        RenderBloom();

        PassThrough();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxPostFXHDRRenderer::RenderBloom()
    {
        if (m_BloomRenderJobs.size() == 0) return;

        Performance::BeginEvent("Bloom");

        // TODO: What happens if more then one DOF effect is available?
        Dt::CBloomComponent* pDataBloomFacet = m_BloomRenderJobs[0].m_pDataBloomFacet;

        assert(pDataBloomFacet != 0);

        // -----------------------------------------------------------------------------
        // Down Sample
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfDownSample = 0; IndexOfDownSample < pDataBloomFacet->GetSize(); ++IndexOfDownSample)
        {
            SDownSampleShaderProperties DownSampleSettings;

            DownSampleSettings.m_InvertTexturesize[0] = 1.0f / static_cast<float>(m_DownSampleSizes[IndexOfDownSample][0] * 2);
            DownSampleSettings.m_InvertTexturesize[1] = 1.0f / static_cast<float>(m_DownSampleSizes[IndexOfDownSample][1] * 2);

            BufferManager::UploadBufferData(m_DownSamplePropertiesBufferPtr, &DownSampleSettings);

            

            ContextManager::SetRenderContext(m_DownSampleRenderContextPtrs[IndexOfDownSample]);

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

            ContextManager::SetShaderPS(m_DownSampleShaderPSPtr);

            ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

            ContextManager::SetConstantBuffer(1, m_DownSamplePropertiesBufferPtr);

            ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(0, m_DownSampleTextureSetPtrs[IndexOfDownSample]->GetTexture(0));

            ContextManager::Draw(3, 0);

            ContextManager::ResetTexture(0);

            ContextManager::ResetSampler(0);

            ContextManager::ResetConstantBuffer(0);

            ContextManager::ResetConstantBuffer(1);

            ContextManager::ResetTopology();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetRenderContext();
        }

        // -----------------------------------------------------------------------------
        // Blur down samples
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfBlurStage = 0; IndexOfBlurStage < pDataBloomFacet->GetSize(); ++IndexOfBlurStage)
        {
            unsigned int NumberOfThreadGroupsX;
            unsigned int NumberOfThreadGroupsY;

            NumberOfThreadGroupsX = (m_DownSampleSizes[IndexOfBlurStage][0] + s_BlurTileSize - 1) / (s_BlurTileSize);
            NumberOfThreadGroupsY = (m_DownSampleSizes[IndexOfBlurStage][1] + s_BlurTileSize - 1) / (s_BlurTileSize);

            SGaussianShaderProperties GaussianSettings;

            GaussianSettings.m_Direction[0] = 1;
            GaussianSettings.m_Direction[1] = 0;
            GaussianSettings.m_MaxPixelCoord[0] = m_DownSampleSizes[IndexOfBlurStage][0];
            GaussianSettings.m_MaxPixelCoord[1] = m_DownSampleSizes[IndexOfBlurStage][1];
            GaussianSettings.m_Weights[0] = 0.018816f;
            GaussianSettings.m_Weights[1] = 0.034474f;
            GaussianSettings.m_Weights[2] = 0.056577f;
            GaussianSettings.m_Weights[3] = 0.083173f;
            GaussianSettings.m_Weights[4] = 0.109523f;
            GaussianSettings.m_Weights[5] = 0.129188f;
            GaussianSettings.m_Weights[6] = 0.136498f;

            // -----------------------------------------------------------------------------
            // Blur
            // -----------------------------------------------------------------------------
            GaussianSettings.m_Direction[0] = 1;
            GaussianSettings.m_Direction[1] = 0;

            BufferManager::UploadBufferData(m_GaussianBlurPropertiesBufferPtr, &GaussianSettings);

            ContextManager::SetShaderCS(m_GaussianBlurShaderPtr);

            ContextManager::SetResourceBuffer(0, m_GaussianBlurPropertiesBufferPtr);

            ContextManager::SetImageTexture(0, m_BlurStagesTextureSetPtrs[IndexOfBlurStage * 2 + 0]->GetTexture(0));
            ContextManager::SetImageTexture(1, m_BlurStagesTextureSetPtrs[IndexOfBlurStage * 2 + 0]->GetTexture(1));

            ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

            ContextManager::ResetImageTexture(0);
            ContextManager::ResetImageTexture(1);

            ContextManager::ResetResourceBuffer(0);

            ContextManager::ResetShaderCS();

            // -----------------------------------------------------------------------------
            // Blur
            // -----------------------------------------------------------------------------
            GaussianSettings.m_Direction[0] = 0;
            GaussianSettings.m_Direction[1] = 1;

            BufferManager::UploadBufferData(m_GaussianBlurPropertiesBufferPtr, &GaussianSettings);

            ContextManager::SetShaderCS(m_GaussianBlurShaderPtr);

            ContextManager::SetResourceBuffer(0, m_GaussianBlurPropertiesBufferPtr);

            ContextManager::SetImageTexture(0, m_BlurStagesTextureSetPtrs[IndexOfBlurStage * 2 + 1]->GetTexture(0));
            ContextManager::SetImageTexture(1, m_BlurStagesTextureSetPtrs[IndexOfBlurStage * 2 + 1]->GetTexture(1));

            ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

            ContextManager::ResetImageTexture(0);
            ContextManager::ResetImageTexture(1);

            ContextManager::ResetResourceBuffer(0);

            ContextManager::ResetShaderCS();
        }

        // -----------------------------------------------------------------------------
        // Set current swap buffer count
        // -----------------------------------------------------------------------------
        int CurrentSwapBufferCount = m_SwapCounter        % 2;
        int NextSwapBufferCount    = (m_SwapCounter += 1) % 2;

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        SBloomShaderProperties BloomShaderProperties;

        BloomShaderProperties.m_BloomThresholdValue = glm::vec4(static_cast<float>(pDataBloomFacet->GetTreshhold()), 0, 0, pDataBloomFacet->GetExposureScale());
        BloomShaderProperties.m_BloomTintIntensity  = pDataBloomFacet->GetTint() * pDataBloomFacet->GetIntensity();

        BufferManager::UploadBufferData(m_BloomPropertiesBufferPtr, &BloomShaderProperties);

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_SwapContextPtrs[NextSwapBufferCount]);
                
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);
        
        ContextManager::SetShaderPS(m_BloomShaderPSPtr);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        
        ContextManager::SetConstantBuffer(1, m_BloomPropertiesBufferPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        
        ContextManager::SetTexture(0, m_SwapTexturePtrs[CurrentSwapBufferCount]->GetTexture(0));
        ContextManager::SetTexture(1, m_SwapTexturePtrs[CurrentSwapBufferCount]->GetTexture(1));
        ContextManager::SetTexture(2, m_BlurStageFinalTextureSetPtrs[pDataBloomFacet->GetSize() - 1]->GetTexture(0));
        
        ContextManager::Draw(3, 0);
        
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);
        ContextManager::ResetTexture(2);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        ContextManager::ResetSampler(2);
        
        ContextManager::ResetConstantBuffer(0);
        
        ContextManager::ResetConstantBuffer(1);
        
        ContextManager::ResetTopology();
                
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxPostFXHDRRenderer::PassThrough()
    {
        // -----------------------------------------------------------------------------
        // Set current swap buffer count
        // -----------------------------------------------------------------------------
        int CurrentSwapBufferCount = m_SwapCounter        % 2;
        int NextSwapBufferCount    = (m_SwapCounter += 1) % 2;

        if (CurrentSwapBufferCount == 0)
        {
            return;
        }

        Performance::BeginEvent("Pass Through");
        
        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_SwapContextPtrs[NextSwapBufferCount]);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);
        
        ContextManager::SetShaderPS(m_PassThroughPSPtr);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        ContextManager::SetTexture(0, m_SwapTexturePtrs[CurrentSwapBufferCount]->GetTexture(0));
        
        ContextManager::Draw(3, 0);
        
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);
        
        ContextManager::ResetConstantBuffer(0);
        
        ContextManager::ResetTopology();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxPostFXHDRRenderer::BuildRenderJobs()
    {
        m_BloomRenderJobs.clear();

        auto DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CBloomComponent>();

        for (auto Component : DataComponents)
        {
            Dt::CBloomComponent* pDtComponent = static_cast<Dt::CBloomComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            SBloomRenderJob NewRenderJob;

            NewRenderJob.m_pDataBloomFacet = pDtComponent;

            m_BloomRenderJobs.push_back(NewRenderJob);
        }
    }
} // namespace

namespace Gfx
{
namespace PostFXHDR
{
    void OnStart()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxPostFXHDRRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxPostFXHDRRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxPostFXHDRRenderer::GetInstance().Render();
    }
} // namespace PostFXHDR
} // namespace Gfx

