
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/camera/cam_control_manager.h"

#include "engine/core/core_console.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_map.h"
#include "engine/data/data_sun_component.h"
#include "engine/data/data_volume_fog_component.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_fog_renderer.h"
#include "engine/graphic/gfx_histogram_renderer.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_sun.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

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
            Dt::CVolumeFogComponent* m_pDtVolumeFogComponent;
            Dt::CSunComponent*       m_pDtSunComponent;
            Gfx::CSun*      m_pGfxSunComponent;
        };

        struct SGaussianShaderProperties
        {
            glm::ivec2 m_Direction;
            glm::ivec2 m_MaxPixelCoord;
            float      m_Weights[7];
        };

        struct SSunLightProperties
        {
            glm::mat4 m_LightViewProjection;
            glm::vec4   m_LightDirection;
            glm::vec4   m_LightColor;
            float          m_SunAngularRadius;
            unsigned int   m_ExposureHistoryIndex;
            float          m_Padding[2];
        };

        struct SVolumeLightingProperties
        {
            glm::vec4 m_WindDirection;
            glm::vec4 m_FogColor;
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
        
        CBufferSetPtr     m_VolumeLightingCSBufferSetPtr;
        CShaderPtr        m_RectangleShaderVSPtr;
        CShaderPtr        m_ESMCSPtr;
        CShaderPtr        m_VolumeLightingCSPtr;
        CShaderPtr        m_VolumeScatteringCSPtr;
        CShaderPtr        m_ApplyPSPtr;
        CRenderContextPtr m_LightRenderContextPtr;
        CTexturePtr       m_ESMTexturePtr;
        CTextureSetPtr    m_ESMTextureSetPtr;

        CTexturePtr     m_VolumeTexturePtr;
        CTextureSetPtr    m_VolumeTextureSetPtr;

        CTexturePtr     m_ScatteringTexturePtr;
        CTextureSetPtr    m_ScatteringTextureSetPtr;

        CTexturePtr m_PermutationTexturePtr;
        CTexturePtr m_GradientPermutationTexturePtr;

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
        : m_VolumeLightingCSBufferSetPtr        ()
        , m_RectangleShaderVSPtr                ()
        , m_ESMCSPtr                            ()
        , m_VolumeLightingCSPtr                 ()
        , m_VolumeScatteringCSPtr               ()
        , m_ApplyPSPtr                          ()
        , m_LightRenderContextPtr               ()
        , m_ESMTexturePtr                       ()
        , m_ESMTextureSetPtr                    ()
        , m_VolumeTexturePtr                    ()
        , m_VolumeTextureSetPtr                 ()
        , m_ScatteringTexturePtr                ()
        , m_ScatteringTextureSetPtr             ()
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
        m_VolumeLightingCSBufferSetPtr  = 0;
        m_RectangleShaderVSPtr          = 0;
        m_ESMCSPtr                      = 0;
        m_VolumeLightingCSPtr           = 0;
        m_VolumeScatteringCSPtr         = 0;
        m_ApplyPSPtr                    = 0;
        m_LightRenderContextPtr         = 0;
        m_ESMTexturePtr                 = 0;
        m_ESMTextureSetPtr              = 0;
        m_VolumeTexturePtr              = 0;
        m_VolumeTextureSetPtr           = 0;
        m_ScatteringTexturePtr          = 0;
        m_ScatteringTextureSetPtr       = 0;
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
        m_RectangleShaderVSPtr  = ShaderManager::CompileVS("vs_fullscreen.glsl", "main");
        m_ESMCSPtr              = ShaderManager::CompileCS("cs_esm.glsl", "main");  
        m_VolumeLightingCSPtr   = ShaderManager::CompileCS("cs_volume_lighting.glsl", "main");
        m_VolumeScatteringCSPtr = ShaderManager::CompileCS("cs_volume_scattering.glsl", "main");  
        m_ApplyPSPtr            = ShaderManager::CompilePS("fs_fog_apply.glsl", "main");  
        m_GaussianBlurShaderPtr = ShaderManager::CompileCS("cs_gaussian_blur.glsl", "main", "#define TILE_SIZE 8\n#define IMAGE_TYPE r32f");
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
        CRenderStatePtr LightStatePtr  = StateManager    ::GetRenderState(CRenderState::AdditionBlend);
        CTargetSetPtr   TargetSetPtr   = TargetSetManager::GetLightAccumulationTargetSet();
       
        // -----------------------------------------------------------------------------

        m_LightRenderContextPtr = ContextManager::CreateRenderContext();

        m_LightRenderContextPtr->SetCamera(QuadCameraPtr);
        m_LightRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_LightRenderContextPtr->SetTargetSet(TargetSetPtr);
        m_LightRenderContextPtr->SetRenderState(LightStatePtr);
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
        RendertargetDescriptor.m_Binding          = CTexture::ShaderResource;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::R32_FLOAT;
        RendertargetDescriptor.m_Usage            = CTexture::GPUReadWrite;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        
        m_ESMTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

		TextureManager::SetTextureLabel(m_ESMTexturePtr, "Fog ESM");

        CTexturePtr ESMSwapTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

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

        auto GetPermutationAtPosition = [&](const glm::uvec2& _rUV) -> glm::vec4
        {
            int A  = GetPermutationAtIndex(_rUV[0]) + _rUV[1];
            int AA = GetPermutationAtIndex(A);
            int AB = GetPermutationAtIndex(A + 1);
            int B  = GetPermutationAtIndex(_rUV[0] + 1) + _rUV[1];
            int BA = GetPermutationAtIndex(B);
            int BB = GetPermutationAtIndex(B + 1);

            return glm::vec4(static_cast<float>(AA), static_cast<float>(AB), static_cast<float>(BA), static_cast<float>(BB)) / 255.0f;
        };


        glm::vec4* pPermutationData = static_cast<glm::vec4*>(Base::CMemory::Allocate(sizeof(glm::vec4) * 256 * 256));

        for (unsigned int Y = 0; Y < 256; ++Y)
        {
            for (unsigned int X = 0; X < 256; ++X)
            {
                pPermutationData[Y * 256 + X] = GetPermutationAtPosition(glm::uvec2(X, Y));
            }
        }

        RendertargetDescriptor.m_NumberOfPixelsU  = 256;
        RendertargetDescriptor.m_NumberOfPixelsV  = 256;
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::ShaderResource;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::R32G32B32A32_FLOAT;
        RendertargetDescriptor.m_Usage            = CTexture::GPUReadWrite;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = pPermutationData;

        m_PermutationTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

        Base::CMemory::Free(pPermutationData);

        // -----------------------------------------------------------------------------

        static glm::vec4 s_PerlinGradients[] = 
        {
            glm::vec4(1,1,0,     0),
            glm::vec4(-1,1,0,    0),
            glm::vec4(1,-1,0,    0),
            glm::vec4(-1,-1,0,   0),
            glm::vec4(1,0,1,     0),
            glm::vec4(-1,0,1,    0),
            glm::vec4(1,0,-1,    0),
            glm::vec4(-1,0,-1,   0),
            glm::vec4(0,1,1,     0),
            glm::vec4(0,-1,1,    0),
            glm::vec4(0,1,-1,    0),
            glm::vec4(0,-1,-1,   0),
            glm::vec4(1,1,0,     0),
            glm::vec4(0,-1,1,    0),
            glm::vec4(-1,1,0,    0),
            glm::vec4(0,-1,-1,   0),
        };

        auto GetPermutationGradientAtIndex = [&](unsigned int _Index) -> glm::vec4
        {
            return s_PerlinGradients[s_PerlinPermutations[_Index] % 16];
        };

        glm::vec4* pPermutationGradientData = static_cast<glm::vec4*>(Base::CMemory::Allocate(sizeof(glm::vec4) * 256));

        for (unsigned int X = 0; X < 256; ++X)
        {
            pPermutationGradientData[X] = GetPermutationGradientAtIndex(X);
        }

        RendertargetDescriptor.m_NumberOfPixelsU  = 256;
        RendertargetDescriptor.m_NumberOfPixelsV  = 1;
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::ShaderResource;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::R32G32B32A32_FLOAT;
        RendertargetDescriptor.m_Usage            = CTexture::GPUReadWrite;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = pPermutationGradientData;

        m_GradientPermutationTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor);

		TextureManager::SetTextureLabel(m_GradientPermutationTexturePtr, "Fog Gradient Permutation");

        Base::CMemory::Free(pPermutationGradientData);

        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_NumberOfPixelsU  = 160;
        RendertargetDescriptor.m_NumberOfPixelsV  = 90;
        RendertargetDescriptor.m_NumberOfPixelsW  = 128;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::ShaderResource;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::R32G32B32A32_FLOAT;
        RendertargetDescriptor.m_Usage            = CTexture::GPUReadWrite;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;

        m_VolumeTexturePtr = TextureManager::CreateTexture3D(RendertargetDescriptor);

		TextureManager::SetTextureLabel(m_VolumeTexturePtr, "Fog Volume Texture");

        m_ScatteringTexturePtr = TextureManager::CreateTexture3D(RendertargetDescriptor);

		TextureManager::SetTextureLabel(m_VolumeTexturePtr, "Fog Scattering Texture");

        // -----------------------------------------------------------------------------

        m_VolumeTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(m_VolumeTexturePtr), static_cast<CTexturePtr>(m_PermutationTexturePtr), static_cast<CTexturePtr>(m_GradientPermutationTexturePtr), static_cast<CTexturePtr>(m_ESMTexturePtr));

        // -----------------------------------------------------------------------------

        m_ScatteringTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(m_VolumeTexturePtr), static_cast<CTexturePtr>(m_ScatteringTexturePtr));
        
        // -----------------------------------------------------------------------------

        m_BlurStagesTextureSetPtrs[0] = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(m_ESMTexturePtr), static_cast<CTexturePtr>(ESMSwapTexturePtr));
        m_BlurStagesTextureSetPtrs[1] = TextureManager::CreateTextureSet(static_cast<CTexturePtr>(ESMSwapTexturePtr), static_cast<CTexturePtr>(m_ESMTexturePtr));
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
        m_VolumeLightingCSBufferSetPtr         = BufferManager::CreateBufferSet(SunLightBufferPtr, VolumeLightingPropertiesBufferPtr);
        m_FogApplyBufferPtr                    = BufferManager::CreateBufferSet(FogApplyPropertiesBufferPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxFogRenderer::OnSetupModels()
    {
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
        // -----------------------------------------------------------------------------
        // Getting volume fog informations from render job
        // TODO: What happens if more then one DOF effect is available?
        // -----------------------------------------------------------------------------
        Gfx::CSun* pGfxSunFacet = m_VolumeFogRenderJobs[0].m_pGfxSunComponent;

        assert(pGfxSunFacet != 0);

        SGaussianShaderProperties GaussianSettings;

        GaussianSettings.m_MaxPixelCoord[0] = 256;
        GaussianSettings.m_MaxPixelCoord[1] = 256;
        GaussianSettings.m_Weights[0] = 0.018816f;
        GaussianSettings.m_Weights[1] = 0.034474f;
        GaussianSettings.m_Weights[2] = 0.056577f;
        GaussianSettings.m_Weights[3] = 0.083173f;
        GaussianSettings.m_Weights[4] = 0.109523f;
        GaussianSettings.m_Weights[5] = 0.129188f;
        GaussianSettings.m_Weights[6] = 0.136498f;

        Performance::BeginEvent("ESM");

        ContextManager::SetShaderCS(m_ESMCSPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        ContextManager::SetTexture(0, pGfxSunFacet->GetShadowMapPtr());

        ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_ESMTexturePtr));

        ContextManager::Dispatch(256, 256, 1);

        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetImageTexture(0);

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
        GaussianSettings.m_Direction[0] = 1;
        GaussianSettings.m_Direction[1] = 0;

        BufferManager::UploadBufferData(m_GaussianBlurPropertiesCSBufferSetPtr->GetBuffer(0), &GaussianSettings);

        ContextManager::SetShaderCS(m_GaussianBlurShaderPtr);

        ContextManager::SetResourceBuffer(0, m_GaussianBlurPropertiesCSBufferSetPtr->GetBuffer(0));

        ContextManager::SetImageTexture(0, m_BlurStagesTextureSetPtrs[0]->GetTexture(0));
        ContextManager::SetImageTexture(1, m_BlurStagesTextureSetPtrs[0]->GetTexture(1));

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

        BufferManager::UploadBufferData(m_GaussianBlurPropertiesCSBufferSetPtr->GetBuffer(0), &GaussianSettings);

        ContextManager::SetShaderCS(m_GaussianBlurShaderPtr);

        ContextManager::SetResourceBuffer(0, m_GaussianBlurPropertiesCSBufferSetPtr->GetBuffer(0));

        ContextManager::SetImageTexture(0, m_BlurStagesTextureSetPtrs[1]->GetTexture(0));
        ContextManager::SetImageTexture(1, m_BlurStagesTextureSetPtrs[1]->GetTexture(1));

        ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

        ContextManager::ResetImageTexture(0);
        ContextManager::ResetImageTexture(1);

        ContextManager::ResetResourceBuffer(0);

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
        Dt::CVolumeFogComponent* pDtVolumeFogComponent = m_VolumeFogRenderJobs[0].m_pDtVolumeFogComponent;
        Dt::CSunComponent*       pDtSunComponent       = m_VolumeFogRenderJobs[0].m_pDtSunComponent;
        Gfx::CSun*               pGfxSunComponent      = m_VolumeFogRenderJobs[0].m_pGfxSunComponent;

        assert(pDtVolumeFogComponent != 0 && pDtSunComponent != 0 && pGfxSunComponent != 0);

        SSunLightProperties LightBuffer;

        LightBuffer.m_LightViewProjection  = pGfxSunComponent->GetCamera()->GetViewProjectionMatrix();
        LightBuffer.m_LightDirection       = glm::normalize(glm::vec4(pDtSunComponent->GetDirection(), 0.0f));
        LightBuffer.m_LightColor           = glm::vec4(pDtSunComponent->GetLightness(), 1.0f);
        LightBuffer.m_SunAngularRadius     = 0.27f * glm::pi<float>() / 180.0f;
        LightBuffer.m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();

        BufferManager::UploadBufferData(m_VolumeLightingCSBufferSetPtr->GetBuffer(0), &LightBuffer);

        // -----------------------------------------------------------------------------

        SVolumeLightingProperties VolumeProperties;

        VolumeProperties.m_WindDirection                      = pDtVolumeFogComponent->GetWindDirection();
        VolumeProperties.m_FogColor                           = pDtVolumeFogComponent->GetFogColor();
        VolumeProperties.m_FrustumDepthInMeter                = pDtVolumeFogComponent->GetFrustumDepthInMeter();
        VolumeProperties.m_ShadowIntensity                    = pDtVolumeFogComponent->GetShadowIntensity();
        VolumeProperties.m_VolumetricFogScatteringCoefficient = pDtVolumeFogComponent->GetScatteringCoefficient();
        VolumeProperties.m_VolumetricFogAbsorptionCoefficient = pDtVolumeFogComponent->GetAbsorptionCoefficient();
        VolumeProperties.m_DensityLevel                       = pDtVolumeFogComponent->GetDensityLevel();
        VolumeProperties.m_DensityAttenuation                 = pDtVolumeFogComponent->GetDensityAttenuation();

        BufferManager::UploadBufferData(m_VolumeLightingCSBufferSetPtr->GetBuffer(1), &VolumeProperties);

        // -----------------------------------------------------------------------------

        ContextManager::SetShaderCS(m_VolumeLightingCSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_VolumeLightingCSBufferSetPtr->GetBuffer(0));
        ContextManager::SetConstantBuffer(2, m_VolumeLightingCSBufferSetPtr->GetBuffer(1));

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

        ContextManager::SetImageTexture(0, m_VolumeTextureSetPtr->GetTexture(0));
        ContextManager::SetImageTexture(1, m_VolumeTextureSetPtr->GetTexture(1));
        ContextManager::SetImageTexture(2, m_VolumeTextureSetPtr->GetTexture(2));
        ContextManager::SetImageTexture(3, m_VolumeTextureSetPtr->GetTexture(3));
        ContextManager::SetImageTexture(4, m_VolumeTextureSetPtr->GetTexture(4));

        unsigned int NumberOfThreadGroupsX;
        unsigned int NumberOfThreadGroupsY;
        unsigned int NumberOfThreadGroupsZ;

        unsigned int s_TileSizeX = 16;
        unsigned int s_TileSizeY = 10;
        unsigned int s_TileSizeZ = 1;

        NumberOfThreadGroupsX = (160 + s_TileSizeX - 1) / (s_TileSizeX);
        NumberOfThreadGroupsY = ( 90 + s_TileSizeY - 1) / (s_TileSizeY);
        NumberOfThreadGroupsZ = (128 + s_TileSizeZ - 1) / (s_TileSizeZ);

        ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, NumberOfThreadGroupsZ);

        ContextManager::ResetImageTexture(0);
        ContextManager::ResetImageTexture(1);
        ContextManager::ResetImageTexture(2);
        ContextManager::ResetImageTexture(3);
        ContextManager::ResetImageTexture(4);
        
        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);
        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetShaderCS();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxFogRenderer::RenderScattering()
    {
        Performance::BeginEvent("Scattering");

        ContextManager::SetShaderCS(m_VolumeScatteringCSPtr);

        ContextManager::SetImageTexture(0, m_ScatteringTextureSetPtr->GetTexture(0));
        ContextManager::SetImageTexture(1, m_ScatteringTextureSetPtr->GetTexture(1));

        unsigned int NumberOfThreadGroupsX;
        unsigned int NumberOfThreadGroupsY;

        unsigned int s_TileSizeX = 16;
        unsigned int s_TileSizeY = 10;

        NumberOfThreadGroupsX = (160 + s_TileSizeX - 1) / (s_TileSizeX);
        NumberOfThreadGroupsY = ( 90 + s_TileSizeY - 1) / (s_TileSizeY);

        ContextManager::Dispatch(NumberOfThreadGroupsX, NumberOfThreadGroupsY, 1);

        ContextManager::ResetImageTexture(0);
        ContextManager::ResetImageTexture(1);

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
        Dt::CVolumeFogComponent* pDataVolumeFogFacet = m_VolumeFogRenderJobs[0].m_pDtVolumeFogComponent;

        assert(pDataVolumeFogFacet != 0);

        // -----------------------------------------------------------------------------
        // Data
        // -----------------------------------------------------------------------------
        SFogApplyProperties FogApplyProperties;

        FogApplyProperties.m_FrustumDepthInMeter = pDataVolumeFogFacet->GetFrustumDepthInMeter();

        BufferManager::UploadBufferData(m_FogApplyBufferPtr->GetBuffer(0), &FogApplyProperties);

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        

        ContextManager::SetRenderContext(m_LightRenderContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

        ContextManager::SetShaderPS(m_ApplyPSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_FogApplyBufferPtr->GetBuffer(0));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0));
        ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());
        ContextManager::SetTexture(2, static_cast<CTexturePtr>(m_ScatteringTexturePtr));

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

    void CGfxFogRenderer::BuildRenderJobs()
    {
        m_VolumeFogRenderJobs.clear();

        auto DataVolumeFogComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CVolumeFogComponent>();

        for (auto VolumeFogComponent : DataVolumeFogComponents)
        {
            Dt::CVolumeFogComponent* pDtComponent = static_cast<Dt::CVolumeFogComponent*>(VolumeFogComponent);

            if (!(pDtComponent->IsActive() && pDtComponent->GetHostEntity()->IsActive())) continue;

            const Dt::CEntity& rCurrentEntity = *pDtComponent->GetHostEntity();

            // -----------------------------------------------------------------------------
            // Looking for sun
            // -----------------------------------------------------------------------------
            const Dt::CSunComponent*  pDtSunComponent  = 0;
            const Gfx::CSun* pGfxSunComponent = 0;

            if (rCurrentEntity.GetComponentFacet()->HasComponent<Dt::CSunComponent>())
            {
                pDtSunComponent = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CSunComponent>();
            }
            else
            {
                auto DataSunComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CSunComponent>();

                for (auto VolumeSunComponent : DataSunComponents)
                {
                    pDtSunComponent = static_cast<Dt::CSunComponent*>(VolumeSunComponent);
                }
            }

            if (pDtSunComponent == nullptr) continue;

            pGfxSunComponent = static_cast<const Gfx::CSun*>(pDtSunComponent->GetFacet(Dt::CSunComponent::Graphic));

            // -----------------------------------------------------------------------------
            // Build job
            // -----------------------------------------------------------------------------
            SVolumeFogRenderJob NewRenderJob;

            NewRenderJob.m_pDtVolumeFogComponent = pDtComponent;
            NewRenderJob.m_pGfxSunComponent      = const_cast<Gfx::CSun*>(pGfxSunComponent);
            NewRenderJob.m_pDtSunComponent       = const_cast<Dt::CSunComponent*>(pDtSunComponent);

            m_VolumeFogRenderJobs.push_back(NewRenderJob);
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

