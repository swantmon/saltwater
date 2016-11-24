
#include "graphic/gfx_precompiled.h"

#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_facet.h"
#include "data/data_model_manager.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_sky_facet.h"
#include "graphic/gfx_sky_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

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

        CSkyFacetPtr CreateSky(unsigned int _FaceSize, const Base::Char* _pFileName);

    private:

        struct SRenderContext
        {
            CShaderPtr        m_VSPtr;
            CShaderPtr        m_GSPtr;
            CShaderPtr        m_PSPtr;
            CBufferSetPtr     m_VSBufferSetPtr;
            CBufferSetPtr     m_GSBufferSetPtr;
            CBufferSetPtr     m_PSBufferSetPtr;
            CInputLayoutPtr   m_InputLayoutPtr;
            CMeshPtr          m_MeshPtr;
            CTextureSetPtr    m_TextureSetPtr;
            CSamplerSetPtr    m_SamplerSetPtr;
        };

        struct SCubemapBufferGS
        {
            Base::Float4x4 m_CubeProjectionMatrix;
            Base::Float4x4 m_CubeViewMatrix[6];
        };

        struct SCubemapBufferPS
        {
            float m_HDRFactor;
            float m_IsHDR;
        };

        class CInternSkyFacet : public CSkyFacet
        {
        public:

            CInternSkyFacet();
            ~CInternSkyFacet();

        public:

            CRenderContextPtr m_RenderContextPtr;
            CTargetSetPtr     m_TargetSetPtr;
            CViewPortSetPtr   m_ViewPortSetPtr;
            CTexture2DPtr     m_InputTexture2DPtr;
            CTextureSetPtr    m_InputTextureSetPtr;

        private:

            friend class CGfxSkyManager;
        };

    private:

        typedef Base::CPool<CInternSkyFacet, 1> CSkyfacets;

    private:

        SRenderContext m_SkyboxFromPanorama;
        SRenderContext m_SkyboxFromCubemap;
        CSkyfacets     m_Skyfacets;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        CInternSkyFacet& AllocateSkyFacet(unsigned int _FaceSize);

        void RenderSkyboxFromPanorama(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromCubemap(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);
    };
} // namespace 

namespace 
{
    CGfxSkyManager::CInternSkyFacet::CInternSkyFacet()
        : CSkyFacet           ()
        , m_RenderContextPtr  ()
        , m_TargetSetPtr      ()
        , m_ViewPortSetPtr    ()
        , m_InputTexture2DPtr ()
        , m_InputTextureSetPtr()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxSkyManager::CInternSkyFacet::~CInternSkyFacet()
    {
        m_RenderContextPtr   = 0;
        m_TargetSetPtr       = 0;
        m_ViewPortSetPtr     = 0;
        m_InputTexture2DPtr  = 0;
        m_InputTextureSetPtr = 0;
    }
} // namespace 

