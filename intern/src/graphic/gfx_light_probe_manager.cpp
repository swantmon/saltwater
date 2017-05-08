
#include "graphic/gfx_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_probe_facet.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_light_probe_facet.h"
#include "graphic/gfx_light_probe_manager.h"
#include "graphic/gfx_sky_facet.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_texture_set.h"
#include "graphic/gfx_view_manager.h"






#include "data/data_actor_type.h"
#include "data/data_entity.h"
#include "data/data_point_light_facet.h"
#include "data/data_transformation_facet.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_actor_facet.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_point_light_facet.h"
#include "data/data_sun_facet.h"
#include "graphic/gfx_sun_facet.h"







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

        static const unsigned int s_MaxNumberOfLightsPerProbe = 10;

    private:

        struct SViewBuffer
        {
            Base::Float4x4 m_View;
            Base::Float4x4 m_Projection;
        };

        struct SConstantBufferGS
        {
            Base::Float4x4 m_CubeProjectionMatrix;
            Base::Float4x4 m_CubeViewMatrix[6];
        };

        struct SConstantBufferPS
        {
            float m_HDRFactor;
            float m_IsHDR;
        };

        struct SPerDrawCallConstantBufferVS
        {
            Base::Float4x4 m_ModelMatrix;
        };

        struct SSpecularCubemapSettings
        {
            float m_LinearRoughness;
            float m_NumberOfMiplevels;
        };

        struct SLightProperties
        {
            Base::Float4x4 m_LightViewProjection;
            Base::Float4   m_LightPosition;
            Base::Float4   m_LightDirection;
            Base::Float4   m_LightColor;
            Base::Float4   m_LightSettings;             // InvSqrAttenuationRadius / SunAngularRadius, AngleScale, AngleOffset, WithShadow
            unsigned int   m_ExposureHistoryIndex;
            unsigned int   m_LightType;
            unsigned int   m_Padding0;
            unsigned int   m_Padding1;
        };

        class CInternLightProbeFacet : public CLightProbeFacet
        {
        public:

            struct SRenderContext
            {
                CInputLayoutPtr m_InputLayoutPtr;
                CTexture2DPtr   m_Texture2DPtr;
                CTextureSetPtr  m_TextureSetPtr;
                CTargetSetPtr   m_TargetSetPtr;
                CViewPortSetPtr m_ViewPortSetPtr;
            };

        public:

            typedef std::vector<CTargetSetPtr>   CTargetSets;
            typedef std::vector<CViewPortSetPtr> CViewPortSets;

        public:

            CInternLightProbeFacet();
            ~CInternLightProbeFacet();

        public:

            CTextureSetPtr m_InputCubemapSetPtr;

            CTargetSets   m_SpecularHDRTargetSetPtrs;
            CViewPortSets m_SpecularViewPortSetPtrs;

            CTargetSetPtr   m_DiffuseHDRTargetSetPtr;
            CViewPortSetPtr m_DiffuseViewPortSetPtr;

            SRenderContext m_SkyboxFromGeometry;

        private:

            friend class CGfxLightProbeManager;
        };

    private:

        typedef Base::CPool<CInternLightProbeFacet, 1> CLightProbeFacets;

    private:

        CMeshPtr m_EnvironmentSpherePtr;

        CShaderPtr m_FilteringVSPtr;
        CShaderPtr m_FilteringGSPtr;
        CShaderPtr m_FilteringDiffusePSPtr;
        CShaderPtr m_FilteringSpecularPSPtr;
        CShaderPtr m_CubemapGeometryVSPtr;
        CShaderPtr m_CubemapGSPtr;
        CShaderPtr m_CubemapPSPtr;
        CShaderPtr m_CubemapTexturePSPtr;

        CBufferPtr m_LightPropertiesBufferPtr;

        CBufferSetPtr m_CubemapGSBufferSetPtr;
        CBufferSetPtr m_FilteringPSBufferSetPtr;
        CBufferSetPtr m_CustomVSBufferSetPtr;
        CBufferSetPtr m_CustomPSBufferSetPtr;
        CBufferSetPtr m_VSBufferSetPtr;
        CBufferSetPtr m_GSBufferSetPtr;
        CBufferSetPtr m_PSBufferSetPtr;

        CInputLayoutPtr m_PositionInputLayoutPtr;

        CRenderContextPtr m_CubemapRenderContextPtr;

        CLightProbeFacets m_LightprobeFacets;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        CInternLightProbeFacet& AllocateLightProbeFacet(unsigned int _SpecularFaceSize, unsigned int _DiffuseFaceSize);

        void RenderEnvironment(CInternLightProbeFacet& _rInterLightProbeFacet);

        void RenderEntities(CInternLightProbeFacet& _rInterLightProbeFacet);

        void RenderFiltering(CInternLightProbeFacet& _rInterLightProbeFacet);

        void BuildLightJobs();
    };
} // namespace 

