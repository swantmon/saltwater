
#include "graphic/gfx_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_component.h"
#include "data/data_component_manager.h"
#include "data/data_component_facet.h"
#include "data/data_entity.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_probe_component.h"
#include "data/data_map.h"
#include "data/data_mesh_component.h"
#include "data/data_model_manager.h"
#include "data/data_point_light_component.h"
#include "data/data_sky_component.h"
#include "data/data_sun_component.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_component.h"
#include "graphic/gfx_component_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_probe_component.h"
#include "graphic/gfx_light_probe_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_component.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_point_light_component.h"
#include "graphic/gfx_reflection_renderer.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_sky_component.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sun_component.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_texture_set.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

namespace 
{
    class CGfxLightProbeManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxLightProbeManager)

    public:

        CGfxLightProbeManager();
        ~CGfxLightProbeManager();

    public:

        void OnStart();
        void OnExit();

        void Update();

    private:

        static const unsigned int s_MaxNumberOfLightsPerProbe = 4;

    private:

        struct SLightJob
        {
            CTexturePtr m_ShadowTexturePtrs[s_MaxNumberOfLightsPerProbe];
            CTexturePtr m_SpecularTexturePtr;
            CTexturePtr m_DiffuseTexturePtr;
        };

        struct SGeometryVPBuffer
        {
            glm::mat4 m_Projection;
            glm::mat4 m_View;
        };

        struct SGeometryMBuffer
        {
            glm::mat4 m_ModelMatrix;
        };

        struct SCubemapGeometryBuffer
        {
            glm::mat4 m_CubeProjectionMatrix;
            glm::mat4 m_CubeViewMatrix[6];
        };

        struct SLightPropertiesBuffer
        {
            glm::mat4 m_LightViewProjection;
            glm::vec4   m_LightPosition;
            glm::vec4   m_LightDirection;
            glm::vec4   m_LightColor;
            glm::vec4   m_LightSettings;
            unsigned int   m_LightType;
            unsigned int   m_Padding0;
            unsigned int   m_Padding1;
            unsigned int   m_Padding2;
        };

        struct SReflectionProbePropertiesBuffer
        {
            glm::vec4 m_Properties;
        };

        struct SCameraPropertiesBuffer
        {
            glm::vec4 m_CameraPosition;
            unsigned int m_ExposureHistoryIndex;
        };

        struct SFilterPropertiesBuffer
        {
            float m_LinearRoughness;
            float m_NumberOfMiplevels;
            float m_Intensity;
        };

        class CInternComponent : public CLightProbeComponent
        {
        public:

            typedef std::vector<CTargetSetPtr>   CTargetSets;
            typedef std::vector<CViewPortSetPtr> CViewPortSets;

        public:

            CInternComponent();
            ~CInternComponent();

        public:

            CTargetSetPtr   m_TargetSetPtr;
            CViewPortSetPtr m_ViewPortSetPtr;

            CTexturePtr m_ReflectionCubemapPtr;

            CTargetSets   m_SpecularHDRTargetSetPtrs;
            CViewPortSets m_SpecularViewPortSetPtrs;

            CTargetSetPtr   m_DiffuseHDRTargetSetPtr;
            CViewPortSetPtr m_DiffuseViewPortSetPtr;

        private:

            friend class CGfxLightProbeManager;
        };

    private:

        CMeshPtr m_EnvironmentSpherePtr;
        CMeshPtr m_SkyboxBoxPtr;

        CShaderPtr m_FilteringVSPtr;
        CShaderPtr m_FilteringDiffusePSPtr;
        CShaderPtr m_FilteringSpecularPSPtr;
        CShaderPtr m_CubemapGSPtr;
        CShaderPtr m_CubemapVSPtr;
        CShaderPtr m_CubemapPSPtr;

        CBufferPtr m_LightPropertiesBufferPtr;
        CBufferPtr m_CubemapGSBufferPtr;
        CBufferPtr m_FilteringPSBufferPtr;
        CBufferPtr m_SurfaceMaterialBufferPtr;
        CBufferPtr m_CameraPropertiesBufferPtr;
        CBufferPtr m_ReflectionProbePropertiesBufferPtr;
        CBufferPtr m_GeometryVPBufferPtr;
        CBufferPtr m_GeometryMBufferPtr;

        CInputLayoutPtr m_P3N3InputLayoutPtr;

        SLightJob m_LightJob;

    private:

        void OnDirtyComponent(Dt::IComponent* _pComponent);

        CInternComponent* AllocateLightProbeFacet(Base::ID _ID, unsigned int _SpecularFaceSize, unsigned int _DiffuseFaceSize);

        void Render(const Dt::CEntity& _rEntity, CInternComponent& _rInterLightProbeFacet, const Dt::CLightProbeComponent& _rDtLightProbeFacet);

        void RenderEnvironment(CInternComponent& _rInterLightProbeFacet);

        void RenderEntities(CInternComponent& _rInterLightProbeFacet, const glm::vec3& _rPosition);

        void RenderFiltering(CInternComponent& _rInterLightProbeFacet, const Dt::CLightProbeComponent& _rDtLightProbeFacet);

        void UpdateLightProperties();

        void UpdateGeometryBuffer(const glm::vec3& _rPosition, float _Near, float _Far);
    };
} // namespace 

namespace 
{
    CGfxLightProbeManager::CInternComponent::CInternComponent()
        : CLightProbeComponent          ()
        , m_DiffuseHDRTargetSetPtr  ()
        , m_DiffuseViewPortSetPtr   ()
        , m_SpecularHDRTargetSetPtrs()
        , m_SpecularViewPortSetPtrs ()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxLightProbeManager::CInternComponent::~CInternComponent()
    {
        m_TargetSetPtr   = 0;
        m_ViewPortSetPtr = 0;

        m_ReflectionCubemapPtr = 0;

        m_DiffuseHDRTargetSetPtr = 0;
        m_DiffuseViewPortSetPtr  = 0;

        m_SpecularHDRTargetSetPtrs.clear();
        m_SpecularViewPortSetPtrs .clear();
    }
} // namespace 

