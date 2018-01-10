
#include "graphic/gfx_precompiled.h"

#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_dof_facet.h"
#include "data/data_entity.h"
#include "data/data_fx_type.h"
#include "data/data_post_aa_facet.h"
#include "data/data_map.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_postfx_renderer.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_smaa_look_up_textures.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include <sstream>

using namespace Gfx;

namespace
{
    class CGfxPostFXRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxPostFXRenderer)
        
    public:
        CGfxPostFXRenderer();
        ~CGfxPostFXRenderer();
        
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
        
        enum EPostEffectShader
        {
            DOFDownSample,
            DOFNear,
            DOFBlurNear,
            DOFApply,
            GaussianBlur,
            FXAA,
            SMAAEdgeDetect,
            SMAAWeightsCalc,
            SMAABlending,
            NumberOfPostEffects,
            UndefinedPostEffect = -1
        };

    private:

        struct SPostAARenderJob
        {
            Dt::CPostAAFXFacet* m_pDataPostAAFacet;
        };

        struct SDOFRenderJob
        {
            Dt::CDOFFXFacet* m_pDataDOFFacet;
        };
        
        struct SDOFDownProperties
        {
            Base::Float2 m_DofNear;             //< Distance up to everything should be blured (y = 1; x = -y / Far_Distance)
            Base::Float2 m_DofRowDelta;         //< (x = 1; y = 0.25 / ScreenHeight)
        };
        
        struct SDOFApplyProperties
        {
            Base::Float4 m_DofLerpScale;
            Base::Float4 m_DofLerpBias;
            Base::Float3 m_DofEqFar;            //< Distance since everything should be blured (x = 1 / 1 - Start_Distance; y = 1 - x)
        };
        
        struct SGaussianSettings
        {
            Base::Float2 m_Direction;
            float        m_Weights[7];
        };

    private:

        typedef std::vector<SPostAARenderJob> CPostAARenderJobs;
        typedef std::vector<SDOFRenderJob> CDOFRenderJobs;
        
    private:
        
        CMeshPtr          m_QuadModelPtr;

        CViewPortSetPtr   m_SystemViewPortSetPtr;
        
        CBufferSetPtr     m_DOFVSBufferSetPtr;
        CBufferSetPtr     m_DOFDownPropertiesPSBufferPtr;
        CBufferSetPtr     m_DOFApplyPropertiesPSBufferPtr;
        CBufferSetPtr     m_GaussianBlurPropertiesPSBufferPtr;
        
        CInputLayoutPtr   m_FullQuadInputLayoutPtr;
        CShaderPtr        m_RectangleShaderVSPtr;
        CShaderPtr        m_PassThroughShaderPSPtr;
        
        CShaderPtr        m_PostEffectShaderVSPtrs[NumberOfPostEffects];
        CShaderPtr        m_PostEffectShaderPSPtrs[NumberOfPostEffects];
        CTextureSetPtr    m_SwapTextureSetPtrs[2];
        CRenderContextPtr m_SwapRenderContextPtrs[2];
        CTargetSetPtr     m_SwapTargetSetPtrs[2];

        CTextureSetPtr    m_FullTextureSetPtrs[1];
        CRenderContextPtr m_FullRenderContextPtrs[1];
        CTargetSetPtr     m_FullTargetSetPtrs[1];

        CTextureSetPtr    m_HalfTexturePtrs[1];
        CRenderContextPtr m_HalfRenderContextPtrs[1];
        CTargetSetPtr     m_HalfTargetSetPtrs[1];
        
        CTextureSetPtr    m_QuarterTextureSetPtrs[3];
        CRenderContextPtr m_QuarterRenderContextPtrs[3];
        CTargetSetPtr     m_QuarterTargetSetPtrs[3];

        CTextureSetPtr    m_SMAATextureSetPtr[2];
        CRenderContextPtr m_SMAAEdgeDetectContextPtr;
        CRenderContextPtr m_SMAAWeightCalcContextPtr;
        CTargetSetPtr     m_SMAAEdgeTargetSetPtr;
        CTargetSetPtr     m_SMAAWeightsCalcTargetSetPtr;
        CTexturePtr       m_SMAAAreaTexture;
        CTexturePtr       m_SMAASearchTexture;

        CPostAARenderJobs m_PostAARenderJobs;
        CDOFRenderJobs  m_DOFRenderJobs;
        
        unsigned int m_SwapCounter;
        
    private:
        
        void RenderDOF();
        void RenderPostAA();
        void RenderFXAA();
        void RenderSMAA();
        void RenderToSystem();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxPostFXRenderer::CGfxPostFXRenderer()
        : m_QuadModelPtr                     ()
        , m_SystemViewPortSetPtr             ()
        , m_FullQuadInputLayoutPtr           ()
        , m_RectangleShaderVSPtr             ()
        , m_PassThroughShaderPSPtr           ()
        , m_DOFVSBufferSetPtr                ()
        , m_DOFDownPropertiesPSBufferPtr     ()
        , m_DOFApplyPropertiesPSBufferPtr    ()
        , m_GaussianBlurPropertiesPSBufferPtr()
        , m_SwapTargetSetPtrs                ()
        , m_SMAAAreaTexture                  ()
        , m_SMAASearchTexture                ()
        , m_PostAARenderJobs                 ()
        , m_DOFRenderJobs                    ()
        , m_SwapCounter                      (0)
    {
        m_PostAARenderJobs.reserve(2);
        m_DOFRenderJobs   .reserve(2);

        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxPostFXRenderer::OnResize));
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxPostFXRenderer::~CGfxPostFXRenderer()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnExit()
    {
        m_QuadModelPtr                      = 0;
        m_SystemViewPortSetPtr              = 0;
        m_FullQuadInputLayoutPtr            = 0;
        m_RectangleShaderVSPtr              = 0;
        m_PassThroughShaderPSPtr            = 0;
        m_DOFVSBufferSetPtr                 = 0;
        m_DOFDownPropertiesPSBufferPtr      = 0;
        m_DOFApplyPropertiesPSBufferPtr     = 0;
        m_GaussianBlurPropertiesPSBufferPtr = 0;
        m_SwapTextureSetPtrs[0]             = 0;
        m_SwapTextureSetPtrs[1]             = 0;
        m_SwapRenderContextPtrs[0]          = 0;
        m_SwapRenderContextPtrs[1]          = 0;
        m_SwapTargetSetPtrs[0]              = 0;
        m_SwapTargetSetPtrs[1]              = 0;
        m_FullTextureSetPtrs[0]             = 0;
        m_FullRenderContextPtrs[0]          = 0;
        m_FullTargetSetPtrs[0]              = 0;
        m_HalfTexturePtrs[0]                = 0;
        m_HalfRenderContextPtrs[0]          = 0;
        m_HalfTargetSetPtrs[0]              = 0;
        m_QuarterTextureSetPtrs[0]          = 0;
        m_QuarterTextureSetPtrs[1]          = 0;
        m_QuarterTextureSetPtrs[2]          = 0;
        m_QuarterRenderContextPtrs[0]       = 0;
        m_QuarterRenderContextPtrs[1]       = 0;
        m_QuarterRenderContextPtrs[2]       = 0;
        m_QuarterTargetSetPtrs[0]           = 0;
        m_QuarterTargetSetPtrs[1]           = 0;
        m_QuarterTargetSetPtrs[2]           = 0;
        m_SMAATextureSetPtr[0]              = 0;
        m_SMAATextureSetPtr[1]              = 0;
        m_SMAAEdgeDetectContextPtr          = 0;
        m_SMAAWeightCalcContextPtr          = 0;
        m_SMAAEdgeTargetSetPtr              = 0;
        m_SMAAWeightsCalcTargetSetPtr       = 0;
        m_SMAAAreaTexture                   = 0;
        m_SMAASearchTexture                 = 0;
        
        for (unsigned int IndexOfPostEffectShader = 0; IndexOfPostEffectShader < NumberOfPostEffects; ++ IndexOfPostEffectShader)
        {
            m_PostEffectShaderVSPtrs[IndexOfPostEffectShader] = 0;
            m_PostEffectShaderPSPtrs[IndexOfPostEffectShader] = 0;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnSetupShader()
    {
        CShaderPtr ShaderVSPtr                = ShaderManager::CompileVS("vs_screen_p_quad.glsl"    , "main");
        CShaderPtr ShaderDOFDownVSPtr         = ShaderManager::CompileVS("vs_dof_down_sample.glsl"  , "main");
        CShaderPtr ShaderDOFNearBlurVSPtr     = ShaderManager::CompileVS("vs_dof_near_blur.glsl"    , "main");
        CShaderPtr PassThroughPSPtr           = ShaderManager::CompilePS("fs_pass_through.glsl"     , "main");
        CShaderPtr ShaderDOFDownPSPtr         = ShaderManager::CompilePS("fs_dof_down_sample.glsl"  , "main");
        CShaderPtr ShaderDOFNearBlurPSPtr     = ShaderManager::CompilePS("fs_dof_near_blur.glsl"    , "main");
        CShaderPtr ShaderDOFNearPSPtr         = ShaderManager::CompilePS("fs_dof_near.glsl"         , "main");
        CShaderPtr ShaderDOFApplyPSPtr        = ShaderManager::CompilePS("fs_dof_apply.glsl"        , "main");
        CShaderPtr ShaderGaussianBlurPSPtr    = ShaderManager::CompilePS("fs_gaussian_blur.glsl"    , "main");
        CShaderPtr ShaderFXAAPSPtr            = ShaderManager::CompilePS("fs_fxaa.glsl"             , "main");

        Base::Int2 WindowSize = Gfx::Main::GetActiveWindowSize();

        std::stringstream SMAADefineStream;

        SMAADefineStream << "#define SMAA_RT_METRICS " << "vec4(1.0f / "
            << WindowSize[0] << ".0f, 1.0f / " << WindowSize[1] << ".0f, "
            << WindowSize[0] << ".0f, " << WindowSize[1] << ".0f)";

        std::string SMAADefineString = SMAADefineStream.str();
        
        const char* pDefine = SMAADefineString.c_str();

        CShaderPtr ShaderSMAAEdgeDetectVSPtr  = ShaderManager::CompileVS("vs_smaa_edge_detect.glsl" , "main", pDefine);
        CShaderPtr ShaderSMAAEdgeDetectPSPtr  = ShaderManager::CompilePS("fs_smaa_edge_detect.glsl" , "main", pDefine);
        CShaderPtr ShaderSMAAWeightsCalcVSPtr = ShaderManager::CompileVS("vs_smaa_weights_calc.glsl", "main", pDefine);
        CShaderPtr ShaderSMAAWeightsCalcPSPtr = ShaderManager::CompilePS("fs_smaa_weights_calc.glsl", "main", pDefine);
        CShaderPtr ShaderSMAABlendingVSPtr    = ShaderManager::CompileVS("vs_smaa_blending.glsl"    , "main", pDefine);
        CShaderPtr ShaderSMAABlendingPSPtr    = ShaderManager::CompilePS("fs_smaa_blending.glsl"    , "main", pDefine);
        
        m_RectangleShaderVSPtr   = ShaderVSPtr;
        m_PassThroughShaderPSPtr = PassThroughPSPtr;
        
        m_PostEffectShaderVSPtrs[DOFDownSample] = ShaderDOFDownVSPtr;
        m_PostEffectShaderPSPtrs[DOFDownSample] = ShaderDOFDownPSPtr;
        
        m_PostEffectShaderVSPtrs[DOFNear] = ShaderVSPtr;
        m_PostEffectShaderPSPtrs[DOFNear] = ShaderDOFNearPSPtr;
        
        m_PostEffectShaderVSPtrs[DOFBlurNear] = ShaderDOFNearBlurVSPtr;
        m_PostEffectShaderPSPtrs[DOFBlurNear] = ShaderDOFNearBlurPSPtr;
        
        m_PostEffectShaderVSPtrs[DOFApply] = ShaderVSPtr;
        m_PostEffectShaderPSPtrs[DOFApply] = ShaderDOFApplyPSPtr;
        
        m_PostEffectShaderVSPtrs[GaussianBlur] = ShaderVSPtr;
        m_PostEffectShaderPSPtrs[GaussianBlur] = ShaderGaussianBlurPSPtr;
        
        m_PostEffectShaderVSPtrs[FXAA] = ShaderVSPtr;
        m_PostEffectShaderPSPtrs[FXAA] = ShaderFXAAPSPtr;

        m_PostEffectShaderVSPtrs[SMAAEdgeDetect] = ShaderSMAAEdgeDetectVSPtr;
        m_PostEffectShaderPSPtrs[SMAAEdgeDetect] = ShaderSMAAEdgeDetectPSPtr;

        m_PostEffectShaderVSPtrs[SMAAWeightsCalc] = ShaderSMAAWeightsCalcVSPtr;
        m_PostEffectShaderPSPtrs[SMAAWeightsCalc] = ShaderSMAAWeightsCalcPSPtr;

        m_PostEffectShaderVSPtrs[SMAABlending] = ShaderSMAABlendingVSPtr;
        m_PostEffectShaderPSPtrs[SMAABlending] = ShaderSMAABlendingPSPtr;
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor InputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        m_FullQuadInputLayoutPtr = ShaderManager::CreateInputLayout(InputLayout, 1, ShaderVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnSetupRenderTargets()
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        Base::Int2 Size = Main::GetActiveWindowSize();
        
        Base::Int2 HalfSize   (Size[0] / 2, Size[1] / 2);
        Base::Int2 QuarterSize(Size[0] / 4, Size[1] / 4);
        
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
        RendertargetDescriptor.m_Format           = CTexture::R8G8B8A8_UBYTE;
        
        CTexturePtr ColorTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Swap Color

        TextureManager::SetTextureLabel(ColorTexturePtr, "PostFX Swap");

        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_NumberOfPixelsU = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV = Size[1];

        CTexturePtr FullTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // First Full

        TextureManager::SetTextureLabel(FullTexturePtr, "PostFX Temp Full Resolution");

        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_NumberOfPixelsU = HalfSize[0];
        RendertargetDescriptor.m_NumberOfPixelsV = HalfSize[1];

        CTexturePtr HalfTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // First Half

        TextureManager::SetTextureLabel(HalfTexturePtr, "PostFX Temp Half Resolution");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_NumberOfPixelsU  = QuarterSize[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = QuarterSize[1];
        
        CTexturePtr QuarterOneTexturePtr   = TextureManager::CreateTexture2D(RendertargetDescriptor); // First Quarter
        CTexturePtr QuarterTwoTexturePtr   = TextureManager::CreateTexture2D(RendertargetDescriptor); // Second Quarter
        CTexturePtr QuarterThreeTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Third Quarter

        TextureManager::SetTextureLabel(QuarterOneTexturePtr, "PostFX Temp Quarter One");
        TextureManager::SetTextureLabel(QuarterTwoTexturePtr, "PostFX Temp Quarter Two");
        TextureManager::SetTextureLabel(QuarterThreeTexturePtr, "PostFX Temp Quarter Three");
        
        // -----------------------------------------------------------------------------
        // Create swap buffer target set
        // -----------------------------------------------------------------------------
        CTexturePtr SwapRenderbuffer[1];
        
        SwapRenderbuffer[0] = ColorTexturePtr;

        CTexturePtr FullRenderbuffer[1];

        FullRenderbuffer[0] = FullTexturePtr;

        CTexturePtr HalfRenderbuffer[1];

        HalfRenderbuffer[0] = HalfTexturePtr;
        
        CTexturePtr QuarterOneRenderbuffer[1];
        
        QuarterOneRenderbuffer[0] = QuarterOneTexturePtr;
        
        CTexturePtr QuarterTwoRenderbuffer[1];
        
        QuarterTwoRenderbuffer[0] = QuarterTwoTexturePtr;
        
        CTexturePtr QuarterThreeRenderbuffer[1];
        
        QuarterThreeRenderbuffer[0] = QuarterThreeTexturePtr;
        
        m_SwapTargetSetPtrs[0] = TargetSetManager::GetDefaultTargetSet();
        m_SwapTargetSetPtrs[1] = TargetSetManager::CreateTargetSet(SwapRenderbuffer, 1);

        m_FullTargetSetPtrs[0] = TargetSetManager::CreateTargetSet(FullRenderbuffer, 1);

        m_HalfTargetSetPtrs[0] = TargetSetManager::CreateTargetSet(HalfRenderbuffer, 1);
        
        m_QuarterTargetSetPtrs[0] = TargetSetManager::CreateTargetSet(QuarterOneRenderbuffer  , 1);
        m_QuarterTargetSetPtrs[1] = TargetSetManager::CreateTargetSet(QuarterTwoRenderbuffer  , 1);
        m_QuarterTargetSetPtrs[2] = TargetSetManager::CreateTargetSet(QuarterThreeRenderbuffer, 1);

        // -----------------------------------------------------------------------------
        // SMAA Render Targets
        // -----------------------------------------------------------------------------

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
        RendertargetDescriptor.m_Format           = CTexture::R8G8_UBYTE;

        CTexturePtr EdgesTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        TextureManager::SetTextureLabel(EdgesTexturePtr, "SMAA Target Texture");

        RendertargetDescriptor.m_Format = CTexture::R8G8B8A8_UBYTE;

        CTexturePtr BlendWeightsTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        TextureManager::SetTextureLabel(BlendWeightsTexturePtr, "SMAA Blend Weights Texture");

        m_SMAAEdgeTargetSetPtr        = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(EdgesTexturePtr));
        m_SMAAWeightsCalcTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(BlendWeightsTexturePtr));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnSetupStates()
    {
        // -----------------------------------------------------------------------------
        // Get screen resolutions
        // -----------------------------------------------------------------------------
        Base::Int2 Size       = Main::GetActiveWindowSize();
        Base::Int2 NativeSize = Main::GetActiveNativeWindowSize();
        
        Base::Int2 HalfSize   (Size[0] / 2, Size[1] / 2);
        Base::Int2 QuarterSize(Size[0] / 4, Size[1] / 4);
        
        // -----------------------------------------------------------------------------
        // Build view ports
        // -----------------------------------------------------------------------------
        SViewPortDescriptor ViewPortDesc;

        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_Width    = static_cast<float>(NativeSize[0]);
        ViewPortDesc.m_Height   = static_cast<float>(NativeSize[1]);
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;
        
        CViewPortPtr SystemViewPort = ViewManager::CreateViewPort(ViewPortDesc);

        m_SystemViewPortSetPtr = ViewManager::CreateViewPortSet(SystemViewPort);

        // -----------------------------------------------------------------------------

        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_Width    = static_cast<float>(HalfSize[0]);
        ViewPortDesc.m_Height   = static_cast<float>(HalfSize[1]);
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;
        
        CViewPortPtr HalfViewPort = ViewManager::CreateViewPort(ViewPortDesc);
        
        CViewPortSetPtr HalfViewPortSetPtr = ViewManager::CreateViewPortSet(HalfViewPort);

        // -----------------------------------------------------------------------------
        
        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_Width    = static_cast<float>(QuarterSize[0]);
        ViewPortDesc.m_Height   = static_cast<float>(QuarterSize[1]);
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;
        
        CViewPortPtr QuarterViewPort = ViewManager::CreateViewPort(ViewPortDesc);
        
        CViewPortSetPtr QuarterViewPortSetPtr = ViewManager::CreateViewPortSet(QuarterViewPort);
        
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
        SwapOneContextPtr->SetTargetSet(m_SwapTargetSetPtrs[0]);
        SwapOneContextPtr->SetRenderState(RenderStatePtr);
        
        m_SwapRenderContextPtrs[0] = SwapOneContextPtr;
        
        // -----------------------------------------------------------------------------
        
        CRenderContextPtr SwapTwoContextPtr = ContextManager::CreateRenderContext();
        
        SwapTwoContextPtr->SetCamera(CameraPtr);
        SwapTwoContextPtr->SetViewPortSet(ViewPortSetPtr);
        SwapTwoContextPtr->SetTargetSet(m_SwapTargetSetPtrs[1]);
        SwapTwoContextPtr->SetRenderState(RenderStatePtr);
        
        m_SwapRenderContextPtrs[1] = SwapTwoContextPtr;

        // -----------------------------------------------------------------------------

        CRenderContextPtr FullContextPtr = ContextManager::CreateRenderContext();

        FullContextPtr->SetCamera(CameraPtr);
        FullContextPtr->SetViewPortSet(ViewPortSetPtr);
        FullContextPtr->SetTargetSet(m_FullTargetSetPtrs[0]);
        FullContextPtr->SetRenderState(RenderStatePtr);

        m_FullRenderContextPtrs[0] = FullContextPtr;

        // -----------------------------------------------------------------------------

        CRenderContextPtr HalfContextPtr = ContextManager::CreateRenderContext();

        HalfContextPtr->SetCamera(CameraPtr);
        HalfContextPtr->SetViewPortSet(HalfViewPortSetPtr);
        HalfContextPtr->SetTargetSet(m_HalfTargetSetPtrs[0]);
        HalfContextPtr->SetRenderState(RenderStatePtr);

        m_HalfRenderContextPtrs[0] = HalfContextPtr;
        
        // -----------------------------------------------------------------------------

        CRenderContextPtr QuarterOneContextPtr = ContextManager::CreateRenderContext();
        
        QuarterOneContextPtr->SetCamera(CameraPtr);
        QuarterOneContextPtr->SetViewPortSet(QuarterViewPortSetPtr);
        QuarterOneContextPtr->SetTargetSet(m_QuarterTargetSetPtrs[0]);
        QuarterOneContextPtr->SetRenderState(RenderStatePtr);
        
        m_QuarterRenderContextPtrs[0] = QuarterOneContextPtr;
        
        CRenderContextPtr QuarterTwoContextPtr = ContextManager::CreateRenderContext();
        
        QuarterTwoContextPtr->SetCamera(CameraPtr);
        QuarterTwoContextPtr->SetViewPortSet(QuarterViewPortSetPtr);
        QuarterTwoContextPtr->SetTargetSet(m_QuarterTargetSetPtrs[1]);
        QuarterTwoContextPtr->SetRenderState(RenderStatePtr);
        
        m_QuarterRenderContextPtrs[1] = QuarterTwoContextPtr;
        
        CRenderContextPtr QuarterThreeContextPtr = ContextManager::CreateRenderContext();
        
        QuarterThreeContextPtr->SetCamera(CameraPtr);
        QuarterThreeContextPtr->SetViewPortSet(QuarterViewPortSetPtr);
        QuarterThreeContextPtr->SetTargetSet(m_QuarterTargetSetPtrs[2]);
        QuarterThreeContextPtr->SetRenderState(RenderStatePtr);
        
        m_QuarterRenderContextPtrs[2] = QuarterThreeContextPtr;

        // -----------------------------------------------------------------------------

        m_SMAAEdgeDetectContextPtr = ContextManager::CreateRenderContext();

        m_SMAAEdgeDetectContextPtr->SetCamera(CameraPtr);
        m_SMAAEdgeDetectContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_SMAAEdgeDetectContextPtr->SetTargetSet(m_SMAAEdgeTargetSetPtr);
        m_SMAAEdgeDetectContextPtr->SetRenderState(RenderStatePtr);

        m_SMAAWeightCalcContextPtr = ContextManager::CreateRenderContext();

        m_SMAAWeightCalcContextPtr->SetCamera(CameraPtr);
        m_SMAAWeightCalcContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_SMAAWeightCalcContextPtr->SetTargetSet(m_SMAAWeightsCalcTargetSetPtr);
        m_SMAAWeightCalcContextPtr->SetRenderState(RenderStatePtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnSetupTextures()
    {
        CTexturePtr ColorOneTexturePtr     = m_SwapTargetSetPtrs[0]->GetRenderTarget(0);
        CTexturePtr ColorTwoTexturePtr     = m_SwapTargetSetPtrs[1]->GetRenderTarget(0);
        CTexturePtr FullTexturePtr         = m_FullTargetSetPtrs[0]->GetRenderTarget(0);
        CTexturePtr HalfTexturePtr         = m_HalfTargetSetPtrs[0]->GetRenderTarget(0);
        CTexturePtr QuarterOneTexturePtr   = m_QuarterTargetSetPtrs[0]->GetRenderTarget(0);
        CTexturePtr QuarterTwoTexturePtr   = m_QuarterTargetSetPtrs[1]->GetRenderTarget(0);
        CTexturePtr QuarterThreeTexturePtr = m_QuarterTargetSetPtrs[2]->GetRenderTarget(0);
        CTexturePtr NormalTexturePtr       = TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1);
        CTexturePtr DepthTexturePtr        = TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget();
        
        m_SwapTextureSetPtrs[0] = TextureManager::CreateTextureSet(ColorOneTexturePtr, DepthTexturePtr);
        m_SwapTextureSetPtrs[1] = TextureManager::CreateTextureSet(ColorTwoTexturePtr, DepthTexturePtr);

        m_FullTextureSetPtrs[0] = TextureManager::CreateTextureSet(FullTexturePtr);

        m_HalfTexturePtrs[0] = TextureManager::CreateTextureSet(HalfTexturePtr);
        
        m_QuarterTextureSetPtrs[0] = TextureManager::CreateTextureSet(QuarterOneTexturePtr);
        m_QuarterTextureSetPtrs[1] = TextureManager::CreateTextureSet(QuarterTwoTexturePtr);
        m_QuarterTextureSetPtrs[2] = TextureManager::CreateTextureSet(QuarterThreeTexturePtr);

        // -----------------------------------------------------------------------------
        // Setting up SMAA look up textures
        // -----------------------------------------------------------------------------
        STextureDescriptor AreaTexDescriptor = {};

        AreaTexDescriptor.m_NumberOfPixelsU  = AREATEX_WIDTH;
        AreaTexDescriptor.m_NumberOfPixelsV  = AREATEX_HEIGHT;
        AreaTexDescriptor.m_NumberOfPixelsW  = 1;
        AreaTexDescriptor.m_NumberOfMipMaps  = 1;
        AreaTexDescriptor.m_NumberOfTextures = 1;
        AreaTexDescriptor.m_Binding          = CTexture::ShaderResource;
        AreaTexDescriptor.m_Access           = CTexture::CPUWrite;
        AreaTexDescriptor.m_Format           = CTexture::R8G8_UBYTE;
        AreaTexDescriptor.m_Usage            = CTexture::GPURead;
        AreaTexDescriptor.m_Semantic         = CTexture::UndefinedSemantic;
        AreaTexDescriptor.m_pFileName        = 0;
        AreaTexDescriptor.m_pPixels          = const_cast<void*>(static_cast<const void*>(&areaTexBytes[0]));

        m_SMAAAreaTexture = TextureManager::CreateTexture2D(AreaTexDescriptor);

        TextureManager::SetTextureLabel(m_SMAAAreaTexture, "SMAA Area Texture");

        STextureDescriptor SearchTexDescriptor = {};

        SearchTexDescriptor.m_NumberOfPixelsU  = SEARCHTEX_WIDTH;
        SearchTexDescriptor.m_NumberOfPixelsV  = SEARCHTEX_HEIGHT;
        SearchTexDescriptor.m_NumberOfPixelsW  = 1;
        SearchTexDescriptor.m_NumberOfMipMaps  = 1;
        SearchTexDescriptor.m_NumberOfTextures = 1;
        SearchTexDescriptor.m_Binding          = CTexture::ShaderResource;
        SearchTexDescriptor.m_Access           = CTexture::CPUWrite;
        SearchTexDescriptor.m_Format           = CTexture::R8_UBYTE;
        SearchTexDescriptor.m_Usage            = CTexture::GPURead;
        SearchTexDescriptor.m_Semantic         = CTexture::UndefinedSemantic;
        SearchTexDescriptor.m_pFileName        = 0;
        SearchTexDescriptor.m_pPixels          = const_cast<void*>(static_cast<const void*>(&searchTexBytes[0]));

        m_SMAASearchTexture = TextureManager::CreateTexture2D(SearchTexDescriptor);

        TextureManager::SetTextureLabel(m_SMAASearchTexture, "SMAA Search Texture");

        auto EdgesTexPtr = m_SMAAEdgeTargetSetPtr->GetRenderTarget(0);
        auto WeightsTexPtr = m_SMAAWeightsCalcTargetSetPtr->GetRenderTarget(0);

        CTexturePtr TexturePtrs[2][5] =
        {
            { ColorOneTexturePtr, EdgesTexPtr, WeightsTexPtr, static_cast<CTexturePtr>(m_SMAAAreaTexture), static_cast<CTexturePtr>(m_SMAASearchTexture) },
            { ColorTwoTexturePtr, EdgesTexPtr, WeightsTexPtr, static_cast<CTexturePtr>(m_SMAAAreaTexture), static_cast<CTexturePtr>(m_SMAASearchTexture) }
        };

        m_SMAATextureSetPtr[0] = TextureManager::CreateTextureSet(TexturePtrs[0], 5);
        m_SMAATextureSetPtr[1] = TextureManager::CreateTextureSet(TexturePtrs[1], 5);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnSetupBuffers()
    {                
        // -----------------------------------------------------------------------------
        // Setup view buffer for post rendering
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SDOFDownProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr DOFDownPropertiesBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SDOFApplyProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr DOFApplyPropertiesBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SGaussianSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr GaussianSettingsBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SGaussianSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr GaussianSettingsResourceBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------

        m_DOFDownPropertiesPSBufferPtr         = BufferManager::CreateBufferSet(DOFDownPropertiesBuffer);
                                               
        m_DOFApplyPropertiesPSBufferPtr        = BufferManager::CreateBufferSet(DOFApplyPropertiesBuffer);
                                               
        m_GaussianBlurPropertiesPSBufferPtr    = BufferManager::CreateBufferSet(GaussianSettingsBuffer);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnSetupModels()
    {
        m_QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnUnloadMap()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CGfxPostFXRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {
        m_SwapTargetSetPtrs[0] = 0;
        m_SwapTargetSetPtrs[1] = 0;
        
        m_FullTargetSetPtrs[0] = 0;
        
        m_HalfTargetSetPtrs[0] = 0;
        
        m_QuarterTargetSetPtrs[0] = 0;
        m_QuarterTargetSetPtrs[1] = 0;
        m_QuarterTargetSetPtrs[2] = 0;

        m_SMAAEdgeTargetSetPtr        = 0;
        m_SMAAWeightsCalcTargetSetPtr = 0;

        // -----------------------------------------------------------------------------

        std::stringstream SMAADefineStream;

        SMAADefineStream << "#define SMAA_RT_METRICS " << "vec4(1.0f / "
            << _Width << ".0f, 1.0f / " << _Height << ".0f, "
            << _Width << ".0f, " << _Height << ".0f)";

        std::string SMAADefineString = SMAADefineStream.str();

        const char* pDefine = SMAADefineString.c_str();

        CShaderPtr ShaderSMAAEdgeDetectVSPtr  = ShaderManager::CompileVS("vs_smaa_edge_detect.glsl" , "main", pDefine);
        CShaderPtr ShaderSMAAEdgeDetectPSPtr  = ShaderManager::CompilePS("fs_smaa_edge_detect.glsl" , "main", pDefine);
        CShaderPtr ShaderSMAAWeightsCalcVSPtr = ShaderManager::CompileVS("vs_smaa_weights_calc.glsl", "main", pDefine);
        CShaderPtr ShaderSMAAWeightsCalcPSPtr = ShaderManager::CompilePS("fs_smaa_weights_calc.glsl", "main", pDefine);
        CShaderPtr ShaderSMAABlendingVSPtr    = ShaderManager::CompileVS("vs_smaa_blending.glsl"    , "main", pDefine);
        CShaderPtr ShaderSMAABlendingPSPtr    = ShaderManager::CompilePS("fs_smaa_blending.glsl"    , "main", pDefine);

        // -----------------------------------------------------------------------------

        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        Base::Int2 Size(_Width, _Height);
        
        Base::Int2 HalfSize   (Size[0] / 2, Size[1] / 2);
        Base::Int2 QuarterSize(Size[0] / 4, Size[1] / 4);
        
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
        RendertargetDescriptor.m_Format           = CTexture::R8G8B8A8_UBYTE;
        
        CTexturePtr ColorTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Swap Color

        TextureManager::SetTextureLabel(ColorTexturePtr, "PostFX Swap");

        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_NumberOfPixelsU = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV = Size[1];

        CTexturePtr FullTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // First Full

        TextureManager::SetTextureLabel(FullTexturePtr, "PostFX Temp Full Resolution");

        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_NumberOfPixelsU = HalfSize[0];
        RendertargetDescriptor.m_NumberOfPixelsV = HalfSize[1];

        CTexturePtr HalfTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // First Half

        TextureManager::SetTextureLabel(HalfTexturePtr, "PostFX Temp Half Resolution");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_NumberOfPixelsU  = QuarterSize[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = QuarterSize[1];
        
        CTexturePtr QuarterOneTexturePtr   = TextureManager::CreateTexture2D(RendertargetDescriptor); // First Quarter
        CTexturePtr QuarterTwoTexturePtr   = TextureManager::CreateTexture2D(RendertargetDescriptor); // Second Quarter
        CTexturePtr QuarterThreeTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Third Quarter

        TextureManager::SetTextureLabel(QuarterOneTexturePtr, "PostFX Temp Quarter One");
        TextureManager::SetTextureLabel(QuarterTwoTexturePtr, "PostFX Temp Quarter Two");
        TextureManager::SetTextureLabel(QuarterThreeTexturePtr, "PostFX Temp Quarter Three");
        
        // -----------------------------------------------------------------------------
        // Create swap buffer target set
        // -----------------------------------------------------------------------------
        CTexturePtr SwapRenderbuffer[1];
        
        SwapRenderbuffer[0] = ColorTexturePtr;

        CTexturePtr FullRenderbuffer[1];

        FullRenderbuffer[0] = FullTexturePtr;

        CTexturePtr HalfRenderbuffer[1];

        HalfRenderbuffer[0] = HalfTexturePtr;
        
        CTexturePtr QuarterOneRenderbuffer[1];
        
        QuarterOneRenderbuffer[0] = QuarterOneTexturePtr;
        
        CTexturePtr QuarterTwoRenderbuffer[1];
        
        QuarterTwoRenderbuffer[0] = QuarterTwoTexturePtr;
        
        CTexturePtr QuarterThreeRenderbuffer[1];
        
        QuarterThreeRenderbuffer[0] = QuarterThreeTexturePtr;
        
        m_SwapTargetSetPtrs[0] = TargetSetManager::GetDefaultTargetSet();
        m_SwapTargetSetPtrs[1] = TargetSetManager::CreateTargetSet(SwapRenderbuffer, 1);

        m_FullTargetSetPtrs[0] = TargetSetManager::CreateTargetSet(FullRenderbuffer, 1);

        m_HalfTargetSetPtrs[0] = TargetSetManager::CreateTargetSet(HalfRenderbuffer, 1);
        
        m_QuarterTargetSetPtrs[0] = TargetSetManager::CreateTargetSet(QuarterOneRenderbuffer  , 1);
        m_QuarterTargetSetPtrs[1] = TargetSetManager::CreateTargetSet(QuarterTwoRenderbuffer  , 1);
        m_QuarterTargetSetPtrs[2] = TargetSetManager::CreateTargetSet(QuarterThreeRenderbuffer, 1);


        // -----------------------------------------------------------------------------
        // SMAA Render Targets
        // -----------------------------------------------------------------------------
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
        RendertargetDescriptor.m_Format           = CTexture::R8G8_UBYTE;

        CTexturePtr EdgesTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        TextureManager::SetTextureLabel(EdgesTexturePtr, "SMAA Target Texture");

        RendertargetDescriptor.m_Format = CTexture::R8G8B8A8_UBYTE;

        CTexturePtr BlendWeightsTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        TextureManager::SetTextureLabel(BlendWeightsTexturePtr, "SMAA Blend Weights Texture");

        m_SMAAEdgeTargetSetPtr        = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(EdgesTexturePtr));
        m_SMAAWeightsCalcTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(BlendWeightsTexturePtr));

        // -----------------------------------------------------------------------------

        // -----------------------------------------------------------------------------
        // Build view ports
        // -----------------------------------------------------------------------------
        Base::Int2 NativeSize = Main::GetActiveNativeWindowSize();

        SViewPortDescriptor ViewPortDesc;

        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_Width    = static_cast<float>(NativeSize[0]);
        ViewPortDesc.m_Height   = static_cast<float>(NativeSize[1]);
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;

        CViewPortPtr SystemViewPort = ViewManager::CreateViewPort(ViewPortDesc);

        m_SystemViewPortSetPtr = ViewManager::CreateViewPortSet(SystemViewPort);

        // -----------------------------------------------------------------------------

        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_Width    = static_cast<float>(HalfSize[0]);
        ViewPortDesc.m_Height   = static_cast<float>(HalfSize[1]);
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;
        
        CViewPortPtr HalfViewPort = ViewManager::CreateViewPort(ViewPortDesc);
        
        CViewPortSetPtr HalfViewPortSetPtr = ViewManager::CreateViewPortSet(HalfViewPort);

        // -----------------------------------------------------------------------------
        
        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_Width    = static_cast<float>(QuarterSize[0]);
        ViewPortDesc.m_Height   = static_cast<float>(QuarterSize[1]);
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;
        
        CViewPortPtr QuarterViewPort = ViewManager::CreateViewPort(ViewPortDesc);
        
        CViewPortSetPtr QuarterViewPortSetPtr = ViewManager::CreateViewPortSet(QuarterViewPort);
        
        // -----------------------------------------------------------------------------
        // Setup states
        // -----------------------------------------------------------------------------
        m_SwapRenderContextPtrs[0]->SetTargetSet(m_SwapTargetSetPtrs[0]);
        m_SwapRenderContextPtrs[1]->SetTargetSet(m_SwapTargetSetPtrs[1]);

        m_FullRenderContextPtrs[0]->SetTargetSet(m_FullTargetSetPtrs[0]);

        m_HalfRenderContextPtrs[0]->SetViewPortSet(HalfViewPortSetPtr);
        m_HalfRenderContextPtrs[0]->SetTargetSet(m_HalfTargetSetPtrs[0]);
        
        m_QuarterRenderContextPtrs[0]->SetViewPortSet(QuarterViewPortSetPtr);
        m_QuarterRenderContextPtrs[0]->SetTargetSet(m_QuarterTargetSetPtrs[0]);

        m_QuarterRenderContextPtrs[1]->SetViewPortSet(QuarterViewPortSetPtr);
        m_QuarterRenderContextPtrs[1]->SetTargetSet(m_QuarterTargetSetPtrs[1]);

        m_QuarterRenderContextPtrs[2]->SetViewPortSet(QuarterViewPortSetPtr);
        m_QuarterRenderContextPtrs[2]->SetTargetSet(m_QuarterTargetSetPtrs[2]);

        m_SMAAEdgeDetectContextPtr->SetTargetSet(m_SMAAEdgeTargetSetPtr);
        m_SMAAWeightCalcContextPtr->SetTargetSet(m_SMAAWeightsCalcTargetSetPtr);

        // -----------------------------------------------------------------------------

        CTexturePtr ColorOneTexturePtr = m_SwapTargetSetPtrs[0]->GetRenderTarget(0);
        CTexturePtr ColorTwoTexturePtr = m_SwapTargetSetPtrs[1]->GetRenderTarget(0);
        CTexturePtr DepthTexturePtr    = TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget();
        
        m_SwapTextureSetPtrs[0] = TextureManager::CreateTextureSet(ColorOneTexturePtr, DepthTexturePtr);
        m_SwapTextureSetPtrs[1] = TextureManager::CreateTextureSet(ColorTwoTexturePtr, DepthTexturePtr);

        m_FullTextureSetPtrs[0] = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(FullTexturePtr));

        m_HalfTexturePtrs[0] = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(HalfTexturePtr));
        
        m_QuarterTextureSetPtrs[0] = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(QuarterOneTexturePtr));
        m_QuarterTextureSetPtrs[1] = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(QuarterTwoTexturePtr));
        m_QuarterTextureSetPtrs[2] = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(QuarterThreeTexturePtr));

        // -----------------------------------------------------------------------------
        // Setting up SMAA look up textures
        // -----------------------------------------------------------------------------
        auto EdgesTexPtr   = m_SMAAEdgeTargetSetPtr->GetRenderTarget(0);
        auto WeightsTexPtr = m_SMAAWeightsCalcTargetSetPtr->GetRenderTarget(0);

        CTexturePtr TexturePtrs[2][5] =
        {
            { ColorOneTexturePtr, EdgesTexPtr, WeightsTexPtr, static_cast<CTexturePtr>(m_SMAAAreaTexture), static_cast<CTexturePtr>(m_SMAASearchTexture) },
            { ColorTwoTexturePtr, EdgesTexPtr, WeightsTexPtr, static_cast<CTexturePtr>(m_SMAAAreaTexture), static_cast<CTexturePtr>(m_SMAASearchTexture) }
        };

        m_SMAATextureSetPtr[0] = TextureManager::CreateTextureSet(TexturePtrs[0], 5);
        m_SMAATextureSetPtr[1] = TextureManager::CreateTextureSet(TexturePtrs[1], 5);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::Render()
    {
        Performance::BeginEvent("Post Process");

        m_SwapCounter = 0;
        
        RenderDOF();
        RenderPostAA();

        RenderToSystem();

        Performance::EndEvent();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::RenderDOF()
    {
        if (m_DOFRenderJobs.size() == 0) return;

        Performance::BeginEvent("Depth of Field");

        // TODO: What happens if more then one DOF effect is available?
        Dt::CDOFFXFacet* pDataDOFFacet = m_DOFRenderJobs[0].m_pDataDOFFacet;

        assert(pDataDOFFacet != 0);

        // -----------------------------------------------------------------------------
        // Get screen resolutions
        // -----------------------------------------------------------------------------
        Base::Int2 Size = Main::GetActiveWindowSize();
        
        Base::Int2 HalfSize   (Size[0] / 2, Size[1] / 2);
        Base::Int2 QuarterSize(Size[0] / 4, Size[1] / 4);
        
        // -----------------------------------------------------------------------------
        // Set current swap buffer count
        // -----------------------------------------------------------------------------
        int CurrentSwapBufferCount = m_SwapCounter        % 2;
        int NextSwapBufferCount    = (m_SwapCounter += 1) % 2;

        // -----------------------------------------------------------------------------
        // Data
        // -----------------------------------------------------------------------------
        SDOFDownProperties DOFDownProperties;

        DOFDownProperties.m_DofNear     = pDataDOFFacet->GetNear();
        DOFDownProperties.m_DofRowDelta = Base::Float2(1.0f, 0.25f / Size[1]);

        BufferManager::UploadBufferData(m_DOFDownPropertiesPSBufferPtr->GetBuffer(0), &DOFDownProperties);

        // -----------------------------------------------------------------------------

        SDOFApplyProperties DOFApplyProperties;

        DOFApplyProperties.m_DofEqFar     = pDataDOFFacet->GetEqFar();
        DOFApplyProperties.m_DofLerpBias  = pDataDOFFacet->GetLerpBias();
        DOFApplyProperties.m_DofLerpScale = pDataDOFFacet->GetLerpScale();

        BufferManager::UploadBufferData(m_DOFApplyPropertiesPSBufferPtr->GetBuffer(0), &DOFApplyProperties);
        
        // -----------------------------------------------------------------------------
        // Rendering: Copy from one swap buffer to the other one
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_SwapRenderContextPtrs[NextSwapBufferCount]);
        
        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);
        
        ContextManager::SetShaderPS(m_PassThroughShaderPSPtr);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        
        ContextManager::SetTexture(0, m_SwapTextureSetPtrs[CurrentSwapBufferCount]->GetTexture(0));
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);
        
        ContextManager::ResetConstantBuffer(0);
        
        ContextManager::ResetTopology();
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBuffer();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetRenderContext();
        
        // -----------------------------------------------------------------------------
        // Rendering: Down Sampling
        // -----------------------------------------------------------------------------        
        ContextManager::SetRenderContext(m_QuarterRenderContextPtrs[0]);
        
        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[DOFDownSample]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[DOFDownSample]);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        
        ContextManager::SetConstantBuffer(1, m_DOFDownPropertiesPSBufferPtr->GetBuffer(0));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        
        ContextManager::SetTexture(0, m_SwapTextureSetPtrs[CurrentSwapBufferCount]->GetTexture(0));
        ContextManager::SetTexture(1, m_SwapTextureSetPtrs[CurrentSwapBufferCount]->GetTexture(1));
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        
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
        // Rendering: Do gaussian blur with down sampled image
        // -----------------------------------------------------------------------------
        SGaussianSettings GaussianSettings;

        GaussianSettings.m_Weights[0] = 0.0f;
        GaussianSettings.m_Weights[1] = 0.000003f;
        GaussianSettings.m_Weights[2] = 0.000229f;
        GaussianSettings.m_Weights[3] = 0.005977f;
        GaussianSettings.m_Weights[4] = 0.060598f;
        GaussianSettings.m_Weights[5] = 0.241730f;
        GaussianSettings.m_Weights[6] = 0.382925f;
        
        ContextManager::SetRenderContext(m_QuarterRenderContextPtrs[1]);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[GaussianBlur]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[GaussianBlur]);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        
        ContextManager::SetConstantBuffer(1, m_GaussianBlurPropertiesPSBufferPtr->GetBuffer(0));
        
        GaussianSettings.m_Direction[0] = 1.0f * 1.0f / static_cast<float>(QuarterSize[0]);
        GaussianSettings.m_Direction[1] = 0.0f * 1.0f / static_cast<float>(QuarterSize[1]);
        
        BufferManager::UploadBufferData(m_GaussianBlurPropertiesPSBufferPtr->GetBuffer(0), &GaussianSettings);
        
        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, m_QuarterTextureSetPtrs[0]->GetTexture(0));
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);
        
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
        
        ContextManager::SetRenderContext(m_QuarterRenderContextPtrs[2]);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[GaussianBlur]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[GaussianBlur]);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_GaussianBlurPropertiesPSBufferPtr->GetBuffer(0));
        
        GaussianSettings.m_Direction[0] = 0.0f * 1.0f / static_cast<float>(QuarterSize[0]);
        GaussianSettings.m_Direction[1] = 1.0f * 1.0f / static_cast<float>(QuarterSize[1]);
        
        BufferManager::UploadBufferData(m_GaussianBlurPropertiesPSBufferPtr->GetBuffer(0), &GaussianSettings);
        
        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, m_QuarterTextureSetPtrs[1]->GetTexture(0));
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);
        
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
        // Rendering: Calculate CoC / Near
        // -----------------------------------------------------------------------------        
        ContextManager::SetRenderContext(m_QuarterRenderContextPtrs[1]);
        
        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[DOFNear]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[DOFNear]);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        
        ContextManager::SetTexture(0, m_QuarterTextureSetPtrs[0]->GetTexture(0));
        ContextManager::SetTexture(1, m_QuarterTextureSetPtrs[2]->GetTexture(0));
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        
        ContextManager::ResetConstantBuffer(0);
        
        ContextManager::ResetTopology();
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBuffer();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Rendering: Blur near
        // -----------------------------------------------------------------------------        
        ContextManager::SetRenderContext(m_QuarterRenderContextPtrs[0]);
        
        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[DOFBlurNear]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[DOFBlurNear]);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        
        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, m_QuarterTextureSetPtrs[1]->GetTexture(0));
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);
        
        ContextManager::ResetConstantBuffer(0);
        
        ContextManager::ResetTopology();
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBuffer();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetRenderContext();
        
        // -----------------------------------------------------------------------------
        // Rendering: Apply depth of field as post effect
        // -----------------------------------------------------------------------------        
        ContextManager::SetRenderFlags(CRenderState::AlphaBlend);
        
        ContextManager::SetRenderContext(m_SwapRenderContextPtrs[NextSwapBufferCount]);
        
        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[DOFApply]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[DOFApply]);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        
        ContextManager::SetConstantBuffer(1, m_DOFApplyPropertiesPSBufferPtr->GetBuffer(0));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        
        ContextManager::SetTexture(0, m_SwapTextureSetPtrs[CurrentSwapBufferCount]->GetTexture(0));
        ContextManager::SetTexture(1, m_SwapTextureSetPtrs[CurrentSwapBufferCount]->GetTexture(1));
        ContextManager::SetTexture(2, m_QuarterTextureSetPtrs[0]->GetTexture(0));
        ContextManager::SetTexture(3, m_QuarterTextureSetPtrs[2]->GetTexture(0));
        
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
        
        ContextManager::SetRenderFlags(0);

        Performance::EndEvent();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::RenderPostAA()
    {
        if (m_PostAARenderJobs.size() == 0) return;

        // TODO: What happens if more then one PostAA effect is available?
        Dt::CPostAAFXFacet* pDataPostAAFacet = m_PostAARenderJobs[0].m_pDataPostAAFacet;

        assert(pDataPostAAFacet != 0);

        switch (pDataPostAAFacet->GetType())
        {
        case Dt::CPostAAFXFacet::FXAA:
            RenderFXAA();
            break;
        case Dt::CPostAAFXFacet::SMAA:
            RenderSMAA();
            break;
        default:
            break;
        }        
    }

    // -----------------------------------------------------------------------------

    void CGfxPostFXRenderer::RenderFXAA()
    {
        Performance::BeginEvent("FXAA");

        // -----------------------------------------------------------------------------
        // Set current swap buffer count
        // -----------------------------------------------------------------------------
        int CurrentSwapBufferCount = m_SwapCounter        % 2;
        int NextSwapBufferCount    = (m_SwapCounter += 1) % 2;

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_SwapRenderContextPtrs[NextSwapBufferCount]);
        
        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[FXAA]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[FXAA]);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        
        ContextManager::SetTexture(0, m_SwapTextureSetPtrs[CurrentSwapBufferCount]->GetTexture(0));
        ContextManager::SetTexture(1, m_SwapTextureSetPtrs[CurrentSwapBufferCount]->GetTexture(1));
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        
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

    void CGfxPostFXRenderer::RenderSMAA()
    {
        Performance::BeginEvent("SMAA");

        // -----------------------------------------------------------------------------
        // Set current swap buffer count
        // -----------------------------------------------------------------------------
        int CurrentSwapBufferCount = m_SwapCounter % 2;
        int NextSwapBufferCount = (m_SwapCounter += 1) % 2;

        TargetSetManager::ClearTargetSet(m_SMAAEdgeTargetSetPtr);
        TargetSetManager::ClearTargetSet(m_SMAAWeightsCalcTargetSetPtr);

        // -----------------------------------------------------------------------------
        // Edge detection
        // -----------------------------------------------------------------------------

        ContextManager::SetRenderContext(m_SMAAEdgeDetectContextPtr);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleStrip);

        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[SMAAEdgeDetect]);

        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[SMAAEdgeDetect]);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, m_SMAATextureSetPtr[CurrentSwapBufferCount]->GetTexture(0));

        ContextManager::Draw(3, 0);

        // -----------------------------------------------------------------------------
        // Blending weights calculation
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_SMAAWeightCalcContextPtr);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleStrip);

        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[SMAAWeightsCalc]);

        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[SMAAWeightsCalc]);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, m_SMAATextureSetPtr[CurrentSwapBufferCount]->GetTexture(1));
        ContextManager::SetTexture(1, m_SMAATextureSetPtr[CurrentSwapBufferCount]->GetTexture(3));
        ContextManager::SetTexture(2, m_SMAATextureSetPtr[CurrentSwapBufferCount]->GetTexture(4));

        ContextManager::Draw(3, 0);

        // -----------------------------------------------------------------------------
        // Neighborhood blending
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_SwapRenderContextPtrs[NextSwapBufferCount]);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleStrip);

        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[SMAABlending]);

        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[SMAABlending]);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, m_SMAATextureSetPtr[CurrentSwapBufferCount]->GetTexture(0));
        ContextManager::SetTexture(1, m_SMAATextureSetPtr[CurrentSwapBufferCount]->GetTexture(2));

        ContextManager::Draw(3, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);
        ContextManager::ResetTexture(2);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        ContextManager::ResetSampler(2);

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
    
    void CGfxPostFXRenderer::RenderToSystem()
    {
        Performance::BeginEvent("System Swap");

        // -----------------------------------------------------------------------------
        // Set current swap buffer count
        // -----------------------------------------------------------------------------
        int CurrentSwapBufferCount = m_SwapCounter % 2;
        
        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        ContextManager::SetTargetSet(TargetSetManager::GetSystemTargetSet());

        ContextManager::SetViewPortSet(m_SystemViewPortSetPtr);

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));
        
        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);
        
        ContextManager::SetShaderPS(m_PassThroughShaderPSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, m_SwapTextureSetPtrs[CurrentSwapBufferCount]->GetTexture(0));

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

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

    void CGfxPostFXRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_PostAARenderJobs.clear();
        m_DOFRenderJobs .clear();

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::FX);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() == Dt::SFXType::PostAA)
            {
                Dt::CPostAAFXFacet* pDataPostAAFacet = static_cast<Dt::CPostAAFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

                assert(pDataPostAAFacet != 0);

                // -----------------------------------------------------------------------------
                // Set sun into a new render job
                // -----------------------------------------------------------------------------
                SPostAARenderJob NewRenderJob;

                NewRenderJob.m_pDataPostAAFacet = pDataPostAAFacet;

                m_PostAARenderJobs.push_back(NewRenderJob);
            }
            else if (rCurrentEntity.GetType() == Dt::SFXType::DOF)
            {
                Dt::CDOFFXFacet* pDataDOFFacet = static_cast<Dt::CDOFFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

                assert(pDataDOFFacet != 0);

                // -----------------------------------------------------------------------------
                // Set sun into a new render job
                // -----------------------------------------------------------------------------
                SDOFRenderJob NewRenderJob;

                NewRenderJob.m_pDataDOFFacet = pDataDOFFacet;

                m_DOFRenderJobs.push_back(NewRenderJob);
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::FX);
        }
    }
} // namespace

namespace Gfx
{
namespace PostFX
{
    void OnStart()
    {
        CGfxPostFXRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxPostFXRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxPostFXRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxPostFXRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxPostFXRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxPostFXRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxPostFXRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxPostFXRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxPostFXRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxPostFXRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxPostFXRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxPostFXRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxPostFXRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxPostFXRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxPostFXRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxPostFXRenderer::GetInstance().Render();
    }
} // namespace PostFX
} // namespace Gfx

