
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_time.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_map.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_sun_component.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_selection_renderer.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_sky.h"
#include "engine/graphic/gfx_sky_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    const float g_RadiusGround     = 6360.0f;
    const float g_RadiusAtmosphere = 6420.0f;

    const unsigned int g_TransmittanceWidth  = 256;
    const unsigned int g_TransmittanceHeight = 64;

    const unsigned int g_InscatterAltitude = 128;
    const unsigned int g_InscatterMu       = 128;         // view / zenith
    const unsigned int g_InscatterMuS      = 32;          // sun / zenith
    const unsigned int g_InscatterNu       = 8;           // view / sun
    const unsigned int g_InscatterWidth    = g_InscatterMuS * g_InscatterNu;
    const unsigned int g_InscatterHeight   = g_InscatterMu;
    const unsigned int g_InscatterDepth    = g_InscatterAltitude;

    const unsigned int g_IrradianceWidth  = 64;
    const unsigned int g_IrradianceHeight = 16;
}

namespace 
{
    class CGfxSkyManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxSkyManager)

    public:

        CGfxSkyManager();
        ~CGfxSkyManager();

    public:

        void OnStart();
        void OnExit();

        void Update();

    private:

        struct SRenderContext
        {
            CShaderPtr    m_VSPtr;
            CShaderPtr    m_GSPtr;
            CShaderPtr    m_PSPtr;
            CBufferSetPtr m_GSBufferSetPtr;
            CBufferSetPtr m_PSBufferSetPtr;
            CMeshPtr      m_MeshPtr;
        };

        struct SModelMatrixBuffer
        {
            glm::mat4 m_ModelMatrix;
        };

        struct SCubemapBufferGS
        {
            glm::mat4 m_CubeProjectionMatrix;
            glm::mat4 m_CubeViewMatrix[6];
        };

        struct SOutputBufferPS
        {
            float m_HDRFactor;
            float m_IsHDR;
        };

        struct SPrecomputeConstants
        {
            float m_TransmittanceWidth;
            float m_TransmittanceHeight;
            float m_IrradianceWidth;
            float m_IrradianceHeight;
            float m_InscatterMus;
            float m_InscatterMu;
            float m_InscatterNu;
            float m_InscatterAltitude;
        };

        struct SPSIrradianceK
        {
            float k;
            float Padding[3];
        };

        struct SGSLayer
        {
            unsigned int m_Layer;
            float Padding[3];
        };

        struct SPSLayerValues
        {
            glm::vec4 m_Dhdh;
            float m_Radius;
            float Padding[3];
        };

        struct SPSScatteringOrder
        {
            float m_FirstOrder;
            float Padding[3];
        };

        struct SPSPASSettings
        {
            glm::vec4 g_SunDirection;
            glm::vec4 g_SunIntensity;
            unsigned int ps_ExposureHistoryIndex;
        };

        class CInternSky : public CSky
        {
        public:

            CInternSky();
            ~CInternSky();

        public:

            CRenderContextPtr m_RenderContextPtr;
            CTargetSetPtr     m_TargetSetPtr;
            CViewPortSetPtr   m_ViewPortSetPtr;
            CTexturePtr       m_InputTexturePtr;

        private:

            friend class CGfxSkyManager;
        };

        private:

            typedef Base::CManagedPool<CInternSky, 1, 0> CSkies;

        private:

        CSkies m_Skies;

        SRenderContext    m_SkyboxFromAtmosphere;
        SRenderContext    m_SkyboxFromPanorama;
        SRenderContext    m_SkyboxFromCubemap;

        CBufferPtr m_PSPrecomputeConstants;

        CTexturePtr m_TransmittanceTable;
        CTexturePtr m_IrradianceTable;
        CTexturePtr m_InscatterTable;

    private:

        void OnDirtyComponent(Dt::IComponent* _pComponent);

        void RenderSkybox(Dt::CSkyComponent* _pDataSkyFacet, CInternSky* _pOutput);

        void RenderSkyboxFromAtmopsphericScattering(CInternSky* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromPanorama(CInternSky* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromCubemap(CInternSky* _pOutput, float _Intensity = 1.0f);

        void PrecomputeScattering();

        void ResetRenderContext();
    };
} // namespace 

namespace 
{
    CGfxSkyManager::CInternSky::CInternSky()
        : CSky              ()
        , m_RenderContextPtr()
        , m_TargetSetPtr    ()
        , m_ViewPortSetPtr  ()
        , m_InputTexturePtr ()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxSkyManager::CInternSky::~CInternSky()
    {
        m_RenderContextPtr   = 0;
        m_TargetSetPtr       = 0;
        m_ViewPortSetPtr     = 0;
        m_InputTexturePtr    = 0;
    }
} // namespace 

namespace 
{
    CGfxSkyManager::CGfxSkyManager()
        : m_Skies             ()
        , m_SkyboxFromPanorama()
        , m_SkyboxFromCubemap ()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxSkyManager::~CGfxSkyManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        CShaderPtr CubemapVSPtr           = ShaderManager::CompileVS("atmosphere/vs_cubemap_generation.glsl", "main");
        CShaderPtr CubemapGSPtr           = ShaderManager::CompileGS("atmosphere/gs_cubemap_generation.glsl", "main");
        CShaderPtr CubemapPanoramaPSPtr   = ShaderManager::CompilePS("atmosphere/fs_spherical_cubemap_generation.glsl", "main");
        CShaderPtr CubemapCubemapPSPtr    = ShaderManager::CompilePS("atmosphere/fs_cubemap_cubemap_generation.glsl", "main");
        CShaderPtr CubemapScatteringPSPtr = ShaderManager::CompilePS("scattering/scattering_post_effect.glsl", "main");

        m_SkyboxFromAtmosphere.m_VSPtr = CubemapVSPtr;
        m_SkyboxFromAtmosphere.m_GSPtr = CubemapGSPtr;
        m_SkyboxFromAtmosphere.m_PSPtr = CubemapScatteringPSPtr;