namespace 
{
    CGfxLightProbeManager::CInternLightProbeFacet::CInternLightProbeFacet()
        : CLightProbeFacet          ()
        , m_InputCubemapSetPtr      ()
        , m_DiffuseHDRTargetSetPtr  ()
        , m_DiffuseViewPortSetPtr   ()
        , m_SpecularHDRTargetSetPtrs()
        , m_SpecularViewPortSetPtrs ()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxLightProbeManager::CInternLightProbeFacet::~CInternLightProbeFacet()
    {
        m_SkyboxFromGeometry.m_InputLayoutPtr = 0;
        m_SkyboxFromGeometry.m_TargetSetPtr   = 0;
        m_SkyboxFromGeometry.m_ViewPortSetPtr = 0;
        m_SkyboxFromGeometry.m_TextureSetPtr  = 0;
        m_SkyboxFromGeometry.m_Texture2DPtr   = 0;

        m_InputCubemapSetPtr     = 0;
        m_DiffuseHDRTargetSetPtr = 0;
        m_DiffuseViewPortSetPtr  = 0;

        m_SpecularHDRTargetSetPtrs.clear();
        m_SpecularViewPortSetPtrs .clear();
    }
} // namespace 

namespace 
{
    CGfxLightProbeManager::CGfxLightProbeManager()
        : m_EnvironmentSpherePtr   ()
        , m_FilteringVSPtr         ()
        , m_FilteringGSPtr         ()
        , m_FilteringDiffusePSPtr  ()
        , m_FilteringSpecularPSPtr ()
        , m_CubemapGSBufferSetPtr  ()
        , m_FilteringPSBufferSetPtr()
        , m_CustomVSBufferSetPtr   ()
        , m_CustomPSBufferSetPtr   ()
        , m_PositionInputLayoutPtr ()
        , m_CubemapRenderContextPtr()
        , m_LightprobeFacets       ()
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

        m_FilteringGSPtr = ShaderManager::CompileGS("gs_lightprobe_sampling.glsl", "main");

        m_FilteringDiffusePSPtr = ShaderManager::CompilePS("fs_lightprobe_diffuse_sampling.glsl", "main");

        m_FilteringSpecularPSPtr = ShaderManager::CompilePS("fs_lightprobe_specular_sampling.glsl", "main");

        m_CubemapGeometryVSPtr = ShaderManager::CompileVS("vs_x1.glsl", "main");
        m_CubemapGSPtr         = ShaderManager::CompileGS("gs_x1.glsl", "main");
        m_CubemapPSPtr         = ShaderManager::CompilePS("fs_x1.glsl", "main");
        m_CubemapTexturePSPtr  = ShaderManager::CompilePS("fs_x1.glsl", "main", "#define USE_TEX_DIFFUSE\n #define USE_TEX_NORMAL\n #define USE_TEX_ROUGHNESS\n #define USE_TEX_METALLIC\n");

        // -----------------------------------------------------------------------------

