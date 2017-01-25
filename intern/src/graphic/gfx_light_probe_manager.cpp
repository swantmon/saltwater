
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

        struct SSpecularCubemapSettings
        {
            float m_LinearRoughness;
            float m_NumberOfMiplevels;
        };

        class CInternLightProbeFacet : public CLightProbeFacet
        {
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

        CShaderPtr m_CustomVSPtr;
        CShaderPtr m_CustomGSPtr;
        CShaderPtr m_CustomPSPtr;

        CBufferSetPtr m_CubemapGSBufferSetPtr;
        CBufferSetPtr m_FilteringPSBufferSetPtr;
        CBufferSetPtr m_CustomVSBufferSetPtr;
        CBufferSetPtr m_CustomPSBufferSetPtr;

        CInputLayoutPtr m_PositionInputLayoutPtr;

        CSamplerSetPtr m_PSSamplerSetPtr;

        CRenderContextPtr m_CubemapRenderContextPtr;

        CLightProbeFacets m_LightprobeFacets;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        CInternLightProbeFacet& AllocateLightProbeFacet(unsigned int _SpecularFaceSize, unsigned int _DiffuseFaceSize);

        void RenderEnvironment(CInternLightProbeFacet& _rInterLightProbeFacet);

        void RenderFiltering(CInternLightProbeFacet& _rInterLightProbeFacet);
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
        , m_CustomVSPtr            ()
        , m_CustomGSPtr            ()
        , m_CustomPSPtr            ()
        , m_CubemapGSBufferSetPtr  ()
        , m_FilteringPSBufferSetPtr()
        , m_CustomVSBufferSetPtr   ()
        , m_CustomPSBufferSetPtr   ()
        , m_PositionInputLayoutPtr ()
        , m_PSSamplerSetPtr        ()
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

        m_CustomVSPtr = ShaderManager::CompileVS("vs_spherical_env_cubemap_generation.glsl", "main");

        m_CustomGSPtr = ShaderManager::CompileGS("gs_spherical_env_cubemap_generation.glsl", "main");

        m_CustomPSPtr = ShaderManager::CompilePS("fs_cubemap_env_cubemap_generation.glsl", "main");

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
        // Sampler
        // -----------------------------------------------------------------------------
        CSamplerPtr LinearFilter = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);

        m_PSSamplerSetPtr = SamplerManager::CreateSamplerSet(LinearFilter, LinearFilter, LinearFilter);

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
        Base::Float3 LookDirection;
        
        float lookAt =  1.5f;
        
        SConstantBufferGS DefaultGSValues;
        
        DefaultGSValues.m_CubeProjectionMatrix.SetRHFieldOfView(Base::RadiansToDegree(Base::SConstants<float>::s_Pi * 0.5f), 1.0f, 1.0f, 20000.0f);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(-lookAt, 0.0f, 0.0f);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[0].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(lookAt, 0.0f, 0.0f);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[1].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, lookAt, 0.0f);
        UpDirection = Base::Float3(0.0f, 0.0f, -1.0f);
        
        DefaultGSValues.m_CubeViewMatrix[3].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, -lookAt, 0.0f);
        UpDirection = Base::Float3(0.0f, 0.0f, 1.0f);
        
        DefaultGSValues.m_CubeViewMatrix[2].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, 0.0f, -lookAt);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[4].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, 0.0f, lookAt);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[5].LookAt(EyePosition, LookDirection, UpDirection);
        
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
        
        m_CustomVSBufferSetPtr    = BufferManager::CreateBufferSet(ViewBuffer);

        m_CubemapGSBufferSetPtr   = BufferManager::CreateBufferSet(GSBuffer);

        m_CustomPSBufferSetPtr    = BufferManager::CreateBufferSet(PSBuffer);
        
        m_FilteringPSBufferSetPtr = BufferManager::CreateBufferSet(SpecularPSBuffer);

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

        m_CustomVSPtr = 0;
        m_CustomGSPtr = 0;
        m_CustomPSPtr = 0;

        m_CubemapGSBufferSetPtr = 0;
        m_FilteringPSBufferSetPtr = 0;

        m_CustomVSBufferSetPtr = 0;
        m_CustomPSBufferSetPtr = 0;

        m_PositionInputLayoutPtr = 0;

        m_PSSamplerSetPtr = 0;

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
                    RenderEnvironment(*pGraphicGlobalProbeFacet);

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
            RenderEnvironment(rGraphicSkyboxFacet);

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
            RenderEnvironment(*pGraphicGlobalProbeLightFacet);

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
        // Create rest of the global probe that is available at any type
        // -> specular and diffuse cubemap
        // -----------------------------------------------------------------------------
        unsigned int SizeOfSpecularCubemap =_SpecularFaceSize;
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
        // TODO by tschwandt
        // Render environment with meshes inside. Now the environment is only made of the sky...

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

            ContextManager::SetConstantBufferSetGS(m_CubemapGSBufferSetPtr);

            ContextManager::SetConstantBufferSetPS(m_FilteringPSBufferSetPtr);

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

            ContextManager::ResetConstantBufferSetPS();

            ContextManager::ResetConstantBufferSetGS();

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

            ContextManager::SetConstantBufferSetGS(m_CubemapGSBufferSetPtr);

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

            ContextManager::ResetConstantBufferSetGS();

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