        m_SkyboxFromPanorama.m_VSPtr = CubemapVSPtr;
        m_SkyboxFromPanorama.m_GSPtr = CubemapGSPtr;
        m_SkyboxFromPanorama.m_PSPtr = CubemapPanoramaPSPtr;

        m_SkyboxFromCubemap.m_VSPtr = CubemapVSPtr;
        m_SkyboxFromCubemap.m_GSPtr = CubemapGSPtr;
        m_SkyboxFromCubemap.m_PSPtr = CubemapCubemapPSPtr;

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        SCubemapBufferGS DefaultGSValues;
        
        std::array<glm::vec3, 6> LookDirections = {
            glm::vec3(+1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
            glm::vec3( 0.0f, +1.0f,  0.0f),
            glm::vec3( 0.0f,  0.0f, -1.0f),
            glm::vec3( 0.0f,  0.0f, +1.0f),
        };

        std::array<glm::vec3, 6> UpDirections = {
            glm::vec3(0.0f, 1.0f,  0.0f),
            glm::vec3(0.0f, 1.0f,  0.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, +1.0f),
            glm::vec3(0.0f, 1.0f,  0.0f),
            glm::vec3(0.0f, 1.0f,  0.0f),
        };

        DefaultGSValues.m_CubeProjectionMatrix = glm::perspective(glm::half_pi<float>(), 1.0f, 0.1f, 1.0f);

        for (int IndexOfCubeface = 0; IndexOfCubeface < 6; ++IndexOfCubeface)
        {
            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface] = glm::lookAt(glm::vec3(0.0f), LookDirections[IndexOfCubeface], UpDirections[IndexOfCubeface]);

            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface] *= glm::eulerAngleX(glm::radians(90.0f));
        }

        // -----------------------------------------------------------------------------

        SBufferDescriptor ConstanteBufferDesc;
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCubemapBufferGS);
        ConstanteBufferDesc.m_pBytes        = &DefaultGSValues;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr CubemapGSWorldRotatedBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
               
        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SOutputBufferPS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr OuputPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SModelMatrixBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr ModelMatrixBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPSPASSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr PSPASSettings = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        m_SkyboxFromAtmosphere.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSWorldRotatedBuffer);
        m_SkyboxFromAtmosphere.m_PSBufferSetPtr = BufferManager::CreateBufferSet(PSPASSettings);

        m_SkyboxFromPanorama.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSWorldRotatedBuffer);
        m_SkyboxFromPanorama.m_PSBufferSetPtr = BufferManager::CreateBufferSet(OuputPSBufferPtr);

        m_SkyboxFromCubemap.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSWorldRotatedBuffer);
        m_SkyboxFromCubemap.m_PSBufferSetPtr = BufferManager::CreateBufferSet(OuputPSBufferPtr);

        // -----------------------------------------------------------------------------
        // Models
        // -----------------------------------------------------------------------------
        CMeshPtr CubemapTextureSpherePtr = MeshManager::CreateSphereIsometric(1.0f, 2);

        m_SkyboxFromAtmosphere.m_MeshPtr = CubemapTextureSpherePtr;

        m_SkyboxFromPanorama.m_MeshPtr = CubemapTextureSpherePtr;

        m_SkyboxFromCubemap.m_MeshPtr = CubemapTextureSpherePtr;

        // -----------------------------------------------------------------------------
        // Register dirty entity handler for automatic sky creation
        // -----------------------------------------------------------------------------
        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(DATA_DIRTY_COMPONENT_METHOD(&CGfxSkyManager::OnDirtyComponent));

        // -----------------------------------------------------------------------------
        // Precompute Scattering
        // -----------------------------------------------------------------------------
        PrecomputeScattering();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::OnExit()
    {
        m_Skies.Clear();

        ResetRenderContext();

        m_PSPrecomputeConstants = 0;

        m_TransmittanceTable = 0;
        m_InscatterTable     = 0;
        m_IrradianceTable    = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::Update()
    {
        auto DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CSkyComponent>();

        for (auto Component : DataComponents)
        {
            Dt::CSkyComponent* pDataSkyboxFacet = static_cast<Dt::CSkyComponent*>(Component);

            assert(pDataSkyboxFacet->GetHostEntity());

            if (!pDataSkyboxFacet->IsActive()) continue;

            if (pDataSkyboxFacet->GetRefreshMode() == Dt::CSkyComponent::Dynamic)
            {
                CInternSky* pGraphicSkyboxFacet = static_cast<CInternSky*>(pDataSkyboxFacet->GetFacet(Dt::CSkyComponent::Graphic));

                RenderSkybox(pDataSkyboxFacet, pGraphicSkyboxFacet);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        if (_pComponent->GetTypeID() != Base::CTypeInfo::GetTypeID<Dt::CSkyComponent>()) return;

        Dt::CSkyComponent* pSkyComponent = static_cast<Dt::CSkyComponent*>(_pComponent);

        // -----------------------------------------------------------------------------
        // Lamda function
        // -----------------------------------------------------------------------------
        auto UpdateCubemap = [&](Dt::CSkyComponent* _pSkyComponent, CInternSky* _pInternSky)->void
        {
            BASE_UNUSED(_pSkyComponent);

            // -----------------------------------------------------------------------------
            // Reset
            // -----------------------------------------------------------------------------
            _pInternSky->m_RenderContextPtr->SetViewPortSet(0);
            _pInternSky->m_RenderContextPtr->SetTargetSet(0);
            _pInternSky->m_ViewPortSetPtr = 0;
            _pInternSky->m_TargetSetPtr = 0;
            _pInternSky->m_CubemapPtr = 0;

            // -----------------------------------------------------------------------------
            // Create new environment map
            // -----------------------------------------------------------------------------
            STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU  = pSkyComponent->GetQualityInPixel();
            TextureDescriptor.m_NumberOfPixelsV  = pSkyComponent->GetQualityInPixel();
            TextureDescriptor.m_NumberOfPixelsW  = 1;
            TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
            TextureDescriptor.m_NumberOfTextures = 6;
            TextureDescriptor.m_Binding          = CTexture::ShaderResource | CTexture::RenderTarget;
            TextureDescriptor.m_Access           = CTexture::CPUWrite;
            TextureDescriptor.m_Format           = CTexture::Unknown;
            TextureDescriptor.m_Usage            = CTexture::GPURead;
            TextureDescriptor.m_Semantic         = CTexture::Diffuse;
            TextureDescriptor.m_pFileName        = 0;
            TextureDescriptor.m_pPixels          = 0;
            TextureDescriptor.m_Format           = CTexture::R16G16B16_FLOAT;
        
            _pInternSky->m_CubemapPtr = TextureManager::CreateCubeTexture(TextureDescriptor);

            TextureManager::SetTextureLabel(_pInternSky->m_CubemapPtr, "Sky Texture");

            // -----------------------------------------------------------------------------
            // Target Set
            // -----------------------------------------------------------------------------
            CTexturePtr FirstMipmapCubeTexture = TextureManager::GetMipmapFromTexture2D(_pInternSky->m_CubemapPtr, 0);

            _pInternSky->m_TargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(FirstMipmapCubeTexture));

            // -----------------------------------------------------------------------------
            // Viewport
            // -----------------------------------------------------------------------------
            SViewPortDescriptor ViewPortDesc;

            ViewPortDesc.m_TopLeftX = 0.0f;
            ViewPortDesc.m_TopLeftY = 0.0f;
            ViewPortDesc.m_MinDepth = 0.0f;
            ViewPortDesc.m_MaxDepth = 1.0f;

            ViewPortDesc.m_Width = static_cast<float>(FirstMipmapCubeTexture->GetNumberOfPixelsU());
            ViewPortDesc.m_Height = static_cast<float>(FirstMipmapCubeTexture->GetNumberOfPixelsV());

            CViewPortPtr MipMapViewPort = ViewManager::CreateViewPort(ViewPortDesc);

            _pInternSky->m_ViewPortSetPtr = ViewManager::CreateViewPortSet(MipMapViewPort);

            // -----------------------------------------------------------------------------
            // Render context
            // -----------------------------------------------------------------------------
            _pInternSky->m_RenderContextPtr->SetViewPortSet(_pInternSky->m_ViewPortSetPtr);
            _pInternSky->m_RenderContextPtr->SetTargetSet(_pInternSky->m_TargetSetPtr);
        };

        auto UpdateContent = [&](Dt::CSkyComponent* _pSkyComponent, CInternSky* _pGfxSkybox)->void
        {
            _pGfxSkybox->m_InputTexturePtr = 0;

            if (_pSkyComponent->HasTexture())
            {
                _pGfxSkybox->m_InputTexturePtr = _pSkyComponent->GetTexture();
            }

            // -----------------------------------------------------------------------------
            // Render sky because of the change
            // -----------------------------------------------------------------------------
            RenderSkybox(_pSkyComponent, _pGfxSkybox);
        };

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pSkyComponent->GetDirtyFlags();

        if ((DirtyFlags & Dt::CSkyComponent::DirtyCreate))
        {
            // -----------------------------------------------------------------------------
            // Create facet
            // -----------------------------------------------------------------------------
            CInternSky* pInternSky = m_Skies.Allocate();

            // -----------------------------------------------------------------------------
            // Render context
            // -----------------------------------------------------------------------------
            CCameraPtr      CameraPtr = ViewManager::GetMainCamera();
            CRenderStatePtr NoDepthStatePtr = StateManager::GetRenderState(CRenderState::NoDepth | CRenderState::NoCull | CRenderState::AlphaBlend);

            CRenderContextPtr CubemapRenderContextPtr = ContextManager::CreateRenderContext();

            CubemapRenderContextPtr->SetCamera(CameraPtr);
            CubemapRenderContextPtr->SetRenderState(NoDepthStatePtr);

            pInternSky->m_RenderContextPtr = CubemapRenderContextPtr;

            // -----------------------------------------------------------------------------
            // Cubemap
            // -----------------------------------------------------------------------------
            UpdateCubemap(pSkyComponent, pInternSky);

            // -----------------------------------------------------------------------------
            // Content
            // -----------------------------------------------------------------------------
            UpdateContent(pSkyComponent, pInternSky);

            // -----------------------------------------------------------------------------
            // Link to component
            // -----------------------------------------------------------------------------
            pSkyComponent->SetFacet(Dt::CSkyComponent::Graphic, pInternSky);

            pInternSky->m_TimeStamp = Core::Time::GetNumberOfFrame() + 1;
        }
        else if ((DirtyFlags & Dt::CSkyComponent::DirtyInfo))
        {
            CInternSky* pInternSky = static_cast<CInternSky*>(pSkyComponent->GetFacet(Dt::CSkyComponent::Graphic));

            // -----------------------------------------------------------------------------
            // Check cubemap quality
            // -----------------------------------------------------------------------------
            if (pInternSky->m_CubemapPtr->GetNumberOfPixelsU() != pSkyComponent->GetQualityInPixel())
            {
                UpdateCubemap(pSkyComponent, pInternSky);
            }

            // -----------------------------------------------------------------------------
            // Content
            // -----------------------------------------------------------------------------
            UpdateContent(pSkyComponent, pInternSky);

            // -----------------------------------------------------------------------------
            // Set time
            // -----------------------------------------------------------------------------
            pInternSky->m_TimeStamp = Core::Time::GetNumberOfFrame() + 1;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::RenderSkybox(Dt::CSkyComponent* _pDataSkyFacet, CInternSky* _pOutput)
    {
        switch (_pDataSkyFacet->GetType())
        {
            case Dt::CSkyComponent::Procedural: RenderSkyboxFromAtmopsphericScattering(_pOutput, _pDataSkyFacet->GetIntensity()); break;
            case Dt::CSkyComponent::Panorama:   RenderSkyboxFromPanorama(_pOutput, _pDataSkyFacet->GetIntensity()); break;
            case Dt::CSkyComponent::Cubemap:    RenderSkyboxFromCubemap(_pOutput, _pDataSkyFacet->GetIntensity()); break;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::RenderSkyboxFromAtmopsphericScattering(CInternSky* _pOutput, float _Intensity)
    {
        CRenderContextPtr RenderContextPtr = _pOutput->m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_SkyboxFromAtmosphere.m_VSPtr;
        CShaderPtr        GSPtr            = m_SkyboxFromAtmosphere.m_GSPtr;
        CShaderPtr        PSPtr            = m_SkyboxFromAtmosphere.m_PSPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_SkyboxFromAtmosphere.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_SkyboxFromAtmosphere.m_PSBufferSetPtr;
        CMeshPtr          MeshPtr          = m_SkyboxFromAtmosphere.m_MeshPtr;

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::CSunComponent* pDtSunComponent = 0;

        auto DataSunComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CSunComponent>();

        for (auto Component : DataSunComponents)
        {
            Dt::CSunComponent* pDtComponent = static_cast<Dt::CSunComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            pDtSunComponent = pDtComponent;
        }

        Performance::BeginEvent("Skybox from PAS");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SPSPASSettings PSBuffer;

        PSBuffer.g_SunDirection           = pDtSunComponent == nullptr ? glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) : glm::eulerAngleX(glm::radians(90.0f)) * glm::vec4(pDtSunComponent->GetDirection(), 0.0f);
        PSBuffer.g_SunIntensity           = glm::vec4(_Intensity);
        PSBuffer.ps_ExposureHistoryIndex  = 0;

        BufferManager::UploadBufferData(PSBufferSetPtr->GetBuffer(0), &PSBuffer);

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           
        ContextManager::SetTargetSet(_pOutput->m_RenderContextPtr->GetTargetSet());

        ContextManager::SetViewPortSet(_pOutput->m_RenderContextPtr->GetViewPortSet());

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::NoCull));

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetVertexBuffer(MeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(MeshPtr->GetLOD(0)->GetSurface()->GetMVPShaderVS()->GetInputLayout());

        ContextManager::SetConstantBuffer(2, GSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(3, PSBufferSetPtr->GetBuffer(0));
        
        ContextManager::SetConstantBuffer(1, m_PSPrecomputeConstants);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, static_cast<CTexturePtr>(m_TransmittanceTable));

        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(1, static_cast<CTexturePtr>(m_InscatterTable));

        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(2, static_cast<CTexturePtr>(m_IrradianceTable));

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetConstantBuffer(3);

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Update mip maps
        // -----------------------------------------------------------------------------
        TextureManager::UpdateMipmap(_pOutput->m_CubemapPtr);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::RenderSkyboxFromPanorama(CInternSky* _pOutput, float _Intensity)
    {
        if (_pOutput->m_InputTexturePtr == 0)
        {
            ENGINE_CONSOLE_INFO("Skybox can't be rendered from panorama because of missing image.");
            return;
        }

        // -----------------------------------------------------------------------------

        CRenderContextPtr RenderContextPtr = _pOutput->m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_SkyboxFromPanorama.m_VSPtr;
        CShaderPtr        GSPtr            = m_SkyboxFromPanorama.m_GSPtr;
        CShaderPtr        PSPtr            = m_SkyboxFromPanorama.m_PSPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_SkyboxFromPanorama.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_SkyboxFromPanorama.m_PSBufferSetPtr;
        CMeshPtr          MeshPtr          = m_SkyboxFromPanorama.m_MeshPtr;

        Performance::BeginEvent("Skybox from Panorama");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SOutputBufferPS PSBuffer;

        PSBuffer.m_HDRFactor = _Intensity;
        PSBuffer.m_IsHDR     = _pOutput->m_InputTexturePtr->GetSemantic() == Gfx::CTexture::HDR ? 1.0f : 0.0f;

        BufferManager::UploadBufferData(PSBufferSetPtr->GetBuffer(0), &PSBuffer);

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(RenderContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetVertexBuffer(MeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(MeshPtr->GetLOD(0)->GetSurface()->GetMVPShaderVS()->GetInputLayout());

        ContextManager::SetConstantBuffer(2, GSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(4, PSBufferSetPtr->GetBuffer(0));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, static_cast<CTexturePtr>(_pOutput->m_InputTexturePtr));

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetConstantBuffer(4);

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Update mip maps
        // -----------------------------------------------------------------------------
        TextureManager::UpdateMipmap(_pOutput->m_CubemapPtr);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::RenderSkyboxFromCubemap(CInternSky* _pOutput, float _Intensity)
    {
        if (_pOutput->m_InputTexturePtr == 0)
        {
            ENGINE_CONSOLE_INFO("Skybox can't be rendered from cube map because of missing image.");
            return;
        }

        // -----------------------------------------------------------------------------

        CRenderContextPtr RenderContextPtr = _pOutput->m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_SkyboxFromCubemap.m_VSPtr;
        CShaderPtr        GSPtr            = m_SkyboxFromCubemap.m_GSPtr;
        CShaderPtr        PSPtr            = m_SkyboxFromCubemap.m_PSPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_SkyboxFromCubemap.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_SkyboxFromCubemap.m_PSBufferSetPtr;
        CMeshPtr          MeshPtr          = m_SkyboxFromCubemap.m_MeshPtr;

        Performance::BeginEvent("Skybox from Cubemap");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SOutputBufferPS PSBuffer;

        PSBuffer.m_HDRFactor = _Intensity;
        PSBuffer.m_IsHDR     = _pOutput->m_InputTexturePtr->GetSemantic() == Gfx::CTexture::HDR ? 1.0f : 0.0f;

        BufferManager::UploadBufferData(PSBufferSetPtr->GetBuffer(0), &PSBuffer);

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(RenderContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetVertexBuffer(MeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(MeshPtr->GetLOD(0)->GetSurface()->GetMVPShaderVS()->GetInputLayout());

        ContextManager::SetConstantBuffer(2, GSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(4, PSBufferSetPtr->GetBuffer(0));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, static_cast<CTexturePtr>(_pOutput->m_InputTexturePtr));

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetConstantBuffer(4);

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Update mip maps
        // -----------------------------------------------------------------------------
        TextureManager::UpdateMipmap(_pOutput->m_CubemapPtr);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::PrecomputeScattering()
    {
        // Determine the number of steps for multiple scattering
        // We use a default of 4 orders for Desktop and 0 for mobile

        const int DefaultOrderCount = (Main::GetGraphicsAPI().m_GraphicsAPI == CGraphicsInfo::OpenGL) ? 4 : 0;

        const int OrderCount = Core::CProgramParameters::GetInstance().Get("graphics:atmosphere:order_count", DefaultOrderCount);

        // -----------------------------------------------------------------------------
        // Textures
        // -----------------------------------------------------------------------------
        STextureDescriptor TextureDesc;

        TextureDesc.m_NumberOfPixelsU  = 0;
        TextureDesc.m_NumberOfPixelsV  = 0;
        TextureDesc.m_NumberOfPixelsW  = 1;
        TextureDesc.m_NumberOfMipMaps  = 1;
        TextureDesc.m_NumberOfTextures = 1;
        TextureDesc.m_Binding          = CTexture::RenderTarget | CTexture::ShaderResource;
        TextureDesc.m_Access           = CTexture::CPUWrite;
        TextureDesc.m_Format           = CTexture::Unknown;
        TextureDesc.m_Usage            = CTexture::GPURead;
        TextureDesc.m_Semantic         = CTexture::Diffuse;
        TextureDesc.m_pFileName        = 0;
        TextureDesc.m_pPixels          = 0;
        TextureDesc.m_Format           = CTexture::R16G16B16A16_FLOAT;
        
        TextureDesc.m_NumberOfPixelsU = g_TransmittanceWidth;
        TextureDesc.m_NumberOfPixelsV = g_TransmittanceHeight;

        m_TransmittanceTable = TextureManager::CreateTexture2D(TextureDesc);

        TextureDesc.m_NumberOfPixelsU = g_IrradianceWidth;
        TextureDesc.m_NumberOfPixelsV = g_IrradianceHeight;

        m_IrradianceTable = TextureManager::CreateTexture2D(TextureDesc);
        CTexturePtr m_DeltaE          = TextureManager::CreateTexture2D(TextureDesc);

        TextureDesc.m_NumberOfPixelsU = g_InscatterWidth;
        TextureDesc.m_NumberOfPixelsV = g_InscatterHeight;
        TextureDesc.m_NumberOfPixelsW = g_InscatterDepth;

        m_InscatterTable = TextureManager::CreateTexture3D(TextureDesc);
        CTexturePtr m_DeltaSR        = TextureManager::CreateTexture3D(TextureDesc);
        CTexturePtr m_DeltaSM        = TextureManager::CreateTexture3D(TextureDesc);
        CTexturePtr m_DeltaJ         = TextureManager::CreateTexture3D(TextureDesc);

		// -----------------------------------------------------------------------------
		// Labels
		// -----------------------------------------------------------------------------
		TextureManager::SetTextureLabel(m_TransmittanceTable, "PAS: Transmittance Table");
		TextureManager::SetTextureLabel(m_IrradianceTable, "PAS: Irradiance Table");
		TextureManager::SetTextureLabel(m_InscatterTable, "PAS: Inscatter Table");

        // -----------------------------------------------------------------------------
        // Target sets & view ports
        // -----------------------------------------------------------------------------
        CTargetSetPtr m_TransmittanceTableTS = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(m_TransmittanceTable));

        CTargetSetPtr m_DeltaETS = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(m_DeltaE));

        CTargetSetPtr m_DeltaSRSMTS = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(m_DeltaSR), static_cast<CTexturePtr>(m_DeltaSM));

        CTargetSetPtr m_IrradianceTableTS = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(m_IrradianceTable));

        CTargetSetPtr m_InscatterTableTS = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(m_InscatterTable));

        CTargetSetPtr m_DeltaJTS = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(m_DeltaJ));

        CTargetSetPtr m_DeltaSRTS = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(m_DeltaSR));

        SViewPortDescriptor ViewPortDesc;

        ViewPortDesc.m_TopLeftX = 0;
        ViewPortDesc.m_TopLeftY = 0;
        ViewPortDesc.m_MinDepth = 0;
        ViewPortDesc.m_MaxDepth  = 1.0f;

        ViewPortDesc.m_Width     = g_TransmittanceWidth;
        ViewPortDesc.m_Height    = g_TransmittanceHeight;

        CViewPortSetPtr m_TransmittanceVPS = ViewManager::CreateViewPortSet(ViewManager::CreateViewPort(ViewPortDesc));

        ViewPortDesc.m_Width     = g_IrradianceWidth;
        ViewPortDesc.m_Height    = g_IrradianceHeight;

        CViewPortSetPtr m_IrradianceVPS = ViewManager::CreateViewPortSet(ViewManager::CreateViewPort(ViewPortDesc));

        ViewPortDesc.m_Width     = g_InscatterWidth;
        ViewPortDesc.m_Height    = g_InscatterHeight;

        CViewPortSetPtr m_InscatterVPS = ViewManager::CreateViewPortSet(ViewManager::CreateViewPort(ViewPortDesc));

        // -----------------------------------------------------------------------------
        // Materials
        // -----------------------------------------------------------------------------
        SBufferDescriptor BufferDesc;

        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = 0;
        BufferDesc.m_pBytes        = 0;
        BufferDesc.m_pClassKey     = 0;

        BufferDesc.m_NumberOfBytes = sizeof(SPrecomputeConstants);

        m_PSPrecomputeConstants = BufferManager::CreateBuffer(BufferDesc);

        BufferDesc.m_NumberOfBytes = sizeof(SGSLayer);

        CBufferPtr m_GSLayer = BufferManager::CreateBuffer(BufferDesc);

        BufferDesc.m_NumberOfBytes = sizeof(SPSLayerValues);

        CBufferPtr m_PSLayerValues = BufferManager::CreateBuffer(BufferDesc);

        BufferDesc.m_NumberOfBytes = sizeof(SPSScatteringOrder);

        CBufferPtr m_PSScatteringOrder = BufferManager::CreateBuffer(BufferDesc);

        BufferDesc.m_NumberOfBytes = sizeof(SPSIrradianceK);

        CBufferPtr m_PSIrradianceK = BufferManager::CreateBuffer(BufferDesc);
        
        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        CShaderPtr VSPtr = ShaderManager::CompileVS("scattering/vs.glsl", "main");;
        CShaderPtr GSPtr = ShaderManager::CompileGS("scattering/gs.glsl", "main");;

        // -----------------------------------------------------------------------------
        // Transmittance
        // -----------------------------------------------------------------------------

        CShaderPtr m_TransmittanceMaterial = ShaderManager::CompilePS("scattering/scattering_transmittance.glsl", "main");

        // -----------------------------------------------------------------------------
        // Irradiance
        // -----------------------------------------------------------------------------

        CShaderPtr m_IrradianceSingleMaterial = ShaderManager::CompilePS("scattering/scattering_irradiance_single.glsl", "main");

        CShaderPtr m_IrradianceMultipleMaterial = ShaderManager::CompilePS("scattering/scattering_irradiance_multiple.glsl", "main");

        CShaderPtr m_IrradianceCopyMaterial = ShaderManager::CompilePS("scattering/scattering_irradiance_copy.glsl", "main");

        // -----------------------------------------------------------------------------
        // Inscatter
        // -----------------------------------------------------------------------------

        CShaderPtr m_InscatterSingleMaterial = ShaderManager::CompilePS("scattering/scattering_inscatter_single.glsl", "main");

        CShaderPtr m_InscatterMultipleAMaterial = ShaderManager::CompilePS("scattering/scattering_inscatter_multiple_a.glsl", "main");

        CShaderPtr m_InscatterMultipleBMaterial = ShaderManager::CompilePS("scattering/scattering_inscatter_multiple_b.glsl", "main");

        CShaderPtr m_InscatterCopySingleMaterial = ShaderManager::CompilePS("scattering/scattering_inscatter_copy_single.glsl", "main");

        CShaderPtr m_InscatterCopyMultipleMaterial = ShaderManager::CompilePS("scattering/scattering_inscatter_copy_multiple.glsl", "main");

        // -----------------------------------------------------------------------------
        // Mesh
        // -----------------------------------------------------------------------------
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::VertexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = 0;
        BufferDesc.m_pBytes        = 0;
        BufferDesc.m_pClassKey     = 0;

        CBufferPtr EmptyVertexBufferPtr = BufferManager::CreateBuffer(BufferDesc);

        // -----------------------------------------------------------------------------
        // Render
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Precompute Atmospheric Scattering");

        auto GetLayerValues = [&](unsigned int _Layer, float& _rRadius, glm::vec4& _rDhdH)
        {
            float Radius = _Layer / glm::max((g_InscatterAltitude - 1.0f), 1.0f);

            Radius = Radius * Radius;
            Radius = glm::sqrt(g_RadiusGround * g_RadiusGround + Radius * (g_RadiusAtmosphere * g_RadiusAtmosphere - g_RadiusGround * g_RadiusGround)) + (_Layer == 0 ? 0.01f : (_Layer == g_InscatterAltitude - 1 ? -0.001f : 0.0f));

            float DMin  = g_RadiusAtmosphere - Radius;
            float DMax  = glm::sqrt(Radius * Radius - g_RadiusGround * g_RadiusGround) + glm::sqrt(g_RadiusAtmosphere * g_RadiusAtmosphere - g_RadiusGround * g_RadiusGround);

            float DMinP = Radius - g_RadiusGround;
            float DMaxP = glm::sqrt(Radius * Radius - g_RadiusGround * g_RadiusGround);

            _rRadius = Radius;

            _rDhdH = glm::vec4(DMin, DMax, DMinP, DMaxP);
        };

        // -----------------------------------------------------------------------------
        // Transmittance
        // -----------------------------------------------------------------------------
        SPrecomputeConstants PrecomuteBuffer;

        PrecomuteBuffer.m_TransmittanceWidth  = static_cast<float>(g_TransmittanceWidth);
        PrecomuteBuffer.m_TransmittanceHeight = static_cast<float>(g_TransmittanceHeight);
        PrecomuteBuffer.m_IrradianceWidth     = static_cast<float>(g_IrradianceWidth);
        PrecomuteBuffer.m_IrradianceHeight    = static_cast<float>(g_IrradianceHeight);
        PrecomuteBuffer.m_InscatterMus        = static_cast<float>(g_InscatterMuS);
        PrecomuteBuffer.m_InscatterMu         = static_cast<float>(g_InscatterMu);
        PrecomuteBuffer.m_InscatterNu         = static_cast<float>(g_InscatterNu);
        PrecomuteBuffer.m_InscatterAltitude   = static_cast<float>(g_InscatterAltitude);
        
        BufferManager::UploadBufferData(m_PSPrecomputeConstants, &PrecomuteBuffer);

        Performance::BeginEvent("Transmittance");

        ContextManager::SetTargetSet(m_TransmittanceTableTS);

        ContextManager::SetViewPortSet(m_TransmittanceVPS);

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetTopology(STopology::TriangleStrip);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderPS(m_TransmittanceMaterial);

        ContextManager::SetVertexBuffer(EmptyVertexBufferPtr);

        ContextManager::SetConstantBuffer(1, m_PSPrecomputeConstants);

        ContextManager::Draw(3, 0);

#ifdef PLATFORM_ANDROID
        ContextManager::Flush();
#endif // PLATFORM_ANDROID

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Irradiance single
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Irradiance Single");

        ContextManager::SetTargetSet(m_DeltaETS);

        ContextManager::SetViewPortSet(m_IrradianceVPS);

        ContextManager::SetShaderPS(m_IrradianceSingleMaterial);

        ContextManager::SetTexture(0, static_cast<CTexturePtr>(m_TransmittanceTable));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::Draw(3, 0);

#ifdef PLATFORM_ANDROID
        ContextManager::Flush();
#endif // PLATFORM_ANDROID

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Inscatter single
        // -----------------------------------------------------------------------------
        glm::vec4 Dhdh;
        float Radius;

        Performance::BeginEvent("Inscatter Single");

        ContextManager::SetTargetSet(m_DeltaSRSMTS);

        ContextManager::SetViewPortSet(m_InscatterVPS);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(m_InscatterSingleMaterial);

        ContextManager::SetConstantBuffer(2, m_GSLayer);

        ContextManager::SetConstantBuffer(3, m_PSLayerValues);

        for (unsigned int Layer = 0; Layer < g_InscatterDepth; ++Layer)
        {
            SGSLayer GSLayer;

            GSLayer.m_Layer = Layer;

            BufferManager::UploadBufferData(m_GSLayer, &GSLayer);

            GetLayerValues(Layer, Radius, Dhdh);

            SPSLayerValues PSLayerValues;

            PSLayerValues.m_Dhdh   = Dhdh;
            PSLayerValues.m_Radius = Radius;

            BufferManager::UploadBufferData(m_PSLayerValues, &PSLayerValues);

            ContextManager::Draw(3, 0);

#ifdef PLATFORM_ANDROID
            ContextManager::Flush();
#endif // PLATFORM_ANDROID
        }

        ContextManager::ResetShaderGS();

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Copy irradiance 
        // -----------------------------------------------------------------------------
        SPSIrradianceK PSIrradianceK;

        PSIrradianceK.k = 0;

        BufferManager::UploadBufferData(m_PSIrradianceK, &PSIrradianceK);

        Performance::BeginEvent("Copy Irradiance");

        ContextManager::SetTargetSet(m_IrradianceTableTS);

        ContextManager::SetViewPortSet(m_IrradianceVPS);

        ContextManager::SetShaderPS(m_IrradianceCopyMaterial);

        ContextManager::SetTexture(6, static_cast<CTexturePtr>(m_DeltaE));

        ContextManager::SetSampler(6, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetConstantBuffer(5, m_PSIrradianceK);

        ContextManager::Draw(3, 0);

#ifdef PLATFORM_ANDROID
        ContextManager::Flush();
#endif // PLATFORM_ANDROID

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Copy inscatter single
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Copy Inscatter Single");

        ContextManager::SetTargetSet(m_InscatterTableTS);

        ContextManager::SetViewPortSet(m_InscatterVPS);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(m_InscatterCopySingleMaterial);

        ContextManager::SetConstantBuffer(2, m_GSLayer);

        ContextManager::SetTexture(3, static_cast<CTexturePtr>(m_DeltaSR));

        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(4, static_cast<CTexturePtr>(m_DeltaSM));

        ContextManager::SetSampler(4, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        for (unsigned int Layer = 0; Layer < g_InscatterDepth; ++Layer)
        {
            SGSLayer GSLayer;

            GSLayer.m_Layer = Layer;

            BufferManager::UploadBufferData(m_GSLayer, &GSLayer);

            ContextManager::Draw(3, 0);

#ifdef PLATFORM_ANDROID
            ContextManager::Flush();
#endif // PLATFORM_ANDROID
        }

        ContextManager::ResetShaderGS();

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Multiple scattering
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Multiple Scattering");

        ContextManager::SetTexture(5, static_cast<CTexturePtr>(m_DeltaJ));

        ContextManager::SetSampler(5, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        for (int Order = 2; Order <= OrderCount; ++Order)
        {
            Performance::BeginEvent("Order");

            SPSScatteringOrder PSScatteringOrder;

            PSScatteringOrder.m_FirstOrder = (Order == 2 ? 1.0f : 0.0f);

            BufferManager::UploadBufferData(m_PSScatteringOrder, &PSScatteringOrder);

            // -----------------------------------------------------------------------------
            // DeltaJ Inscatter
            // -----------------------------------------------------------------------------
            Performance::BeginEvent("DeltaJ Inscatter");

            ContextManager::SetTargetSet(m_DeltaJTS);

            ContextManager::SetViewPortSet(m_InscatterVPS);

            ContextManager::SetShaderGS(GSPtr);

            ContextManager::SetShaderPS(m_InscatterMultipleAMaterial);

            ContextManager::SetConstantBuffer(2, m_GSLayer);

            ContextManager::SetConstantBuffer(3, m_PSLayerValues);

            ContextManager::SetConstantBuffer(4, m_PSScatteringOrder);

            for (unsigned int Layer = 0; Layer < g_InscatterDepth; ++Layer)
            {
                SGSLayer GSLayer;

                GSLayer.m_Layer = Layer;

                BufferManager::UploadBufferData(m_GSLayer, &GSLayer);

                GetLayerValues(Layer, Radius, Dhdh);

                SPSLayerValues PSLayerValues;

                PSLayerValues.m_Dhdh = Dhdh;
                PSLayerValues.m_Radius = Radius;

                BufferManager::UploadBufferData(m_PSLayerValues, &PSLayerValues);

                ContextManager::Draw(3, 0);

#ifdef PLATFORM_ANDROID
                ContextManager::Flush();
#endif // PLATFORM_ANDROID
            }

            ContextManager::ResetShaderGS();

            Performance::EndEvent();

            // -----------------------------------------------------------------------------
            // DeltaE Irradiance
            // -----------------------------------------------------------------------------
            Performance::BeginEvent("DeltaE Irradiance");

            ContextManager::SetTargetSet(m_DeltaETS);

            ContextManager::SetViewPortSet(m_IrradianceVPS);

            ContextManager::SetShaderPS(m_IrradianceMultipleMaterial);

            ContextManager::Draw(3, 0);

#ifdef PLATFORM_ANDROID
            ContextManager::Flush();
#endif // PLATFORM_ANDROID

            Performance::EndEvent();

            // -----------------------------------------------------------------------------
            // DeltaS Inscatter
            // -----------------------------------------------------------------------------
            Performance::BeginEvent("DeltaS Inscatter");

            ContextManager::SetTargetSet(m_DeltaSRTS);

            ContextManager::SetViewPortSet(m_InscatterVPS);

            ContextManager::SetShaderGS(GSPtr);

            ContextManager::SetShaderPS(m_InscatterMultipleBMaterial);

            ContextManager::SetConstantBuffer(2, m_GSLayer);

            ContextManager::SetConstantBuffer(3, m_PSLayerValues);

            ContextManager::SetConstantBuffer(4, m_PSScatteringOrder);

            for (unsigned int Layer = 0; Layer < g_InscatterDepth; ++Layer)
            {
                SGSLayer GSLayer;

                GSLayer.m_Layer = Layer;

                BufferManager::UploadBufferData(m_GSLayer, &GSLayer);

                GetLayerValues(Layer, Radius, Dhdh);

                SPSLayerValues PSLayerValues;

                PSLayerValues.m_Dhdh = Dhdh;
                PSLayerValues.m_Radius = Radius;

                BufferManager::UploadBufferData(m_PSLayerValues, &PSLayerValues);

                ContextManager::Draw(3, 0);

#ifdef PLATFORM_ANDROID
                ContextManager::Flush();
#endif // PLATFORM_ANDROID
            }

            ContextManager::ResetShaderGS();

            Performance::EndEvent();

            // -----------------------------------------------------------------------------
            // Enable blending
            // -----------------------------------------------------------------------------
            ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::AdditionBlend));

            // -----------------------------------------------------------------------------
            // Copy deltaE to irradiance
            // -----------------------------------------------------------------------------
            PSIrradianceK.k = 1;

            BufferManager::UploadBufferData(m_PSIrradianceK, &PSIrradianceK);

            Performance::BeginEvent("Add DeltaE to Irradiance");

            ContextManager::SetTargetSet(m_IrradianceTableTS);

            ContextManager::SetViewPortSet(m_IrradianceVPS);

            ContextManager::SetShaderPS(m_IrradianceCopyMaterial);

            ContextManager::Draw(3, 0);

#ifdef PLATFORM_ANDROID
            ContextManager::Flush();
#endif // PLATFORM_ANDROID

            Performance::EndEvent();

            // -----------------------------------------------------------------------------
            // Copy DeltaS to Inscatter
            // -----------------------------------------------------------------------------
            Performance::BeginEvent("Add DeltaS to Inscatter");

            ContextManager::SetTargetSet(m_InscatterTableTS);

            ContextManager::SetViewPortSet(m_InscatterVPS);

            ContextManager::SetShaderGS(GSPtr);

            ContextManager::SetShaderPS(m_InscatterCopyMultipleMaterial);

            for (unsigned int Layer = 0; Layer < g_InscatterDepth; ++Layer)
            {
                SGSLayer GSLayer;

                GSLayer.m_Layer = Layer;

                BufferManager::UploadBufferData(m_GSLayer, &GSLayer);

                GetLayerValues(Layer, Radius, Dhdh);

                SPSLayerValues PSLayerValues;

                PSLayerValues.m_Dhdh = Dhdh;
                PSLayerValues.m_Radius = Radius;

                BufferManager::UploadBufferData(m_PSLayerValues, &PSLayerValues);

                ContextManager::Draw(3, 0);

#ifdef PLATFORM_ANDROID
                ContextManager::Flush();
#endif // PLATFORM_ANDROID
            }

            ContextManager::ResetShaderGS();

            Performance::EndEvent();

            // -----------------------------------------------------------------------------
            // Disable blending
            // -----------------------------------------------------------------------------
            ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

            Performance::EndEvent();
        }

        Performance::EndEvent();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::ResetRenderContext()
    {        
        m_SkyboxFromAtmosphere.m_VSPtr              = 0;
        m_SkyboxFromAtmosphere.m_GSPtr              = 0;
        m_SkyboxFromAtmosphere.m_PSPtr              = 0;
        m_SkyboxFromAtmosphere.m_GSBufferSetPtr     = 0;
        m_SkyboxFromAtmosphere.m_PSBufferSetPtr     = 0;
        m_SkyboxFromAtmosphere.m_MeshPtr            = 0;

        m_SkyboxFromPanorama.m_VSPtr              = 0;
        m_SkyboxFromPanorama.m_GSPtr              = 0;
        m_SkyboxFromPanorama.m_PSPtr              = 0;
        m_SkyboxFromPanorama.m_GSBufferSetPtr     = 0;
        m_SkyboxFromPanorama.m_PSBufferSetPtr     = 0;
        m_SkyboxFromPanorama.m_MeshPtr            = 0;

        m_SkyboxFromCubemap.m_VSPtr              = 0;
        m_SkyboxFromCubemap.m_GSPtr              = 0;
        m_SkyboxFromCubemap.m_PSPtr              = 0;
        m_SkyboxFromCubemap.m_GSBufferSetPtr     = 0;
        m_SkyboxFromCubemap.m_PSBufferSetPtr     = 0;
        m_SkyboxFromCubemap.m_MeshPtr            = 0;
    }
} // namespace 

namespace Gfx
{
namespace SkyManager
{
    void OnStart()
    {
        CGfxSkyManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxSkyManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxSkyManager::GetInstance().Update();
    }
} // namespace SkyManager
} // namespace Gfx