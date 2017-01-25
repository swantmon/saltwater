
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"
#include "data/data_sky_facet.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_selection_renderer.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_sky_facet.h"
#include "graphic/gfx_sky_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include "opencv2/opencv.hpp"

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
            CBufferSetPtr     m_VertexBufferSetPtr;
            CBufferPtr        m_IndexBufferPtr;
            CTextureSetPtr    m_TextureSetPtr;
        };

        struct SModelMatrixBuffer
        {
            Base::Float4x4 m_ModelMatrix;
        };

        struct SCubemapBufferGS
        {
            Base::Float4x4 m_CubeProjectionMatrix;
            Base::Float4x4 m_CubeViewMatrix[6];
        };

        struct SOutputBufferPS
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

        SRenderContext    m_SkyboxFromPanorama;
        SRenderContext    m_SkyboxFromCubemap;
        SRenderContext    m_SkyboxFromTexture;
        SRenderContext    m_SkyboxFromGeometry;
        SRenderContext    m_SkyboxFromLUT;
        CTexture2DPtr     m_LookUpTexturePtr;
        CTextureSetPtr    m_LookupTextureSetPtr;
        CSelectionTicket* m_pSelectionTicket;
        CSkyfacets        m_Skyfacets;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        CInternSkyFacet& AllocateSkyFacet(unsigned int _FaceSize);

        void RenderSkybox(Dt::CSkyFacet* _pDataSkyFacet, CInternSkyFacet* _pOutput);

        void RenderSkyboxFromPanorama(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromCubemap(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromTexture(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromGeometry(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromLUT(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void PrecomputeLUT();
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
        : m_SkyboxFromPanorama ()
        , m_SkyboxFromCubemap  ()
        , m_SkyboxFromTexture  ()
        , m_SkyboxFromGeometry ()
        , m_SkyboxFromLUT      ()
        , m_LookUpTexturePtr   (0)
        , m_LookupTextureSetPtr(0)
        , m_pSelectionTicket   (0)
        , m_Skyfacets          ()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxSkyManager::~CGfxSkyManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::OnStart()
    {
        m_SkyboxFromPanorama.m_VSPtr              = 0;
        m_SkyboxFromPanorama.m_GSPtr              = 0;
        m_SkyboxFromPanorama.m_PSPtr              = 0;
        m_SkyboxFromPanorama.m_VSBufferSetPtr     = 0;
        m_SkyboxFromPanorama.m_GSBufferSetPtr     = 0;
        m_SkyboxFromPanorama.m_PSBufferSetPtr     = 0;
        m_SkyboxFromPanorama.m_InputLayoutPtr     = 0;
        m_SkyboxFromPanorama.m_MeshPtr            = 0;
        m_SkyboxFromPanorama.m_VertexBufferSetPtr = 0;
        m_SkyboxFromPanorama.m_IndexBufferPtr     = 0;
        m_SkyboxFromPanorama.m_TextureSetPtr      = 0;

        m_SkyboxFromCubemap.m_VSPtr              = 0;
        m_SkyboxFromCubemap.m_GSPtr              = 0;
        m_SkyboxFromCubemap.m_PSPtr              = 0;
        m_SkyboxFromCubemap.m_VSBufferSetPtr     = 0;
        m_SkyboxFromCubemap.m_GSBufferSetPtr     = 0;
        m_SkyboxFromCubemap.m_PSBufferSetPtr     = 0;
        m_SkyboxFromCubemap.m_InputLayoutPtr     = 0;
        m_SkyboxFromCubemap.m_MeshPtr            = 0;
        m_SkyboxFromCubemap.m_VertexBufferSetPtr = 0;
        m_SkyboxFromCubemap.m_IndexBufferPtr     = 0;
        m_SkyboxFromCubemap.m_TextureSetPtr      = 0;

        m_SkyboxFromTexture.m_VSPtr              = 0;
        m_SkyboxFromTexture.m_GSPtr              = 0;
        m_SkyboxFromTexture.m_PSPtr              = 0;
        m_SkyboxFromTexture.m_VSBufferSetPtr     = 0;
        m_SkyboxFromTexture.m_GSBufferSetPtr     = 0;
        m_SkyboxFromTexture.m_PSBufferSetPtr     = 0;
        m_SkyboxFromTexture.m_InputLayoutPtr     = 0;
        m_SkyboxFromTexture.m_MeshPtr            = 0;
        m_SkyboxFromTexture.m_VertexBufferSetPtr = 0;
        m_SkyboxFromTexture.m_IndexBufferPtr     = 0;
        m_SkyboxFromTexture.m_TextureSetPtr      = 0;

        m_SkyboxFromGeometry.m_VSPtr              = 0;
        m_SkyboxFromGeometry.m_GSPtr              = 0;
        m_SkyboxFromGeometry.m_PSPtr              = 0;
        m_SkyboxFromGeometry.m_VSBufferSetPtr     = 0;
        m_SkyboxFromGeometry.m_GSBufferSetPtr     = 0;
        m_SkyboxFromGeometry.m_PSBufferSetPtr     = 0;
        m_SkyboxFromGeometry.m_InputLayoutPtr     = 0;
        m_SkyboxFromGeometry.m_MeshPtr            = 0;
        m_SkyboxFromGeometry.m_VertexBufferSetPtr = 0;
        m_SkyboxFromGeometry.m_IndexBufferPtr     = 0;
        m_SkyboxFromGeometry.m_TextureSetPtr      = 0;

        m_SkyboxFromLUT.m_VSPtr              = 0;
        m_SkyboxFromLUT.m_GSPtr              = 0;
        m_SkyboxFromLUT.m_PSPtr              = 0;
        m_SkyboxFromLUT.m_VSBufferSetPtr     = 0;
        m_SkyboxFromLUT.m_GSBufferSetPtr     = 0;
        m_SkyboxFromLUT.m_PSBufferSetPtr     = 0;
        m_SkyboxFromLUT.m_InputLayoutPtr     = 0;
        m_SkyboxFromLUT.m_MeshPtr            = 0;
        m_SkyboxFromLUT.m_VertexBufferSetPtr = 0;
        m_SkyboxFromLUT.m_IndexBufferPtr     = 0;
        m_SkyboxFromLUT.m_TextureSetPtr      = 0;


        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        CShaderPtr CubemapVSPtr         = ShaderManager::CompileVS("vs_spherical_env_cubemap_generation.glsl", "main");
        CShaderPtr CubemapTextureVSPtr  = ShaderManager::CompileVS("vs_texture_env_cubemap_generation.glsl", "main");
        CShaderPtr CubemapGeometryVSPtr = ShaderManager::CompileVS("vs_geometry_env_cubemap_generation.glsl", "main");
        CShaderPtr CubemapGSPtr         = ShaderManager::CompileGS("gs_spherical_env_cubemap_generation.glsl", "main");
        CShaderPtr CubemapRotateGSPtr   = ShaderManager::CompileGS("gs_spherical_rotate_env_cubemap_generation.glsl", "main");
        CShaderPtr CubemapPanoramaPSPtr = ShaderManager::CompilePS("fs_spherical_env_cubemap_generation.glsl", "main");
        CShaderPtr CubemapCubemapPSPtr  = ShaderManager::CompilePS("fs_cubemap_env_cubemap_generation.glsl", "main");
        CShaderPtr CubemapTexturePSPtr  = ShaderManager::CompilePS("fs_texture_env_cubemap_generation.glsl", "main");
        CShaderPtr CubemapLUTPSPtr      = ShaderManager::CompilePS("fs_lut_env_cubemap_generation.glsl", "main");

        const SInputElementDescriptor P3N3T2InputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0, 0 , 32, CInputLayout::PerVertex, 0 },
            { "NORMAL"  , 0, CInputLayout::Float3Format, 0, 12, 32, CInputLayout::PerVertex, 0 },
            { "TEXCOORD", 0, CInputLayout::Float2Format, 0, 24, 32, CInputLayout::PerVertex, 0 },
        };

        CInputLayoutPtr P3N3T2CubemapInputLayoutPtr = ShaderManager::CreateInputLayout(P3N3T2InputLayout, 3, CubemapVSPtr);

        const SInputElementDescriptor P3T2InputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0,  0, 20, CInputLayout::PerVertex, 0 },
            { "TEXCOORD", 0, CInputLayout::Float2Format, 0, 12, 20, CInputLayout::PerVertex, 0 },
        };

        CInputLayoutPtr P3T2CubemapInputLayoutPtr = ShaderManager::CreateInputLayout(P3T2InputLayout, 2, CubemapVSPtr);

        m_SkyboxFromPanorama.m_VSPtr          = CubemapVSPtr;
        m_SkyboxFromPanorama.m_GSPtr          = CubemapGSPtr;
        m_SkyboxFromPanorama.m_PSPtr          = CubemapPanoramaPSPtr;
        m_SkyboxFromPanorama.m_InputLayoutPtr = P3N3T2CubemapInputLayoutPtr;

        m_SkyboxFromCubemap.m_VSPtr          = CubemapVSPtr;
        m_SkyboxFromCubemap.m_GSPtr          = CubemapGSPtr;
        m_SkyboxFromCubemap.m_PSPtr          = CubemapCubemapPSPtr;
        m_SkyboxFromCubemap.m_InputLayoutPtr = P3N3T2CubemapInputLayoutPtr;

        m_SkyboxFromTexture.m_VSPtr          = CubemapTextureVSPtr;
        m_SkyboxFromTexture.m_GSPtr          = CubemapGSPtr;
        m_SkyboxFromTexture.m_PSPtr          = CubemapTexturePSPtr;
        m_SkyboxFromTexture.m_InputLayoutPtr = P3N3T2CubemapInputLayoutPtr;

        m_SkyboxFromGeometry.m_VSPtr          = CubemapGeometryVSPtr;
        m_SkyboxFromGeometry.m_GSPtr          = CubemapGSPtr;
        m_SkyboxFromGeometry.m_PSPtr          = CubemapTexturePSPtr;
        m_SkyboxFromGeometry.m_InputLayoutPtr = P3T2CubemapInputLayoutPtr;

        m_SkyboxFromLUT.m_VSPtr          = CubemapVSPtr;
        m_SkyboxFromLUT.m_GSPtr          = CubemapRotateGSPtr;
        m_SkyboxFromLUT.m_PSPtr          = CubemapLUTPSPtr;
        m_SkyboxFromLUT.m_InputLayoutPtr = P3N3T2CubemapInputLayoutPtr;

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        SBufferDescriptor ConstanteBufferDesc;

        Base::Float3 EyePosition = Base::Float3::s_Zero;
        Base::Float3 UpDirection;
        Base::Float3 LookDirection;
        
        SCubemapBufferGS DefaultGSValues;
        
        DefaultGSValues.m_CubeProjectionMatrix.SetRHFieldOfView(Base::RadiansToDegree(Base::SConstants<float>::s_Pi * 0.5f), 1.0f, 0.3f, 20000.0f);
        
        // -----------------------------------------------------------------------------
        // By creating a cube map in OpenGL, several facts should be considered:
        //  1. OpenGL cubemaps has an left handed coord system
        //  2. Texcoords starts in the upper left corner (normally in the lower left 
        //     corner)
        // -----------------------------------------------------------------------------

        // -----------------------------------------------------------------------------
        // Creating VS matrix for spherical image to cube map:
        // -> y-Axis is mirrored (normally "z" but we will rotate cube later) 
        // -> Orientation of every side is flipped
        // -> At the end we rotate the matrix because the spherical image is y-up
        // -----------------------------------------------------------------------------
        LookDirection = EyePosition + Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[0].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[1].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - Base::Float3::s_AxisY;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisZ;
        
        DefaultGSValues.m_CubeViewMatrix[2].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3::s_AxisY;
        UpDirection   = Base::Float3::s_AxisZ;
        
        DefaultGSValues.m_CubeViewMatrix[3].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3::s_AxisZ;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[4].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - Base::Float3::s_AxisZ;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[5].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        for (unsigned int IndexOfCubeface = 0; IndexOfCubeface < 6; ++ IndexOfCubeface)
        {
            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface] *= Base::Float4x4().SetRotationX(Base::DegreesToRadians(-90.0f));
        }
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCubemapBufferGS);
        ConstanteBufferDesc.m_pBytes        = &DefaultGSValues;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr CubemapGSSphericalBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        // -----------------------------------------------------------------------------
        // Creating VS matrix for cube map to cube map:
        // -> Orientation of every side is flipped
        // -> Mirroring isn't necessary because it is done inside the cubemap
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[0].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[1].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - Base::Float3::s_AxisY;
        UpDirection   = Base::Float3::s_AxisZ;
        
        DefaultGSValues.m_CubeViewMatrix[2].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3::s_AxisY;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisZ;
        
        DefaultGSValues.m_CubeViewMatrix[3].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - Base::Float3::s_AxisZ;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[4].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3::s_AxisZ;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[5].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCubemapBufferGS);
        ConstanteBufferDesc.m_pBytes        = &DefaultGSValues;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr CubemapGSCubemapBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        // Creating VS matrix for world space to cube map:
        // -> Orientation of every side is flipped
        // -> Mirroring isn't necessary because it is done inside the cubemap
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[0].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[1].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3::s_AxisY;
        UpDirection   = Base::Float3::s_Zero - Base::Float3::s_AxisZ;
        
        DefaultGSValues.m_CubeViewMatrix[2].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - Base::Float3::s_AxisY;
        UpDirection   = Base::Float3::s_AxisZ;
        
        DefaultGSValues.m_CubeViewMatrix[3].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3::s_AxisZ;
        UpDirection   = Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[4].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - Base::Float3::s_AxisZ;
        UpDirection   = Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[5].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCubemapBufferGS);
        ConstanteBufferDesc.m_pBytes        = &DefaultGSValues;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr CubemapGSWorldBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
               
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

        m_SkyboxFromPanorama.m_VSBufferSetPtr = 0;
        m_SkyboxFromPanorama.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSSphericalBuffer);
        m_SkyboxFromPanorama.m_PSBufferSetPtr = BufferManager::CreateBufferSet(OuputPSBufferPtr);

        m_SkyboxFromCubemap.m_VSBufferSetPtr = 0;
        m_SkyboxFromCubemap.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSCubemapBuffer);
        m_SkyboxFromCubemap.m_PSBufferSetPtr = BufferManager::CreateBufferSet(OuputPSBufferPtr);

        m_SkyboxFromTexture.m_VSBufferSetPtr = BufferManager::CreateBufferSet(ModelMatrixBufferPtr);
        m_SkyboxFromTexture.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSWorldBuffer);
        m_SkyboxFromTexture.m_PSBufferSetPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), OuputPSBufferPtr);

        m_SkyboxFromGeometry.m_VSBufferSetPtr = BufferManager::CreateBufferSet(ModelMatrixBufferPtr);
        m_SkyboxFromGeometry.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSWorldBuffer);
        m_SkyboxFromGeometry.m_PSBufferSetPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), OuputPSBufferPtr);

        m_SkyboxFromLUT.m_VSBufferSetPtr = 0;
        m_SkyboxFromLUT.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSSphericalBuffer, ModelMatrixBufferPtr);
        m_SkyboxFromLUT.m_PSBufferSetPtr = BufferManager::CreateBufferSet(OuputPSBufferPtr);

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

        // -----------------------------------------------------------------------------

        ModelFileDesc.m_pFileName = "curvedplane.obj";
        ModelFileDesc.m_GenFlag = Dt::SGeneratorFlag::Nothing;

        Dt::CModel& rCurvedPlaneModel = Dt::ModelManager::CreateModel(ModelFileDesc);

        ModelDescr.m_pModel = &rCurvedPlaneModel.GetMesh(0);

        CMeshPtr CurvedPlanePtr = MeshManager::CreateMesh(ModelDescr);

        // -----------------------------------------------------------------------------

        static float PlaneVertexBufferData[] =
        {
          //x   , y   , z   , tx  , ty
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        };
        
        static unsigned int PlaneIndexBufferData[] =
        {
            0, 1, 2, 0, 2, 3,
        };
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::VertexBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(PlaneVertexBufferData);
        ConstanteBufferDesc.m_pBytes        = &PlaneVertexBufferData[0];
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PlanePositionBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::IndexBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(PlaneIndexBufferData);
        ConstanteBufferDesc.m_pBytes        = &PlaneIndexBufferData[0];
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PlaneIndexBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        m_SkyboxFromPanorama.m_MeshPtr            = CubemapTextureSpherePtr;
        m_SkyboxFromPanorama.m_VertexBufferSetPtr = 0;
        m_SkyboxFromPanorama.m_IndexBufferPtr     = 0;

        m_SkyboxFromCubemap.m_MeshPtr            = CubemapTextureSpherePtr;
        m_SkyboxFromCubemap.m_VertexBufferSetPtr = 0;
        m_SkyboxFromCubemap.m_IndexBufferPtr     = 0;

        m_SkyboxFromTexture.m_MeshPtr            = CurvedPlanePtr;
        m_SkyboxFromTexture.m_VertexBufferSetPtr = 0;
        m_SkyboxFromTexture.m_IndexBufferPtr     = 0;

        m_SkyboxFromGeometry.m_MeshPtr            = 0;
        m_SkyboxFromGeometry.m_VertexBufferSetPtr = BufferManager::CreateVertexBufferSet(PlanePositionBufferPtr);;
        m_SkyboxFromGeometry.m_IndexBufferPtr     = PlaneIndexBufferPtr;

        m_SkyboxFromLUT.m_MeshPtr            = CubemapTextureSpherePtr;
        m_SkyboxFromLUT.m_VertexBufferSetPtr = 0;
        m_SkyboxFromLUT.m_IndexBufferPtr     = 0;

        // -----------------------------------------------------------------------------
        // Register dirty entity handler for automatic sky creation
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CGfxSkyManager::OnDirtyEntity));

        // -----------------------------------------------------------------------------
        // Acquire an selection ticket at selection renderer
        // -----------------------------------------------------------------------------
        m_pSelectionTicket = &SelectionRenderer::AcquireTicket(0, 0, 1, 1, SPickFlag::AR);

        // -----------------------------------------------------------------------------
        // Generate LUT
        // -----------------------------------------------------------------------------
        PrecomputeLUT();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::OnExit()
    {
        SelectionRenderer::Clear(*m_pSelectionTicket);

        m_SkyboxFromPanorama.m_VSPtr              = 0;
        m_SkyboxFromPanorama.m_GSPtr              = 0;
        m_SkyboxFromPanorama.m_PSPtr              = 0;
        m_SkyboxFromPanorama.m_VSBufferSetPtr     = 0;
        m_SkyboxFromPanorama.m_GSBufferSetPtr     = 0;
        m_SkyboxFromPanorama.m_PSBufferSetPtr     = 0;
        m_SkyboxFromPanorama.m_InputLayoutPtr     = 0;
        m_SkyboxFromPanorama.m_MeshPtr            = 0;
        m_SkyboxFromPanorama.m_VertexBufferSetPtr = 0;
        m_SkyboxFromPanorama.m_IndexBufferPtr     = 0;
        m_SkyboxFromPanorama.m_TextureSetPtr      = 0;

        m_SkyboxFromCubemap.m_VSPtr              = 0;
        m_SkyboxFromCubemap.m_GSPtr              = 0;
        m_SkyboxFromCubemap.m_PSPtr              = 0;
        m_SkyboxFromCubemap.m_VSBufferSetPtr     = 0;
        m_SkyboxFromCubemap.m_GSBufferSetPtr     = 0;
        m_SkyboxFromCubemap.m_PSBufferSetPtr     = 0;
        m_SkyboxFromCubemap.m_InputLayoutPtr     = 0;
        m_SkyboxFromCubemap.m_MeshPtr            = 0;
        m_SkyboxFromCubemap.m_VertexBufferSetPtr = 0;
        m_SkyboxFromCubemap.m_IndexBufferPtr     = 0;
        m_SkyboxFromCubemap.m_TextureSetPtr      = 0;

        m_SkyboxFromTexture.m_VSPtr              = 0;
        m_SkyboxFromTexture.m_GSPtr              = 0;
        m_SkyboxFromTexture.m_PSPtr              = 0;
        m_SkyboxFromTexture.m_VSBufferSetPtr     = 0;
        m_SkyboxFromTexture.m_GSBufferSetPtr     = 0;
        m_SkyboxFromTexture.m_PSBufferSetPtr     = 0;
        m_SkyboxFromTexture.m_InputLayoutPtr     = 0;
        m_SkyboxFromTexture.m_MeshPtr            = 0;
        m_SkyboxFromTexture.m_VertexBufferSetPtr = 0;
        m_SkyboxFromTexture.m_IndexBufferPtr     = 0;
        m_SkyboxFromTexture.m_TextureSetPtr      = 0;

        m_SkyboxFromGeometry.m_VSPtr              = 0;
        m_SkyboxFromGeometry.m_GSPtr              = 0;
        m_SkyboxFromGeometry.m_PSPtr              = 0;
        m_SkyboxFromGeometry.m_VSBufferSetPtr     = 0;
        m_SkyboxFromGeometry.m_GSBufferSetPtr     = 0;
        m_SkyboxFromGeometry.m_PSBufferSetPtr     = 0;
        m_SkyboxFromGeometry.m_InputLayoutPtr     = 0;
        m_SkyboxFromGeometry.m_MeshPtr            = 0;
        m_SkyboxFromGeometry.m_VertexBufferSetPtr = 0;
        m_SkyboxFromGeometry.m_IndexBufferPtr     = 0;
        m_SkyboxFromGeometry.m_TextureSetPtr      = 0;

        m_SkyboxFromLUT.m_VSPtr              = 0;
        m_SkyboxFromLUT.m_GSPtr              = 0;
        m_SkyboxFromLUT.m_PSPtr              = 0;
        m_SkyboxFromLUT.m_VSBufferSetPtr     = 0;
        m_SkyboxFromLUT.m_GSBufferSetPtr     = 0;
        m_SkyboxFromLUT.m_PSBufferSetPtr     = 0;
        m_SkyboxFromLUT.m_InputLayoutPtr     = 0;
        m_SkyboxFromLUT.m_MeshPtr            = 0;
        m_SkyboxFromLUT.m_VertexBufferSetPtr = 0;
        m_SkyboxFromLUT.m_IndexBufferPtr     = 0;
        m_SkyboxFromLUT.m_TextureSetPtr      = 0;

        m_LookUpTexturePtr    = 0;
        m_LookupTextureSetPtr = 0;

        m_Skyfacets.Clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::Update()
    {
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() == Dt::SLightType::Sky)
            {
                Dt::CSkyFacet*   pDataSkyboxFacet = static_cast<Dt::CSkyFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

                if (pDataSkyboxFacet->GetRefreshMode() == Dt::CSkyFacet::Dynamic)
                {
                    CInternSkyFacet* pGraphicSkyboxFacet = static_cast<CInternSkyFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                    RenderSkybox(pDataSkyboxFacet, pGraphicSkyboxFacet);
                }
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }
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

        unsigned int DirtyFlags;
        unsigned int Hash;

        CInternSkyFacet* pGraphicSkyboxFacet;
        Dt::CSkyFacet*   pDataSkyboxFacet;

        // -----------------------------------------------------------------------------
        // Entity check
        // -----------------------------------------------------------------------------
        if (_pEntity->GetCategory() != Dt::SEntityCategory::Light) return;
        if (_pEntity->GetType()     != Dt::SLightType::Sky) return;

        // -----------------------------------------------------------------------------
        // Get data
        // -----------------------------------------------------------------------------
        pDataSkyboxFacet = static_cast<Dt::CSkyFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Data));

        if (pDataSkyboxFacet == nullptr) return;

        // -----------------------------------------------------------------------------
        // Lamda function
        // -----------------------------------------------------------------------------
        auto UpdateFacet = [&](Dt::CSkyFacet* _pDataSkyboxFacet, CInternSkyFacet* _pGraphicSkyboxFacet)->void
        {
            // -----------------------------------------------------------------------------
            // Get hash
            // -----------------------------------------------------------------------------
            Hash = 0;

            if (_pDataSkyboxFacet->GetType() == Dt::CSkyFacet::Panorama)
            {
                if (_pDataSkyboxFacet->GetHasPanorama()) Hash = _pDataSkyboxFacet->GetPanorama()->GetHash();
            }
            else if (_pDataSkyboxFacet->GetType() == Dt::CSkyFacet::Cubemap)
            {
                if (_pDataSkyboxFacet->GetHasCubemap()) Hash = _pDataSkyboxFacet->GetCubemap()->GetHash();
            }
            else if (_pDataSkyboxFacet->GetType() == Dt::CSkyFacet::Texture || _pDataSkyboxFacet->GetType() == Dt::CSkyFacet::TextureGeometry || _pDataSkyboxFacet->GetType() == Dt::CSkyFacet::TextureLUT)
            {
                if (_pDataSkyboxFacet->GetHasTexture()) Hash = _pDataSkyboxFacet->GetTexture()->GetHash();
            }

            // -----------------------------------------------------------------------------
            // Check hash and update data + render
            // -----------------------------------------------------------------------------
            if (Hash != 0)
            {
                CTexture2DPtr TexturePtr = TextureManager::GetTexture2DByHash(Hash);

                if (TexturePtr.IsValid())
                {
                    _pGraphicSkyboxFacet->m_InputTexture2DPtr = TexturePtr;

                    _pGraphicSkyboxFacet->m_InputTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(TexturePtr));

                    RenderSkybox(_pDataSkyboxFacet, _pGraphicSkyboxFacet);
                }
            }

            // -----------------------------------------------------------------------------
            // Set time
            // -----------------------------------------------------------------------------
            _pGraphicSkyboxFacet->m_TimeStamp = Core::Time::GetNumberOfFrame();
        };

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        DirtyFlags = _pEntity->GetDirtyFlags();

        if ((DirtyFlags & Dt::CEntity::DirtyCreate))
        {
            // -----------------------------------------------------------------------------
            // Create facet
            // -----------------------------------------------------------------------------
            pGraphicSkyboxFacet = &AllocateSkyFacet(2048);

            // -----------------------------------------------------------------------------
            // Update
            // -----------------------------------------------------------------------------
            UpdateFacet(pDataSkyboxFacet, pGraphicSkyboxFacet);

            // -----------------------------------------------------------------------------
            // Save facet
            // -----------------------------------------------------------------------------
            _pEntity->SetDetailFacet(Dt::SFacetCategory::Graphic, pGraphicSkyboxFacet);
        }
        else if ((DirtyFlags & Dt::CEntity::DirtyDetail))
        {
            pGraphicSkyboxFacet = static_cast<CInternSkyFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Graphic));

            // -----------------------------------------------------------------------------
            // Update
            // -----------------------------------------------------------------------------
            UpdateFacet(pDataSkyboxFacet, pGraphicSkyboxFacet);

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
        CRenderStatePtr     NoDepthStatePtr = StateManager::GetRenderState(CRenderState::NoDepth | CRenderState::NoCull | CRenderState::AlphaBlend);

        CRenderContextPtr CubemapRenderContextPtr = ContextManager::CreateRenderContext();

        CubemapRenderContextPtr->SetCamera(CameraPtr);
        CubemapRenderContextPtr->SetViewPortSet(rGraphicSkyboxFacet.m_ViewPortSetPtr);
        CubemapRenderContextPtr->SetTargetSet(rGraphicSkyboxFacet.m_TargetSetPtr);
        CubemapRenderContextPtr->SetRenderState(NoDepthStatePtr);

        rGraphicSkyboxFacet.m_RenderContextPtr = CubemapRenderContextPtr;

        return rGraphicSkyboxFacet;
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::RenderSkybox(Dt::CSkyFacet* _pDataSkyFacet, CInternSkyFacet* _pOutput)
    {
        switch (_pDataSkyFacet->GetType())
        {
            case Dt::CSkyFacet::Procedural: break;
            case Dt::CSkyFacet::Panorama:        RenderSkyboxFromPanorama(_pOutput, _pDataSkyFacet->GetIntensity()); break;
            case Dt::CSkyFacet::Cubemap:         RenderSkyboxFromCubemap(_pOutput, _pDataSkyFacet->GetIntensity()); break;
            case Dt::CSkyFacet::Texture:         RenderSkyboxFromTexture(_pOutput, _pDataSkyFacet->GetIntensity()); break;
            case Dt::CSkyFacet::TextureGeometry: RenderSkyboxFromGeometry(_pOutput, _pDataSkyFacet->GetIntensity()); break;
            case Dt::CSkyFacet::TextureLUT:      RenderSkyboxFromLUT(_pOutput, _pDataSkyFacet->GetIntensity()); break;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::RenderSkyboxFromPanorama(CInternSkyFacet* _pOutput, float _Intensity)
    {
        CRenderContextPtr RenderContextPtr = _pOutput->m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_SkyboxFromPanorama.m_VSPtr;
        CShaderPtr        GSPtr            = m_SkyboxFromPanorama.m_GSPtr;
        CShaderPtr        PSPtr            = m_SkyboxFromPanorama.m_PSPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_SkyboxFromPanorama.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_SkyboxFromPanorama.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr   = m_SkyboxFromPanorama.m_InputLayoutPtr;
        CMeshPtr          MeshPtr          = m_SkyboxFromPanorama.m_MeshPtr;
        CTextureSetPtr    TextureSetPtr    = m_SkyboxFromPanorama.m_TextureSetPtr;

        Performance::BeginEvent("Skybox from Panorama");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SOutputBufferPS PSBuffer;

        PSBuffer.m_HDRFactor = _Intensity;
        PSBuffer.m_IsHDR     = _pOutput->m_InputTexture2DPtr->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

        BufferManager::UploadConstantBufferData(PSBufferSetPtr->GetBuffer(0), &PSBuffer);

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           
        const unsigned int pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(RenderContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetVertexBufferSet(MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(InputLayoutPtr);

        ContextManager::SetConstantBufferSetGS(GSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(PSBufferSetPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(_pOutput->m_InputTexture2DPtr));

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

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
        CBufferSetPtr     GSBufferSetPtr   = m_SkyboxFromCubemap.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_SkyboxFromCubemap.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr   = m_SkyboxFromCubemap.m_InputLayoutPtr;
        CMeshPtr          MeshPtr          = m_SkyboxFromCubemap.m_MeshPtr;
        CTextureSetPtr    TextureSetPtr    = m_SkyboxFromCubemap.m_TextureSetPtr;

        Performance::BeginEvent("Skybox from Cubemap");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SOutputBufferPS PSBuffer;

        PSBuffer.m_HDRFactor = _Intensity;
        PSBuffer.m_IsHDR     = _pOutput->m_InputTexture2DPtr->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

        BufferManager::UploadConstantBufferData(PSBufferSetPtr->GetBuffer(0), &PSBuffer);

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           
        const unsigned int pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(RenderContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetVertexBufferSet(MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(InputLayoutPtr);

        ContextManager::SetConstantBufferSetGS(GSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(PSBufferSetPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(_pOutput->m_InputTexture2DPtr));

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

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

        // -----------------------------------------------------------------------------
        // Update mip maps
        // -----------------------------------------------------------------------------
        TextureManager::UpdateMipmap(_pOutput->m_CubemapPtr);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::RenderSkyboxFromTexture(CInternSkyFacet* _pOutput, float _Intensity)
    {
        CRenderContextPtr RenderContextPtr = _pOutput->m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_SkyboxFromTexture.m_VSPtr;
        CShaderPtr        GSPtr            = m_SkyboxFromTexture.m_GSPtr;
        CShaderPtr        PSPtr            = m_SkyboxFromTexture.m_PSPtr;
        CBufferSetPtr     VSBufferSetPtr   = m_SkyboxFromTexture.m_VSBufferSetPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_SkyboxFromTexture.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_SkyboxFromTexture.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr   = m_SkyboxFromTexture.m_InputLayoutPtr;
        CMeshPtr          MeshPtr          = m_SkyboxFromTexture.m_MeshPtr;
        CTextureSetPtr    TextureSetPtr    = m_SkyboxFromTexture.m_TextureSetPtr;

        Performance::BeginEvent("Skybox from Texture");

        // -----------------------------------------------------------------------------
        // Calculate some camera values
        // -----------------------------------------------------------------------------
        CCameraPtr MainCameraPtr = ViewManager::GetMainCamera();
        CViewPtr   MainViewPtr   = MainCameraPtr->GetView();

        float ScaleY = MainCameraPtr->GetProjectionMatrix()[1][1] * 0.5f;
        float ScaleX = MainCameraPtr->GetProjectionMatrix()[0][0] * 0.5f;

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SModelMatrixBuffer ViewBuffer;

        ViewBuffer.m_ModelMatrix  = Base::Float4x4::s_Identity;
        ViewBuffer.m_ModelMatrix *= Base::Float4x4().SetScale(-1.0f, 1.0f, 1.0f);
        ViewBuffer.m_ModelMatrix *= MainViewPtr->GetRotationMatrix().GetTransposed();
        ViewBuffer.m_ModelMatrix *= Base::Float4x4().SetTranslation(0.0f, 0.0f, -0.1f);
        ViewBuffer.m_ModelMatrix *= Base::Float4x4().SetScale(ScaleY, ScaleX, 1.0f);

        BufferManager::UploadConstantBufferData(VSBufferSetPtr->GetBuffer(0), &ViewBuffer);

        // -----------------------------------------------------------------------------

        SOutputBufferPS PSBuffer;

        PSBuffer.m_HDRFactor = _Intensity;
        PSBuffer.m_IsHDR     = _pOutput->m_InputTexture2DPtr->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

        BufferManager::UploadConstantBufferData(PSBufferSetPtr->GetBuffer(1), &PSBuffer);

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           
        const unsigned int pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(RenderContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetVertexBufferSet(MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(InputLayoutPtr);

        ContextManager::SetConstantBufferSetVS(VSBufferSetPtr);

        ContextManager::SetConstantBufferSetGS(GSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(PSBufferSetPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(_pOutput->m_InputTexture2DPtr));

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBufferSetPS();

        ContextManager::ResetConstantBufferSetGS();

        ContextManager::ResetConstantBufferSetVS();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

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

    void CGfxSkyManager::RenderSkyboxFromGeometry(CInternSkyFacet* _pOutput, float _Intensity)
    {
        CRenderContextPtr RenderContextPtr   = _pOutput->m_RenderContextPtr;
        CShaderPtr        VSPtr              = m_SkyboxFromGeometry.m_VSPtr;
        CShaderPtr        GSPtr              = m_SkyboxFromGeometry.m_GSPtr;
        CShaderPtr        PSPtr              = m_SkyboxFromGeometry.m_PSPtr;
        CBufferSetPtr     VSBufferSetPtr     = m_SkyboxFromGeometry.m_VSBufferSetPtr;
        CBufferSetPtr     GSBufferSetPtr     = m_SkyboxFromGeometry.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr     = m_SkyboxFromGeometry.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr     = m_SkyboxFromGeometry.m_InputLayoutPtr;
        CBufferSetPtr     VertexBufferSetPtr = m_SkyboxFromGeometry.m_VertexBufferSetPtr;
        CBufferPtr        IndexBufferPtr     = m_SkyboxFromGeometry.m_IndexBufferPtr;
        CTextureSetPtr    TextureSetPtr      = m_SkyboxFromGeometry.m_TextureSetPtr;

        Performance::BeginEvent("Skybox from Geometry");

        CCameraPtr MainCameraPtr = ViewManager::GetMainCamera();
        CViewPtr   MainViewPtr = MainCameraPtr->GetView();

        const Base::Float3* pWorldSpaceCameraFrustum = MainCameraPtr->GetWorldSpaceFrustum();

        Base::Float3 FarBottomLeft  = pWorldSpaceCameraFrustum[4];
        Base::Float3 FarTopLeft     = pWorldSpaceCameraFrustum[5];
        Base::Float3 FarBottomRight = pWorldSpaceCameraFrustum[6];
        Base::Float3 FarTopRight    = pWorldSpaceCameraFrustum[7];

        // -----------------------------------------------------------------------------
        // Calculate far plane and setup plane
        // -----------------------------------------------------------------------------
        float PlaneGeometryBuffer[20];

        PlaneGeometryBuffer[0] = FarTopLeft[0];
        PlaneGeometryBuffer[1] = FarTopLeft[1];
        PlaneGeometryBuffer[2] = FarTopLeft[2];
        PlaneGeometryBuffer[3] = 0.0f;
        PlaneGeometryBuffer[4] = 1.0f;

        PlaneGeometryBuffer[5] = FarTopRight[0];
        PlaneGeometryBuffer[6] = FarTopRight[1];
        PlaneGeometryBuffer[7] = FarTopRight[2];
        PlaneGeometryBuffer[8] = 1.0f;
        PlaneGeometryBuffer[9] = 1.0f;

        PlaneGeometryBuffer[10] = FarBottomRight[0];
        PlaneGeometryBuffer[11] = FarBottomRight[1];
        PlaneGeometryBuffer[12] = FarBottomRight[2];
        PlaneGeometryBuffer[13] = 1.0f;
        PlaneGeometryBuffer[14] = 0.0f;

        PlaneGeometryBuffer[15] = FarBottomLeft[0];
        PlaneGeometryBuffer[16] = FarBottomLeft[1];
        PlaneGeometryBuffer[17] = FarBottomLeft[2];
        PlaneGeometryBuffer[18] = 0.0f;
        PlaneGeometryBuffer[19] = 0.0f;

        BufferManager::UploadVertexBufferData(VertexBufferSetPtr->GetBuffer(0), &PlaneGeometryBuffer);

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SModelMatrixBuffer ViewBuffer;

        ViewBuffer.m_ModelMatrix  = Base::Float4x4::s_Identity;
        ViewBuffer.m_ModelMatrix *= Base::Float4x4().SetScale(-1.0f, 1.0f, 1.0f);

        BufferManager::UploadConstantBufferData(VSBufferSetPtr->GetBuffer(0), &ViewBuffer);

        // -----------------------------------------------------------------------------

        SOutputBufferPS PSBuffer;

        PSBuffer.m_HDRFactor = _Intensity;
        PSBuffer.m_IsHDR = _pOutput->m_InputTexture2DPtr->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

        BufferManager::UploadConstantBufferData(PSBufferSetPtr->GetBuffer(1), &PSBuffer);

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           
        const unsigned int pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(RenderContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetVertexBufferSet(VertexBufferSetPtr, pOffset);

        ContextManager::SetIndexBuffer(IndexBufferPtr, 0);

        ContextManager::SetInputLayout(InputLayoutPtr);

        ContextManager::SetConstantBufferSetVS(VSBufferSetPtr);

        ContextManager::SetConstantBufferSetGS(GSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(PSBufferSetPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(_pOutput->m_InputTexture2DPtr));

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(6, 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBufferSetPS();

        ContextManager::ResetConstantBufferSetGS();

        ContextManager::ResetConstantBufferSetVS();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

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

    void CGfxSkyManager::RenderSkyboxFromLUT(CInternSkyFacet* _pOutput, float _Intensity)
    {
        CRenderContextPtr RenderContextPtr = _pOutput->m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_SkyboxFromLUT.m_VSPtr;
        CShaderPtr        GSPtr            = m_SkyboxFromLUT.m_GSPtr;
        CShaderPtr        PSPtr            = m_SkyboxFromLUT.m_PSPtr;
        CBufferSetPtr     VSBufferSetPtr   = m_SkyboxFromLUT.m_VSBufferSetPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_SkyboxFromLUT.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_SkyboxFromLUT.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr   = m_SkyboxFromLUT.m_InputLayoutPtr;
        CMeshPtr          MeshPtr          = m_SkyboxFromLUT.m_MeshPtr;
        CTextureSetPtr    TextureSetPtr    = m_SkyboxFromLUT.m_TextureSetPtr;

        Performance::BeginEvent("Skybox from LUT");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // TODO: Currently it is not totally clear how to rotate the sphere
        // to get a proper environment without env. rotation. Now the rotation works 
        // only if the camera is inside the playing area.
        // Otherwise we have an gimbal lock.
        // -----------------------------------------------------------------------------
        SModelMatrixBuffer ViewBuffer;

        Base::Float3 Rotation;
        ViewManager::GetMainCamera()->GetView()->GetRotationMatrix().GetRotation(Rotation);

        ViewBuffer.m_ModelMatrix  = Base::Float4x4::s_Identity;
        ViewBuffer.m_ModelMatrix *= Base::Float4x4().SetRotationY(Rotation[1]);

        BufferManager::UploadConstantBufferData(GSBufferSetPtr->GetBuffer(1), &ViewBuffer);

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SOutputBufferPS PSBuffer;

        PSBuffer.m_HDRFactor = _Intensity;
        PSBuffer.m_IsHDR     = _pOutput->m_InputTexture2DPtr->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

        BufferManager::UploadConstantBufferData(PSBufferSetPtr->GetBuffer(0), &PSBuffer);

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           
        const unsigned int pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(RenderContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderGS(GSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetVertexBufferSet(MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(InputLayoutPtr);

        ContextManager::SetConstantBufferSetVS(VSBufferSetPtr);

        ContextManager::SetConstantBufferSetGS(GSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(PSBufferSetPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(_pOutput->m_InputTexture2DPtr));
        ContextManager::SetTexture(1, static_cast<CTextureBasePtr>(m_LookUpTexturePtr));

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);

        ContextManager::ResetConstantBufferSetPS();

        ContextManager::ResetConstantBufferSetGS();

        ContextManager::ResetConstantBufferSetVS();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

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

    void CGfxSkyManager::PrecomputeLUT()
    {
        using namespace cv;

        #define CROP_PERCENTAGE 0.8f
        #define IMAGE_EDGE_LENGTH 512

        auto CropImage = [&](const Mat& _rOriginal, Mat& _rCroppedImage, Mat& _rLeftPart, Mat& _rRightPart, Mat& _rTopPart, Mat& _rBottomPart)
        {
            int LengthX;
            int LengthY;
            int ShortedLength;
            int PercentualShortedLength;

            LengthX = _rOriginal.size[0];
            LengthY = _rOriginal.size[1];

            ShortedLength = LengthX < LengthY ? LengthX : LengthY;

            PercentualShortedLength = static_cast<int>(static_cast<float>(ShortedLength) * CROP_PERCENTAGE);

            unsigned int X = (LengthX - PercentualShortedLength) / 2;
            unsigned int Y = (LengthY - PercentualShortedLength) / 2;

            Rect CroppedRectangel(Y, X, PercentualShortedLength, PercentualShortedLength);

            _rCroppedImage = _rOriginal(CroppedRectangel);

            resize(_rCroppedImage, _rCroppedImage, Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH));

            // -----------------------------------------------------------------------------

            _rLeftPart   = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
            _rRightPart  = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
            _rTopPart    = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
            _rBottomPart = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());

            // -----------------------------------------------------------------------------

            Point2f MaskPoints[4];
            Point2f DestPoints[4];
            Mat     WarpMat;

            DestPoints[0] = Point2f(static_cast<float>(0)                    , static_cast<float>(0));
            DestPoints[1] = Point2f(static_cast<float>(0)                    , static_cast<float>(IMAGE_EDGE_LENGTH - 1));
            DestPoints[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH - 1), static_cast<float>(IMAGE_EDGE_LENGTH - 1));
            DestPoints[3] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH - 1), static_cast<float>(0));

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(0)    , static_cast<float>(0));
            MaskPoints[1] = Point2f(static_cast<float>(0)    , static_cast<float>(LengthX - 1));
            MaskPoints[2] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X + PercentualShortedLength - 1));
            MaskPoints[3] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X - 1));

            WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

            warpPerspective(_rOriginal, _rLeftPart, WarpMat, _rLeftPart.size());

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X - 1));
            MaskPoints[1] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X + PercentualShortedLength - 1));
            MaskPoints[2] = Point2f(static_cast<float>(LengthY - 1)                    , static_cast<float>(LengthX - 1));
            MaskPoints[3] = Point2f(static_cast<float>(LengthY - 1)                    , static_cast<float>(0));

            WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

            warpPerspective(_rOriginal, _rRightPart, WarpMat, _rRightPart.size());

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(0)                              , static_cast<float>(0));
            MaskPoints[1] = Point2f(static_cast<float>(Y - 1)                          , static_cast<float>(X - 1));
            MaskPoints[2] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X - 1));
            MaskPoints[3] = Point2f(static_cast<float>(LengthY - 1)                    , static_cast<float>(0));

            WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

            warpPerspective(_rOriginal, _rTopPart, WarpMat, _rTopPart.size());

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(Y - 1)                          , static_cast<float>(X + PercentualShortedLength - 1));
            MaskPoints[1] = Point2f(static_cast<float>(0)                              , static_cast<float>(LengthX - 1));
            MaskPoints[2] = Point2f(static_cast<float>(LengthY - 1)                    , static_cast<float>(LengthX - 1));
            MaskPoints[3] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X + PercentualShortedLength - 1));

            WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

            warpPerspective(_rOriginal, _rBottomPart, WarpMat, _rBottomPart.size());
        };

        // -----------------------------------------------------------------------------

        auto CombineFaces = [&](const Mat& _rOne, const Mat& _rTwo, const Point2f* _pDestinationOne, const Point2f* _pDestinationTwo)->cv::Mat
        {
            Mat CombinedOne, CombinedTwo;

            CombinedOne = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOne.type());
            CombinedTwo = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOne.type());

            // -----------------------------------------------------------------------------

            Point2f MaskPoints[3];

            MaskPoints[0] = Point2f(static_cast<float>(0)                , static_cast<float>(0));
            MaskPoints[1] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH));
            MaskPoints[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

            Mat WarpMat;

            // -----------------------------------------------------------------------------

            WarpMat = getAffineTransform(MaskPoints, _pDestinationOne);

            warpAffine(_rOne, CombinedOne, WarpMat, CombinedOne.size());

            // -----------------------------------------------------------------------------

            WarpMat = getAffineTransform(MaskPoints, _pDestinationTwo);

            warpAffine(_rTwo, CombinedTwo, WarpMat, CombinedTwo.size());

            return CombinedOne + CombinedTwo;
        };

        // -----------------------------------------------------------------------------

        auto CombineRightFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
        {
            Point2f DestPointsOne[3];
            Point2f DestPointsTwo[3];

            DestPointsOne[0] = Point2f(static_cast<float>(0)                    , static_cast<float>(0));
            DestPointsOne[1] = Point2f(static_cast<float>(0)                    , static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(IMAGE_EDGE_LENGTH));

            DestPointsTwo[0] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(0));
            DestPointsTwo[1] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH)    , static_cast<float>(IMAGE_EDGE_LENGTH));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            return Combination;
        };

        // -----------------------------------------------------------------------------

        auto CombineLeftFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
        {
            Point2f DestPointsOne[3];
            Point2f DestPointsTwo[3];

            DestPointsOne[0] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(0));
            DestPointsOne[1] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH)    , static_cast<float>(IMAGE_EDGE_LENGTH));

            DestPointsTwo[0] = Point2f(static_cast<float>(0)                    , static_cast<float>(0));
            DestPointsTwo[1] = Point2f(static_cast<float>(0)                    , static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(IMAGE_EDGE_LENGTH));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            return Combination;
        };

        // -----------------------------------------------------------------------------

        auto CombineTopFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
        {
            Point2f DestPointsOne[3];
            Point2f DestPointsTwo[3];

            DestPointsOne[0] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH / 2));
            DestPointsOne[1] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

            DestPointsTwo[0] = Point2f(static_cast<float>(0)                , static_cast<float>(0));
            DestPointsTwo[1] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH / 2));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH / 2));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            return Combination;
        };

        // -----------------------------------------------------------------------------

        auto CombineBottomFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
        {
            Point2f DestPointsOne[3];
            Point2f DestPointsTwo[3];

            DestPointsOne[0] = Point2f(static_cast<float>(0)                , static_cast<float>(0));
            DestPointsOne[1] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH / 2));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH / 2));

            DestPointsTwo[0] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH / 2));
            DestPointsTwo[1] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            return Combination;
        };

        cv::Mat OriginalFrontImage, FrontCroped, FrontLeftPart, FrontRightPart, FrontTopPart, FrontBottomPart;
        cv::Mat OriginalBackImage, BackCroped, BackLeftPart, BackRightPart, BackTopPart, BackBottomPart;

        cv::Mat CombinedRight, CombinedLeft, CombinedTop, CombinedBottom;

        CombinedRight .create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);
        CombinedLeft  .create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);
        CombinedTop   .create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);
        CombinedBottom.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);
        FrontCroped   .create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);
        BackCroped    .create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);

        OriginalFrontImage.create(cv::Size(1280, 720), CV_32FC2);

        for (int CurrentY = 0; CurrentY < OriginalFrontImage.rows; CurrentY++)
        {
            for (int CurrentX = 0; CurrentX < OriginalFrontImage.cols; CurrentX++)
            {
                OriginalFrontImage.at<Vec2f>(Point(CurrentX, CurrentY)) = cv::Vec2f(static_cast<float>(CurrentX) / static_cast<float>(OriginalFrontImage.cols), static_cast<float>(CurrentY) / static_cast<float>(OriginalFrontImage.rows));
            }
        }

        // -----------------------------------------------------------------------------

        flip(OriginalFrontImage, OriginalBackImage, 1);

        // -----------------------------------------------------------------------------
        // Crop front image
        // -----------------------------------------------------------------------------
        CropImage(OriginalFrontImage, FrontCroped, FrontLeftPart, FrontRightPart, FrontTopPart, FrontBottomPart);
        CropImage(OriginalBackImage, BackCroped, BackLeftPart, BackRightPart, BackTopPart, BackBottomPart);

        // -----------------------------------------------------------------------------
        // Flip back images because of negative direction on back face
        // -----------------------------------------------------------------------------
        flip(BackTopPart, BackTopPart, -1);
        flip(BackBottomPart, BackBottomPart, -1);

        // -----------------------------------------------------------------------------
        // Fill Images
        // -----------------------------------------------------------------------------
        CombinedRight  = CombineRightFaces(FrontRightPart, BackLeftPart);
        CombinedLeft   = CombineLeftFaces(FrontLeftPart, BackRightPart);
        CombinedTop    = CombineTopFaces(FrontTopPart, BackTopPart);
        CombinedBottom = CombineBottomFaces(FrontBottomPart, BackBottomPart);


        // -----------------------------------------------------------------------------
        // Create and update texture
        // -----------------------------------------------------------------------------
        STextureDescriptor TextureDescriptor;
        
        TextureDescriptor.m_NumberOfPixelsU  = IMAGE_EDGE_LENGTH;
        TextureDescriptor.m_NumberOfPixelsV  = IMAGE_EDGE_LENGTH;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 6;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPURead;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTextureBase::R32G32_FLOAT;
        
        m_LookUpTexturePtr = TextureManager::CreateCubeTexture(TextureDescriptor);

        m_LookupTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_LookUpTexturePtr));

        Base::UInt2 CubemapResolution = Base::UInt2(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH);

        Base::AABB2UInt CubemapRect(Base::UInt2(0), CubemapResolution);

        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 0, CubemapRect, CubemapRect[1][0], CombinedRight.data, false);
        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 1, CubemapRect, CubemapRect[1][0], CombinedLeft.data, false);
        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 2, CubemapRect, CubemapRect[1][0], CombinedTop.data, false);
        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 3, CubemapRect, CubemapRect[1][0], CombinedBottom.data, false);
        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 4, CubemapRect, CubemapRect[1][0], FrontCroped.data, false);
        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 5, CubemapRect, CubemapRect[1][0], BackCroped.data, false);

        Gfx::TextureManager::UpdateMipmap(m_LookUpTexturePtr);
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