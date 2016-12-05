
#include "graphic/gfx_precompiled.h"

#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_dof_facet.h"
#include "data/data_entity.h"
#include "data/data_fx_type.h"
#include "data/data_fxaa_facet.h"
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
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

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

        struct SFXAARenderJob
        {
            Dt::CFXAAFXFacet* m_pDataFXAAFacet;
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
        
        struct SFXAAProperties
        {
            Base::Float3 m_Luma;
        };

    private:

        typedef std::vector<SFXAARenderJob> CFXAARenderJobs;
        typedef std::vector<SDOFRenderJob> CDOFRenderJobs;
        
    private:
        
        CMeshPtr          m_QuadModelPtr;
        
        CBufferSetPtr     m_BaseVSBufferSetPtr;
        CBufferSetPtr     m_DOFVSBufferSetPtr;
        
        CBufferSetPtr     m_DOFDownPropertiesPSBufferPtr;
        CBufferSetPtr     m_DOFApplyPropertiesPSBufferPtr;
        CBufferSetPtr     m_GaussianBlurPropertiesPSBufferPtr;
        CBufferSetPtr     m_FXAAPropertiesPSBufferPtr;
        
        CInputLayoutPtr   m_FullQuadInputLayoutPtr;
        CShaderPtr        m_RectangleShaderVSPtr;
        CShaderPtr        m_PassThroughShaderPSPtr;
        CSamplerSetPtr    m_PSSamplerSetPtr;
        CSamplerSetPtr    m_PSSamplerWrapSetPtr;
        CRenderContextPtr m_SystemContextPtr;
        
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

        CFXAARenderJobs m_FXAARenderJobs;
        CDOFRenderJobs  m_DOFRenderJobs;
        
        unsigned int m_SwapCounter;
        
    private:
        
        void RenderDOF();
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
        , m_FullQuadInputLayoutPtr           ()
        , m_RectangleShaderVSPtr             ()
        , m_PassThroughShaderPSPtr           ()
        , m_BaseVSBufferSetPtr               ()
        , m_DOFVSBufferSetPtr                ()
        , m_DOFDownPropertiesPSBufferPtr     ()
        , m_DOFApplyPropertiesPSBufferPtr    ()
        , m_GaussianBlurPropertiesPSBufferPtr()
        , m_FXAAPropertiesPSBufferPtr        ()
        , m_SystemContextPtr                 ()
        , m_SwapTargetSetPtrs                ()
        , m_PSSamplerSetPtr                  ()
        , m_PSSamplerWrapSetPtr              ()
        , m_FXAARenderJobs                   ()
        , m_DOFRenderJobs                    ()
        , m_SwapCounter                      (0)
    {
        m_FXAARenderJobs.reserve(2);
        m_DOFRenderJobs .reserve(2);
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
        m_FullQuadInputLayoutPtr            = 0;
        m_RectangleShaderVSPtr              = 0;
        m_PassThroughShaderPSPtr            = 0;
        m_BaseVSBufferSetPtr                = 0;
        m_DOFVSBufferSetPtr                 = 0;
        m_DOFDownPropertiesPSBufferPtr      = 0;
        m_DOFApplyPropertiesPSBufferPtr     = 0;
        m_GaussianBlurPropertiesPSBufferPtr = 0;
        m_FXAAPropertiesPSBufferPtr         = 0;
        m_SystemContextPtr                  = 0;
        m_PSSamplerSetPtr                   = 0;
        m_PSSamplerWrapSetPtr               = 0;
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

        CShaderPtr ShaderSMAAEdgeDetectVSPtr  = ShaderManager::CompileVS("vs_smaa_edge_detect.glsl" , "main");
        CShaderPtr ShaderSMAAEdgeDetectPSPtr  = ShaderManager::CompilePS("fs_smaa_edge_detect.glsl" , "main");
        CShaderPtr ShaderSMAAWeightsCalcVSPtr = ShaderManager::CompileVS("vs_smaa_weights_calc.glsl", "main");
        CShaderPtr ShaderSMAAWeightsCalcPSPtr = ShaderManager::CompilePS("fs_smaa_weights_calc.glsl", "main");
        CShaderPtr ShaderSMAABlendingVSPtr    = ShaderManager::CompileVS("vs_smaa_blending.glsl"    , "main");
        CShaderPtr ShaderSMAABlendingPSPtr    = ShaderManager::CompilePS("fs_smaa_blending.glsl"    , "main");
        
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
        RendertargetDescriptor.m_Binding          = CTextureBase::RenderTarget | CTextureBase::ShaderResource;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::Unknown;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        RendertargetDescriptor.m_Format           = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr ColorTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Swap Color

        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_NumberOfPixelsU = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV = Size[1];

        CTexture2DPtr FullTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // First Full

        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_NumberOfPixelsU = HalfSize[0];
        RendertargetDescriptor.m_NumberOfPixelsV = HalfSize[1];

        CTexture2DPtr HalfTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // First Half
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_NumberOfPixelsU  = QuarterSize[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = QuarterSize[1];
        
        CTexture2DPtr QuarterOneTexturePtr   = TextureManager::CreateTexture2D(RendertargetDescriptor); // First Quarter
        CTexture2DPtr QuarterTwoTexturePtr   = TextureManager::CreateTexture2D(RendertargetDescriptor); // Second Quarter
        CTexture2DPtr QuarterThreeTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Third Quarter
        
        // -----------------------------------------------------------------------------
        // Create swap buffer target set
        // -----------------------------------------------------------------------------
        CTextureBasePtr SwapRenderbuffer[1];
        
        SwapRenderbuffer[0] = ColorTexturePtr;

        CTextureBasePtr FullRenderbuffer[1];

        FullRenderbuffer[0] = FullTexturePtr;

        CTextureBasePtr HalfRenderbuffer[1];

        HalfRenderbuffer[0] = HalfTexturePtr;
        
        CTextureBasePtr QuarterOneRenderbuffer[1];
        
        QuarterOneRenderbuffer[0] = QuarterOneTexturePtr;
        
        CTextureBasePtr QuarterTwoRenderbuffer[1];
        
        QuarterTwoRenderbuffer[0] = QuarterTwoTexturePtr;
        
        CTextureBasePtr QuarterThreeRenderbuffer[1];
        
        QuarterThreeRenderbuffer[0] = QuarterThreeTexturePtr;
        
        m_SwapTargetSetPtrs[0] = TargetSetManager::GetDefaultTargetSet();
        m_SwapTargetSetPtrs[1] = TargetSetManager::CreateTargetSet(SwapRenderbuffer, 1);

        m_FullTargetSetPtrs[0] = TargetSetManager::CreateTargetSet(FullRenderbuffer, 1);

        m_HalfTargetSetPtrs[0] = TargetSetManager::CreateTargetSet(HalfRenderbuffer, 1);
        
        m_QuarterTargetSetPtrs[0] = TargetSetManager::CreateTargetSet(QuarterOneRenderbuffer  , 1);
        m_QuarterTargetSetPtrs[1] = TargetSetManager::CreateTargetSet(QuarterTwoRenderbuffer  , 1);
        m_QuarterTargetSetPtrs[2] = TargetSetManager::CreateTargetSet(QuarterThreeRenderbuffer, 1);

        //////////////////////////////////////////////////////////
        // SMAA Render Targets
        //////////////////////////////////////////////////////////

        RendertargetDescriptor.m_NumberOfPixelsU = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW = 1;
        RendertargetDescriptor.m_NumberOfMipMaps = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding = CTextureBase::RenderTarget | CTextureBase::ShaderResource;
        RendertargetDescriptor.m_Access = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format = CTextureBase::Unknown;
        RendertargetDescriptor.m_Usage = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName = 0;
        RendertargetDescriptor.m_pPixels = 0;
        RendertargetDescriptor.m_Format = CTextureBase::R8G8_UBYTE;

        CTextureBasePtr EdgesTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        RendertargetDescriptor.m_Format = CTextureBase::R8G8B8A8_UBYTE;

        CTextureBasePtr BlendWeightsTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        m_SMAAEdgeTargetSetPtr = TargetSetManager::CreateTargetSet(EdgesTexturePtr);
        m_SMAAWeightsCalcTargetSetPtr = TargetSetManager::CreateTargetSet(BlendWeightsTexturePtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::OnSetupStates()
    {
        // -----------------------------------------------------------------------------
        // Get screen resolutions
        // -----------------------------------------------------------------------------
        Base::Int2 Size = Main::GetActiveWindowSize();
        
        Base::Int2 HalfSize   (Size[0] / 2, Size[1] / 2);
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
        
        CRenderContextPtr SystemContextPtr = ContextManager::CreateRenderContext();
        
        SystemContextPtr->SetCamera(CameraPtr);
        SystemContextPtr->SetViewPortSet(ViewPortSetPtr);
        SystemContextPtr->SetTargetSet(TargetSetPtr);
        SystemContextPtr->SetRenderState(RenderStatePtr);
        
        m_SystemContextPtr = SystemContextPtr;
        
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
        
        CSamplerPtr LinearFilter = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);
        
        CSamplerPtr PointFilter = SamplerManager::GetSampler(CSampler::MinMagMipPointWrap);
        
        m_PSSamplerSetPtr = SamplerManager::CreateSamplerSet(LinearFilter, LinearFilter, LinearFilter, LinearFilter);
        
        m_PSSamplerWrapSetPtr =  SamplerManager::CreateSamplerSet(LinearFilter, PointFilter, LinearFilter);

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
        CTextureBasePtr ColorOneTexturePtr     = m_SwapTargetSetPtrs[0]->GetRenderTarget(0);
        CTextureBasePtr ColorTwoTexturePtr     = m_SwapTargetSetPtrs[1]->GetRenderTarget(0);
        CTextureBasePtr FullTexturePtr         = m_FullTargetSetPtrs[0]->GetRenderTarget(0);
        CTextureBasePtr HalfTexturePtr         = m_HalfTargetSetPtrs[0]->GetRenderTarget(0);
        CTextureBasePtr QuarterOneTexturePtr   = m_QuarterTargetSetPtrs[0]->GetRenderTarget(0);
        CTextureBasePtr QuarterTwoTexturePtr   = m_QuarterTargetSetPtrs[1]->GetRenderTarget(0);
        CTextureBasePtr QuarterThreeTexturePtr = m_QuarterTargetSetPtrs[2]->GetRenderTarget(0);
        CTextureBasePtr NormalTexturePtr       = TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1);
        CTextureBasePtr DepthTexturePtr        = TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget();
        
        m_SwapTextureSetPtrs[0] = TextureManager::CreateTextureSet(ColorOneTexturePtr, DepthTexturePtr);
        m_SwapTextureSetPtrs[1] = TextureManager::CreateTextureSet(ColorTwoTexturePtr, DepthTexturePtr);

        m_FullTextureSetPtrs[0] = TextureManager::CreateTextureSet(FullTexturePtr);

        m_HalfTexturePtrs[0] = TextureManager::CreateTextureSet(HalfTexturePtr);
        
        m_QuarterTextureSetPtrs[0] = TextureManager::CreateTextureSet(QuarterOneTexturePtr);
        m_QuarterTextureSetPtrs[1] = TextureManager::CreateTextureSet(QuarterTwoTexturePtr);
        m_QuarterTextureSetPtrs[2] = TextureManager::CreateTextureSet(QuarterThreeTexturePtr);

        //////////////////////////////////////////////////////////////////
        // Setting up SMAA look up textures
        //////////////////////////////////////////////////////////////////

        STextureDescriptor AreaTexDescriptor = {};

        AreaTexDescriptor.m_NumberOfPixelsU = AREATEX_WIDTH;
        AreaTexDescriptor.m_NumberOfPixelsV = AREATEX_HEIGHT;
        AreaTexDescriptor.m_NumberOfPixelsW = 1;
        AreaTexDescriptor.m_NumberOfMipMaps = 1;
        AreaTexDescriptor.m_NumberOfTextures = 1;
        AreaTexDescriptor.m_Binding = CTextureBase::ShaderResource;
        AreaTexDescriptor.m_Access = CTextureBase::CPUWrite;
        AreaTexDescriptor.m_Format = CTextureBase::R8G8_UBYTE;
        AreaTexDescriptor.m_Usage = CTextureBase::GPURead;
        AreaTexDescriptor.m_Semantic = CTextureBase::UndefinedSemantic;
        AreaTexDescriptor.m_pFileName = 0;
        AreaTexDescriptor.m_pPixels = const_cast<void*>(static_cast<const void*>(&areaTexBytes[0]));

        CTextureBasePtr SMAAAreaTexture = TextureManager::CreateTexture2D(AreaTexDescriptor);

        STextureDescriptor SearchTexDescriptor = {};

        SearchTexDescriptor.m_NumberOfPixelsU = SEARCHTEX_WIDTH;
        SearchTexDescriptor.m_NumberOfPixelsV = SEARCHTEX_HEIGHT;
        SearchTexDescriptor.m_NumberOfPixelsW = 1;
        SearchTexDescriptor.m_NumberOfMipMaps = 1;
        SearchTexDescriptor.m_NumberOfTextures = 1;
        SearchTexDescriptor.m_Binding = CTextureBase::ShaderResource;
        SearchTexDescriptor.m_Access = CTextureBase::CPUWrite;
        SearchTexDescriptor.m_Format = CTextureBase::R8_UBYTE;
        SearchTexDescriptor.m_Usage = CTextureBase::GPURead;
        SearchTexDescriptor.m_Semantic = CTextureBase::UndefinedSemantic;
        SearchTexDescriptor.m_pFileName = 0;
        SearchTexDescriptor.m_pPixels = const_cast<void*>(static_cast<const void*>(&searchTexBytes[0]));

        CTextureBasePtr SMAASearchTexture = TextureManager::CreateTexture2D(SearchTexDescriptor);

        m_SMAATextureSetPtr[0] = TextureManager::CreateTextureSet(ColorOneTexturePtr, SMAAAreaTexture, SMAASearchTexture);
        m_SMAATextureSetPtr[1] = TextureManager::CreateTextureSet(ColorTwoTexturePtr, SMAAAreaTexture, SMAASearchTexture);
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
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SFXAAProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr FXAABuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        m_BaseVSBufferSetPtr                   = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS());
                                               
        m_DOFDownPropertiesPSBufferPtr         = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), DOFDownPropertiesBuffer);
                                               
        m_DOFApplyPropertiesPSBufferPtr        = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), DOFApplyPropertiesBuffer);
                                               
        m_GaussianBlurPropertiesPSBufferPtr    = BufferManager::CreateBufferSet(GaussianSettingsBuffer);
        
        m_FXAAPropertiesPSBufferPtr            = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), FXAABuffer);
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
        RenderFXAA();
		RenderSMAA();

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
        SDOFDownProperties* pDOFDownProperties = static_cast<SDOFDownProperties*>(BufferManager::MapConstantBuffer(m_DOFDownPropertiesPSBufferPtr->GetBuffer(0)));

        pDOFDownProperties->m_DofNear     = pDataDOFFacet->GetNear();
        pDOFDownProperties->m_DofRowDelta = Base::Float2(1.0f, 0.25f / Size[1]);

        BufferManager::UnmapConstantBuffer(m_DOFDownPropertiesPSBufferPtr->GetBuffer(0));

        // -----------------------------------------------------------------------------

        SDOFApplyProperties* pDOFApplyProperties = static_cast<SDOFApplyProperties*>(BufferManager::MapConstantBuffer(m_DOFApplyPropertiesPSBufferPtr->GetBuffer(1)));

        pDOFApplyProperties->m_DofEqFar     = pDataDOFFacet->GetEqFar();
        pDOFApplyProperties->m_DofLerpBias  = pDataDOFFacet->GetLerpBias();
        pDOFApplyProperties->m_DofLerpScale = pDataDOFFacet->GetLerpScale();

        BufferManager::UnmapConstantBuffer(m_DOFApplyPropertiesPSBufferPtr->GetBuffer(1));
        
        // -----------------------------------------------------------------------------
        // Rendering: Copy from one swap buffer to the other one
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = {0, 0};

        ContextManager::SetRenderContext(m_SwapRenderContextPtrs[NextSwapBufferCount]);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);
        
        ContextManager::SetShaderPS(m_PassThroughShaderPSPtr);
        
        ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);
        
        ContextManager::SetTextureSetPS(m_SwapTextureSetPtrs[CurrentSwapBufferCount]);
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTextureSetPS();
        
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
        // Rendering: Down Sampling
        // -----------------------------------------------------------------------------        
        ContextManager::SetRenderContext(m_QuarterRenderContextPtrs[0]);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[DOFDownSample]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[DOFDownSample]);
        
        ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);
        
        ContextManager::SetConstantBufferSetPS(m_DOFDownPropertiesPSBufferPtr);
        
        ContextManager::SetTextureSetPS(m_SwapTextureSetPtrs[CurrentSwapBufferCount]);
        
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
        // Rendering: Do gaussian blur with down sampled image
        // -----------------------------------------------------------------------------
        SGaussianSettings* pGaussianSettings;
        
        ContextManager::SetRenderContext(m_QuarterRenderContextPtrs[1]);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[GaussianBlur]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[GaussianBlur]);
        
        ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);
        
        ContextManager::SetConstantBufferSetPS(m_GaussianBlurPropertiesPSBufferPtr);
        
        pGaussianSettings = static_cast<SGaussianSettings*>(BufferManager::MapConstantBuffer(m_GaussianBlurPropertiesPSBufferPtr->GetBuffer(0)));
        
        pGaussianSettings->m_Direction[0]          = 1.0f * 1.0f / static_cast<float>(QuarterSize[0]);
        pGaussianSettings->m_Direction[1]          = 0.0f * 1.0f / static_cast<float>(QuarterSize[1]);
        pGaussianSettings->m_Weights[0]            = 0.0f;
        pGaussianSettings->m_Weights[1]            = 0.000003f;
        pGaussianSettings->m_Weights[2]            = 0.000229f;
        pGaussianSettings->m_Weights[3]            = 0.005977f;
        pGaussianSettings->m_Weights[4]            = 0.060598f;
        pGaussianSettings->m_Weights[5]            = 0.241730f;
        pGaussianSettings->m_Weights[6]            = 0.382925f;
        
        BufferManager::UnmapConstantBuffer(m_GaussianBlurPropertiesPSBufferPtr->GetBuffer(0));
        
        ContextManager::SetTextureSetPS(m_QuarterTextureSetPtrs[0]);
        
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
        
        ContextManager::SetRenderContext(m_QuarterRenderContextPtrs[2]);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[GaussianBlur]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[GaussianBlur]);
        
        ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);
        
        ContextManager::SetConstantBufferSetPS(m_GaussianBlurPropertiesPSBufferPtr);
        
        pGaussianSettings = static_cast<SGaussianSettings*>(BufferManager::MapConstantBuffer(m_GaussianBlurPropertiesPSBufferPtr->GetBuffer(0)));
        
        pGaussianSettings->m_Direction[0]          = 0.0f * 1.0f / static_cast<float>(QuarterSize[0]);
        pGaussianSettings->m_Direction[1]          = 1.0f * 1.0f / static_cast<float>(QuarterSize[1]);
        pGaussianSettings->m_Weights[0]            = 0.0f;
        pGaussianSettings->m_Weights[1]            = 0.000003f;
        pGaussianSettings->m_Weights[2]            = 0.000229f;
        pGaussianSettings->m_Weights[3]            = 0.005977f;
        pGaussianSettings->m_Weights[4]            = 0.060598f;
        pGaussianSettings->m_Weights[5]            = 0.241730f;
        pGaussianSettings->m_Weights[6]            = 0.382925f;
        
        BufferManager::UnmapConstantBuffer(m_GaussianBlurPropertiesPSBufferPtr->GetBuffer(0));
        
        ContextManager::SetTextureSetPS(m_QuarterTextureSetPtrs[1]);
        
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
        // Rendering: Calculate CoC / Near
        // -----------------------------------------------------------------------------        
        ContextManager::SetRenderContext(m_QuarterRenderContextPtrs[1]);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[DOFNear]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[DOFNear]);
        
        ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);
        
        ContextManager::SetTextureSetPS(m_QuarterTextureSetPtrs[0]);
        
        ContextManager::SetTextureSetPS(m_QuarterTextureSetPtrs[2]);
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTextureSetPS();
        
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
        // Rendering: Blur near
        // -----------------------------------------------------------------------------        
        ContextManager::SetRenderContext(m_QuarterRenderContextPtrs[0]);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[DOFBlurNear]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[DOFBlurNear]);
        
        ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);
        
        ContextManager::SetTextureSetPS(m_QuarterTextureSetPtrs[1]);
        
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTextureSetPS();
        
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
        // Rendering: Apply depth of field as post effect
        // -----------------------------------------------------------------------------        
        ContextManager::SetRenderFlags(CRenderState::AlphaBlend);
        
        ContextManager::SetRenderContext(m_SwapRenderContextPtrs[NextSwapBufferCount]);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[DOFApply]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[DOFApply]);
        
        ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);
        
        ContextManager::SetConstantBufferSetPS(m_DOFApplyPropertiesPSBufferPtr);
        
        ContextManager::SetTextureSetPS(m_SwapTextureSetPtrs[CurrentSwapBufferCount]);
        
        ContextManager::SetTextureSetPS(m_QuarterTextureSetPtrs[0]);
        
        ContextManager::SetTextureSetPS(m_QuarterTextureSetPtrs[2]);
        
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
        
        ContextManager::SetRenderFlags(0);

        Performance::EndEvent();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxPostFXRenderer::RenderFXAA()
    {
        if (m_FXAARenderJobs.size() == 0) return;

        Performance::BeginEvent("FXAA");

        // TODO: What happens if more then one FXAA effect is available?
        Dt::CFXAAFXFacet* pDataFXAAFacet = m_FXAARenderJobs[0].m_pDataFXAAFacet;

        assert(pDataFXAAFacet != 0);
        
        // -----------------------------------------------------------------------------
        // Set current swap buffer count
        // -----------------------------------------------------------------------------
        int CurrentSwapBufferCount = m_SwapCounter        % 2;
        int NextSwapBufferCount    = (m_SwapCounter += 1) % 2;

        // -----------------------------------------------------------------------------
        // Data
        // -----------------------------------------------------------------------------
        SFXAAProperties* pFXAAProperties = static_cast<SFXAAProperties*>(BufferManager::MapConstantBuffer(m_FXAAPropertiesPSBufferPtr->GetBuffer(1)));

        pFXAAProperties->m_Luma = pDataFXAAFacet->GetLuma();

        BufferManager::UnmapConstantBuffer(m_FXAAPropertiesPSBufferPtr->GetBuffer(1));

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = {0, 0};
        
        ContextManager::SetRenderContext(m_SwapRenderContextPtrs[NextSwapBufferCount]);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[FXAA]);
        
        ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[FXAA]);
        
        ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);
        
        ContextManager::SetConstantBufferSetPS(m_FXAAPropertiesPSBufferPtr);
        
        ContextManager::SetTextureSetPS(m_SwapTextureSetPtrs[CurrentSwapBufferCount]);
        
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

        const unsigned int pOffset[] = { 0, 0 };

        ContextManager::SetRenderContext(m_SMAAEdgeDetectContextPtr);

        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

		ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

		ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);

		ContextManager::SetTopology(STopology::TriangleStrip);

		ContextManager::SetShaderVS(m_PostEffectShaderVSPtrs[SMAAEdgeDetect]);

		ContextManager::SetShaderPS(m_PostEffectShaderPSPtrs[SMAAEdgeDetect]);

		//ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);

		//ContextManager::SetConstantBufferSetPS(m_SMAAPropertiesPSBufferPtr);

		ContextManager::SetTextureSetPS(m_SMAATextureSetPtr[CurrentSwapBufferCount]);

		ContextManager::Draw(3, 0);

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
        const unsigned int pOffset[] = {0, 0};
        
        ContextManager::SetRenderContext(m_SystemContextPtr);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);
        
        ContextManager::SetShaderPS(m_PassThroughShaderPSPtr);

        ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);

        ContextManager::SetTextureSetPS(m_SwapTextureSetPtrs[CurrentSwapBufferCount]);

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTextureSetPS();

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

    void CGfxPostFXRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_FXAARenderJobs.clear();
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
            if (rCurrentEntity.GetType() == Dt::SFXType::FXAA)
            {
                Dt::CFXAAFXFacet* pDataFXAAFacet = static_cast<Dt::CFXAAFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

                assert(pDataFXAAFacet != 0);

                // -----------------------------------------------------------------------------
                // Set sun into a new render job
                // -----------------------------------------------------------------------------
                SFXAARenderJob NewRenderJob;

                NewRenderJob.m_pDataFXAAFacet = pDataFXAAFacet;

                m_FXAARenderJobs.push_back(NewRenderJob);
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

