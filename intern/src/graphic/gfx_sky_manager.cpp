
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
#include "data/data_sun_facet.h"

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
    const float g_RadiusGround     = 6360.0f;
    const float g_RadiusAtmosphere = 6420.0f;

    const uint g_TransmittanceWidth  = 256;
    const uint g_TransmittanceHeight = 64;

    const uint g_InscatterAltitude = 128;
    const uint g_InscatterMu       = 128;         // view / zenith
    const uint g_InscatterMuS      = 32;          // sun / zenith
    const uint g_InscatterNu       = 8;           // view / sun
    const uint g_InscatterWidth    = g_InscatterMuS * g_InscatterNu;
    const uint g_InscatterHeight   = g_InscatterMu;
    const uint g_InscatterDepth    = g_InscatterAltitude;

    const uint g_IrradianceWidth  = 64;
    const uint g_IrradianceHeight = 16;
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
            uint m_Layer;
            float Padding[3];
        };

        struct SPSLayerValues
        {
            Base::Float4 m_Dhdh;
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
            Base::Float4 g_SunDirection;
            Base::Float4 g_SunIntensity;
            uint         ps_ExposureHistoryIndex;
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

        SRenderContext    m_SkyboxFromAtmosphere;
        SRenderContext    m_SkyboxFromPanorama;
        SRenderContext    m_SkyboxFromCubemap;
        SRenderContext    m_SkyboxFromTexture;
        SRenderContext    m_SkyboxFromGeometry;
        SRenderContext    m_SkyboxFromLUT;
        CTexture2DPtr     m_LookUpTexturePtr;
        CTextureSetPtr    m_LookupTextureSetPtr;
        CSelectionTicket* m_pSelectionTicket;
        CSkyfacets        m_Skyfacets;



        CBufferPtr m_PSPrecomputeConstants;

        CTexture2DPtr m_TransmittanceTable;
        CTexture2DPtr m_IrradianceTable;
        CTexture3DPtr m_InscatterTable;



    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        CInternSkyFacet& AllocateSkyFacet(unsigned int _FaceSize);

        void RenderSkybox(Dt::CSkyFacet* _pDataSkyFacet, CInternSkyFacet* _pOutput);

        void RenderSkyboxFromAtmopsphericScattering(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromPanorama(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromCubemap(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromTexture(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromGeometry(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void RenderSkyboxFromLUT(CInternSkyFacet* _pOutput, float _Intensity = 1.0f);

        void PrecomputeScattering();

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
        m_SkyboxFromAtmosphere.m_VSPtr              = 0;
        m_SkyboxFromAtmosphere.m_GSPtr              = 0;
        m_SkyboxFromAtmosphere.m_PSPtr              = 0;
        m_SkyboxFromAtmosphere.m_VSBufferSetPtr     = 0;
        m_SkyboxFromAtmosphere.m_GSBufferSetPtr     = 0;
        m_SkyboxFromAtmosphere.m_PSBufferSetPtr     = 0;
        m_SkyboxFromAtmosphere.m_InputLayoutPtr     = 0;
        m_SkyboxFromAtmosphere.m_MeshPtr            = 0;
        m_SkyboxFromAtmosphere.m_VertexBufferSetPtr = 0;
        m_SkyboxFromAtmosphere.m_IndexBufferPtr     = 0;
        m_SkyboxFromAtmosphere.m_TextureSetPtr      = 0;

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

        CShaderPtr m_PostEffectMaterial = ShaderManager::CompilePS("scattering/scattering_post_effect.glsl", "main");

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

        m_SkyboxFromAtmosphere.m_VSPtr          = CubemapVSPtr;
        m_SkyboxFromAtmosphere.m_GSPtr          = CubemapGSPtr;
        m_SkyboxFromAtmosphere.m_PSPtr          = m_PostEffectMaterial;
        m_SkyboxFromAtmosphere.m_InputLayoutPtr = P3N3T2CubemapInputLayoutPtr;

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
        //  1. OpenGL cubemaps has an left handed coord system inside the cube and
        //    right handed coord system outside the cube
        //  2. Texcoords starts in the upper left corner (normally in the lower left 
        //     corner)
        //
        // RHS:
        //          +--------+
        //          |        |
        //          |   Y+   |
        //          |        |
        // +--------+--------+--------+--------+
        // |        |        |        |        |
        // |   X-   |   Z+   |   X+   |   Z-   |
        // |        |        |        |        |
        // +--------+--------+--------+--------+
        //          |        |
        //          |   Y-   |
        //          |        |
        //          +--------+
        //
        // LHS:
        //          +--------+
        //          |        |
        //          |   Y+   |
        //          |        |
        // +--------+--------+--------+--------+
        // |        |        |        |        |
        // |   X-   |   Z-   |   X+   |   Z+   |
        // |        |        |        |        |
        // +--------+--------+--------+--------+
        //          |        |
        //          |   Y-   |
        //          |        |
        //          +--------+
        // -----------------------------------------------------------------------------

        // -----------------------------------------------------------------------------
        // Creating VS matrix for spherical image to cube map:
        // -> Viewer is inside the cube > LHS
        // -----------------------------------------------------------------------------
        LookDirection = EyePosition + Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[0].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - Base::Float3::s_AxisX;
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
        
        LookDirection = EyePosition + Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[0].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - Base::Float3::s_AxisX;
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

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPSPASSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr PSPASSettings = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        m_SkyboxFromAtmosphere.m_VSBufferSetPtr = 0;
        m_SkyboxFromAtmosphere.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSSphericalBuffer);
        m_SkyboxFromAtmosphere.m_PSBufferSetPtr = BufferManager::CreateBufferSet(PSPASSettings);

        m_SkyboxFromPanorama.m_VSBufferSetPtr = 0;
        m_SkyboxFromPanorama.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSSphericalBuffer);
        m_SkyboxFromPanorama.m_PSBufferSetPtr = BufferManager::CreateBufferSet(OuputPSBufferPtr);

        m_SkyboxFromCubemap.m_VSBufferSetPtr = 0;
        m_SkyboxFromCubemap.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSCubemapBuffer);
        m_SkyboxFromCubemap.m_PSBufferSetPtr = BufferManager::CreateBufferSet(OuputPSBufferPtr);

        m_SkyboxFromTexture.m_VSBufferSetPtr = BufferManager::CreateBufferSet(ModelMatrixBufferPtr);
        m_SkyboxFromTexture.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSWorldBuffer);
        m_SkyboxFromTexture.m_PSBufferSetPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBuffer(), OuputPSBufferPtr);

        m_SkyboxFromGeometry.m_VSBufferSetPtr = BufferManager::CreateBufferSet(ModelMatrixBufferPtr);
        m_SkyboxFromGeometry.m_GSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSWorldBuffer);
        m_SkyboxFromGeometry.m_PSBufferSetPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBuffer(), OuputPSBufferPtr);

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

        ModelDescr.m_pMesh = &rSphereModel.GetMesh(0);

        CMeshPtr CubemapTextureSpherePtr = MeshManager::CreateMesh(ModelDescr);

        // -----------------------------------------------------------------------------

        ModelFileDesc.m_pFileName = "curvedplane.obj";
        ModelFileDesc.m_GenFlag = Dt::SGeneratorFlag::Nothing;

        Dt::CModel& rCurvedPlaneModel = Dt::ModelManager::CreateModel(ModelFileDesc);

        ModelDescr.m_pMesh = &rCurvedPlaneModel.GetMesh(0);

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

        m_SkyboxFromAtmosphere.m_MeshPtr            = CubemapTextureSpherePtr;
        m_SkyboxFromAtmosphere.m_VertexBufferSetPtr = 0;
        m_SkyboxFromAtmosphere.m_IndexBufferPtr     = 0;

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

        // -----------------------------------------------------------------------------
        // Precompute Scattering
        // -----------------------------------------------------------------------------
        PrecomputeScattering();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::OnExit()
    {
        SelectionRenderer::Clear(*m_pSelectionTicket);

        m_SkyboxFromAtmosphere.m_VSPtr              = 0;
        m_SkyboxFromAtmosphere.m_GSPtr              = 0;
        m_SkyboxFromAtmosphere.m_PSPtr              = 0;
        m_SkyboxFromAtmosphere.m_VSBufferSetPtr     = 0;
        m_SkyboxFromAtmosphere.m_GSBufferSetPtr     = 0;
        m_SkyboxFromAtmosphere.m_PSBufferSetPtr     = 0;
        m_SkyboxFromAtmosphere.m_InputLayoutPtr     = 0;
        m_SkyboxFromAtmosphere.m_MeshPtr            = 0;
        m_SkyboxFromAtmosphere.m_VertexBufferSetPtr = 0;
        m_SkyboxFromAtmosphere.m_IndexBufferPtr     = 0;
        m_SkyboxFromAtmosphere.m_TextureSetPtr      = 0;

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

        m_PSPrecomputeConstants = 0;

        m_TransmittanceTable = 0;
        m_InscatterTable     = 0;
        m_IrradianceTable    = 0;

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
                }
                else
                {
                    _pGraphicSkyboxFacet->m_InputTexture2DPtr = nullptr;

                    _pGraphicSkyboxFacet->m_InputTextureSetPtr = nullptr;
                }
            }

            // -----------------------------------------------------------------------------
            // Render sky because of the change
            // -----------------------------------------------------------------------------
            RenderSkybox(_pDataSkyboxFacet, _pGraphicSkyboxFacet);

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
            case Dt::CSkyFacet::Procedural:      RenderSkyboxFromAtmopsphericScattering(_pOutput, _pDataSkyFacet->GetIntensity()); break;
            case Dt::CSkyFacet::Panorama:        RenderSkyboxFromPanorama(_pOutput, _pDataSkyFacet->GetIntensity()); break;
            case Dt::CSkyFacet::Cubemap:         RenderSkyboxFromCubemap(_pOutput, _pDataSkyFacet->GetIntensity()); break;
            case Dt::CSkyFacet::Texture:         RenderSkyboxFromTexture(_pOutput, _pDataSkyFacet->GetIntensity()); break;
            case Dt::CSkyFacet::TextureGeometry: RenderSkyboxFromGeometry(_pOutput, _pDataSkyFacet->GetIntensity()); break;
            case Dt::CSkyFacet::TextureLUT:      RenderSkyboxFromLUT(_pOutput, _pDataSkyFacet->GetIntensity()); break;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyManager::RenderSkyboxFromAtmopsphericScattering(CInternSkyFacet* _pOutput, float _Intensity)
    {
        CRenderContextPtr RenderContextPtr = _pOutput->m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_SkyboxFromAtmosphere.m_VSPtr;
        CShaderPtr        GSPtr            = m_SkyboxFromAtmosphere.m_GSPtr;
        CShaderPtr        PSPtr            = m_SkyboxFromAtmosphere.m_PSPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_SkyboxFromAtmosphere.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_SkyboxFromAtmosphere.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr   = m_SkyboxFromAtmosphere.m_InputLayoutPtr;
        CMeshPtr          MeshPtr          = m_SkyboxFromAtmosphere.m_MeshPtr;

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::CSunLightFacet* pDataSunFacet = 0;

        for (Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Light); CurrentEntity != Dt::Map::EntitiesEnd() && pDataSunFacet == 0; CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light))
        {
            if (CurrentEntity->GetType() == Dt::SLightType::Sun)
            {
                pDataSunFacet = static_cast<Dt::CSunLightFacet*>(CurrentEntity->GetDetailFacet(Dt::SFacetCategory::Data));
            }
        }

        Performance::BeginEvent("Skybox from PAS");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SPSPASSettings PSBuffer;

        PSBuffer.g_SunDirection           = pDataSunFacet == nullptr ? Base::Float4(0.0f, 1.0f, 0.0f, 0.0f) : Base::Float4x4().SetRotationX(Base::DegreesToRadians(90.0f)) * Base::Float4(pDataSunFacet->GetDirection(), 0.0f);
        PSBuffer.g_SunIntensity           = Base::Float4(_Intensity);
        PSBuffer.ps_ExposureHistoryIndex  = 0;

        BufferManager::UploadConstantBufferData(PSBufferSetPtr->GetBuffer(0), &PSBuffer);

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           
        const unsigned int pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Setup
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

        ContextManager::SetVertexBufferSet(MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(InputLayoutPtr);

        ContextManager::SetConstantBuffer(2, GSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(3, PSBufferSetPtr->GetBuffer(0));
        
        ContextManager::SetConstantBuffer(1, m_PSPrecomputeConstants);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(m_TransmittanceTable));

        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(1, static_cast<CTextureBasePtr>(m_InscatterTable));

        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(2, static_cast<CTextureBasePtr>(m_IrradianceTable));

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetConstantBuffer(4);

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

    void CGfxSkyManager::RenderSkyboxFromPanorama(CInternSkyFacet* _pOutput, float _Intensity)
    {
        if (_pOutput->m_InputTexture2DPtr == 0)
        {
            BASE_CONSOLE_INFO("Skybox can't be rendered from panorama because of missing image.");
            return;
        }

        // -----------------------------------------------------------------------------

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

        ContextManager::SetConstantBuffer(2, GSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(4, PSBufferSetPtr->GetBuffer(0));

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

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetConstantBuffer(4);

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
        if (_pOutput->m_InputTexture2DPtr == 0)
        {
            BASE_CONSOLE_INFO("Skybox can't be rendered from cube map because of missing image.");
            return;
        }

        // -----------------------------------------------------------------------------

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

        ContextManager::SetConstantBuffer(2, GSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(4, PSBufferSetPtr->GetBuffer(0));

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

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetConstantBuffer(4);

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
        if (_pOutput->m_InputTexture2DPtr == 0)
        {
            BASE_CONSOLE_INFO("Skybox can't be rendered from texture because of missing image.");
            return;
        }

        // -----------------------------------------------------------------------------

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

        ContextManager::SetConstantBuffer(1, VSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(2, GSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(0, PSBufferSetPtr->GetBuffer(0));
        ContextManager::SetConstantBuffer(4, PSBufferSetPtr->GetBuffer(1));

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

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetConstantBuffer(4);

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
        if(_pOutput->m_InputTexture2DPtr == 0)
        {
            BASE_CONSOLE_INFO("Skybox can't be rendered from geometry beacuse of missing image.");
            return;
        }

        // -----------------------------------------------------------------------------

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

        ContextManager::SetConstantBuffer(1, VSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(2, GSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(0, PSBufferSetPtr->GetBuffer(0));
        ContextManager::SetConstantBuffer(4, PSBufferSetPtr->GetBuffer(1));

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

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetConstantBuffer(4);

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
        if (_pOutput->m_InputTexture2DPtr == 0)
        {
            BASE_CONSOLE_INFO("Skybox can't be rendered from LUT because of missing image.");
            return;
        }

        // -----------------------------------------------------------------------------

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

        ContextManager::SetConstantBuffer(2, GSBufferSetPtr->GetBuffer(0));
        ContextManager::SetConstantBuffer(3, GSBufferSetPtr->GetBuffer(1));

        ContextManager::SetConstantBuffer(4, PSBufferSetPtr->GetBuffer(0));

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

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetConstantBuffer(4);

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

    void CGfxSkyManager::PrecomputeScattering()
    {
        // -----------------------------------------------------------------------------
        // Textures
        // -----------------------------------------------------------------------------
        STextureDescriptor TextureDesc;

        TextureDesc.m_NumberOfPixelsU  = 0;
        TextureDesc.m_NumberOfPixelsV  = 0;
        TextureDesc.m_NumberOfPixelsW  = 1;
        TextureDesc.m_NumberOfMipMaps  = 1;
        TextureDesc.m_NumberOfTextures = 1;
        TextureDesc.m_Binding          = CTextureBase::RenderTarget | CTextureBase::ShaderResource;
        TextureDesc.m_Access           = CTextureBase::CPUWrite;
        TextureDesc.m_Format           = CTextureBase::Unknown;
        TextureDesc.m_Usage            = CTextureBase::GPURead;
        TextureDesc.m_Semantic         = CTextureBase::Diffuse;
        TextureDesc.m_pFileName        = 0;
        TextureDesc.m_pPixels          = 0;
        TextureDesc.m_Format           = CTextureBase::R16G16B16A16_FLOAT;
        
        TextureDesc.m_NumberOfPixelsU = g_TransmittanceWidth;
        TextureDesc.m_NumberOfPixelsV = g_TransmittanceHeight;

        m_TransmittanceTable = TextureManager::CreateTexture2D(TextureDesc);

        TextureDesc.m_NumberOfPixelsU = g_IrradianceWidth;
        TextureDesc.m_NumberOfPixelsV = g_IrradianceHeight;

        m_IrradianceTable = TextureManager::CreateTexture2D(TextureDesc);
        CTexture2DPtr m_DeltaE          = TextureManager::CreateTexture2D(TextureDesc);

        TextureDesc.m_NumberOfPixelsU = g_InscatterWidth;
        TextureDesc.m_NumberOfPixelsV = g_InscatterHeight;
        TextureDesc.m_NumberOfPixelsW = g_InscatterDepth;

        m_InscatterTable = TextureManager::CreateTexture3D(TextureDesc);
        CTexture3DPtr m_DeltaSR        = TextureManager::CreateTexture3D(TextureDesc);
        CTexture3DPtr m_DeltaSM        = TextureManager::CreateTexture3D(TextureDesc);
        CTexture3DPtr m_DeltaJ         = TextureManager::CreateTexture3D(TextureDesc);

        // -----------------------------------------------------------------------------
        // Target sets & view ports
        // -----------------------------------------------------------------------------
        CTargetSetPtr m_TransmittanceTableTS = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(m_TransmittanceTable));

        CTargetSetPtr m_DeltaETS = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(m_DeltaE));

        CTargetSetPtr m_DeltaSRSMTS = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(m_DeltaSR), static_cast<CTextureBasePtr>(m_DeltaSM));

        CTargetSetPtr m_IrradianceTableTS = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(m_IrradianceTable));

        CTargetSetPtr m_InscatterTableTS = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(m_InscatterTable));

        CTargetSetPtr m_DeltaJTS = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(m_DeltaJ));

        CTargetSetPtr m_DeltaSRTS = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(m_DeltaSR));

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

        //////////////////////////////////////////////////////////////
        // Transmittance
        //////////////////////////////////////////////////////////////

        CShaderPtr m_TransmittanceMaterial = ShaderManager::CompilePS("scattering/scattering_transmittance.glsl", "main");

        //////////////////////////////////////////////////////////////
        // Irradiance
        //////////////////////////////////////////////////////////////

        CShaderPtr m_IrradianceSingleMaterial = ShaderManager::CompilePS("scattering/scattering_irradiance_single.glsl", "main");

        CShaderPtr m_IrradianceMultipleMaterial = ShaderManager::CompilePS("scattering/scattering_irradiance_multiple.glsl", "main");

        CShaderPtr m_IrradianceCopyMaterial = ShaderManager::CompilePS("scattering/scattering_irradiance_copy.glsl", "main");

        //////////////////////////////////////////////////////////////
        // Inscatter
        //////////////////////////////////////////////////////////////

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

        CBufferSetPtr EmptyVertexBufferSetPtr = BufferManager::CreateVertexBufferSet(EmptyVertexBufferPtr);

        // -----------------------------------------------------------------------------
        // Render
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Precompute Atmospheric Scattering");

        const unsigned int pOffset[] = { 0, 0 };

        auto GetLayerValues = [&](unsigned int _Layer, float& _rRadius, Base::Float4& _rDhdH)
        {
            float Radius = _Layer / Base::Max((g_InscatterAltitude - 1.0f), 1.0f);

            Radius = Radius * Radius;
            Radius = Base::Sqrt(g_RadiusGround * g_RadiusGround + Radius * (g_RadiusAtmosphere * g_RadiusAtmosphere - g_RadiusGround * g_RadiusGround)) + (_Layer == 0 ? 0.01f : (_Layer == g_InscatterAltitude - 1 ? -0.001f : 0.0f));

            float DMin  = g_RadiusAtmosphere - Radius;
            float DMax  = Base::Sqrt(Radius * Radius - g_RadiusGround * g_RadiusGround) + Base::Sqrt(g_RadiusAtmosphere * g_RadiusAtmosphere - g_RadiusGround * g_RadiusGround);

            float DMinP = Radius - g_RadiusGround;
            float DMaxP = Base::Sqrt(Radius * Radius - g_RadiusGround * g_RadiusGround);

            _rRadius = Radius;

            _rDhdH = Base::Float4(DMin, DMax, DMinP, DMaxP);
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
        
        BufferManager::UploadConstantBufferData(m_PSPrecomputeConstants, &PrecomuteBuffer);

        Performance::BeginEvent("Transmittance");

        ContextManager::SetTargetSet(m_TransmittanceTableTS);

        ContextManager::SetViewPortSet(m_TransmittanceVPS);

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::NoDepth));

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetTopology(STopology::TriangleStrip);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderPS(m_TransmittanceMaterial);

        ContextManager::SetVertexBufferSet(EmptyVertexBufferSetPtr, pOffset);

        ContextManager::SetConstantBuffer(1, m_PSPrecomputeConstants);

        ContextManager::Draw(3, 0);

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Irradiance single
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Irradiance Single");

        ContextManager::SetTargetSet(m_DeltaETS);

        ContextManager::SetViewPortSet(m_IrradianceVPS);

        ContextManager::SetShaderPS(m_IrradianceSingleMaterial);

        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(m_TransmittanceTable));

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::Draw(3, 0);

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Inscatter single
        // -----------------------------------------------------------------------------
        Base::Float4 Dhdh;
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

            BufferManager::UploadConstantBufferData(m_GSLayer, &GSLayer);

            GetLayerValues(Layer, Radius, Dhdh);

            SPSLayerValues PSLayerValues;

            PSLayerValues.m_Dhdh   = Dhdh;
            PSLayerValues.m_Radius = Radius;

            BufferManager::UploadConstantBufferData(m_PSLayerValues, &PSLayerValues);

            ContextManager::Draw(3, 0);
        }

        ContextManager::ResetShaderGS();

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Copy irradiance 
        // -----------------------------------------------------------------------------
        SPSIrradianceK PSIrradianceK;

        PSIrradianceK.k = 0;

        BufferManager::UploadConstantBufferData(m_PSIrradianceK, &PSIrradianceK);

        Performance::BeginEvent("Copy Irradiance");

        ContextManager::SetTargetSet(m_IrradianceTableTS);

        ContextManager::SetViewPortSet(m_IrradianceVPS);

        ContextManager::SetShaderPS(m_IrradianceCopyMaterial);

        ContextManager::SetTexture(6, static_cast<CTextureBasePtr>(m_DeltaE));

        ContextManager::SetSampler(6, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetConstantBuffer(5, m_PSIrradianceK);

        ContextManager::Draw(3, 0);

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

        ContextManager::SetTexture(3, static_cast<CTextureBasePtr>(m_DeltaSR));

        ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(4, static_cast<CTextureBasePtr>(m_DeltaSM));

        ContextManager::SetSampler(4, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        for (unsigned int Layer = 0; Layer < g_InscatterDepth; ++Layer)
        {
            SGSLayer GSLayer;

            GSLayer.m_Layer = Layer;

            BufferManager::UploadConstantBufferData(m_GSLayer, &GSLayer);

            ContextManager::Draw(3, 0);
        }

        ContextManager::ResetShaderGS();

        Performance::EndEvent();

        // -----------------------------------------------------------------------------
        // Multiple scattering
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Multiple Scattering");

        ContextManager::SetTexture(5, static_cast<CTextureBasePtr>(m_DeltaJ));

        ContextManager::SetSampler(5, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        for (unsigned int Order = 2; Order <= 4; ++Order)
        {
            Performance::BeginEvent("Order");

            SPSScatteringOrder PSScatteringOrder;

            PSScatteringOrder.m_FirstOrder = (Order == 2 ? 1.0f : 0.0f);

            BufferManager::UploadConstantBufferData(m_PSScatteringOrder, &PSScatteringOrder);

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

                BufferManager::UploadConstantBufferData(m_GSLayer, &GSLayer);

                GetLayerValues(Layer, Radius, Dhdh);

                SPSLayerValues PSLayerValues;

                PSLayerValues.m_Dhdh = Dhdh;
                PSLayerValues.m_Radius = Radius;

                BufferManager::UploadConstantBufferData(m_PSLayerValues, &PSLayerValues);

                ContextManager::Draw(3, 0);
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

                BufferManager::UploadConstantBufferData(m_GSLayer, &GSLayer);

                GetLayerValues(Layer, Radius, Dhdh);

                SPSLayerValues PSLayerValues;

                PSLayerValues.m_Dhdh = Dhdh;
                PSLayerValues.m_Radius = Radius;

                BufferManager::UploadConstantBufferData(m_PSLayerValues, &PSLayerValues);

                ContextManager::Draw(3, 0);
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
            SPSIrradianceK PSIrradianceK;

            PSIrradianceK.k = 1;

            BufferManager::UploadConstantBufferData(m_PSIrradianceK, &PSIrradianceK);

            Performance::BeginEvent("Add DeltaE to Irradiance");

            ContextManager::SetTargetSet(m_IrradianceTableTS);

            ContextManager::SetViewPortSet(m_IrradianceVPS);

            ContextManager::SetShaderPS(m_IrradianceCopyMaterial);

            ContextManager::Draw(3, 0);

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

                BufferManager::UploadConstantBufferData(m_GSLayer, &GSLayer);

                GetLayerValues(Layer, Radius, Dhdh);

                SPSLayerValues PSLayerValues;

                PSLayerValues.m_Dhdh = Dhdh;
                PSLayerValues.m_Radius = Radius;

                BufferManager::UploadConstantBufferData(m_PSLayerValues, &PSLayerValues);

                ContextManager::Draw(3, 0);
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