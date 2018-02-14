
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_component_facet.h"
#include "data/data_entity.h"
#include "data/data_light_probe_component.h"
#include "data/data_map.h"
#include "data/data_ssr_component.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_component_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_reflection_renderer.h"
#include "graphic/gfx_light_probe_component.h"
#include "graphic/gfx_light_probe_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sun_component.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
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

        void OnResize(unsigned int _Width, unsigned int _Height);
        
        void Update();
        void Render();

        CTexturePtr GetBRDF();

    private:

        static const unsigned int s_MaxNumberOfProbes = 4;
        
    private:

        struct SLightProbeRenderJob
        {
            CTexturePtr m_Texture0Ptr;
            CTexturePtr m_Texture1Ptr;
            CTexturePtr m_Texture2Ptr;
        };

        struct SSSRRenderJob
        {
            Dt::CSSRComponent* m_pDataSSRFacet;
        };
        
        struct SPerDrawCallConstantBuffer
        {
            glm::mat4 m_ModelMatrix;
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
            glm::vec4 m_UVBoundaries;
            glm::vec2 m_InverseTextureSize;
            float        m_MipmapLevel;
        };

        struct SProbePropertiesBuffer
        {
            glm::mat4 m_WorldToProbeLS;
            glm::vec4   m_ProbePosition;
            glm::vec4   m_UnitaryBox;
            glm::vec4   m_LightSettings;
            unsigned int   m_LightType;
            unsigned int   m_Padding0;
            unsigned int   m_Padding1;
            unsigned int   m_Padding2;
        };
        
        struct SIBLSettings
        {
            glm::vec4 m_IBLSettings;
        };

    private:

        typedef std::vector<SLightProbeRenderJob> CLightProbeRenderJobs;
        typedef std::vector<SSSRRenderJob>        CSSRRenderJobs;
        
    private:
        
        CMeshPtr          m_QuadModelPtr;

        CBufferPtr        m_ProbePropertiesBufferPtr;
        
        CBufferPtr        m_ImageLightBufferPtr;
                          
        CBufferPtr        m_SSRLightBufferPtr;
                          
        CBufferPtr        m_HCBBufferPtr;
        
        CInputLayoutPtr   m_QuadInputLayoutPtr;
        
        CShaderPtr        m_RectangleShaderVSPtr;
        
        CShaderPtr        m_ImageLightShaderPSPtr;

        CShaderPtr        m_SSRShaderPSPtr;

        CShaderPtr        m_HCBShaderPSPtr;

        CShaderPtr        m_BRDFShaderPtr;
        
        CTexturePtr     m_BRDFTexture2DPtr;

        CTexturePtr     m_HCBTexture2DPtr;

        CTargetSetPtr     m_SSRTargetSetPtr;
        
        CRenderContextPtr m_LightAccumulationRenderContextPtr;

        std::vector<CTexturePtr>   m_HCBTexturePtrs;

        std::vector<CTargetSetPtr>     m_HCBTargetSetPtrs;

        std::vector<CViewPortSetPtr>   m_HCBViewPortSetPtrs;

        CLightProbeRenderJobs m_LightProbeRenderJobs;
        CSSRRenderJobs        m_SSRRenderJobs;
        
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
        , m_ProbePropertiesBufferPtr         ()
        , m_ImageLightBufferPtr              ()
        , m_SSRLightBufferPtr                ()
        , m_HCBBufferPtr                     ()
        , m_QuadInputLayoutPtr               ()
        , m_RectangleShaderVSPtr             ()
        , m_ImageLightShaderPSPtr            ()
        , m_SSRShaderPSPtr                   ()
        , m_HCBShaderPSPtr                   ()
        , m_BRDFShaderPtr                    ()
        , m_BRDFTexture2DPtr                 ()
        , m_HCBTexture2DPtr                  ()
        , m_SSRTargetSetPtr                  ()
        , m_LightAccumulationRenderContextPtr()
        , m_LightProbeRenderJobs             ()
        , m_SSRRenderJobs                    ()
    {
        m_LightProbeRenderJobs.reserve(1);
        m_SSRRenderJobs       .reserve(1);

        // -----------------------------------------------------------------------------
        // Register for resizing events
        // -----------------------------------------------------------------------------
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxReflectionRenderer::OnResize));
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
        m_ProbePropertiesBufferPtr          = 0;
        m_ImageLightBufferPtr               = 0;
        m_SSRLightBufferPtr                 = 0;
        m_HCBBufferPtr                      = 0;
        m_QuadInputLayoutPtr                = 0;
        m_RectangleShaderVSPtr              = 0;
        m_ImageLightShaderPSPtr             = 0;
        m_SSRShaderPSPtr                    = 0;
        m_HCBShaderPSPtr                    = 0;
        m_BRDFShaderPtr                     = 0;
        m_BRDFTexture2DPtr                  = 0;
        m_HCBTexture2DPtr                   = 0;
        m_SSRTargetSetPtr                   = 0;
        m_LightAccumulationRenderContextPtr = 0;

        for (unsigned int IndexOfElement = 0; IndexOfElement < m_HCBTexturePtrs.size(); ++IndexOfElement)
        {
            m_HCBTexturePtrs    [IndexOfElement] = 0;
            m_HCBTargetSetPtrs  [IndexOfElement] = 0;
            m_HCBViewPortSetPtrs[IndexOfElement] = 0;
        }

        m_HCBTexturePtrs    .clear();
        m_HCBTargetSetPtrs  .clear();
        m_HCBViewPortSetPtrs.clear();

        m_LightProbeRenderJobs.clear();
        m_SSRRenderJobs       .clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnSetupShader()
    {
        m_RectangleShaderVSPtr  = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");
        
        m_ImageLightShaderPSPtr = ShaderManager::CompilePS("fs_light_imagelight.glsl" , "main");

        m_SSRShaderPSPtr        = ShaderManager::CompilePS("fs_ssr.glsl", "main");

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
        
        CTexturePtr SSRTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // SSR

		TextureManager::SetTextureLabel(SSRTexturePtr, "Screen Space Reflection Target");

        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::RenderTarget | CTexture::ShaderResource;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::Unknown;
        RendertargetDescriptor.m_Usage            = CTexture::GPURead;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        RendertargetDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;
        
        m_HCBTexture2DPtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // HCB

		TextureManager::SetTextureLabel(m_HCBTexture2DPtr, "Hierarchical Color Buffer Target");

        TextureManager::UpdateMipmap(m_HCBTexture2DPtr);

        // -----------------------------------------------------------------------------
        // Create target set
        // -----------------------------------------------------------------------------
        m_SSRTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(SSRTexturePtr));

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
            CTexturePtr MipmapTexture = TextureManager::GetMipmapFromTexture2D(m_HCBTexture2DPtr, IndexOfMipmap);

            CTargetSetPtr MipmapTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(MipmapTexture));

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
        
        CRenderStatePtr     LightStatePtr      = StateManager::GetRenderState(CRenderState::AdditionBlend);

        // -----------------------------------------------------------------------------
        
        m_LightAccumulationRenderContextPtr = ContextManager::CreateRenderContext();
        
        m_LightAccumulationRenderContextPtr->SetCamera(MainCameraPtr);
        m_LightAccumulationRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_LightAccumulationRenderContextPtr->SetTargetSet(LightTargetSetPtr);
        m_LightAccumulationRenderContextPtr->SetRenderState(LightStatePtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::OnSetupTextures()
    {
        STextureDescriptor TextureDescriptor;
        
        TextureDescriptor.m_NumberOfPixelsU  = 512;
        TextureDescriptor.m_NumberOfPixelsV  = 512;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::R32G32B32A32_FLOAT;
        TextureDescriptor.m_Usage            = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        
        m_BRDFTexture2DPtr = TextureManager::CreateTexture2D(TextureDescriptor);

		TextureManager::SetTextureLabel(m_BRDFTexture2DPtr, "BRDF");
        
        // -----------------------------------------------------------------------------

        m_HCBTexturePtrs.push_back(TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0));

        for (unsigned int IndexOfMipmap = 1; IndexOfMipmap < m_HCBTexture2DPtr->GetNumberOfMipLevels(); ++IndexOfMipmap)
        {
            m_HCBTexturePtrs.push_back(m_HCBTargetSetPtrs[IndexOfMipmap - 1]->GetRenderTarget(0));
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
        
        m_SSRLightBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SHCBProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_HCBBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SIBLSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_ImageLightBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SProbePropertiesBuffer) * s_MaxNumberOfProbes;
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_ProbePropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
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

        ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_BRDFTexture2DPtr));

        ContextManager::Dispatch(64, 64, 1);

        ContextManager::ResetImageTexture(0);

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

    void CGfxReflectionRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {
        // -----------------------------------------------------------------------------
        // Clear old sets
        // -----------------------------------------------------------------------------

        for (unsigned int IndexOfElement = 0; IndexOfElement < m_HCBTexturePtrs.size(); ++IndexOfElement)
        {
            m_HCBTexturePtrs    [IndexOfElement] = 0;
            m_HCBTargetSetPtrs  [IndexOfElement] = 0;
            m_HCBViewPortSetPtrs[IndexOfElement] = 0;
        }

        m_HCBTexturePtrs    .clear();
        m_HCBTargetSetPtrs  .clear();
        m_HCBViewPortSetPtrs.clear();

        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        glm::ivec2 Size(_Width, _Height);

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
        
        CTexturePtr SSRTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // SSR

		TextureManager::SetTextureLabel(SSRTexturePtr, "Screen Space Reflection Target");

        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::RenderTarget | CTexture::ShaderResource;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::Unknown;
        RendertargetDescriptor.m_Usage            = CTexture::GPURead;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        RendertargetDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;
        
        m_HCBTexture2DPtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // HCB

		TextureManager::SetTextureLabel(m_HCBTexture2DPtr, "Hierarchical Color Buffer Target");

        TextureManager::UpdateMipmap(m_HCBTexture2DPtr);

        // -----------------------------------------------------------------------------
        // Create target set
        // -----------------------------------------------------------------------------
        m_SSRTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(SSRTexturePtr));

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
            CTexturePtr MipmapTexture = TextureManager::GetMipmapFromTexture2D(m_HCBTexture2DPtr, IndexOfMipmap);

            CTargetSetPtr MipmapTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(MipmapTexture));

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

        // -----------------------------------------------------------------------------

        m_HCBTexturePtrs.push_back(TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0));

        for (unsigned int IndexOfMipmap = 1; IndexOfMipmap < m_HCBTexture2DPtr->GetNumberOfMipLevels(); ++IndexOfMipmap)
        {
            m_HCBTexturePtrs.push_back(m_HCBTargetSetPtrs[IndexOfMipmap - 1]->GetRenderTarget(0));
        }
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

    CTexturePtr CGfxReflectionRenderer::GetBRDF()
    {
        return static_cast<CTexturePtr>(m_BRDFTexture2DPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReflectionRenderer::RenderIBL()
    {
        if (m_LightProbeRenderJobs.size() == 0 && m_SSRRenderJobs.size() == 0) return;

        Performance::BeginEvent("IBL");

        

        // -----------------------------------------------------------------------------
        // IBL data
        // -----------------------------------------------------------------------------
        SIBLSettings IBLSettings;

        IBLSettings.m_IBLSettings    = glm::vec4(0.0f);
        IBLSettings.m_IBLSettings[0] = m_SSRRenderJobs.size() > 0 ? 1.0f : 0.0f;

        BufferManager::UploadBufferData(m_ImageLightBufferPtr, &IBLSettings);

        // -----------------------------------------------------------------------------
        // Bind shadow and reflection textures
        // -----------------------------------------------------------------------------
        CLightProbeRenderJobs::const_iterator CurrentLightJob = m_LightProbeRenderJobs.begin();
        CLightProbeRenderJobs::const_iterator EndOfLightJobs  = m_LightProbeRenderJobs.end();

        unsigned int IndexOfSpecularCubemap = 6;
        unsigned int IndexOfDiffuseCubemap  = IndexOfSpecularCubemap + s_MaxNumberOfProbes;
        unsigned int IndexOfShadowCubemap   = IndexOfDiffuseCubemap  + s_MaxNumberOfProbes;

        for (; CurrentLightJob != EndOfLightJobs; ++ CurrentLightJob)
        {
            const SLightProbeRenderJob& rJob = *CurrentLightJob;

            ContextManager::SetSampler(IndexOfSpecularCubemap, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
            ContextManager::SetSampler(IndexOfDiffuseCubemap, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
            ContextManager::SetSampler(IndexOfShadowCubemap, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

            ContextManager::SetTexture(IndexOfSpecularCubemap, rJob.m_Texture0Ptr);
            ContextManager::SetTexture(IndexOfDiffuseCubemap, rJob.m_Texture1Ptr);
            ContextManager::SetTexture(IndexOfShadowCubemap, rJob.m_Texture2Ptr);

            ++ IndexOfSpecularCubemap;
            ++ IndexOfDiffuseCubemap;
            ++ IndexOfShadowCubemap;
        }

        // -----------------------------------------------------------------------------
        // Render probes together with SSR result
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_LightAccumulationRenderContextPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);
        
        ContextManager::SetShaderPS(m_ImageLightShaderPSPtr);
        
        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_ImageLightBufferPtr);

        ContextManager::SetResourceBuffer(0, m_ProbePropertiesBufferPtr);
        
        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(4, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(5, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
        ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
        ContextManager::SetTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2));
        ContextManager::SetTexture(3, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());
        ContextManager::SetTexture(4, static_cast<CTexturePtr>(m_BRDFTexture2DPtr));
        ContextManager::SetTexture(5, m_SSRTargetSetPtr->GetRenderTarget(0));
            
        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);
        ContextManager::ResetTexture(2);
        ContextManager::ResetTexture(3);
        ContextManager::ResetTexture(4);
        ContextManager::ResetTexture(5);
        ContextManager::ResetTexture(6);
        ContextManager::ResetTexture(7);
        ContextManager::ResetTexture(8);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        ContextManager::ResetSampler(2);
        ContextManager::ResetSampler(3);
        ContextManager::ResetSampler(4);
        ContextManager::ResetSampler(5);
        ContextManager::ResetSampler(6);
        ContextManager::ResetSampler(7);
        ContextManager::ResetSampler(8);
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetResourceBuffer(0);
        ContextManager::ResetResourceBuffer(1);
        
        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBuffer();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetTopology();
        
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReflectionRenderer::RenderHCB()
    {
        if (m_SSRRenderJobs.size() == 0) return;

        Performance::BeginEvent("HCB");

        

        ContextManager::SetBlendState(StateManager::GetBlendState(0));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(0));

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

        ContextManager::SetShaderPS(m_HCBShaderPSPtr);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_HCBBufferPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        ContextManager::SetTexture(0, TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0));

        for (unsigned int IndexOfMipmap = 0; IndexOfMipmap < m_HCBTexture2DPtr->GetNumberOfMipLevels(); ++ IndexOfMipmap)
        {
            SHCBProperties HCBProperties;

            HCBProperties.m_UVBoundaries       = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            HCBProperties.m_InverseTextureSize = glm::vec2(1.0f, 1.0f);
            HCBProperties.m_MipmapLevel        = static_cast<float>(IndexOfMipmap);

            if (IndexOfMipmap > 0)
            {
                HCBProperties.m_InverseTextureSize = glm::vec2(1.0f / m_HCBViewPortSetPtrs[IndexOfMipmap - 1]->GetViewPorts()[0]->GetWidth(), 1.0f / m_HCBViewPortSetPtrs[IndexOfMipmap - 1]->GetViewPorts()[0]->GetHeight());
            }

            BufferManager::UploadBufferData(m_HCBBufferPtr, &HCBProperties);

            // -----------------------------------------------------------------------------

            ContextManager::SetTargetSet(m_HCBTargetSetPtrs[IndexOfMipmap]);

            ContextManager::SetViewPortSet(m_HCBViewPortSetPtrs[IndexOfMipmap]);

            ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        }

        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetInputLayout();

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReflectionRenderer::RenderScreenSpaceReflections()
    {
        if (m_SSRRenderJobs.size() == 0) return;

        Performance::BeginEvent("SSR");

        // TODO: What happens if more then one SSR effect is available?
        Dt::CSSRComponent* pDataSSRFacet = m_SSRRenderJobs[0].m_pDataSSRFacet;

        assert(pDataSSRFacet != 0);

        // -----------------------------------------------------------------------------
        // Upload dynamic data
        // -----------------------------------------------------------------------------
        CCameraPtr CameraPtr = ViewManager::GetMainCamera();

        SSSRProperties SSRProperties;

        glm::vec3 Position = CameraPtr->GetView()->GetPosition();

        SSRProperties.m_SSRIntesity          = pDataSSRFacet->GetIntensity();
        SSRProperties.m_SSRRougnessMaskScale = pDataSSRFacet->GetRoughnessMask();
        SSRProperties.m_SSRDistance          = pDataSSRFacet->GetDistance();
        SSRProperties.m_PreviousFrame        = pDataSSRFacet->GetUseLastFrame() ? 1.0f : 0.0f;

        BufferManager::UploadBufferData(m_SSRLightBufferPtr, &SSRProperties);

        // -----------------------------------------------------------------------------
        // Screen Space Reflections
        // -----------------------------------------------------------------------------
        

        ContextManager::SetBlendState(StateManager::GetBlendState(0));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(0));

        ContextManager::SetTargetSet(m_SSRTargetSetPtr);

        ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

        ContextManager::SetShaderPS(m_SSRShaderPSPtr);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_SSRLightBufferPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
        ContextManager::SetSampler(4, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(5, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
        ContextManager::SetTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
        ContextManager::SetTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2));
        ContextManager::SetTexture(3, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());
        ContextManager::SetTexture(4, static_cast<CTexturePtr>(m_HCBTexture2DPtr));
        ContextManager::SetTexture(5, static_cast<CTexturePtr>(m_BRDFTexture2DPtr));

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);
        ContextManager::ResetTexture(2);
        ContextManager::ResetTexture(3);
        ContextManager::ResetTexture(4);
        ContextManager::ResetTexture(5);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);
        ContextManager::ResetSampler(2);
        ContextManager::ResetSampler(3);
        ContextManager::ResetSampler(4);
        ContextManager::ResetSampler(5);

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReflectionRenderer::BuildRenderJobs()
    {
        SProbePropertiesBuffer LightBuffer[s_MaxNumberOfProbes];
        unsigned int           IndexOfLight;

        // -----------------------------------------------------------------------------
        // Initialize buffer
        // -----------------------------------------------------------------------------
        IndexOfLight = 0;

        for (; IndexOfLight < s_MaxNumberOfProbes; ++ IndexOfLight)
        {
            LightBuffer[IndexOfLight].m_LightType      = 0;
            LightBuffer[IndexOfLight].m_WorldToProbeLS = glm::mat4(1.0f);
            LightBuffer[IndexOfLight].m_ProbePosition  = glm::vec4(0.0f);
            LightBuffer[IndexOfLight].m_UnitaryBox     = glm::vec4(1.0f);
            LightBuffer[IndexOfLight].m_LightSettings  = glm::vec4(0.0f);
        }

        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_LightProbeRenderJobs.clear();
        m_SSRRenderJobs       .clear();

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentLightEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Dynamic);
        Dt::Map::CEntityIterator EndOfLightEntities = Dt::Map::EntitiesEnd();

        for (IndexOfLight = 0; CurrentLightEntity != EndOfLightEntities && IndexOfLight < s_MaxNumberOfProbes; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentLightEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetComponentFacet()->HasComponent<Dt::CLightProbeComponent>())
            {
                Dt::CLightProbeComponent*  pDataLightProbeFacet    = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CLightProbeComponent>();
                Gfx::CLightProbeComponent* pGraphicLightProbeFacet = Gfx::CComponentManager::GetInstance().GetComponent<Gfx::CLightProbeComponent>(pDataLightProbeFacet->GetID());

                assert(pDataLightProbeFacet != 0 && pGraphicLightProbeFacet != 0);

                // -----------------------------------------------------------------------------
                // Fill data
                // -----------------------------------------------------------------------------
                LightBuffer[IndexOfLight].m_LightType        = static_cast<int>(pDataLightProbeFacet->GetType()) + 1;
                LightBuffer[IndexOfLight].m_WorldToProbeLS   = glm::inverse(rCurrentEntity.GetTransformationFacet()->GetWorldMatrix());
                LightBuffer[IndexOfLight].m_ProbePosition    = glm::vec4(rCurrentEntity.GetWorldPosition(), 1.0f);
                LightBuffer[IndexOfLight].m_UnitaryBox       = glm::vec4(pDataLightProbeFacet->GetBoxSize(), 0.0f);
                LightBuffer[IndexOfLight].m_LightSettings[0] = static_cast<float>(pGraphicLightProbeFacet->GetSpecularPtr()->GetNumberOfMipLevels() - 1);
                LightBuffer[IndexOfLight].m_LightSettings[1] = pDataLightProbeFacet->GetParallaxCorrection() == true ? 1.0f : 0.0f;
                LightBuffer[IndexOfLight].m_LightSettings[2] = 0.0f;
                LightBuffer[IndexOfLight].m_LightSettings[3] = 0.0f;

                ++IndexOfLight;

                // -----------------------------------------------------------------------------
                // Set probe into a new render job
                // -----------------------------------------------------------------------------
                SLightProbeRenderJob NewRenderJob;

                NewRenderJob.m_Texture0Ptr = pGraphicLightProbeFacet->GetSpecularPtr();
                NewRenderJob.m_Texture1Ptr = pGraphicLightProbeFacet->GetDiffusePtr();
                NewRenderJob.m_Texture2Ptr = pGraphicLightProbeFacet->GetDepthPtr();

                m_LightProbeRenderJobs.push_back(NewRenderJob);
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentLightEntity = CurrentLightEntity.Next(Dt::SEntityCategory::Dynamic);
        }

        BufferManager::UploadBufferData(m_ProbePropertiesBufferPtr, &LightBuffer);

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEffectEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Dynamic);
        Dt::Map::CEntityIterator EndOfEffectEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEffectEntity != EndOfEffectEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEffectEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetComponentFacet()->HasComponent<Dt::CSSRComponent>())
            {
                Dt::CSSRComponent* pDataSSRFacet = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CSSRComponent>();

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
            CurrentEffectEntity = CurrentEffectEntity.Next(Dt::SEntityCategory::Dynamic);
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

    // -----------------------------------------------------------------------------

    CTexturePtr GetBRDF()
    {
        return CGfxReflectionRenderer::GetInstance().GetBRDF();
    }
} // namespace ReflectionRenderer
} // namespace Gfx