namespace 
{
    CGfxSkyManager::CGfxSkyManager()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxSkyManager::~CGfxSkyManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::OnStart()
    {
        m_SkyboxFromPanorama.m_VSPtr            = 0;
        m_SkyboxFromPanorama.m_GSPtr            = 0;
        m_SkyboxFromPanorama.m_PSPtr            = 0;
        m_SkyboxFromPanorama.m_VSBufferSetPtr   = 0;
        m_SkyboxFromPanorama.m_GSBufferSetPtr   = 0;
        m_SkyboxFromPanorama.m_PSBufferSetPtr   = 0;
        m_SkyboxFromPanorama.m_InputLayoutPtr   = 0;
        m_SkyboxFromPanorama.m_MeshPtr          = 0;
        m_SkyboxFromPanorama.m_TextureSetPtr    = 0;
        m_SkyboxFromPanorama.m_SamplerSetPtr    = 0;

        m_SkyboxFromCubemap.m_VSPtr            = 0;
        m_SkyboxFromCubemap.m_GSPtr            = 0;
        m_SkyboxFromCubemap.m_PSPtr            = 0;
        m_SkyboxFromCubemap.m_VSBufferSetPtr   = 0;
        m_SkyboxFromCubemap.m_GSBufferSetPtr   = 0;
        m_SkyboxFromCubemap.m_PSBufferSetPtr   = 0;
        m_SkyboxFromCubemap.m_InputLayoutPtr   = 0;
        m_SkyboxFromCubemap.m_MeshPtr          = 0;
        m_SkyboxFromCubemap.m_TextureSetPtr    = 0;
        m_SkyboxFromCubemap.m_SamplerSetPtr    = 0;

        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        CShaderPtr CubemapVSPtr         = ShaderManager::CompileVS("vs_spherical_env_cubemap_generation.glsl", "main");
        CShaderPtr CubemapGSPtr         = ShaderManager::CompileGS("gs_spherical_env_cubemap_generation.glsl", "main");
        CShaderPtr CubemapPanoramaPSPtr = ShaderManager::CompilePS("fs_spherical_env_cubemap_generation.glsl", "main");
        CShaderPtr CubemapCubemapPSPtr  = ShaderManager::CompilePS("fs_cubemap_env_cubemap_generation.glsl", "main");

        const SInputElementDescriptor PositionInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0, 0 , 32, CInputLayout::PerVertex, 0 },
            { "NORMAL"  , 0, CInputLayout::Float3Format, 0, 12, 32, CInputLayout::PerVertex, 0 },
            { "TEXCOORD", 0, CInputLayout::Float2Format, 0, 24, 32, CInputLayout::PerVertex, 0 },
        };

        CInputLayoutPtr P3N3T2CubemapInputLayoutPtr = ShaderManager::CreateInputLayout(PositionInputLayout, 3, CubemapVSPtr);

        m_SkyboxFromPanorama.m_VSPtr          = CubemapVSPtr;
        m_SkyboxFromPanorama.m_GSPtr          = CubemapGSPtr;
        m_SkyboxFromPanorama.m_PSPtr          = CubemapPanoramaPSPtr;
        m_SkyboxFromPanorama.m_InputLayoutPtr = P3N3T2CubemapInputLayoutPtr;

        m_SkyboxFromCubemap.m_VSPtr          = CubemapVSPtr;
        m_SkyboxFromCubemap.m_GSPtr          = CubemapGSPtr;
        m_SkyboxFromCubemap.m_PSPtr          = CubemapCubemapPSPtr;
        m_SkyboxFromCubemap.m_InputLayoutPtr = P3N3T2CubemapInputLayoutPtr;

        // -----------------------------------------------------------------------------
        // Sampler
        // -----------------------------------------------------------------------------
        CSamplerPtr LinearFilter = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);

        CSamplerSetPtr SamplerSetPtr = SamplerManager::CreateSamplerSet(LinearFilter);

        m_SkyboxFromPanorama.m_SamplerSetPtr = SamplerSetPtr;

        m_SkyboxFromCubemap.m_SamplerSetPtr = SamplerSetPtr;

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;

        Base::Float3 EyePosition = Base::Float3(0.0f);
        Base::Float3 UpDirection;
        Base::Float3 LookDirection;
        
        SCubemapBufferGS DefaultGSValues;
        
        DefaultGSValues.m_CubeProjectionMatrix.SetRHFieldOfView(Base::RadiansToDegree(Base::SConstants<float>::s_Pi * 0.5f), 1.0f, 0.3f, 20000.0f);
        
        // -----------------------------------------------------------------------------
        // By creating a cube map in OpenGL, several facts should be considered:
        //  1. OpenGL cubemaps has an right handed coord system
        //  2. Texcoords starts in the upper left corner (normally in the lower left 
        //     corner)
        // -----------------------------------------------------------------------------

        // Right; +X
        LookDirection = EyePosition + Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[1].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        // Left; -X
        LookDirection = EyePosition - Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[0].LookAt(EyePosition, LookDirection, UpDirection);

        // -----------------------------------------------------------------------------

        // Front; +Y
        LookDirection = EyePosition + Base::Float3::s_AxisY;
        UpDirection = Base::Float3::s_Zero - Base::Float3::s_AxisZ;

        DefaultGSValues.m_CubeViewMatrix[3].LookAt(EyePosition, LookDirection, UpDirection);

        // -----------------------------------------------------------------------------

        // Back; -Y
        LookDirection = EyePosition - Base::Float3::s_AxisY;
        UpDirection   = Base::Float3::s_AxisZ;

        DefaultGSValues.m_CubeViewMatrix[2].LookAt(EyePosition, LookDirection, UpDirection);

        // -----------------------------------------------------------------------------
        
        // Top; +Z
        LookDirection = EyePosition - Base::Float3::s_AxisZ;
        UpDirection = Base::Float3::s_Zero - Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[4].LookAt(EyePosition, LookDirection, UpDirection);

        // -----------------------------------------------------------------------------

        // Bottom; -Z
        LookDirection = EyePosition + Base::Float3::s_AxisZ;
        UpDirection = Base::Float3::s_Zero - Base::Float3::s_AxisY;

        DefaultGSValues.m_CubeViewMatrix[5].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCubemapBufferGS);
        ConstanteBufferDesc.m_pBytes        = &DefaultGSValues;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr CubemapGSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
               
        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCubemapBufferPS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr CubemapPSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        CBufferSetPtr CubemapGSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSBuffer);
        CBufferSetPtr CubemapPSBufferSetPtr = BufferManager::CreateBufferSet(CubemapPSBuffer);

        m_SkyboxFromPanorama.m_VSBufferSetPtr = 0;
        m_SkyboxFromPanorama.m_GSBufferSetPtr = CubemapGSBufferSetPtr;
        m_SkyboxFromPanorama.m_PSBufferSetPtr = CubemapPSBufferSetPtr;

        m_SkyboxFromCubemap.m_VSBufferSetPtr = 0;
        m_SkyboxFromCubemap.m_GSBufferSetPtr = CubemapGSBufferSetPtr;
        m_SkyboxFromCubemap.m_PSBufferSetPtr = CubemapPSBufferSetPtr;

        // -----------------------------------------------------------------------------
        // Models
        // -----------------------------------------------------------------------------
        SMeshDescriptor ModelDescr;
        Dt::SModelFileDescriptor ModelFileDesc;

        ModelFileDesc.m_pFileName = "envsphere.obj";
        ModelFileDesc.m_GenFlag = Dt::SGeneratorFlag::Nothing;

        Dt::CModel& rSphereModel = Dt::ModelManager::CreateModel(ModelFileDesc);

        ModelDescr.m_pModel = &rSphereModel.GetMesh(0);

        CMeshPtr CubemapTextureSpherePtr = MeshManager::CreateMesh(ModelDescr);

        m_SkyboxFromPanorama.m_MeshPtr = CubemapTextureSpherePtr;

        m_SkyboxFromCubemap.m_MeshPtr  = CubemapTextureSpherePtr;

        // -----------------------------------------------------------------------------
        // Register dirty entity handler for automatic sky creation
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxSkyManager::OnDirtyEntity));
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::OnExit()
    {
        m_SkyboxFromPanorama.m_VSPtr          = 0;
        m_SkyboxFromPanorama.m_GSPtr          = 0;
        m_SkyboxFromPanorama.m_PSPtr          = 0;
        m_SkyboxFromPanorama.m_VSBufferSetPtr = 0;
        m_SkyboxFromPanorama.m_GSBufferSetPtr = 0;
        m_SkyboxFromPanorama.m_PSBufferSetPtr = 0;
        m_SkyboxFromPanorama.m_InputLayoutPtr = 0;
        m_SkyboxFromPanorama.m_MeshPtr        = 0;
        m_SkyboxFromPanorama.m_TextureSetPtr  = 0;
        m_SkyboxFromPanorama.m_SamplerSetPtr  = 0;

        m_SkyboxFromCubemap.m_VSPtr          = 0;
        m_SkyboxFromCubemap.m_GSPtr          = 0;
        m_SkyboxFromCubemap.m_PSPtr          = 0;
        m_SkyboxFromCubemap.m_VSBufferSetPtr = 0;
        m_SkyboxFromCubemap.m_GSBufferSetPtr = 0;
        m_SkyboxFromCubemap.m_PSBufferSetPtr = 0;
        m_SkyboxFromCubemap.m_InputLayoutPtr = 0;
        m_SkyboxFromCubemap.m_MeshPtr        = 0;
        m_SkyboxFromCubemap.m_TextureSetPtr  = 0;
        m_SkyboxFromCubemap.m_SamplerSetPtr  = 0;

        m_Skyfacets.Clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::Update()
    {

    }

    // -----------------------------------------------------------------------------

    CSkyFacetPtr CGfxSkyManager::CreateSky(unsigned int _FaceSize, const Base::Char* _pFileName)
    {
        CInternSkyFacet& rInternSky = AllocateSkyFacet(_FaceSize);

        // -----------------------------------------------------------------------------
        // Create texture
        // -----------------------------------------------------------------------------
        STextureDescriptor TextureDescriptor;
        
        TextureDescriptor.m_NumberOfPixelsU  = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV  = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 6;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPURead;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = _pFileName;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTextureBase::R8G8B8A8_UBYTE;
        
        rInternSky.m_InputTexture2DPtr  = TextureManager::CreateCubeTexture(TextureDescriptor);

        rInternSky.m_InputTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(rInternSky.m_InputTexture2DPtr));

        // -----------------------------------------------------------------------------
        // Render cube map
        // TODO: Find out if texture is cube or not
        // -----------------------------------------------------------------------------
        RenderSkyboxFromCubemap(&rInternSky);

        // -----------------------------------------------------------------------------
        // Set time
        // -----------------------------------------------------------------------------
        rInternSky.m_TimeStamp = Core::Time::GetNumberOfFrame();

        return CSkyFacetPtr(&rInternSky);
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        // -----------------------------------------------------------------------------
        // Entity check
        // -----------------------------------------------------------------------------
        if (_pEntity->GetCategory() != Dt::SEntityCategory::Light) return;
        if (_pEntity->GetType()     != Dt::SLightType::Skybox) return;

        // -----------------------------------------------------------------------------
        // Get data
        // -----------------------------------------------------------------------------
        Dt::CSkyFacet* pDataSkyboxFacet = static_cast<Dt::CSkyFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Data));

        if (pDataSkyboxFacet == nullptr) return;

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
            CInternSkyFacet& rGraphicSkyboxFacet = AllocateSkyFacet(2048);

            // -----------------------------------------------------------------------------
            // Create textures and render sky
            // -----------------------------------------------------------------------------
            if (pDataSkyboxFacet->GetType() == Dt::CSkyFacet::Panorama)
            {
                if (pDataSkyboxFacet->GetHasPanorama())
                {
                    unsigned int Hash = pDataSkyboxFacet->GetPanorama()->GetHash();

                    CTexture2DPtr PanoramaPtr = TextureManager::GetTexture2DByHash(Hash);

                    if (PanoramaPtr.IsValid())
                    {
                        rGraphicSkyboxFacet.m_InputTexture2DPtr = PanoramaPtr;

                        rGraphicSkyboxFacet.m_InputTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(PanoramaPtr));

                        RenderSkyboxFromPanorama(&rGraphicSkyboxFacet, pDataSkyboxFacet->GetIntensity());
                    }
                }
            }
            else if (pDataSkyboxFacet->GetType() == Dt::CSkyFacet::Cubemap)
            {
                if (pDataSkyboxFacet->GetHasCubemap())
                {
                    unsigned int Hash = pDataSkyboxFacet->GetCubemap()->GetHash();

                    CTexture2DPtr CubemapPtr = TextureManager::GetTexture2DByHash(Hash);

                    if (CubemapPtr.IsValid())
                    {
                        rGraphicSkyboxFacet.m_InputTexture2DPtr = CubemapPtr;

                        rGraphicSkyboxFacet.m_InputTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(CubemapPtr));

                        RenderSkyboxFromCubemap(&rGraphicSkyboxFacet, pDataSkyboxFacet->GetIntensity());
                    }
                }
            }

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
            Dt::CSkyFacet* pDataSkyboxFacet;
            CInternSkyFacet*  pGraphicSkyboxFacet;

            pDataSkyboxFacet    = static_cast<Dt::CSkyFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Data));
            pGraphicSkyboxFacet = static_cast<CInternSkyFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Graphic));

            if (pDataSkyboxFacet->GetType() == Dt::CSkyFacet::Panorama)
            {
                if (pDataSkyboxFacet->GetHasPanorama())
                {
                    unsigned int Hash = pDataSkyboxFacet->GetPanorama()->GetHash();

                    CTexture2DPtr PanoramaPtr = TextureManager::GetTexture2DByHash(Hash);

                    if (PanoramaPtr.IsValid())
                    {
                        pGraphicSkyboxFacet->m_InputTexture2DPtr = PanoramaPtr;

                        pGraphicSkyboxFacet->m_InputTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(PanoramaPtr));

                        RenderSkyboxFromPanorama(pGraphicSkyboxFacet, pDataSkyboxFacet->GetIntensity());
                    }
                }
            }
            else if (pDataSkyboxFacet->GetType() == Dt::CSkyFacet::Cubemap)
            {
                if (pDataSkyboxFacet->GetHasCubemap())
                {
                    unsigned int Hash = pDataSkyboxFacet->GetCubemap()->GetHash();

                    CTexture2DPtr CubemapPtr = TextureManager::GetTexture2DByHash(Hash);

                    if (CubemapPtr.IsValid())
                    {
                        pGraphicSkyboxFacet->m_InputTexture2DPtr = CubemapPtr;

                        pGraphicSkyboxFacet->m_InputTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(CubemapPtr));

                        RenderSkyboxFromCubemap(pGraphicSkyboxFacet, pDataSkyboxFacet->GetIntensity());
                    }
                }
            }

            // -----------------------------------------------------------------------------
            // Set time
            // -----------------------------------------------------------------------------
            pGraphicSkyboxFacet->m_TimeStamp = Core::Time::GetNumberOfFrame();
        }
    }

    // -----------------------------------------------------------------------------

    CGfxSkyManager::CInternSkyFacet& CGfxSkyManager::AllocateSkyFacet(unsigned int _FaceSize)
    {
        // -----------------------------------------------------------------------------
        // Create facet
        // -----------------------------------------------------------------------------
        CInternSkyFacet& rGraphicSkyboxFacet = m_Skyfacets.Allocate();

        // -----------------------------------------------------------------------------
        // Cubemap
        // -----------------------------------------------------------------------------
        STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = _FaceSize;
        TextureDescriptor.m_NumberOfPixelsV  = _FaceSize;
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
        
        rGraphicSkyboxFacet.m_CubemapPtr = TextureManager::CreateCubeTexture(TextureDescriptor);

        rGraphicSkyboxFacet.m_CubemapSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(rGraphicSkyboxFacet.m_CubemapPtr));

        // -----------------------------------------------------------------------------
        // Target Set
        // -----------------------------------------------------------------------------
        CTexture2DPtr FirstMipmapCubeTexture = TextureManager::GetMipmapFromTexture2D(rGraphicSkyboxFacet.m_CubemapPtr, 0);

        rGraphicSkyboxFacet.m_TargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(FirstMipmapCubeTexture));

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

        rGraphicSkyboxFacet.m_ViewPortSetPtr = ViewManager::CreateViewPortSet(MipMapViewPort);

        // -----------------------------------------------------------------------------
        // Render context
        // -----------------------------------------------------------------------------
        CCameraPtr          CameraPtr       = ViewManager::GetMainCamera();
        CRenderStatePtr     NoDepthStatePtr = StateManager::GetRenderState(CRenderState::NoDepth | CRenderState::NoCull);

        CRenderContextPtr CubemapRenderContextPtr = ContextManager::CreateRenderContext();

        CubemapRenderContextPtr->SetCamera(CameraPtr);
        CubemapRenderContextPtr->SetViewPortSet(rGraphicSkyboxFacet.m_ViewPortSetPtr);
        CubemapRenderContextPtr->SetTargetSet(rGraphicSkyboxFacet.m_TargetSetPtr);
        CubemapRenderContextPtr->SetRenderState(NoDepthStatePtr);

        rGraphicSkyboxFacet.m_RenderContextPtr = CubemapRenderContextPtr;

        return rGraphicSkyboxFacet;
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::RenderSkyboxFromPanorama(CInternSkyFacet* _pOutput, float _Intensity)
    {
        CRenderContextPtr RenderContextPtr = _pOutput->m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_SkyboxFromPanorama.m_VSPtr;
        CShaderPtr        GSPtr            = m_SkyboxFromPanorama.m_GSPtr;
        CShaderPtr        PSPtr            = m_SkyboxFromPanorama.m_PSPtr;
        CBufferSetPtr     VSBufferSetPtr   = m_SkyboxFromPanorama.m_VSBufferSetPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_SkyboxFromPanorama.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_SkyboxFromPanorama.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr   = m_SkyboxFromPanorama.m_InputLayoutPtr;
        CMeshPtr          MeshPtr          = m_SkyboxFromPanorama.m_MeshPtr;
        CTextureSetPtr    TextureSetPtr    = m_SkyboxFromPanorama.m_TextureSetPtr;
        CSamplerSetPtr    SamplerSetPtr    = m_SkyboxFromPanorama.m_SamplerSetPtr;

        Performance::BeginEvent("Skybox from Panorama");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SCubemapBufferPS* pPSBuffer = static_cast<SCubemapBufferPS*>(BufferManager::MapConstantBuffer(PSBufferSetPtr->GetBuffer(0)));

        pPSBuffer->m_HDRFactor = _Intensity;
        pPSBuffer->m_IsHDR     = _pOutput->m_InputTexture2DPtr->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

        BufferManager::UnmapConstantBuffer(PSBufferSetPtr->GetBuffer(0));

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           
        const unsigned int pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(RenderContextPtr);

        ContextManager::SetSamplerSetPS(SamplerSetPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetVertexBufferSet(MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(InputLayoutPtr);

        ContextManager::SetConstantBufferSetGS(GSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(PSBufferSetPtr);

        ContextManager::SetTextureSetPS(_pOutput->m_InputTextureSetPtr);

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTextureSetPS();

        ContextManager::ResetConstantBufferSetPS();

        ContextManager::ResetConstantBufferSetGS();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetSamplerSetPS();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Update mip maps
        // -----------------------------------------------------------------------------
        TextureManager::UpdateMipmap(_pOutput->m_CubemapPtr);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::RenderSkyboxFromCubemap(CInternSkyFacet* _pOutput, float _Intensity)
    {
        CRenderContextPtr RenderContextPtr = _pOutput->m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_SkyboxFromCubemap.m_VSPtr;
        CShaderPtr        GSPtr            = m_SkyboxFromCubemap.m_GSPtr;
        CShaderPtr        PSPtr            = m_SkyboxFromCubemap.m_PSPtr;
        CBufferSetPtr     VSBufferSetPtr   = m_SkyboxFromCubemap.m_VSBufferSetPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_SkyboxFromCubemap.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_SkyboxFromCubemap.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr   = m_SkyboxFromCubemap.m_InputLayoutPtr;
        CMeshPtr          MeshPtr          = m_SkyboxFromCubemap.m_MeshPtr;
        CTextureSetPtr    TextureSetPtr    = m_SkyboxFromCubemap.m_TextureSetPtr;
        CSamplerSetPtr    SamplerSetPtr    = m_SkyboxFromCubemap.m_SamplerSetPtr;

        Performance::BeginEvent("Skybox from Cubemap");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SCubemapBufferPS* pPSBuffer = static_cast<SCubemapBufferPS*>(BufferManager::MapConstantBuffer(PSBufferSetPtr->GetBuffer(0)));

        pPSBuffer->m_HDRFactor = _Intensity;
        pPSBuffer->m_IsHDR     = _pOutput->m_InputTexture2DPtr->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

        BufferManager::UnmapConstantBuffer(PSBufferSetPtr->GetBuffer(0));

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           
        const unsigned int pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(RenderContextPtr);

        ContextManager::SetSamplerSetPS(SamplerSetPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetVertexBufferSet(MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(InputLayoutPtr);

        ContextManager::SetConstantBufferSetGS(GSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(PSBufferSetPtr);

        ContextManager::SetTextureSetPS(_pOutput->m_InputTextureSetPtr);

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTextureSetPS();

        ContextManager::ResetConstantBufferSetPS();

        ContextManager::ResetConstantBufferSetGS();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetTopology();

        ContextManager::ResetSamplerSetPS();

        ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Update mip maps
        // -----------------------------------------------------------------------------
        TextureManager::UpdateMipmap(_pOutput->m_CubemapPtr);

        Performance::EndEvent();
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

    // -----------------------------------------------------------------------------

    CSkyFacetPtr CreateSky(unsigned int _FaceSize, const Base::Char* _pFileName)
    {
        return CGfxSkyManager::GetInstance().CreateSky(_FaceSize, _pFileName);
    }
} // namespace SkyManager
} // namespace Gfx