namespace 
{
    CGfxLightProbeManager::CGfxLightProbeManager()
        : m_EnvironmentSpherePtr  ()
        , m_SkyboxBoxPtr          ()
        , m_FilteringVSPtr        ()
        , m_FilteringDiffusePSPtr ()
        , m_FilteringSpecularPSPtr()
        , m_CubemapGSBufferPtr    ()
        , m_FilteringPSBufferPtr  ()
        , m_P3N3InputLayoutPtr    ()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxLightProbeManager::~CGfxLightProbeManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        m_FilteringVSPtr = ShaderManager::CompileVS("vs_lightprobe_sampling.glsl", "main");

        m_FilteringDiffusePSPtr = ShaderManager::CompilePS("fs_lightprobe_diffuse_sampling.glsl", "main");

        m_FilteringSpecularPSPtr = ShaderManager::CompilePS("fs_lightprobe_specular_sampling.glsl", "main");

        m_CubemapGSPtr = ShaderManager::CompileGS("gs_lightprobe_sampling.glsl", "main");

        m_CubemapVSPtr = ShaderManager::CompileVS("vs_p3.glsl", "main");

        m_CubemapPSPtr = ShaderManager::CompilePS("fs_lightprobe.glsl", "main");

        // -----------------------------------------------------------------------------