        const SInputElementDescriptor PositionInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0, 0 , 32, CInputLayout::PerVertex, 0 },
            { "NORMAL"  , 0, CInputLayout::Float3Format, 0, 12, 32, CInputLayout::PerVertex, 0 },
            { "TEXCOORD", 0, CInputLayout::Float2Format, 0, 24, 32, CInputLayout::PerVertex, 0 },
        };

        m_PositionInputLayoutPtr = ShaderManager::CreateInputLayout(PositionInputLayout, 3, m_FilteringVSPtr);

        // -----------------------------------------------------------------------------
        // Render context
        // -----------------------------------------------------------------------------
        CCameraPtr      CameraPtr = ViewManager::GetMainCamera();
        CRenderStatePtr NoDepthStatePtr = StateManager::GetRenderState(CRenderState::NoDepth);

        m_CubemapRenderContextPtr = ContextManager::CreateRenderContext();

        m_CubemapRenderContextPtr->SetCamera(CameraPtr);
        m_CubemapRenderContextPtr->SetRenderState(NoDepthStatePtr);

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SViewBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr ViewBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSpecularCubemapSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SpecularPSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        Base::Float3 EyePosition = Base::Float3(0.0f);
        Base::Float3 UpDirection;
        Base::Float3 TargetPosition;
        
        float lookAt =  1.5f;
        
        SConstantBufferGS DefaultGSValues;
        
        DefaultGSValues.m_CubeProjectionMatrix.SetRHFieldOfView(Base::RadiansToDegree(Base::SConstants<float>::s_Pi * 0.5f), 1.0f, 2.0f, 2000.0f);
        
        // -----------------------------------------------------------------------------
        
        TargetPosition = EyePosition + Base::Float3(lookAt, 0.0f, 0.0f);
        UpDirection    = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[0].LookAt(EyePosition, TargetPosition, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        TargetPosition = EyePosition + Base::Float3(-lookAt, 0.0f, 0.0f);
        UpDirection    = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[1].LookAt(EyePosition, TargetPosition, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        TargetPosition = EyePosition + Base::Float3(0.0f, lookAt, 0.0f);
        UpDirection    = Base::Float3(0.0f, 0.0f, -1.0f);
        
        DefaultGSValues.m_CubeViewMatrix[2].LookAt(EyePosition, TargetPosition, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        TargetPosition = EyePosition + Base::Float3(0.0f, -lookAt, 0.0f);
        UpDirection    = Base::Float3(0.0f, 0.0f, 1.0f);
        
        DefaultGSValues.m_CubeViewMatrix[3].LookAt(EyePosition, TargetPosition, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        TargetPosition = EyePosition + Base::Float3(0.0f, 0.0f, lookAt);
        UpDirection    = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[4].LookAt(EyePosition, TargetPosition, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        TargetPosition = EyePosition + Base::Float3(0.0f, 0.0f, -lookAt);
        UpDirection    = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[5].LookAt(EyePosition, TargetPosition, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SConstantBufferGS);
        ConstanteBufferDesc.m_pBytes        = &DefaultGSValues;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr GSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SConstantBufferPS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBufferVS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr VSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(CMaterial::SMaterialAttributes);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr SurfaceMaterialBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SLightProperties) * s_MaxNumberOfLightsPerProbe;
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_LightPropertiesBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        m_CustomVSBufferSetPtr    = BufferManager::CreateBufferSet(ViewBuffer);

        m_CubemapGSBufferSetPtr   = BufferManager::CreateBufferSet(GSBuffer);

        m_CustomPSBufferSetPtr    = BufferManager::CreateBufferSet(PSBuffer);
        
        m_FilteringPSBufferSetPtr = BufferManager::CreateBufferSet(SpecularPSBuffer);

        m_VSBufferSetPtr = BufferManager::CreateBufferSet(ViewBuffer, VSBuffer);
        m_GSBufferSetPtr = BufferManager::CreateBufferSet(GSBuffer);
        m_PSBufferSetPtr = BufferManager::CreateBufferSet(SurfaceMaterialBufferPtr, m_LightPropertiesBufferPtr);

        // -----------------------------------------------------------------------------
        // Models
        // -----------------------------------------------------------------------------
        Dt::SModelFileDescriptor ModelFileDesc;

        ModelFileDesc.m_pFileName = "envsphere.obj";
        ModelFileDesc.m_GenFlag = Dt::SGeneratorFlag::Nothing;

        Dt::CModel& rSphereModel = Dt::ModelManager::CreateModel(ModelFileDesc);

        SMeshDescriptor ModelDesc;

        ModelDesc.m_pModel = &rSphereModel.GetMesh(0);

        m_EnvironmentSpherePtr = MeshManager::CreateMesh(ModelDesc);

        // -----------------------------------------------------------------------------
        // Register dirty entity handler for automatic sky creation
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxLightProbeManager::OnDirtyEntity));
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::OnExit()
    {
        m_EnvironmentSpherePtr = 0;

        m_FilteringVSPtr = 0;
        m_FilteringGSPtr = 0;
        m_FilteringDiffusePSPtr = 0;
        m_FilteringSpecularPSPtr = 0;
        m_CubemapGeometryVSPtr = 0;
        m_CubemapGSPtr = 0;
        m_CubemapPSPtr = 0;
        m_CubemapTexturePSPtr = 0;

        m_LightPropertiesBufferPtr = 0;

        m_CubemapGSBufferSetPtr = 0;
        m_FilteringPSBufferSetPtr = 0;
        m_VSBufferSetPtr = 0;
        m_GSBufferSetPtr = 0;
        m_PSBufferSetPtr = 0;

        m_CustomVSBufferSetPtr = 0;
        m_CustomPSBufferSetPtr = 0;

        m_PositionInputLayoutPtr = 0;

        m_CubemapRenderContextPtr = 0;

        m_LightprobeFacets.Clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::Update()
    {
        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() == Dt::SLightType::LightProbe)
            {
                Dt::CLightProbeFacet*   pDataGlobalProbeFacet    = static_cast<Dt::CLightProbeFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                CInternLightProbeFacet* pGraphicGlobalProbeFacet = static_cast<CInternLightProbeFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                // -----------------------------------------------------------------------------
                // Check update needs
                // TODO by tschwandt
                // check dynamic mode on dirty entity and add entities to list -> performance
                // -----------------------------------------------------------------------------
                if (pDataGlobalProbeFacet->GetRefreshMode() == Dt::CLightProbeFacet::Dynamic)
                {
                    BuildLightJobs();

                    RenderEntities(*pGraphicGlobalProbeFacet);

                    RenderFiltering(*pGraphicGlobalProbeFacet);
                }
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        // -----------------------------------------------------------------------------
        // Entity check
        // -----------------------------------------------------------------------------
        if (_pEntity->GetCategory() != Dt::SEntityCategory::Light) return;
        if (_pEntity->GetType()     != Dt::SLightType::LightProbe) return;

        // -----------------------------------------------------------------------------
        // Get data
        // -----------------------------------------------------------------------------
        Dt::CLightProbeFacet* pDataGlobalLightProbeFacet = static_cast<Dt::CLightProbeFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Data));

        if (pDataGlobalLightProbeFacet == nullptr) return;

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = _pEntity->GetDirtyFlags();

        if ((DirtyFlags & Dt::CEntity::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create facet
            // -----------------------------------------------------------------------------
            CInternLightProbeFacet& rGraphicSkyboxFacet = AllocateLightProbeFacet(pDataGlobalLightProbeFacet->GetQualityInPixel(), 128);

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            BuildLightJobs();

            RenderEntities(rGraphicSkyboxFacet);

            RenderFiltering(rGraphicSkyboxFacet);

            // -----------------------------------------------------------------------------
            // Set time
            // -----------------------------------------------------------------------------
            rGraphicSkyboxFacet.m_TimeStamp = Core::Time::GetNumberOfFrame();

            // -----------------------------------------------------------------------------
            // Save facet
            // -----------------------------------------------------------------------------
            _pEntity->SetDetailFacet(Dt::SFacetCategory::Graphic, &rGraphicSkyboxFacet);
        }
        else if ((DirtyFlags & Dt::CEntity::DirtyDetail) != 0)
        {
            Dt::CLightProbeFacet*   pDataGlobalProbeLightFacet;
            CInternLightProbeFacet*  pGraphicGlobalProbeLightFacet;

            pDataGlobalProbeLightFacet    = static_cast<Dt::CLightProbeFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Data));
            pGraphicGlobalProbeLightFacet = static_cast<CInternLightProbeFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Graphic));

            // TODO by tschwandt
            // 1. what happens on dirty cubemap and not sky?
            // 2. render general light probe with settings
            BuildLightJobs();

            RenderEntities(*pGraphicGlobalProbeLightFacet);

            RenderFiltering(*pGraphicGlobalProbeLightFacet);

            // -----------------------------------------------------------------------------
            // Set time
            // -----------------------------------------------------------------------------
            Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

            pGraphicGlobalProbeLightFacet->m_TimeStamp = FrameTime;
        }
    }

    // -----------------------------------------------------------------------------

    CGfxLightProbeManager::CInternLightProbeFacet& CGfxLightProbeManager::AllocateLightProbeFacet(unsigned int _SpecularFaceSize, unsigned int _DiffuseFaceSize)
    {
        Gfx::STextureDescriptor  TextureDescriptor;
        Gfx::SViewPortDescriptor ViewPortDesc;

        // -----------------------------------------------------------------------------
        // Create facet
        // -----------------------------------------------------------------------------
        CInternLightProbeFacet& rGraphicLightProbeFacet = m_LightprobeFacets.Allocate();

        // -----------------------------------------------------------------------------
        // Create stuff for local probe
        // -----------------------------------------------------------------------------
        TextureDescriptor.m_NumberOfPixelsU  = _SpecularFaceSize;
        TextureDescriptor.m_NumberOfPixelsV  = _SpecularFaceSize;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 6;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource | CTextureBase::RenderTarget;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPURead;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTextureBase::R16G16B16A16_FLOAT;

        rGraphicLightProbeFacet.m_SkyboxFromGeometry.m_Texture2DPtr = TextureManager::CreateCubeTexture(TextureDescriptor);

        rGraphicLightProbeFacet.m_SkyboxFromGeometry.m_TextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(rGraphicLightProbeFacet.m_SkyboxFromGeometry.m_Texture2DPtr));

        CTargetSetPtr SkyCubeTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(rGraphicLightProbeFacet.m_SkyboxFromGeometry.m_Texture2DPtr));

        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;
        ViewPortDesc.m_Width    = _SpecularFaceSize;
        ViewPortDesc.m_Height   = _SpecularFaceSize;

        CViewPortPtr SkyCubeViewPortPtr = ViewManager::CreateViewPort(ViewPortDesc);

        CViewPortSetPtr SkyCubeViewPortSetPtr = ViewManager::CreateViewPortSet(SkyCubeViewPortPtr);

        rGraphicLightProbeFacet.m_SkyboxFromGeometry.m_TargetSetPtr   = SkyCubeTargetSetPtr;
        rGraphicLightProbeFacet.m_SkyboxFromGeometry.m_ViewPortSetPtr = SkyCubeViewPortSetPtr;

        // -----------------------------------------------------------------------------
        // Create rest of the global probe that is available at any type
        // -> specular and diffuse cubemap
        // -----------------------------------------------------------------------------
        unsigned int SizeOfSpecularCubemap = _SpecularFaceSize;
        unsigned int SizeOfDiffuseCubemap  = _DiffuseFaceSize;

        TextureDescriptor.m_NumberOfPixelsU  = SizeOfSpecularCubemap;
        TextureDescriptor.m_NumberOfPixelsV  = SizeOfSpecularCubemap;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 6;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource | CTextureBase::RenderTarget;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPURead;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTextureBase::R16G16B16A16_FLOAT;
        
        CTexture2DPtr SpecularCube = TextureManager::CreateCubeTexture(TextureDescriptor);
        
        // -----------------------------------------------------------------------------
        
        TextureDescriptor.m_NumberOfPixelsU  = SizeOfDiffuseCubemap;
        TextureDescriptor.m_NumberOfPixelsV  = SizeOfDiffuseCubemap;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        
        CTexture2DPtr DiffuseCube = TextureManager::CreateCubeTexture(TextureDescriptor);
        
        rGraphicLightProbeFacet.m_FilteredSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(SpecularCube), static_cast<CTextureBasePtr>(DiffuseCube));
        
        // -----------------------------------------------------------------------------
        // For all cube maps create a render target for every mip map
        // -----------------------------------------------------------------------------        
        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;

        CInternLightProbeFacet::CTargetSets&   rSpecularTargetSets   = rGraphicLightProbeFacet.m_SpecularHDRTargetSetPtrs;
        CInternLightProbeFacet::CViewPortSets& rSpecularViewPortSets = rGraphicLightProbeFacet.m_SpecularViewPortSetPtrs;

        for (unsigned int IndexOfMipmap = 0; IndexOfMipmap < SpecularCube->GetNumberOfMipLevels(); ++ IndexOfMipmap)
        {
            // -----------------------------------------------------------------------------
            // Target set
            // -----------------------------------------------------------------------------
            CTexture2DPtr MipmapCubeTexture = TextureManager::GetMipmapFromTexture2D(SpecularCube, IndexOfMipmap);
            
            CTargetSetPtr SpecularMipmapTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(MipmapCubeTexture));
            
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
            CTargetSetPtr DiffuseMipmapTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(DiffuseCube));
            
            // -----------------------------------------------------------------------------
            // View port
            // -----------------------------------------------------------------------------
            ViewPortDesc.m_Width    = static_cast<float>(DiffuseCube->GetNumberOfPixelsU());
            ViewPortDesc.m_Height   = static_cast<float>(DiffuseCube->GetNumberOfPixelsV());
            
            CViewPortPtr DiffuseMipmapViewPort = ViewManager::CreateViewPort(ViewPortDesc);
            
            CViewPortSetPtr DiffuseViewPortSetPtr = ViewManager::CreateViewPortSet(DiffuseMipmapViewPort);
            
            // -----------------------------------------------------------------------------
            // Put into light probe
            // -----------------------------------------------------------------------------
            rGraphicLightProbeFacet.m_DiffuseHDRTargetSetPtr = DiffuseMipmapTargetSetPtr;
            rGraphicLightProbeFacet.m_DiffuseViewPortSetPtr  = DiffuseViewPortSetPtr;
        }

        return rGraphicLightProbeFacet;
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::RenderEnvironment(CInternLightProbeFacet& _rInterLightProbeFacet)
    {
        // -----------------------------------------------------------------------------
        // Find sky entity
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity;
        Dt::Map::CEntityIterator EndOfEntities;

        CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        EndOfEntities = Dt::Map::EntitiesEnd();

        Dt::CEntity* pSkyEntity = 0;

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() == Dt::SLightType::Sky)
            {
                pSkyEntity = &rCurrentEntity;

                break;
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }

        if (pSkyEntity == 0)
        {
            return;
        }

        Gfx::CSkyFacet* pSkyFacet = static_cast<Gfx::CSkyFacet*>(pSkyEntity->GetDetailFacet(Dt::SFacetCategory::Graphic));

        assert(pSkyFacet);

        _rInterLightProbeFacet.m_InputCubemapSetPtr = pSkyFacet->GetCubemapSetPtr();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::RenderEntities(CInternLightProbeFacet& _rInterLightProbeFacet)
    {
        // -----------------------------------------------------------------------------
        // Find actors
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity;
        Dt::Map::CEntityIterator EndOfEntities;

        // -----------------------------------------------------------------------------
        // Clear target set
        // -----------------------------------------------------------------------------
        TargetSetManager::ClearTargetSet(_rInterLightProbeFacet.m_SkyboxFromGeometry.m_TargetSetPtr);

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = { 0, 0 };

        ContextManager::SetTargetSet        (_rInterLightProbeFacet.m_SkyboxFromGeometry.m_TargetSetPtr);
        ContextManager::SetViewPortSet      (_rInterLightProbeFacet.m_SkyboxFromGeometry.m_ViewPortSetPtr);
        ContextManager::SetBlendState       (StateManager::GetBlendState(0));
        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(0));
        ContextManager::SetRasterizerState  (StateManager::GetRasterizerState(0));

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_CubemapGeometryVSPtr);

        ContextManager::SetShaderGS(m_CubemapGSPtr);

        ContextManager::SetShaderPS(m_CubemapPSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_VSBufferSetPtr->GetBuffer(0));
        ContextManager::SetConstantBuffer(2, m_VSBufferSetPtr->GetBuffer(1));
        ContextManager::SetConstantBuffer(3, m_GSBufferSetPtr->GetBuffer(0));
        ContextManager::SetConstantBuffer(4, m_PSBufferSetPtr->GetBuffer(0));
        

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());       
        ContextManager::SetResourceBuffer(1, m_LightPropertiesBufferPtr);

        // -----------------------------------------------------------------------------
        // Actors
        // -----------------------------------------------------------------------------
        CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Actor);
        EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() != Dt::SActorType::Mesh || rCurrentEntity.GetLayer() != Dt::SEntityLayer::Default)
            {
                CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);

                continue;
            }

            CMeshActorFacet* pGraphicModelActorFacet = static_cast<CMeshActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

            CMeshPtr MeshPtr = pGraphicModelActorFacet->GetMesh();

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SViewBuffer ViewBuffer;

            ViewBuffer.m_View  = Base::Float4x4::s_Identity;
            ViewBuffer.m_View *= Base::Float4x4().SetTranslation(0.0f, 0.0f, -10.0f);

            BufferManager::UploadConstantBufferData(m_VSBufferSetPtr->GetBuffer(0), &ViewBuffer);


            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();

            BufferManager::UploadConstantBufferData(m_VSBufferSetPtr->GetBuffer(1), &ModelBuffer);

            // -----------------------------------------------------------------------------
            // Set every surface of this entity into a new render job
            // -----------------------------------------------------------------------------
            unsigned int NumberOfSurfaces = MeshPtr->GetLOD(0)->GetNumberOfSurfaces();

            for (unsigned int IndexOfSurface = 0; IndexOfSurface < NumberOfSurfaces; ++IndexOfSurface)
            {
                CSurfacePtr SurfacePtr = MeshPtr->GetLOD(0)->GetSurface(IndexOfSurface);

                if (SurfacePtr == nullptr)
                {
                    break;
                }

                CMaterialPtr MaterialPtr = pGraphicModelActorFacet->GetMaterial(IndexOfSurface);

                if (MaterialPtr == 0)
                {
                    MaterialPtr = SurfacePtr->GetMaterial();
                }

                BufferManager::UploadConstantBufferData(m_PSBufferSetPtr->GetBuffer(0), &MaterialPtr->GetMaterialAttributes());

                if (SurfacePtr->GetMaterial()->GetTextureSetPS()->GetTexture(0) != 0)
                {
                    ContextManager::SetShaderPS(m_CubemapTexturePSPtr);
                }
                else
                {
                    ContextManager::SetShaderPS(m_CubemapPSPtr);
                }

                for (unsigned int IndexOfTexture = 0; IndexOfTexture < MaterialPtr->GetTextureSetPS()->GetNumberOfTextures(); ++IndexOfTexture)
                {
                    ContextManager::SetSampler(IndexOfTexture, MaterialPtr->GetSamplerSetPS()->GetSampler(IndexOfTexture));

                    ContextManager::SetTexture(IndexOfTexture, MaterialPtr->GetTextureSetPS()->GetTexture(IndexOfTexture));
                }

                // -----------------------------------------------------------------------------
                // Render
                // -----------------------------------------------------------------------------
                ContextManager::SetVertexBufferSet(SurfacePtr->GetVertexBuffer(), pOffset);

                ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

                ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

                ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);

                ContextManager::ResetInputLayout();

                ContextManager::ResetIndexBuffer();

                ContextManager::ResetVertexBufferSet();

                for (unsigned int IndexOfTexture = 0; IndexOfTexture < MaterialPtr->GetTextureSetPS()->GetNumberOfTextures(); ++IndexOfTexture)
                {
                    ContextManager::ResetSampler(IndexOfTexture);

                    ContextManager::ResetTexture(IndexOfTexture);
                }
            }
            

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);
        }
        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);
        ContextManager::ResetConstantBuffer(2);
        ContextManager::ResetConstantBuffer(3);

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderDS();

        ContextManager::ResetShaderHS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        ContextManager::ResetTopology();

        // TODO: this has to be the same texture
        TextureManager::UpdateMipmap(_rInterLightProbeFacet.m_SkyboxFromGeometry.m_Texture2DPtr);

        _rInterLightProbeFacet.m_InputCubemapSetPtr = _rInterLightProbeFacet.m_SkyboxFromGeometry.m_TextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::RenderFiltering(CInternLightProbeFacet& _rInterLightProbeFacet)
    {
        // -----------------------------------------------------------------------------
        // Start updating/filtering
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Filter Distance Light Probe");

        unsigned int IndexOfMipmap = 0;

        // -----------------------------------------------------------------------------
        // Refine HDR specular from HDR cube map
        // -----------------------------------------------------------------------------
        CInternLightProbeFacet::CTargetSets&   rSpecularTargetSets   = _rInterLightProbeFacet.m_SpecularHDRTargetSetPtrs;
        CInternLightProbeFacet::CViewPortSets& rSpecularViewPortSets = _rInterLightProbeFacet.m_SpecularViewPortSetPtrs;

        CInternLightProbeFacet::CTargetSets::iterator CurrentOfSpecularMipmap = rSpecularTargetSets.begin();
        CInternLightProbeFacet::CTargetSets::iterator EndOfSpecularMipmaps    = rSpecularTargetSets.end();

        IndexOfMipmap = 0;

        // TODO by tschwandt
        // limit the number of mip levels
        float NumberOfMiplevels = static_cast<float>(rSpecularTargetSets.size());

        float MipmapRoughness      = 0.0f;
        float MipmapRoughnessDelta = 1.0f / NumberOfMiplevels;

        for (; CurrentOfSpecularMipmap != EndOfSpecularMipmaps; ++CurrentOfSpecularMipmap)
        {
            // -----------------------------------------------------------------------------
            // Prepare render target for environment cube map generation per mipmap
            // -----------------------------------------------------------------------------
            m_CubemapRenderContextPtr->SetViewPortSet(rSpecularViewPortSets[IndexOfMipmap]);
            m_CubemapRenderContextPtr->SetTargetSet(rSpecularTargetSets[IndexOfMipmap]);

            TargetSetManager::ClearTargetSet(rSpecularTargetSets[IndexOfMipmap]);

            // -----------------------------------------------------------------------------
            // Upload per mipmap changing data
            // -----------------------------------------------------------------------------
            SSpecularCubemapSettings SpecularCubemapSettings;

            SpecularCubemapSettings.m_LinearRoughness = MipmapRoughness;
            SpecularCubemapSettings.m_NumberOfMiplevels = NumberOfMiplevels - 1.0f;

            BufferManager::UploadConstantBufferData(m_FilteringPSBufferSetPtr->GetBuffer(0), &SpecularCubemapSettings);

            // -----------------------------------------------------------------------------

            const unsigned int pOffset[] = { 0, 0 };

            // -----------------------------------------------------------------------------
            // Setup
            // -----------------------------------------------------------------------------
            ContextManager::SetRenderContext(m_CubemapRenderContextPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(m_FilteringVSPtr);

            ContextManager::SetShaderGS(m_FilteringGSPtr);

            ContextManager::SetShaderPS(m_FilteringSpecularPSPtr);

            ContextManager::SetVertexBufferSet(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(m_PositionInputLayoutPtr);

            ContextManager::SetConstantBuffer(0, m_CubemapGSBufferSetPtr->GetBuffer(0));

            ContextManager::SetConstantBuffer(1, m_FilteringPSBufferSetPtr->GetBuffer(0));

            ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(0, _rInterLightProbeFacet.m_InputCubemapSetPtr->GetTexture(0));

            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            ContextManager::DrawIndexed(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

            // -----------------------------------------------------------------------------
            // Reset
            // -----------------------------------------------------------------------------
            ContextManager::ResetTexture(0);

            ContextManager::ResetSampler(0);

            ContextManager::ResetConstantBuffer(0);

            ContextManager::ResetConstantBuffer(1);

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBufferSet();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderGS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetTopology();

            ContextManager::ResetRenderContext();

            MipmapRoughness += MipmapRoughnessDelta;

            ++IndexOfMipmap;
        }

        // -----------------------------------------------------------------------------
        // Refine HDR diffuse from HDR cube map
        // -----------------------------------------------------------------------------
        {
            // -----------------------------------------------------------------------------
            // Prepare render target for environment cube map generation per mipmap
            // -----------------------------------------------------------------------------
            m_CubemapRenderContextPtr->SetViewPortSet(_rInterLightProbeFacet.m_DiffuseViewPortSetPtr);
            m_CubemapRenderContextPtr->SetTargetSet  (_rInterLightProbeFacet.m_DiffuseHDRTargetSetPtr);

            TargetSetManager::ClearTargetSet(_rInterLightProbeFacet.m_DiffuseHDRTargetSetPtr);

            // -----------------------------------------------------------------------------

            const unsigned int pOffset[] = { 0, 0 };

            // -----------------------------------------------------------------------------
            // Setup
            // -----------------------------------------------------------------------------
            ContextManager::SetRenderContext(m_CubemapRenderContextPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(m_FilteringVSPtr);

            ContextManager::SetShaderGS(m_FilteringGSPtr);

            ContextManager::SetShaderPS(m_FilteringDiffusePSPtr);

            ContextManager::SetVertexBufferSet(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(m_PositionInputLayoutPtr);

            ContextManager::SetConstantBuffer(0, m_CubemapGSBufferSetPtr->GetBuffer(0));

            ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

            ContextManager::SetTexture(0, _rInterLightProbeFacet.m_InputCubemapSetPtr->GetTexture(0));

            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            ContextManager::DrawIndexed(m_EnvironmentSpherePtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

            // -----------------------------------------------------------------------------
            // Reset
            // -----------------------------------------------------------------------------
            ContextManager::ResetTexture(0);

            ContextManager::ResetSampler(0);

            ContextManager::ResetConstantBuffer(0);

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBufferSet();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderGS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetTopology();

            ContextManager::ResetRenderContext();
        }

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxLightProbeManager::BuildLightJobs()
    {
        SLightProperties LightBuffer[s_MaxNumberOfLightsPerProbe];
        unsigned int     IndexOfLight;

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentLightEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfLightEntities = Dt::Map::EntitiesEnd();


        // -----------------------------------------------------------------------------
        // Initiate light buffer
        // -----------------------------------------------------------------------------
        IndexOfLight = 0;

        for (; IndexOfLight < s_MaxNumberOfLightsPerProbe; ++ IndexOfLight)
        {
            LightBuffer[IndexOfLight].m_LightType            = 0;
            LightBuffer[IndexOfLight].m_LightViewProjection  .SetIdentity();
            LightBuffer[IndexOfLight].m_LightPosition        .SetZero();
            LightBuffer[IndexOfLight].m_LightDirection       .SetZero();
            LightBuffer[IndexOfLight].m_LightColor           .SetZero();
            LightBuffer[IndexOfLight].m_LightSettings        .SetZero();
            LightBuffer[IndexOfLight].m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();
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
            if (rCurrentEntity.GetType() == Dt::SLightType::Sun)
            {
                Dt::CSunLightFacet* pDtSunFacet  = static_cast<Dt::CSunLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                Gfx::CSunFacet*     pGfxSunFacet = static_cast<Gfx::CSunFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                assert(pDtSunFacet != 0 && pGfxSunFacet != 0);

                LightBuffer[IndexOfLight].m_LightType            = 1;
                LightBuffer[IndexOfLight].m_LightViewProjection  = pGfxSunFacet->GetCamera()->GetViewProjectionMatrix();
                LightBuffer[IndexOfLight].m_LightDirection       = Base::Float4(pDtSunFacet->GetDirection(), 0.0f).Normalize();
                LightBuffer[IndexOfLight].m_LightColor           = Base::Float4(pDtSunFacet->GetLightness(), 1.0f);
                LightBuffer[IndexOfLight].m_LightSettings[0]     = 0.27f * Base::SConstants<float>::s_Pi / 180.0f;
                LightBuffer[IndexOfLight].m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();

                ++IndexOfLight;
            }
            else if (rCurrentEntity.GetType() == Dt::SLightType::Point)
            {
                Dt::CPointLightFacet*  pDtPointFacet  = static_cast<Dt::CPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                Gfx::CPointLightFacet* pGfxPointFacet = static_cast<Gfx::CPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                assert(pDtPointFacet != 0 && pGfxPointFacet != 0);

                float InvSqrAttenuationRadius = pDtPointFacet->GetReciprocalSquaredAttenuationRadius();
                float AngleScale              = pDtPointFacet->GetAngleScale();
                float AngleOffset             = pDtPointFacet->GetAngleOffset();
                float HasShadows              = pDtPointFacet->GetShadowType() != Dt::CPointLightFacet::NoShadows ? 1.0f : 0.0f;
            
                LightBuffer[IndexOfLight].m_LightType      = 2;
                LightBuffer[IndexOfLight].m_LightPosition  = Base::Float4(rCurrentEntity.GetWorldPosition(), 1.0f);
                LightBuffer[IndexOfLight].m_LightDirection = Base::Float4(pDtPointFacet->GetDirection(), 0.0f).Normalize();
                LightBuffer[IndexOfLight].m_LightColor     = Base::Float4(pDtPointFacet->GetLightness(), 1.0f);
                LightBuffer[IndexOfLight].m_LightSettings  = Base::Float4(InvSqrAttenuationRadius, AngleScale, AngleOffset, HasShadows);

                LightBuffer[IndexOfLight].m_LightViewProjection.SetIdentity();

                if (pDtPointFacet->GetShadowType() != Dt::CPointLightFacet::NoShadows)
                {
                    assert(pGfxPointFacet->GetCamera().IsValid());

                    LightBuffer[IndexOfLight].m_LightViewProjection = pGfxPointFacet->GetCamera()->GetViewProjectionMatrix();
                }

                ++IndexOfLight;
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentLightEntity = CurrentLightEntity.Next(Dt::SEntityCategory::Light);
        }

        BufferManager::UploadConstantBufferData(m_LightPropertiesBufferPtr, &LightBuffer);
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