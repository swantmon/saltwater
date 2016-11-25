
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_vector4.h"

#include "camera/cam_control_manager.h"

#include "data/data_entity.h"
#include "data/data_fx_facet.h"
#include "data/data_light_facet.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_fog_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_main.h"
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

using namespace Gfx;

namespace
{
    class CGfxFogRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxFogRenderer)
        
    public:
        CGfxFogRenderer();
        ~CGfxFogRenderer();
        
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

        static const unsigned int s_BlurTileSize = 8;

    private:

        struct SVolumeFogRenderJob
        {
            Dt::CVolumeFogFXFacet* m_pDataVolumeFogFacet;
        };

        struct SGaussianShaderProperties
        {
            Base::Int2 m_Direction;
            Base::Int2 m_MaxPixelCoord;
            float      m_Weights[7];
        };

        struct SSunLightProperties
        {
            Base::Float4x4 m_LightViewProjection;
            Base::Float4   m_LightDirection;
            Base::Float4   m_LightColor;
            float          m_SunAngularRadius;
            unsigned int   m_ExposureHistoryIndex;
            float          m_Padding[2];
        };

        struct SVolumeLightingProperties
        {
            Base::Float4 m_WindDirection;
            Base::Float4 m_FogColor;
            float        m_FrustumDepthInMeter;
            float        m_ShadowIntensity;
            float        m_VolumetricFogScatteringCoefficient;
            float        m_VolumetricFogAbsorptionCoefficient;
            float        m_DensityLevel;
            float        m_DensityAttenuation;
            float        m_Padding[2];
        };

        struct SFogApplyProperties
        {
            float m_FrustumDepthInMeter;
        };

    private:

        typedef std::vector<SVolumeFogRenderJob> CVolumeFogRenderJobs;
        
    private:
        
        CMeshPtr          m_QuadModelPtr;
        CBufferSetPtr     m_FullQuadViewVSBufferPtr;
        CBufferSetPtr     m_VolumeLightingCSBufferSetPtr;
        CInputLayoutPtr   m_P2InputLayoutPtr;
        CShaderPtr        m_RectangleShaderVSPtr;
        CShaderPtr        m_ESMCSPtr;
        CShaderPtr        m_VolumeLightingCSPtr;
        CShaderPtr        m_VolumeScatteringCSPtr;
        CShaderPtr        m_ApplyPSPtr;
        CSamplerSetPtr    m_PSSamplerSetPtr;
        CRenderContextPtr m_LightRenderContextPtr;
        CTexture2DPtr     m_ESMTexturePtr;
        CTextureSetPtr    m_ESMTextureSetPtr;

        CTexture3DPtr     m_VolumeTexturePtr;
        CTextureSetPtr    m_VolumeTextureSetPtr;

        CTexture3DPtr     m_ScatteringTexturePtr;
        CTextureSetPtr    m_ScatteringTextureSetPtr;

        CTextureSetPtr    m_ApplyTextureSetPtr;

        CTexture2DPtr m_PermutationTexturePtr;
        CTexture2DPtr m_GradientPermutationTexturePtr;

        CBufferSetPtr m_FogApplyBufferPtr;

        CShaderPtr     m_GaussianBlurShaderPtr;
        CBufferSetPtr  m_GaussianBlurPropertiesCSBufferSetPtr;
        CTextureSetPtr m_BlurStagesTextureSetPtrs[2];

        CVolumeFogRenderJobs m_VolumeFogRenderJobs;

    private:

        void RenderESM();
        void RenderVolumeLighting();
        void RenderScattering();
        void RenderApply();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxFogRenderer::CGfxFogRenderer()
        : m_QuadModelPtr                        ()
        , m_FullQuadViewVSBufferPtr             ()
        , m_VolumeLightingCSBufferSetPtr        ()
        , m_P2InputLayoutPtr                    ()
        , m_RectangleShaderVSPtr                ()
        , m_ESMCSPtr                            ()
        , m_VolumeLightingCSPtr                 ()
        , m_VolumeScatteringCSPtr               ()
        , m_ApplyPSPtr                          ()
        , m_PSSamplerSetPtr                     ()
        , m_LightRenderContextPtr               ()
        , m_ESMTexturePtr                       ()
        , m_ESMTextureSetPtr                    ()
        , m_VolumeTexturePtr                    ()
        , m_VolumeTextureSetPtr                 ()
        , m_ScatteringTexturePtr                ()
        , m_ScatteringTextureSetPtr             ()
        , m_ApplyTextureSetPtr                  ()
        , m_PermutationTexturePtr               ()
        , m_GradientPermutationTexturePtr       ()
        , m_FogApplyBufferPtr                   ()
        , m_GaussianBlurShaderPtr               ()
        , m_GaussianBlurPropertiesCSBufferSetPtr()
        , m_BlurStagesTextureSetPtrs            ()
        , m_VolumeFogRenderJobs                 ()
    {
        m_VolumeFogRenderJobs.reserve(1);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxFogRenderer::~CGfxFogRenderer()
    {
    	
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnExit()
    {
        m_QuadModelPtr                  = 0;
        m_FullQuadViewVSBufferPtr       = 0;
        m_VolumeLightingCSBufferSetPtr  = 0;
        m_P2InputLayoutPtr              = 0;
        m_RectangleShaderVSPtr          = 0;
        m_ESMCSPtr                      = 0;
        m_VolumeLightingCSPtr           = 0;
        m_VolumeScatteringCSPtr         = 0;
        m_ApplyPSPtr                    = 0;
        m_PSSamplerSetPtr               = 0;
        m_LightRenderContextPtr         = 0;
        m_ESMTexturePtr                 = 0;
        m_ESMTextureSetPtr              = 0;
        m_VolumeTexturePtr              = 0;
        m_VolumeTextureSetPtr           = 0;
        m_ScatteringTexturePtr          = 0;
        m_ScatteringTextureSetPtr       = 0;
        m_ApplyTextureSetPtr            = 0;
        m_PermutationTexturePtr         = 0;
        m_GradientPermutationTexturePtr = 0;
        m_FogApplyBufferPtr             = 0;

        m_GaussianBlurShaderPtr                = 0;
        m_GaussianBlurPropertiesCSBufferSetPtr = 0;
        m_BlurStagesTextureSetPtrs[0]          = 0;
        m_BlurStagesTextureSetPtrs[1]          = 0;

        m_VolumeFogRenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupShader()
    {       
        m_RectangleShaderVSPtr  = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");
        m_ESMCSPtr              = ShaderManager::CompileCS("cs_esm.glsl", "main");  
        m_VolumeLightingCSPtr   = ShaderManager::CompileCS("cs_volume_lighting.glsl", "main");
        m_VolumeScatteringCSPtr = ShaderManager::CompileCS("cs_volume_scattering.glsl", "main");  
        m_ApplyPSPtr            = ShaderManager::CompilePS("fs_fog_apply.glsl", "main");  
        m_GaussianBlurShaderPtr = ShaderManager::CompileCS("cs_gaussian_blur_r32f.glsl", "main");
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor QuadInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        m_P2InputLayoutPtr = ShaderManager::CreateInputLayout(QuadInputLayout, 1, m_RectangleShaderVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupRenderTargets()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupStates()
    {
        CCameraPtr      QuadCameraPtr  = ViewManager     ::GetFullQuadCamera();
        CViewPortSetPtr ViewPortSetPtr = ViewManager     ::GetViewPortSet();
        CRenderStatePtr LightStatePtr  = StateManager    ::GetRenderState(0);
        CTargetSetPtr   TargetSetPtr   = TargetSetManager::GetLightAccumulationTargetSet();
       
        // -----------------------------------------------------------------------------

        m_LightRenderContextPtr = ContextManager::CreateRenderContext();

        m_LightRenderContextPtr->SetCamera(QuadCameraPtr);
        m_LightRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_LightRenderContextPtr->SetTargetSet(TargetSetPtr);
        m_LightRenderContextPtr->SetRenderState(LightStatePtr);
        
        // -----------------------------------------------------------------------------
        
        CSamplerPtr Sampler[6];

        Sampler[0] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[1] = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);
        Sampler[2] = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);

        m_PSSamplerSetPtr = SamplerManager::CreateSamplerSet(Sampler, 3);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupTextures()
    {
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = 256;
        RendertargetDescriptor.m_NumberOfPixelsV  = 256;
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::ShaderResource;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::R32_FLOAT;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPUReadWrite;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        
        m_ESMTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        CTexture2DPtr ESMSwapTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        // -----------------------------------------------------------------------------

        static int s_PerlinPermutations[] = 
        { 
            151,160,137,91,90,15,
            131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
            190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
            88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
            77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
            102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
            135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
            5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
            223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
            129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
            251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
            49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
            138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
        };

        auto GetPermutationAtIndex = [&](int _Index) -> int
        {
            return s_PerlinPermutations[_Index % 256];
        };

        auto GetPermutationAtPosition = [&](const Base::UInt2& _rUV) -> Base::Float4
        {
            int A  = GetPermutationAtIndex(_rUV[0]) + _rUV[1];
            int AA = GetPermutationAtIndex(A);
            int AB = GetPermutationAtIndex(A + 1);
            int B  = GetPermutationAtIndex(_rUV[0] + 1) + _rUV[1];
            int BA = GetPermutationAtIndex(B);
            int BB = GetPermutationAtIndex(B + 1);

            return Base::Float4(static_cast<float>(AA), static_cast<float>(AB), static_cast<float>(BA), static_cast<float>(BB)) / 255.0f;
        };


        Base::Float4* pPermutationData = static_cast<Base::Float4*>(Base::CMemory::Allocate(sizeof(Base::Float4) * 256 * 256));

        for (unsigned int Y = 0; Y < 256; ++Y)
        {
            for (unsigned int X = 0; X < 256; ++X)
            {
                pPermutationData[Y * 256 + X] = GetPermutationAtPosition(Base::UInt2(X, Y));
            }
        }

        RendertargetDescriptor.m_NumberOfPixelsU  = 256;
        RendertargetDescriptor.m_NumberOfPixelsV  = 256;
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::ShaderResource;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::R32G32B32A32_FLOAT;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPUReadWrite;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = pPermutationData;

        m_PermutationTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        Base::CMemory::Free(pPermutationData);

        // -----------------------------------------------------------------------------

        static Base::Float4 s_PerlinGradients[] = 
        {
            Base::Float4(1,1,0,     0),
            Base::Float4(-1,1,0,    0),
            Base::Float4(1,-1,0,    0),
            Base::Float4(-1,-1,0,   0),
            Base::Float4(1,0,1,     0),
            Base::Float4(-1,0,1,    0),
            Base::Float4(1,0,-1,    0),
            Base::Float4(-1,0,-1,   0),
            Base::Float4(0,1,1,     0),
            Base::Float4(0,-1,1,    0),
            Base::Float4(0,1,-1,    0),
            Base::Float4(0,-1,-1,   0),
            Base::Float4(1,1,0,     0),
            Base::Float4(0,-1,1,    0),
            Base::Float4(-1,1,0,    0),
            Base::Float4(0,-1,-1,   0),
        };

        auto GetPermutationGradientAtIndex = [&](unsigned int _Index) -> Base::Float4
        {
            return s_PerlinGradients[s_PerlinPermutations[_Index] % 16];
        };

        Base::Float4* pPermutationGradientData = static_cast<Base::Float4*>(Base::CMemory::Allocate(sizeof(Base::Float4) * 256));

        for (unsigned int X = 0; X < 256; ++X)
        {
            pPermutationGradientData[X] = GetPermutationGradientAtIndex(X);
        }

        RendertargetDescriptor.m_NumberOfPixelsU  = 256;
        RendertargetDescriptor.m_NumberOfPixelsV  = 1;
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::ShaderResource;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::R32G32B32A32_FLOAT;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPUReadWrite;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = pPermutationGradientData;

        m_GradientPermutationTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        Base::CMemory::Free(pPermutationGradientData);

        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_NumberOfPixelsU  = 160;
        RendertargetDescriptor.m_NumberOfPixelsV  = 90;
        RendertargetDescriptor.m_NumberOfPixelsW  = 128;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::ShaderResource;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::R32G32B32A32_FLOAT;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPUReadWrite;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;

        m_VolumeTexturePtr = TextureManager::CreateTexture3D(RendertargetDescriptor);

        m_ScatteringTexturePtr = TextureManager::CreateTexture3D(RendertargetDescriptor);

        // -----------------------------------------------------------------------------

        m_VolumeTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_VolumeTexturePtr), static_cast<CTextureBasePtr>(m_PermutationTexturePtr), static_cast<CTextureBasePtr>(m_GradientPermutationTexturePtr), static_cast<CTextureBasePtr>(m_ESMTexturePtr));

        // -----------------------------------------------------------------------------

        m_ScatteringTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_VolumeTexturePtr), static_cast<CTextureBasePtr>(m_ScatteringTexturePtr));

        // -----------------------------------------------------------------------------

        CTextureBasePtr LightAccumulationTexture = TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0);

        CTextureBasePtr DepthTexturePtr = TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget();

        m_ApplyTextureSetPtr = TextureManager::CreateTextureSet(LightAccumulationTexture, DepthTexturePtr, static_cast<CTextureBasePtr>(m_ScatteringTexturePtr));

        // -----------------------------------------------------------------------------

        m_BlurStagesTextureSetPtrs[0] = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_ESMTexturePtr), static_cast<CTextureBasePtr>(ESMSwapTexturePtr));
        m_BlurStagesTextureSetPtrs[1] = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(ESMSwapTexturePtr), static_cast<CTextureBasePtr>(m_ESMTexturePtr));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SGaussianShaderProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr GaussianSettingsResourceBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSunLightProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SunLightBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SVolumeLightingProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr VolumeLightingPropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SFogApplyProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr FogApplyPropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        CBufferPtr HistogramExposureHistoryBufferPtr = HistogramRenderer::GetExposureHistoryBuffer();

        // -----------------------------------------------------------------------------

        m_GaussianBlurPropertiesCSBufferSetPtr = BufferManager::CreateBufferSet(GaussianSettingsResourceBuffer);
        m_VolumeLightingCSBufferSetPtr         = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), SunLightBufferPtr, VolumeLightingPropertiesBufferPtr, HistogramExposureHistoryBufferPtr);
        m_FullQuadViewVSBufferPtr              = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS());
        m_FogApplyBufferPtr                  = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), FogApplyPropertiesBufferPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupModels()
    {
        m_QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupEnd()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::Render()
    {
        if (m_VolumeFogRenderJobs.size() == 0) return;

        Performance::BeginEvent("Fog");

        RenderESM();

        RenderVolumeLighting();

        RenderScattering();

        RenderApply();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxFogRenderer::RenderESM()
    {
        Performance::BeginEvent("ESM");


        // -----------------------------------------------------------------------------
        // Get light(s) and compute exponential shadow map
        // TODO: Can this be done in shadow renderer because of other uses with the
        // light?
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        Gfx::CSunFacet* pGraphicSunFacet = 0;

        for (; CurrentEntity != EndOfEntities; CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light))
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            if (rCurrentEntity.GetType() == Dt::SLightType::Sun)
            {
                pGraphicSunFacet = static_cast<Gfx::CSunFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));
            }
        }

        CTextureBasePtr ShadowMapPtr = pGraphicSunFacet->GetTextureSMSet()->GetTexture(0);

        m_ESMTextureSetPtr = TextureManager::CreateTextureSet(ShadowMapPtr, static_cast<CTextureBasePtr>(m_ESMTexturePtr));

        ContextManager::SetShaderCS(m_ESMCSPtr);

        ContextManager::SetTextureSetCS(m_ESMTextureSetPtr);

        ContextManager::Dispatch(256, 256, 1);

        ContextManager::ResetTextureSetCS();

        ContextManager::ResetShaderCS();

        // -----------------------------------------------------------------------------
        // Blur
        // -----------------------------------------------------------------------------
        unsigned int NumberOfThreadGroupsX;
        unsigned int NumberOfThreadGroupsY;

        NumberOfThreadGroupsX = (256 + s_BlurTileSize - 1) / (s_BlurTileSize);
        NumberOfThreadGroupsY = (256 + s_BlurTileSize - 1) / (s_BlurTileSize);

        // -----------------------------------------------------------------------------
        // Blur
        // -----------------------------------------------------------------------------
        SGaussianShaderProperties* pGaussianSettings = static_cast<SGaussianShaderProperties*>(BufferManager::MapConstantBuffer(m_GaussianBlurPropertiesCSBufferSetPtr->GetBuffer(0)));

        pGaussianSettings->m_Direction[0] = 1;
        pGaussianSettings->m_Direction[1] = 0;
        pGaussianSettings->m_MaxPixelCoord[0] = 256;
        pGaussianSettings->m_MaxPixelCoord[1] = 256;
        pGaussianSettings->m_Weights[0] = 0.018816f;
        pGaussianSettings->m_Weights[1] = 0.034474f;
        pGaussianSettings->m_Weights[2] = 0.056577f;
        pGaussianSettings->m_Weights[3] = 0.083173f;
        pGaussianSettings->m_Weights[4] = 0.109523f;
        pGaussianSettings->m_Weights[5] = 0.129188f;
        pGaussianSettings->m_Weights[6] = 0.136498f;

        BufferManager::UnmapConstantBuffer(m_GaussianBlurPropertiesCSBufferSetPtr->GetBuffer(0));

        ContextManager::SetShaderCS(m_GaussianBlurShaderPtr);

        ContextManager::SetConstantBufferSetCS(m_GaussianBlurPropertiesCSBufferSetPtr);

        ContextManager::SetTextureSetCS(m_BlurStagesTextureSetPtrs[0]);

        ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

        ContextManager::ResetTextureSetCS();

        ContextManager::ResetConstantBufferSetCS();

        ContextManager::ResetShaderCS();


        // -----------------------------------------------------------------------------
        // Blur
        // -----------------------------------------------------------------------------
        pGaussianSettings = static_cast<SGaussianShaderProperties*>(BufferManager::MapConstantBuffer(m_GaussianBlurPropertiesCSBufferSetPtr->GetBuffer(0)));

        pGaussianSettings->m_Direction[0] = 0;
        pGaussianSettings->m_Direction[1] = 1;
        pGaussianSettings->m_MaxPixelCoord[0] = 256;
        pGaussianSettings->m_MaxPixelCoord[1] = 256;
        pGaussianSettings->m_Weights[0] = 0.018816f;
        pGaussianSettings->m_Weights[1] = 0.034474f;
        pGaussianSettings->m_Weights[2] = 0.056577f;
        pGaussianSettings->m_Weights[3] = 0.083173f;
        pGaussianSettings->m_Weights[4] = 0.109523f;
        pGaussianSettings->m_Weights[5] = 0.129188f;
        pGaussianSettings->m_Weights[6] = 0.136498f;

        BufferManager::UnmapConstantBuffer(m_GaussianBlurPropertiesCSBufferSetPtr->GetBuffer(0));

        ContextManager::SetShaderCS(m_GaussianBlurShaderPtr);

        ContextManager::SetConstantBufferSetCS(m_GaussianBlurPropertiesCSBufferSetPtr);

        ContextManager::SetTextureSetCS(m_BlurStagesTextureSetPtrs[1]);

        ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

        ContextManager::ResetTextureSetCS();

        ContextManager::ResetConstantBufferSetCS();

        ContextManager::ResetShaderCS();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxFogRenderer::RenderVolumeLighting()
    {
        Performance::BeginEvent("Volume Lighting");

        // -----------------------------------------------------------------------------
        // Getting volume fog informations from render job
        // TODO: What happens if more then one DOF effect is available?
        // -----------------------------------------------------------------------------
        Dt::CVolumeFogFXFacet* pDataVolumeFogFacet = m_VolumeFogRenderJobs[0].m_pDataVolumeFogFacet;

        assert(pDataVolumeFogFacet != 0);

        // -----------------------------------------------------------------------------
        // Getting sun informations
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        Gfx::CSunFacet* pGraphicSunFacet = 0;
        Dt::CSunLightFacet* pDataSunFacet = 0;

        for (; CurrentEntity != EndOfEntities; CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light))
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            if (rCurrentEntity.GetType() == Dt::SLightType::Sun)
            {
                pDataSunFacet = static_cast<Dt::CSunLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                pGraphicSunFacet = static_cast<Gfx::CSunFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));
            }
        }

        SSunLightProperties* pLightBuffer = static_cast<SSunLightProperties*>(BufferManager::MapConstantBuffer(m_VolumeLightingCSBufferSetPtr->GetBuffer(1)));

	    assert(pLightBuffer != nullptr);

	    pLightBuffer->m_LightViewProjection  = pGraphicSunFacet->GetCamera()->GetViewProjectionMatrix();
	    pLightBuffer->m_LightDirection       = Base::Float4(pDataSunFacet->GetDirection(), 0.0f).Normalize();
	    pLightBuffer->m_LightColor           = Base::Float4(pDataSunFacet->GetLightness(), 1.0f);
	    pLightBuffer->m_SunAngularRadius     = 0.27f * Base::SConstants<float>::s_Pi / 180.0f;
	    pLightBuffer->m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();

        BufferManager::UnmapConstantBuffer(m_VolumeLightingCSBufferSetPtr->GetBuffer(1));

        // -----------------------------------------------------------------------------

        SVolumeLightingProperties* pVolumeProperties = static_cast<SVolumeLightingProperties*>(BufferManager::MapConstantBuffer(m_VolumeLightingCSBufferSetPtr->GetBuffer(2)));

	    assert(pVolumeProperties != nullptr);

        pVolumeProperties->m_WindDirection                      = pDataVolumeFogFacet->GetWindDirection();
        pVolumeProperties->m_FogColor                           = pDataVolumeFogFacet->GetFogColor();
        pVolumeProperties->m_FrustumDepthInMeter                = pDataVolumeFogFacet->GetFrustumDepthInMeter();
        pVolumeProperties->m_ShadowIntensity                    = pDataVolumeFogFacet->GetShadowIntensity();
        pVolumeProperties->m_VolumetricFogScatteringCoefficient = pDataVolumeFogFacet->GetScatteringCoefficient();
        pVolumeProperties->m_VolumetricFogAbsorptionCoefficient = pDataVolumeFogFacet->GetAbsorptionCoefficient();
        pVolumeProperties->m_DensityLevel                       = pDataVolumeFogFacet->GetDensityLevel();
        pVolumeProperties->m_DensityAttenuation                 = pDataVolumeFogFacet->GetDensityAttenuation();

        BufferManager::UnmapConstantBuffer(m_VolumeLightingCSBufferSetPtr->GetBuffer(2));

        // -----------------------------------------------------------------------------

        ContextManager::SetShaderCS(m_VolumeLightingCSPtr);

        ContextManager::SetConstantBufferSetCS(m_VolumeLightingCSBufferSetPtr);

        ContextManager::SetTextureSetCS(m_VolumeTextureSetPtr);

        unsigned int NumberOfThreadGroupsX;
        unsigned int NumberOfThreadGroupsY;
        unsigned int NumberOfThreadGroupsZ;

        unsigned int s_TileSizeX = 16;
        unsigned int s_TileSizeY = 10;
        unsigned int s_TileSizeZ = 8;

        NumberOfThreadGroupsX = (160 + s_TileSizeX - 1) / (s_TileSizeX);
        NumberOfThreadGroupsY = ( 90 + s_TileSizeY - 1) / (s_TileSizeY);
        NumberOfThreadGroupsZ = (128 + s_TileSizeZ - 1) / (s_TileSizeZ);

        ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, NumberOfThreadGroupsZ);

        ContextManager::ResetTextureSetCS();
        
        ContextManager::ResetConstantBufferSetCS();

        ContextManager::ResetShaderCS();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxFogRenderer::RenderScattering()
    {
        Performance::BeginEvent("Scattering");

        ContextManager::SetShaderCS(m_VolumeScatteringCSPtr);

        ContextManager::SetTextureSetCS(m_ScatteringTextureSetPtr);

        unsigned int NumberOfThreadGroupsX;
        unsigned int NumberOfThreadGroupsY;

        unsigned int s_TileSizeX = 16;
        unsigned int s_TileSizeY = 10;

        NumberOfThreadGroupsX = (160 + s_TileSizeX - 1) / (s_TileSizeX);
        NumberOfThreadGroupsY = ( 90 + s_TileSizeY - 1) / (s_TileSizeY);

        ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

        ContextManager::ResetTextureSetCS();

        ContextManager::ResetShaderCS();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxFogRenderer::RenderApply()
    {
        Performance::BeginEvent("Apply");

        // -----------------------------------------------------------------------------
        // Getting volume fog informations from render job
        // TODO: What happens if more then one DOF effect is available?
        // -----------------------------------------------------------------------------
        Dt::CVolumeFogFXFacet* pDataVolumeFogFacet = m_VolumeFogRenderJobs[0].m_pDataVolumeFogFacet;

        assert(pDataVolumeFogFacet != 0);

        // -----------------------------------------------------------------------------
        // Data
        // -----------------------------------------------------------------------------
        SFogApplyProperties* pFogApplyProperties = static_cast<SFogApplyProperties*>(BufferManager::MapConstantBuffer(m_FogApplyBufferPtr->GetBuffer(1)));

	    assert(pFogApplyProperties != nullptr);

        pFogApplyProperties->m_FrustumDepthInMeter = pDataVolumeFogFacet->GetFrustumDepthInMeter();

        BufferManager::UnmapConstantBuffer(m_FogApplyBufferPtr->GetBuffer(1));

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = { 0, 0 };

        ContextManager::SetRenderContext(m_LightRenderContextPtr);

        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_P2InputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

        ContextManager::SetShaderPS(m_ApplyPSPtr);

        ContextManager::SetConstantBufferSetVS(m_FullQuadViewVSBufferPtr);

        ContextManager::SetConstantBufferSetPS(m_FogApplyBufferPtr);

        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

        ContextManager::SetTextureSetPS(m_ApplyTextureSetPtr);

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

    void CGfxFogRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_VolumeFogRenderJobs.clear();

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
            if (rCurrentEntity.GetType() == Dt::SFXType::VolumeFog)
            {
                Dt::CVolumeFogFXFacet* pDataSSRFacet = static_cast<Dt::CVolumeFogFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

                assert(pDataSSRFacet != 0);

                // -----------------------------------------------------------------------------
                // Set sun into a new render job
                // -----------------------------------------------------------------------------
                SVolumeFogRenderJob NewRenderJob;

                NewRenderJob.m_pDataVolumeFogFacet = pDataSSRFacet;

                m_VolumeFogRenderJobs.push_back(NewRenderJob);
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
namespace FogRenderer
{
    void OnStart()
    {
        CGfxFogRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxFogRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxFogRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxFogRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxFogRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxFogRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxFogRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxFogRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxFogRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxFogRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxFogRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxFogRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxFogRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxFogRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxFogRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxFogRenderer::GetInstance().Render();
    }
} // namespace FogRenderer
} // namespace Gfx