        const SInputElementDescriptor TriangleInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0,  0, 24, CInputLayout::PerVertex, 0, },
            { "NORMAL"  , 0, CInputLayout::Float3Format, 0, 12, 24, CInputLayout::PerVertex, 0, },
        };

        m_P3N3InputLayoutPtr = ShaderManager::CreateInputLayout(TriangleInputLayout, 2, m_CubemapVSPtr);

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SGeometryVPBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_GeometryVPBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SFilterPropertiesBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_FilteringPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCubemapGeometryBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_CubemapGSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        UpdateGeometryBuffer(glm::vec3(0.0f), 0.1f, 1000.0f);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SGeometryMBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_GeometryMBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(CMaterial::SMaterialAttributes);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_SurfaceMaterialBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCameraPropertiesBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_CameraPropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SReflectionProbePropertiesBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_ReflectionProbePropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SLightPropertiesBuffer) * s_MaxNumberOfLightsPerProbe;
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_LightPropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        // Models
        // -----------------------------------------------------------------------------
        m_EnvironmentSpherePtr = MeshManager::CreateSphereIsometric(1.0f, 3);

        m_SkyboxBoxPtr = MeshManager::CreateBox(2.0f, 2.0f, 2.0f);

        // -----------------------------------------------------------------------------
        // Register dirty handler for automatic light probe / reflection
        // creation
        // -----------------------------------------------------------------------------
        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(DATA_DIRTY_COMPONENT_METHOD(&CGfxLightProbeManager::OnDirtyComponent));
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::OnExit()
    {
        m_EnvironmentSpherePtr = 0;
        m_SkyboxBoxPtr = 0;

        m_FilteringVSPtr = 0;
        m_FilteringDiffusePSPtr  = 0;
        m_FilteringSpecularPSPtr = 0;

        m_CubemapGSPtr = 0;
        m_CubemapVSPtr = 0;
        m_CubemapPSPtr = 0;

        m_LightPropertiesBufferPtr = 0;

        m_CubemapGSBufferPtr   = 0;
        m_FilteringPSBufferPtr = 0;
        m_SurfaceMaterialBufferPtr = 0;
        m_CameraPropertiesBufferPtr = 0;
        m_ReflectionProbePropertiesBufferPtr = 0;
        m_GeometryVPBufferPtr = 0;
        m_GeometryMBufferPtr  = 0;

        m_P3N3InputLayoutPtr = 0;

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < s_MaxNumberOfLightsPerProbe; ++IndexOfTexture)
        {
            m_LightJob.m_ShadowTexturePtrs[IndexOfTexture] = 0;
        }

        m_LightJob.m_SpecularTexturePtr = 0;
        m_LightJob.m_DiffuseTexturePtr  = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::Update()
    {
        auto DataComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CLightProbeComponent>();

        for (auto Component : DataComponents)
        {
            Dt::CLightProbeComponent* pDtProbeFacet = static_cast<Dt::CLightProbeComponent*>(Component);

            if (!(pDtProbeFacet->IsActive() && pDtProbeFacet->GetHostEntity() != nullptr && pDtProbeFacet->GetHostEntity()->IsActive())) continue;

            CInternComponent* pGfxProbeFacet = CComponentManager::GetInstance().GetComponent<CInternComponent>(pDtProbeFacet->GetID());

            // -----------------------------------------------------------------------------
            // Check update needs
            // -----------------------------------------------------------------------------
            if (pDtProbeFacet->GetRefreshMode() == Dt::CLightProbeComponent::Dynamic || pGfxProbeFacet->m_TimeStamp >= Core::Time::GetNumberOfFrame())
            {
                Render(*pDtProbeFacet->GetHostEntity(), *pGfxProbeFacet, *pDtProbeFacet);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        if (_pComponent->GetTypeID() != Base::CTypeInfo::GetTypeID<Dt::CLightProbeComponent>()) return;

        Dt::CLightProbeComponent* pLightProbeComponent = static_cast<Dt::CLightProbeComponent*>(_pComponent);

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pLightProbeComponent->GetDirtyFlags();

        CInternComponent* pGfxComponent = 0;

        if ((DirtyFlags & Dt::CLightProbeComponent::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create facet
            // -----------------------------------------------------------------------------
            pGfxComponent = AllocateLightProbeFacet(pLightProbeComponent->GetID(), pLightProbeComponent->GetQualityInPixel(), 128);
        }
        else
        {
            pGfxComponent = CComponentManager::GetInstance().GetComponent<CInternComponent>(pLightProbeComponent->GetID());
        }

        // -----------------------------------------------------------------------------
        // Set time
        // -----------------------------------------------------------------------------
        pGfxComponent->m_TimeStamp = Core::Time::GetNumberOfFrame() + 1;
    }

    // -----------------------------------------------------------------------------

    CGfxLightProbeManager::CInternComponent* CGfxLightProbeManager::AllocateLightProbeFacet(Base::ID _ID, unsigned int _SpecularFaceSize, unsigned int _DiffuseFaceSize)
    {
        Gfx::STextureDescriptor  TextureDescriptor;
        Gfx::SViewPortDescriptor ViewPortDesc;

        // -----------------------------------------------------------------------------
        // Create facet
        // -----------------------------------------------------------------------------
        CInternComponent* pGfxLightProbeComponent = CComponentManager::GetInstance().Allocate<CInternComponent>(_ID);

        // -----------------------------------------------------------------------------
        // Create stuff for reflection probe
        // -----------------------------------------------------------------------------
        TextureDescriptor.m_NumberOfPixelsU  = _SpecularFaceSize;
        TextureDescriptor.m_NumberOfPixelsV  = _SpecularFaceSize;
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
        TextureDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;

        pGfxLightProbeComponent->m_ReflectionCubemapPtr = TextureManager::CreateCubeTexture(TextureDescriptor);

		TextureManager::SetTextureLabel(pGfxLightProbeComponent->m_ReflectionCubemapPtr, "Light Probe Reflection Texture");

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU = _SpecularFaceSize;
        TextureDescriptor.m_NumberOfPixelsV = _SpecularFaceSize;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource | CTexture::DepthStencilTarget;
        TextureDescriptor.m_Format           = CTexture::R32_FLOAT;

        pGfxLightProbeComponent->m_DepthPtr = TextureManager::CreateCubeTexture(TextureDescriptor);

        TextureManager::SetTextureLabel(pGfxLightProbeComponent->m_DepthPtr, "Light Probe Depth Texture");

        // -----------------------------------------------------------------------------

        CTargetSetPtr ReflectionTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(pGfxLightProbeComponent->m_ReflectionCubemapPtr), static_cast<CTexturePtr>(pGfxLightProbeComponent->m_DepthPtr));

        TargetSetManager::SetTargetSetLabel(ReflectionTargetSetPtr, "Light Probe Reflection Target");

        // -----------------------------------------------------------------------------

        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;
        ViewPortDesc.m_Width    = static_cast<float>(_SpecularFaceSize);
        ViewPortDesc.m_Height   = static_cast<float>(_SpecularFaceSize);

        CViewPortPtr ReflectionViewPortPtr = ViewManager::CreateViewPort(ViewPortDesc);

        CViewPortSetPtr ReflectionViewPortSetPtr = ViewManager::CreateViewPortSet(ReflectionViewPortPtr);

        pGfxLightProbeComponent->m_TargetSetPtr   = ReflectionTargetSetPtr;
        pGfxLightProbeComponent->m_ViewPortSetPtr = ReflectionViewPortSetPtr;

        // -----------------------------------------------------------------------------
        // Create rest of the probe that is available at any type
        // -> specular and diffuse cubemap
        // -----------------------------------------------------------------------------
        unsigned int SizeOfSpecularCubemap = _SpecularFaceSize;
        unsigned int SizeOfDiffuseCubemap  = _DiffuseFaceSize;

        TextureDescriptor.m_NumberOfPixelsU  = SizeOfSpecularCubemap;
        TextureDescriptor.m_NumberOfPixelsV  = SizeOfSpecularCubemap;
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
        TextureDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;
        
        pGfxLightProbeComponent->m_SpecularPtr = TextureManager::CreateCubeTexture(TextureDescriptor);

		TextureManager::SetTextureLabel(pGfxLightProbeComponent->m_SpecularPtr, "Light Probe Specular Texture");
        
        // -----------------------------------------------------------------------------
        
        TextureDescriptor.m_NumberOfPixelsU  = SizeOfDiffuseCubemap;
        TextureDescriptor.m_NumberOfPixelsV  = SizeOfDiffuseCubemap;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource | CTexture::RenderTarget;
        TextureDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;
        
        pGfxLightProbeComponent->m_DiffusePtr = TextureManager::CreateCubeTexture(TextureDescriptor);

		TextureManager::SetTextureLabel(pGfxLightProbeComponent->m_DiffusePtr, "Light Probe Diffuse Texture");
        
        // -----------------------------------------------------------------------------
        // For all cube maps create a render target for every mip map
        // -----------------------------------------------------------------------------        
        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;

        CInternComponent::CTargetSets&   rSpecularTargetSets   = pGfxLightProbeComponent->m_SpecularHDRTargetSetPtrs;
        CInternComponent::CViewPortSets& rSpecularViewPortSets = pGfxLightProbeComponent->m_SpecularViewPortSetPtrs;

        for (unsigned int IndexOfMipmap = 0; IndexOfMipmap < pGfxLightProbeComponent->m_SpecularPtr->GetNumberOfMipLevels(); ++ IndexOfMipmap)
        {
            // -----------------------------------------------------------------------------
            // Target set
            // -----------------------------------------------------------------------------
            CTexturePtr MipmapCubeTexture = TextureManager::GetMipmapFromTexture2D(pGfxLightProbeComponent->m_SpecularPtr, IndexOfMipmap);
            
            CTargetSetPtr SpecularMipmapTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(MipmapCubeTexture));
            
            // -----------------------------------------------------------------------------
            // View port
            // -----------------------------------------------------------------------------
            ViewPortDesc.m_Width    = static_cast<float>(MipmapCubeTexture->GetNumberOfPixelsU());
            ViewPortDesc.m_Height   = static_cast<float>(MipmapCubeTexture->GetNumberOfPixelsV());
            
            CViewPortPtr SpecularMipmapViewPort = ViewManager::CreateViewPort(ViewPortDesc);
            
            CViewPortSetPtr SpecularViewPortSetPtr = ViewManager::CreateViewPortSet(SpecularMipmapViewPort);
            
            // -----------------------------------------------------------------------------
            // Put into light probe
            // -----------------------------------------------------------------------------
            rSpecularTargetSets  .push_back(SpecularMipmapTargetSetPtr);
            rSpecularViewPortSets.push_back(SpecularViewPortSetPtr);
        }
        
        // -----------------------------------------------------------------------------
        
        {
            // -----------------------------------------------------------------------------
            // Target set
            // -----------------------------------------------------------------------------
            CTargetSetPtr DiffuseMipmapTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTexturePtr>(pGfxLightProbeComponent->m_DiffusePtr));
            
            // -----------------------------------------------------------------------------
            // View port
            // -----------------------------------------------------------------------------
            ViewPortDesc.m_Width    = static_cast<float>(pGfxLightProbeComponent->m_DiffusePtr->GetNumberOfPixelsU());
            ViewPortDesc.m_Height   = static_cast<float>(pGfxLightProbeComponent->m_DiffusePtr->GetNumberOfPixelsV());
            
            CViewPortPtr DiffuseMipmapViewPort = ViewManager::CreateViewPort(ViewPortDesc);
            
            CViewPortSetPtr DiffuseViewPortSetPtr = ViewManager::CreateViewPortSet(DiffuseMipmapViewPort);
            
            // -----------------------------------------------------------------------------
            // Put into light probe
            // -----------------------------------------------------------------------------
            pGfxLightProbeComponent->m_DiffuseHDRTargetSetPtr = DiffuseMipmapTargetSetPtr;
            pGfxLightProbeComponent->m_DiffuseViewPortSetPtr  = DiffuseViewPortSetPtr;
        }

        return pGfxLightProbeComponent;
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::Render(const Dt::CEntity& _rEntity, CInternComponent& _rInterLightProbeFacet, const Dt::CLightProbeComponent& _rDtLightProbeFacet)
    {
        Performance::BeginEvent("Light Probe");

        TargetSetManager::ClearTargetSet(_rInterLightProbeFacet.m_TargetSetPtr);

        if (_rDtLightProbeFacet.GetType() == Dt::CLightProbeComponent::Sky)
        {
            RenderEnvironment(_rInterLightProbeFacet);

            TextureManager::UpdateMipmap(_rInterLightProbeFacet.m_ReflectionCubemapPtr);
        }
        else if (_rDtLightProbeFacet.GetType() == Dt::CLightProbeComponent::Local)
        {
            UpdateLightProperties();

            if (_rDtLightProbeFacet.GetClearFlag() == Dt::CLightProbeComponent::Skybox)
            {
                RenderEnvironment(_rInterLightProbeFacet);
            }

            UpdateGeometryBuffer(_rEntity.GetWorldPosition(), _rDtLightProbeFacet.GetNear(), _rDtLightProbeFacet.GetFar());

            RenderEntities(_rInterLightProbeFacet, _rEntity.GetWorldPosition());

            TextureManager::UpdateMipmap(_rInterLightProbeFacet.m_ReflectionCubemapPtr);
        }

        RenderFiltering(_rInterLightProbeFacet, _rDtLightProbeFacet);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::RenderEnvironment(CInternComponent& _rInterLightProbeFacet)
    {
        Performance::BeginEvent("Render Environment");

        // -----------------------------------------------------------------------------
        // Find environment entity
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity;
        Dt::Map::CEntityIterator EndOfEntities;

        // -----------------------------------------------------------------------------
        // Render environment as reflection into cube map
        // -----------------------------------------------------------------------------
        ContextManager::SetTargetSet        (_rInterLightProbeFacet.m_TargetSetPtr);
        ContextManager::SetViewPortSet      (_rInterLightProbeFacet.m_ViewPortSetPtr);
        ContextManager::SetBlendState       (StateManager::GetBlendState(0));
        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));
        ContextManager::SetRasterizerState  (StateManager::GetRasterizerState(CRasterizerState::NoCull));

        ContextManager::SetTopology(STopology::TriangleList);

        // -----------------------------------------------------------------------------
        // Set shader
        // -----------------------------------------------------------------------------
        ContextManager::SetShaderVS(m_CubemapVSPtr);
        ContextManager::SetShaderGS(m_CubemapGSPtr);
        ContextManager::SetShaderPS(m_CubemapPSPtr);
        
        // -----------------------------------------------------------------------------
        // Set buffer
        // -----------------------------------------------------------------------------
        ContextManager::SetConstantBuffer(2, m_CubemapGSBufferPtr);
        ContextManager::SetConstantBuffer(3, m_ReflectionProbePropertiesBufferPtr);

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

        // -----------------------------------------------------------------------------
        // Upload data
        // -----------------------------------------------------------------------------
        UpdateGeometryBuffer(glm::vec3(0.0f), 0.2f, 2.0f);

        SReflectionProbePropertiesBuffer ReflectionProbePropertiesBuffer;

        ReflectionProbePropertiesBuffer.m_Properties    = glm::vec4(0.0f);
        ReflectionProbePropertiesBuffer.m_Properties[0] = static_cast<float>(HistogramRenderer::GetCurrentExposureHistoryIndex());
        ReflectionProbePropertiesBuffer.m_Properties[1] = 1.0f;

        BufferManager::UploadBufferData(m_ReflectionProbePropertiesBufferPtr, &ReflectionProbePropertiesBuffer);

        // -----------------------------------------------------------------------------
        // Actors
        // -----------------------------------------------------------------------------
        CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Dynamic);
        EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (!rCurrentEntity.GetComponentFacet()->HasComponent<Dt::CSkyComponent>()) continue;

            Dt::CSkyComponent* pSkyComponent = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CSkyComponent>();

            Gfx::CSkyComponent* pSkyFacet = CComponentManager::GetInstance().GetComponent<Gfx::CSkyComponent>(pSkyComponent->GetID());

            // -----------------------------------------------------------------------------
            // Set every surface of this entity into a new render job
            // -----------------------------------------------------------------------------
            unsigned int NumberOfSurfaces = m_SkyboxBoxPtr->GetLOD(0)->GetNumberOfSurfaces();

            for (unsigned int IndexOfSurface = 0; IndexOfSurface < NumberOfSurfaces; ++IndexOfSurface)
            {
                // -----------------------------------------------------------------------------
                // Get surface
                // -----------------------------------------------------------------------------
                CSurfacePtr SurfacePtr = m_SkyboxBoxPtr->GetLOD(0)->GetSurface(IndexOfSurface);

                if (SurfacePtr == 0)
                {
                    break;
                }

                // -----------------------------------------------------------------------------
                // Set textures
                // -----------------------------------------------------------------------------
                ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

                ContextManager::SetTexture(0, static_cast<Gfx::CTexturePtr>(pSkyFacet->GetCubemapPtr()));

                // -----------------------------------------------------------------------------
                // Render
                // -----------------------------------------------------------------------------
                ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

                ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

                ContextManager::SetInputLayout(m_P3N3InputLayoutPtr);

                ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);

                ContextManager::ResetInputLayout();

                ContextManager::ResetIndexBuffer();

                ContextManager::ResetVertexBuffer();

                ContextManager::ResetSampler(0);

                ContextManager::ResetTexture(0);
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Dynamic);
        }

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);
        ContextManager::ResetConstantBuffer(2);
        ContextManager::ResetConstantBuffer(3);

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderDS();

        ContextManager::ResetShaderHS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetRasterizerState();

        ContextManager::ResetDepthStencilState();

        ContextManager::ResetViewPortSet();

        ContextManager::ResetTargetSet();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::RenderEntities(CInternComponent& _rInterLightProbeFacet, const glm::vec3& _rPosition)
    {
        Performance::BeginEvent("Render Entities");

        // -----------------------------------------------------------------------------
        // Find actors
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity;
        Dt::Map::CEntityIterator EndOfEntities;

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        

        ContextManager::SetTargetSet        (_rInterLightProbeFacet.m_TargetSetPtr);
        ContextManager::SetViewPortSet      (_rInterLightProbeFacet.m_ViewPortSetPtr);
        ContextManager::SetBlendState       (StateManager::GetBlendState(0));
        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(0));
        ContextManager::SetRasterizerState  (StateManager::GetRasterizerState(0));

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderGS(m_CubemapGSPtr);

        ContextManager::SetConstantBuffer(0, m_GeometryVPBufferPtr);
        ContextManager::SetConstantBuffer(1, m_GeometryMBufferPtr);
        ContextManager::SetConstantBuffer(2, m_CubemapGSBufferPtr);
        ContextManager::SetConstantBuffer(3, m_SurfaceMaterialBufferPtr);
        ContextManager::SetConstantBuffer(4, m_CameraPropertiesBufferPtr);

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());
        ContextManager::SetResourceBuffer(1, m_LightPropertiesBufferPtr);

        // -----------------------------------------------------------------------------
        // Upload data
        // Note: If you don't want to use a geometry shader you can change the
        // view-projections matrix here!
        // -----------------------------------------------------------------------------
        SGeometryVPBuffer ViewBuffer;

        ViewBuffer.m_Projection = glm::mat4(1.0f);
        ViewBuffer.m_View       = glm::mat4(1.0f);

        BufferManager::UploadBufferData(m_GeometryVPBufferPtr, &ViewBuffer);

        // -----------------------------------------------------------------------------
        // Bind shadow and reflection textures
        // -----------------------------------------------------------------------------
        ContextManager::SetSampler(6, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(6, ReflectionRenderer::GetBRDF());

        if (m_LightJob.m_SpecularTexturePtr != 0)
        {
            ContextManager::SetSampler(7, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(7, m_LightJob.m_SpecularTexturePtr);
        }

        if (m_LightJob.m_DiffuseTexturePtr != 0)
        {
            ContextManager::SetSampler(8, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(8, m_LightJob.m_DiffuseTexturePtr);
        }

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < s_MaxNumberOfLightsPerProbe; ++IndexOfTexture)
        {
            if (m_LightJob.m_ShadowTexturePtrs[IndexOfTexture] != 0)
            {
                ContextManager::SetSampler(9 + IndexOfTexture, SamplerManager::GetSampler(CSampler::MinMagLinearMipPointClamp));

                ContextManager::SetTexture(9 + IndexOfTexture, m_LightJob.m_ShadowTexturePtrs[IndexOfTexture]);
            }
        }

        // -----------------------------------------------------------------------------
        // Actors
        // -----------------------------------------------------------------------------
        CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Dynamic);
        EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            Dt::CMeshComponent* pMeshComponent = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CMeshComponent>();

            if (pMeshComponent == 0) continue;

            CMeshComponent* pGraphicModelActorFacet = CComponentManager::GetInstance().GetComponent<CMeshComponent>(pMeshComponent->GetID());

            CMeshPtr MeshPtr = pGraphicModelActorFacet->GetMesh();

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SCameraPropertiesBuffer ProbeProperties;

            ProbeProperties.m_CameraPosition       = glm::vec4(_rPosition, 1.0f);
            ProbeProperties.m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();

            BufferManager::UploadBufferData(m_CameraPropertiesBufferPtr, &ProbeProperties);

            // -----------------------------------------------------------------------------

            SGeometryMBuffer ModelBuffer;

            ModelBuffer.m_ModelMatrix = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();

            BufferManager::UploadBufferData(m_GeometryMBufferPtr, &ModelBuffer);

            // -----------------------------------------------------------------------------
            // Set every surface of this entity into a new render job
            // -----------------------------------------------------------------------------
            unsigned int NumberOfSurfaces = MeshPtr->GetLOD(0)->GetNumberOfSurfaces();

            for (unsigned int IndexOfSurface = 0; IndexOfSurface < NumberOfSurfaces; ++IndexOfSurface)
            {
                // -----------------------------------------------------------------------------
                // Get surface
                // -----------------------------------------------------------------------------
                CSurfacePtr SurfacePtr = MeshPtr->GetLOD(0)->GetSurface(IndexOfSurface);

                if (SurfacePtr == 0)
                {
                    break;
                }

                // -----------------------------------------------------------------------------
                // Get material and upload correct attributes
                // -----------------------------------------------------------------------------
                CMaterialPtr MaterialPtr = pGraphicModelActorFacet->GetMaterial(IndexOfSurface);

                if (MaterialPtr == 0)
                {
                    MaterialPtr = SurfacePtr->GetMaterial();
                }

                assert(MaterialPtr != 0);

                BufferManager::UploadBufferData(m_SurfaceMaterialBufferPtr, &MaterialPtr->GetMaterialAttributes());

                // -----------------------------------------------------------------------------
                // Set shader
                // -----------------------------------------------------------------------------
                ContextManager::SetShaderVS(SurfacePtr->GetMVPShaderVS());

                ContextManager::SetShaderPS(MaterialPtr->GetForwardShaderPS());

                // -----------------------------------------------------------------------------
                // Set textures
                // -----------------------------------------------------------------------------
                for (unsigned int IndexOfTexture = 0; IndexOfTexture < MaterialPtr->GetTextureSetPS()->GetNumberOfTextures(); ++IndexOfTexture)
                {
                    ContextManager::SetSampler(IndexOfTexture, MaterialPtr->GetSamplerSetPS()->GetSampler(IndexOfTexture));

                    ContextManager::SetTexture(IndexOfTexture, MaterialPtr->GetTextureSetPS()->GetTexture(IndexOfTexture));
                }

                // -----------------------------------------------------------------------------
                // Render
                // -----------------------------------------------------------------------------
                ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

                ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

                ContextManager::SetInputLayout(SurfacePtr->GetMVPShaderVS()->GetInputLayout());

                ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Dynamic);
        }

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < 16; ++IndexOfTexture)
        {
            ContextManager::ResetSampler(IndexOfTexture);

            ContextManager::ResetTexture(IndexOfTexture);
        }

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);
        ContextManager::ResetConstantBuffer(2);
        ContextManager::ResetConstantBuffer(3);

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderDS();

        ContextManager::ResetShaderHS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetRasterizerState();

        ContextManager::ResetDepthStencilState();

        ContextManager::ResetViewPortSet();

        ContextManager::ResetTargetSet();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::RenderFiltering(CInternComponent& _rInterLightProbeFacet, const Dt::CLightProbeComponent& _rDtLightProbeFacet)
    {
        // -----------------------------------------------------------------------------
        // Start updating/filtering
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Filter Distance Light Probe");

        // -----------------------------------------------------------------------------
        // Prepare buffer
        // -----------------------------------------------------------------------------
        SFilterPropertiesBuffer CubemapSettings;

        CubemapSettings.m_LinearRoughness   = 0.0f;
        CubemapSettings.m_NumberOfMiplevels = 0.0f;
        CubemapSettings.m_Intensity         = _rDtLightProbeFacet.GetIntensity();

        BufferManager::UploadBufferData(m_FilteringPSBufferPtr, &CubemapSettings);

        UpdateGeometryBuffer(glm::vec3(0.0f), 0.1f, 1000.0f);

        // -----------------------------------------------------------------------------
        // Prepare
        // -----------------------------------------------------------------------------
        ContextManager::SetBlendState(StateManager::GetBlendState(0));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::NoCull));

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_FilteringVSPtr);

        ContextManager::SetShaderGS(m_CubemapGSPtr);

        ContextManager::SetShaderPS(m_FilteringSpecularPSPtr);

        ContextManager::SetVertexBuffer(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetMVPShaderVS()->GetInputLayout());

        ContextManager::SetConstantBuffer(2, m_CubemapGSBufferPtr);

        ContextManager::SetConstantBuffer(3, m_FilteringPSBufferPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, static_cast<CTexturePtr>(_rInterLightProbeFacet.m_ReflectionCubemapPtr));

        // -----------------------------------------------------------------------------
        // Refine HDR specular from HDR cube map
        // -----------------------------------------------------------------------------
        CInternComponent::CTargetSets&   rSpecularTargetSets   = _rInterLightProbeFacet.m_SpecularHDRTargetSetPtrs;
        CInternComponent::CViewPortSets& rSpecularViewPortSets = _rInterLightProbeFacet.m_SpecularViewPortSetPtrs;

        CInternComponent::CTargetSets::iterator CurrentOfSpecularMipmap = rSpecularTargetSets.begin();
        CInternComponent::CTargetSets::iterator EndOfSpecularMipmaps    = rSpecularTargetSets.end();

        unsigned int IndexOfMipmap = 0;

        float NumberOfMiplevels    = static_cast<float>(rSpecularTargetSets.size());
        float MipmapRoughness      = 0.0f;
        float MipmapRoughnessDelta = 1.0f / NumberOfMiplevels;

        for (; CurrentOfSpecularMipmap != EndOfSpecularMipmaps; ++CurrentOfSpecularMipmap)
        {
            // -----------------------------------------------------------------------------
            // Upload per mipmap changing data
            // -----------------------------------------------------------------------------
            CubemapSettings.m_LinearRoughness   = MipmapRoughness;
            CubemapSettings.m_NumberOfMiplevels = NumberOfMiplevels - 1.0f;
            CubemapSettings.m_Intensity         = _rDtLightProbeFacet.GetIntensity();

            BufferManager::UploadBufferData(m_FilteringPSBufferPtr, &CubemapSettings);

            ContextManager::SetTargetSet(rSpecularTargetSets[IndexOfMipmap]);

            ContextManager::SetViewPortSet(rSpecularViewPortSets[IndexOfMipmap]);

            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            ContextManager::DrawIndexed(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

            // -----------------------------------------------------------------------------
            // Next mip
            // -----------------------------------------------------------------------------
            MipmapRoughness += MipmapRoughnessDelta;

            ++IndexOfMipmap;
        }

        // -----------------------------------------------------------------------------
        // Refine HDR diffuse from HDR cube map
        // -----------------------------------------------------------------------------
        ContextManager::SetShaderPS(m_FilteringDiffusePSPtr);

        {
            CubemapSettings.m_LinearRoughness   = 0.0f;
            CubemapSettings.m_NumberOfMiplevels = 0.0f;
            CubemapSettings.m_Intensity         = _rDtLightProbeFacet.GetIntensity();

            BufferManager::UploadBufferData(m_FilteringPSBufferPtr, &CubemapSettings);

            // -----------------------------------------------------------------------------

            ContextManager::SetTargetSet(_rInterLightProbeFacet.m_DiffuseHDRTargetSetPtr);

            ContextManager::SetViewPortSet(_rInterLightProbeFacet.m_DiffuseViewPortSetPtr);

            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            ContextManager::DrawIndexed(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        }

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetConstantBuffer(3);

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetRasterizerState();

        ContextManager::ResetDepthStencilState();

        ContextManager::ResetViewPortSet();

        ContextManager::ResetTargetSet();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::UpdateLightProperties()
    {
        SLightPropertiesBuffer LightBuffer[s_MaxNumberOfLightsPerProbe];
        unsigned int     IndexOfLight;

        // -----------------------------------------------------------------------------
        // Clear jobs
        // -----------------------------------------------------------------------------
        m_LightJob.m_SpecularTexturePtr = 0;
        m_LightJob.m_DiffuseTexturePtr  = 0;

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < s_MaxNumberOfLightsPerProbe; ++IndexOfTexture)
        {
            m_LightJob.m_ShadowTexturePtrs[IndexOfTexture];
        }

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentLightEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Dynamic);
        Dt::Map::CEntityIterator EndOfLightEntities = Dt::Map::EntitiesEnd();

        // -----------------------------------------------------------------------------
        // Initiate light buffer
        // -----------------------------------------------------------------------------
        IndexOfLight = 0;

        for (; IndexOfLight < s_MaxNumberOfLightsPerProbe; ++ IndexOfLight)
        {
            LightBuffer[IndexOfLight].m_LightType           = 0;
            LightBuffer[IndexOfLight].m_LightViewProjection = glm::mat4(1.0f);
            LightBuffer[IndexOfLight].m_LightPosition       = glm::vec4(0.0f);
            LightBuffer[IndexOfLight].m_LightDirection      = glm::vec4(0.0f);
            LightBuffer[IndexOfLight].m_LightColor          = glm::vec4(0.0f);
            LightBuffer[IndexOfLight].m_LightSettings       = glm::vec4(0.0f);
        }

        // -----------------------------------------------------------------------------
        // Fill with data
        // -----------------------------------------------------------------------------
        IndexOfLight = 0;

        for (; CurrentLightEntity != EndOfLightEntities && IndexOfLight < s_MaxNumberOfLightsPerProbe; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentLightEntity;

            // -----------------------------------------------------------------------------
            // Setup buffer
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetComponentFacet()->HasComponent<Dt::CSunComponent>())
            {
                Dt::CSunComponent* pDtSunFacet = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CSunComponent>();
                Gfx::CSunComponent* pGfxSunFacet = CComponentManager::GetInstance().GetComponent<Gfx::CSunComponent>(pDtSunFacet->GetID());

                if (pDtSunFacet != 0 && pGfxSunFacet != 0)
                {
                    float SunAngularRadius = 0.27f * glm::pi<float>() / 180.0f;
                    float HasShadows = 1.0f;

                    LightBuffer[IndexOfLight].m_LightType           = 1;
                    LightBuffer[IndexOfLight].m_LightViewProjection = pGfxSunFacet->GetCamera()->GetViewProjectionMatrix();
                    LightBuffer[IndexOfLight].m_LightDirection      = glm::normalize(glm::vec4(pDtSunFacet->GetDirection(), 0.0f));
                    LightBuffer[IndexOfLight].m_LightColor          = glm::vec4(pDtSunFacet->GetLightness(), 1.0f);
                    LightBuffer[IndexOfLight].m_LightSettings       = glm::vec4(SunAngularRadius, 0.0f, 0.0f, HasShadows);

                    // -----------------------------------------------------------------------------

                    m_LightJob.m_ShadowTexturePtrs[IndexOfLight] = pGfxSunFacet->GetShadowMapPtr();

                    // -----------------------------------------------------------------------------

                    ++IndexOfLight;
                }
            }
            
            if (rCurrentEntity.GetComponentFacet()->HasComponent<Dt::CPointLightComponent>())
            {
                Dt::CPointLightComponent*  pDtPointFacet  = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CPointLightComponent>();
                Gfx::CPointLightComponent* pGfxPointFacet = CComponentManager::GetInstance().GetComponent<Gfx::CPointLightComponent>(pDtPointFacet->GetID());

                if (pDtPointFacet != 0 && pGfxPointFacet != 0)
                {
                    float InvSqrAttenuationRadius = pDtPointFacet->GetReciprocalSquaredAttenuationRadius();
                    float AngleScale              = pDtPointFacet->GetAngleScale();
                    float AngleOffset             = pDtPointFacet->GetAngleOffset();
                    float HasShadows              = pDtPointFacet->GetShadowType() != Dt::CPointLightComponent::NoShadows ? 1.0f : 0.0f;

                    LightBuffer[IndexOfLight].m_LightType      = 2;
                    LightBuffer[IndexOfLight].m_LightPosition  = glm::vec4(rCurrentEntity.GetWorldPosition(), 1.0f);
                    LightBuffer[IndexOfLight].m_LightDirection = glm::normalize(glm::vec4(pDtPointFacet->GetDirection(), 0.0f));
                    LightBuffer[IndexOfLight].m_LightColor     = glm::vec4(pDtPointFacet->GetLightness(), 1.0f);
                    LightBuffer[IndexOfLight].m_LightSettings  = glm::vec4(InvSqrAttenuationRadius, AngleScale, AngleOffset, HasShadows);

                    LightBuffer[IndexOfLight].m_LightViewProjection = glm::mat4(1.0f);

                    if (pDtPointFacet->GetShadowType() != Dt::CPointLightComponent::NoShadows)
                    {
                        assert(pGfxPointFacet->GetCamera().IsValid());

                        LightBuffer[IndexOfLight].m_LightViewProjection = pGfxPointFacet->GetCamera()->GetViewProjectionMatrix();
                    }

                    // -----------------------------------------------------------------------------

                    if (pDtPointFacet->GetShadowType() != Dt::CPointLightComponent::NoShadows)
                    {
                        m_LightJob.m_ShadowTexturePtrs[IndexOfLight] = pGfxPointFacet->GetTextureSMSet()->GetTexture(0);
                    }

                    // -----------------------------------------------------------------------------

                    ++IndexOfLight;
                }
            }
            
            if (rCurrentEntity.GetComponentFacet()->HasComponent<Dt::CLightProbeComponent>() && (m_LightJob.m_SpecularTexturePtr == 0 && m_LightJob.m_DiffuseTexturePtr == 0))
            {
                Dt::CLightProbeComponent* pDtLightProbeFacet = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CLightProbeComponent>();
                Gfx::CLightProbeComponent* pGfxLightProbeFacet   = CComponentManager::GetInstance().GetComponent<Gfx::CLightProbeComponent>(pDtLightProbeFacet->GetID());

                if (pDtLightProbeFacet != 0 && pGfxLightProbeFacet != 0 && pDtLightProbeFacet->GetType() == Dt::CLightProbeComponent::Sky)
                {
                    LightBuffer[IndexOfLight].m_LightType      = 3;
                    LightBuffer[IndexOfLight].m_LightPosition  = glm::vec4(rCurrentEntity.GetWorldPosition(), 1.0f);
                    LightBuffer[IndexOfLight].m_LightDirection = glm::vec4(0.0f);
                    LightBuffer[IndexOfLight].m_LightColor     = glm::vec4(0.0f);
                    LightBuffer[IndexOfLight].m_LightSettings  = glm::vec4(static_cast<float>(pGfxLightProbeFacet->GetSpecularPtr()->GetNumberOfMipLevels() - 1), 0.0f, 0.0f, 0.0f);

                    LightBuffer[IndexOfLight].m_LightViewProjection = glm::mat4(1.0f);

                    ++IndexOfLight;

                    // -----------------------------------------------------------------------------

                    m_LightJob.m_SpecularTexturePtr = pGfxLightProbeFacet->GetSpecularPtr();
                    m_LightJob.m_DiffuseTexturePtr  = pGfxLightProbeFacet->GetDiffusePtr();
                }
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentLightEntity = CurrentLightEntity.Next(Dt::SEntityCategory::Dynamic);
        }

        BufferManager::UploadBufferData(m_LightPropertiesBufferPtr, &LightBuffer);
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::UpdateGeometryBuffer(const glm::vec3& _rPosition, float _Near, float _Far)
    {
        glm::vec3 EyePosition = _rPosition;
        glm::vec3 UpDirection;
        glm::vec3 TargetPosition;

        SCubemapGeometryBuffer Values;

        Values.m_CubeProjectionMatrix = glm::perspective(glm::half_pi<float>(), 1.0f, _Near, _Far);

        // -----------------------------------------------------------------------------

        TargetPosition = EyePosition + glm::vec3(1.0f, 0.0f, 0.0f);
        UpDirection    = glm::vec3(0.0f, 1.0f, 0.0f);

        Values.m_CubeViewMatrix[0] = glm::lookAt(EyePosition, TargetPosition, UpDirection);

        // -----------------------------------------------------------------------------

        TargetPosition = EyePosition - glm::vec3(1.0f, 0.0f, 0.0f);
        UpDirection    = glm::vec3(0.0f, 1.0f, 0.0f);

        Values.m_CubeViewMatrix[1] = glm::lookAt(EyePosition, TargetPosition, UpDirection);

        // -----------------------------------------------------------------------------

        TargetPosition = EyePosition + glm::vec3(0.0f, 1.0f, 0.0f);
        UpDirection    = -glm::vec3(0.0f, 0.0f, 1.0f);

        Values.m_CubeViewMatrix[2] = glm::lookAt(EyePosition, TargetPosition, UpDirection);

        // -----------------------------------------------------------------------------

        TargetPosition = EyePosition - glm::vec3(0.0f, 1.0f, 0.0f);
        UpDirection    = glm::vec3(0.0f, 0.0f, 1.0f);

        Values.m_CubeViewMatrix[3] = glm::lookAt(EyePosition, TargetPosition, UpDirection);

        // -----------------------------------------------------------------------------

        TargetPosition = EyePosition + glm::vec3(0.0f, 0.0f, 1.0f);
        UpDirection    = glm::vec3(0.0f, 1.0f, 0.0f);

        Values.m_CubeViewMatrix[4] = glm::lookAt(EyePosition, TargetPosition, UpDirection);

        // -----------------------------------------------------------------------------

        TargetPosition = EyePosition - glm::vec3(0.0f, 0.0f, 1.0f);
        UpDirection    = glm::vec3(0.0f, 1.0f, 0.0f);

        Values.m_CubeViewMatrix[5] = glm::lookAt(EyePosition, TargetPosition, UpDirection);

        // -----------------------------------------------------------------------------
        // Upload data
        // -----------------------------------------------------------------------------
        BufferManager::UploadBufferData(m_CubemapGSBufferPtr, &Values);
    }
} // namespace 

namespace Gfx
{
namespace LightProbeManager
{
    void OnStart()
    {
        CGfxLightProbeManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxLightProbeManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxLightProbeManager::GetInstance().Update();
    }
} // namespace LightProbeManager
} // namespace Gfx