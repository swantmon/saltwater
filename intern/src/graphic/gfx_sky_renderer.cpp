
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_vector4.h"

#include "camera/cam_control_manager.h"

#include "core/core_time.h"

#include "data/data_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_light_facet.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"

#include "graphic/gfx_actor_facet.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_light_facet.h"
#include "graphic/gfx_light_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_sky_renderer.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include <string>

using namespace Gfx;

namespace
{
    class CGfxSkyRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxSkyRenderer)
        
    public:

        CGfxSkyRenderer();
        ~CGfxSkyRenderer();
        
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

        CTextureSetPtr GetCubemap();
        
    private:

        struct SRenderContext
        {
            CRenderContextPtr m_RenderContextPtr;
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
        
        struct SSkyboxRenderJob
        {
            Dt::CSkyboxFacet*  m_pDataSkybox;
            Gfx::CSkyboxFacet* m_pGraphicSkybox;
        };

        struct SCameraRenderJob
        {
            Dt::CCameraActorFacet* m_pDataCamera;
            Gfx::CCameraActorFacet* m_pGraphicCamera;
        };

        struct SSkytextureBufferPS
        {
            float m_HDRFactor;
            float m_IsHDR;
            float m_ExposureIndex;
        };

        struct SSkyboxVSBuffer
        {
            Base::Float4x4 m_View;
            Base::Float4x4 m_Projection;
        };

        struct SSkyboxFromTextureVSBuffer
        {
            Base::Float4x4 m_ModelMatrix;
        };
        
        struct SSkyboxBufferPS
        {
            Base::Float4   m_InvertedScreenSize;
            unsigned int   m_ExposureHistoryIndex;
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

    private:

        typedef std::vector<SSkyboxRenderJob> CSkyboxRenderJobs;
        typedef std::vector<SCameraRenderJob> CCameraRenderJobs;
        
    private:

        SRenderContext m_SkyboxFromPanorama;
        SRenderContext m_SkyboxFromCubemap;
        SRenderContext m_SkyboxFromTexture;

        SRenderContext m_BackgroundFromSkybox;
        SRenderContext m_BackgroundFromTexture;
    
        CTextureSetPtr    m_CubemapTextureSetPtr;
        CTargetSetPtr     m_CubemapTargetSetPtr;
        CTexture2DPtr     m_CubemapTexture2DPtr;
        CViewPortSetPtr   m_CubemapViewPortSetPtr;

        CSkyboxRenderJobs m_SkyboxRenderJobs;
        CCameraRenderJobs m_CameraRenderJobs;
        
    private:
        
        void RenderSkyboxFromPanorama();

        void RenderSkyboxFromCubemap();

        void RenderSkyboxFromTexture();

        void RenderBackgroundFromSkybox();

        void RenderBackgroundFromTexture();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxSkyRenderer::CGfxSkyRenderer()
        : m_SkyboxFromPanorama         ()
        , m_SkyboxFromCubemap          ()
        , m_SkyboxFromTexture          ()
        , m_BackgroundFromSkybox       ()
        , m_BackgroundFromTexture      ()
        , m_CubemapTextureSetPtr       ()
        , m_CubemapTargetSetPtr        ()
        , m_CubemapTexture2DPtr        ()
        , m_CubemapViewPortSetPtr      ()
        , m_SkyboxRenderJobs           ()
        , m_CameraRenderJobs           ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxSkyRenderer::~CGfxSkyRenderer()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnExit()
    {
        // -----------------------------------------------------------------------------
        // Clear render parts
        // -----------------------------------------------------------------------------
        m_SkyboxFromPanorama.m_RenderContextPtr = 0;
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

        m_SkyboxFromCubemap.m_RenderContextPtr = 0;
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

        m_SkyboxFromTexture.m_RenderContextPtr = 0;
        m_SkyboxFromTexture.m_VSPtr            = 0;
        m_SkyboxFromTexture.m_GSPtr            = 0;
        m_SkyboxFromTexture.m_PSPtr            = 0;
        m_SkyboxFromTexture.m_VSBufferSetPtr   = 0;
        m_SkyboxFromTexture.m_GSBufferSetPtr   = 0;
        m_SkyboxFromTexture.m_PSBufferSetPtr   = 0;
        m_SkyboxFromTexture.m_InputLayoutPtr   = 0;
        m_SkyboxFromTexture.m_MeshPtr          = 0;
        m_SkyboxFromTexture.m_TextureSetPtr    = 0;
        m_SkyboxFromTexture.m_SamplerSetPtr    = 0;

        m_BackgroundFromSkybox.m_RenderContextPtr = 0;
        m_BackgroundFromSkybox.m_VSPtr            = 0;
        m_BackgroundFromSkybox.m_GSPtr            = 0;
        m_BackgroundFromSkybox.m_PSPtr            = 0;
        m_BackgroundFromSkybox.m_VSBufferSetPtr   = 0;
        m_BackgroundFromSkybox.m_GSBufferSetPtr   = 0;
        m_BackgroundFromSkybox.m_PSBufferSetPtr   = 0;
        m_BackgroundFromSkybox.m_InputLayoutPtr   = 0;
        m_BackgroundFromSkybox.m_MeshPtr          = 0;
        m_BackgroundFromSkybox.m_TextureSetPtr    = 0;
        m_BackgroundFromSkybox.m_SamplerSetPtr    = 0;

        m_BackgroundFromTexture.m_RenderContextPtr = 0;
        m_BackgroundFromTexture.m_VSPtr            = 0;
        m_BackgroundFromTexture.m_GSPtr            = 0;
        m_BackgroundFromTexture.m_PSPtr            = 0;
        m_BackgroundFromTexture.m_VSBufferSetPtr   = 0;
        m_BackgroundFromTexture.m_GSBufferSetPtr   = 0;
        m_BackgroundFromTexture.m_PSBufferSetPtr   = 0;
        m_BackgroundFromTexture.m_InputLayoutPtr   = 0;
        m_BackgroundFromTexture.m_MeshPtr          = 0;
        m_BackgroundFromTexture.m_TextureSetPtr    = 0;
        m_BackgroundFromTexture.m_SamplerSetPtr    = 0;

        // -----------------------------------------------------------------------------
        // Other renderer specific stuff
        // -----------------------------------------------------------------------------
        m_CubemapTextureSetPtr  = 0;
        m_CubemapTargetSetPtr   = 0;
        m_CubemapTexture2DPtr   = 0;
        m_CubemapViewPortSetPtr = 0;

        m_SkyboxRenderJobs.clear();
        m_CameraRenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupShader()
    {
        CShaderPtr SkytextureVSPtr = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");;

        CShaderPtr SkytexturePSPtr = ShaderManager::CompilePS("fs_atmosphere_texture.glsl", "main");


        CShaderPtr SkyboxVSPtr  = ShaderManager::CompileVS("vs_cubemap.glsl"           , "main");
        
        CShaderPtr SkyboxPSPtr  = ShaderManager::CompilePS("fs_atmosphere_cubemap.glsl", "main");
        
        
        CShaderPtr CubemapVSPtr        = ShaderManager::CompileVS("vs_spherical_env_cubemap_generation.glsl", "main");

        CShaderPtr CubemapTextureVSPtr = ShaderManager::CompileVS("vs_texture_env_cubemap_generation.glsl", "main");
        
        CShaderPtr CubemapGSPtr        = ShaderManager::CompileGS("gs_spherical_env_cubemap_generation.glsl", "main");
        
        CShaderPtr CubemapPanoramaPSPtr = ShaderManager::CompilePS("fs_spherical_env_cubemap_generation.glsl", "main");

        CShaderPtr CubemapTexturePSPtr = ShaderManager::CompilePS("fs_texture_env_cubemap_generation.glsl", "main");

        CShaderPtr CubemapCubemapPSPtr = ShaderManager::CompilePS("fs_cubemap_env_cubemap_generation.glsl", "main");

        // -----------------------------------------------------------------------------

        const SInputElementDescriptor InputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };

        CInputLayoutPtr P2SkytextureLayoutPtr = ShaderManager::CreateInputLayout(InputLayout, 1, SkytextureVSPtr);

        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor TriangleInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0,  0, 12, CInputLayout::PerVertex, 0, },
        };
        
        CInputLayoutPtr P3SkyboxLayoutPtr = ShaderManager::CreateInputLayout(TriangleInputLayout, 1, SkyboxVSPtr);
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor PositionInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0, 0 , 32, CInputLayout::PerVertex, 0 },
            { "NORMAL"  , 0, CInputLayout::Float3Format, 0, 12, 32, CInputLayout::PerVertex, 0 },
            { "TEXCOORD", 0, CInputLayout::Float2Format, 0, 24, 32, CInputLayout::PerVertex, 0 },
        };
        
        CInputLayoutPtr P3N3T2CubemapInputLayoutPtr = ShaderManager::CreateInputLayout(PositionInputLayout, 3, CubemapVSPtr);

        // -----------------------------------------------------------------------------

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
        m_SkyboxFromTexture.m_InputLayoutPtr = P2SkytextureLayoutPtr;

        m_BackgroundFromSkybox.m_VSPtr          = SkyboxVSPtr;
        m_BackgroundFromSkybox.m_GSPtr          = 0;
        m_BackgroundFromSkybox.m_PSPtr          = SkyboxPSPtr;
        m_BackgroundFromSkybox.m_InputLayoutPtr = P3SkyboxLayoutPtr;

        m_BackgroundFromTexture.m_VSPtr          = SkytextureVSPtr;
        m_BackgroundFromTexture.m_GSPtr          = 0;
        m_BackgroundFromTexture.m_PSPtr          = SkytexturePSPtr;
        m_BackgroundFromTexture.m_InputLayoutPtr = P2SkytextureLayoutPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupRenderTargets()
    {
        // -----------------------------------------------------------------------------
        // Cubemap
        // -----------------------------------------------------------------------------
        STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = 2048;
        TextureDescriptor.m_NumberOfPixelsV  = 2048;
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
        
        m_CubemapTexture2DPtr = TextureManager::CreateCubeTexture(TextureDescriptor);
        
        for (unsigned int IndexOfCubemapLayer = 0; IndexOfCubemapLayer < 6; ++IndexOfCubemapLayer)
        {
            CTexture2DPtr CubeLayer = TextureManager::CreateTexture2D(TextureDescriptor);
            
            TextureManager::CopyToTextureArray2D(m_CubemapTexture2DPtr, IndexOfCubemapLayer, CubeLayer, false);
        }
        
        TextureManager::UpdateMipmap(m_CubemapTexture2DPtr);

        m_CubemapTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_CubemapTexture2DPtr));

        // -----------------------------------------------------------------------------
        // Target Set
        // -----------------------------------------------------------------------------
        CTexture2DPtr FirstMipmapCubeTexture = TextureManager::GetMipmapFromTexture2D(m_CubemapTexture2DPtr, 0);

        m_CubemapTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(FirstMipmapCubeTexture));

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

        m_CubemapViewPortSetPtr = ViewManager::CreateViewPortSet(MipMapViewPort);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupStates()
    {
        CCameraPtr          CameraPtr          = ViewManager     ::GetMainCamera();
        CCameraPtr          QuadCameraPtr      = ViewManager     ::GetFullQuadCamera();
        CViewPortSetPtr     ViewPortSetPtr     = ViewManager     ::GetViewPortSet();
        CRenderStatePtr     NoDepthStatePtr    = StateManager    ::GetRenderState(CRenderState::NoDepth | CRenderState::NoCull);
        CRenderStatePtr     LightStatePtr      = StateManager    ::GetRenderState(0);
        CTargetSetPtr       TargetSetPtr       = TargetSetManager::GetLightAccumulationTargetSet();

        // -----------------------------------------------------------------------------

        CRenderContextPtr CubemapRenderContextPtr = ContextManager::CreateRenderContext();

        CubemapRenderContextPtr->SetCamera(CameraPtr);
        CubemapRenderContextPtr->SetViewPortSet(m_CubemapViewPortSetPtr);
        CubemapRenderContextPtr->SetTargetSet(m_CubemapTargetSetPtr);
        CubemapRenderContextPtr->SetRenderState(NoDepthStatePtr);
        
        // -----------------------------------------------------------------------------
        
        CRenderContextPtr SkyRenderContextPtr = ContextManager::CreateRenderContext();
        
        SkyRenderContextPtr->SetCamera(CameraPtr);
        SkyRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        SkyRenderContextPtr->SetTargetSet(TargetSetPtr);
        SkyRenderContextPtr->SetRenderState(NoDepthStatePtr);
        
        // -----------------------------------------------------------------------------
        
        CSamplerPtr LinearFilter = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);
        
        CSamplerSetPtr SamplerSetPtr = SamplerManager::CreateSamplerSet(LinearFilter);

        // -----------------------------------------------------------------------------

        m_SkyboxFromPanorama.m_RenderContextPtr = CubemapRenderContextPtr;
        m_SkyboxFromPanorama.m_SamplerSetPtr    = SamplerSetPtr;

        m_SkyboxFromCubemap.m_RenderContextPtr = CubemapRenderContextPtr;
        m_SkyboxFromCubemap.m_SamplerSetPtr    = SamplerSetPtr;

        m_SkyboxFromTexture.m_RenderContextPtr = CubemapRenderContextPtr;
        m_SkyboxFromTexture.m_SamplerSetPtr    = SamplerSetPtr;

        m_BackgroundFromSkybox.m_RenderContextPtr = SkyRenderContextPtr;
        m_BackgroundFromSkybox.m_SamplerSetPtr    = SamplerSetPtr;

        m_BackgroundFromTexture.m_RenderContextPtr = SkyRenderContextPtr;
        m_BackgroundFromTexture.m_SamplerSetPtr    = SamplerSetPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupTextures()
    {
        CTextureSetPtr DepthTextureSetPtr = TextureManager::CreateTextureSet(TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());

        // -----------------------------------------------------------------------------

        m_SkyboxFromPanorama.m_TextureSetPtr = 0;

        m_SkyboxFromCubemap.m_TextureSetPtr = 0;

        m_SkyboxFromTexture.m_TextureSetPtr = 0;

        m_BackgroundFromSkybox.m_TextureSetPtr = DepthTextureSetPtr;

        m_BackgroundFromTexture.m_TextureSetPtr = DepthTextureSetPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSkyboxVSBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SkyboxVSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSkyboxFromTextureVSBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SkyboxFromTextureVSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        Base::Float3 EyePosition = Base::Float3(0.0f);
        Base::Float3 UpDirection;
        Base::Float3 LookDirection;
        
        SCubemapBufferGS DefaultGSValues;
        
        DefaultGSValues.m_CubeProjectionMatrix.SetRHFieldOfView(Base::RadiansToDegree(Base::SConstants<float>::s_Pi * 0.5f), 1.0f, 0.3f, 20000.0f);
        
        // -----------------------------------------------------------------------------
        
        // Right; +X
        LookDirection = EyePosition + Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[0].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        // Left; -X
        LookDirection = EyePosition - Base::Float3::s_AxisX;
        UpDirection   = Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[1].LookAt(EyePosition, LookDirection, UpDirection);

        // -----------------------------------------------------------------------------

        // Front; +Y
        LookDirection = EyePosition + Base::Float3::s_AxisY;
        UpDirection = Base::Float3::s_AxisZ;

        DefaultGSValues.m_CubeViewMatrix[2].LookAt(EyePosition, LookDirection, UpDirection);

        // -----------------------------------------------------------------------------

        // Back; -Y
        LookDirection = EyePosition - Base::Float3::s_AxisY;
        UpDirection = Base::Float3::s_Zero - Base::Float3::s_AxisZ;

        DefaultGSValues.m_CubeViewMatrix[3].LookAt(EyePosition, LookDirection, UpDirection);

        // -----------------------------------------------------------------------------
        
        // Top; +Z
        LookDirection = EyePosition - Base::Float3::s_AxisZ;
        UpDirection = Base::Float3::s_AxisY;
        
        DefaultGSValues.m_CubeViewMatrix[4].LookAt(EyePosition, LookDirection, UpDirection);

        // -----------------------------------------------------------------------------

        // Bottom; -Z
        LookDirection = EyePosition + Base::Float3::s_AxisZ;
        UpDirection = Base::Float3::s_AxisY;

        DefaultGSValues.m_CubeViewMatrix[5].LookAt(EyePosition, LookDirection, UpDirection);

        // -----------------------------------------------------------------------------
        
        for (unsigned int IndexOfCubeface = 0; IndexOfCubeface < 6; ++ IndexOfCubeface)
        {
            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface] *= Base::Float4x4().SetRotationX(Base::DegreesToRadians(90.0f));
        }
        
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
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSkyboxBufferPS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SkyboxPSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

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

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSkytextureBufferPS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SkytexturePSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        CBufferPtr HistogramExposureHistoryBufferPtr = HistogramRenderer::GetExposureHistoryBuffer();
        
        // -----------------------------------------------------------------------------
        
        CBufferSetPtr SkytextureVSBufferSetPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS());
        CBufferSetPtr SkytexturePSBufferSetPtr = BufferManager::CreateBufferSet(SkytexturePSBuffer, HistogramExposureHistoryBufferPtr);

        CBufferSetPtr CubemapGSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSBuffer);
        CBufferSetPtr CubemapPSBufferSetPtr = BufferManager::CreateBufferSet(CubemapPSBuffer);

        CBufferSetPtr SkyboxVSBufferSetPtr = BufferManager::CreateBufferSet(SkyboxVSBuffer);
        CBufferSetPtr SkyboxPSBufferSetPtr = BufferManager::CreateBufferSet(SkyboxPSBuffer, HistogramExposureHistoryBufferPtr);

        CBufferSetPtr SkyboxFromTextureVSBufferSetPtr = BufferManager::CreateBufferSet(SkyboxFromTextureVSBufferPtr);

        // -----------------------------------------------------------------------------

        m_SkyboxFromPanorama.m_VSBufferSetPtr = 0;
        m_SkyboxFromPanorama.m_GSBufferSetPtr = CubemapGSBufferSetPtr;
        m_SkyboxFromPanorama.m_PSBufferSetPtr = CubemapPSBufferSetPtr;

        m_SkyboxFromCubemap.m_VSBufferSetPtr = 0;
        m_SkyboxFromCubemap.m_GSBufferSetPtr = CubemapGSBufferSetPtr;
        m_SkyboxFromCubemap.m_PSBufferSetPtr = CubemapPSBufferSetPtr;

        m_SkyboxFromTexture.m_VSBufferSetPtr = SkyboxFromTextureVSBufferSetPtr;
        m_SkyboxFromTexture.m_GSBufferSetPtr = CubemapGSBufferSetPtr;
        m_SkyboxFromTexture.m_PSBufferSetPtr = CubemapPSBufferSetPtr;

        m_BackgroundFromSkybox.m_VSBufferSetPtr = SkyboxVSBufferSetPtr;
        m_BackgroundFromSkybox.m_GSBufferSetPtr = 0;
        m_BackgroundFromSkybox.m_PSBufferSetPtr = SkyboxPSBufferSetPtr;

        m_BackgroundFromTexture.m_VSBufferSetPtr = SkytextureVSBufferSetPtr;
        m_BackgroundFromTexture.m_GSBufferSetPtr = 0;
        m_BackgroundFromTexture.m_PSBufferSetPtr = SkytexturePSBufferSetPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupModels()
    {
        SMeshDescriptor ModelDescr;
               
        CMeshPtr SkyboxBoxPtr = MeshManager::CreateBox(2.0f, 2.0f, 2.0f);
               
        // -----------------------------------------------------------------------------
        
        Dt::SModelFileDescriptor ModelFileDesc;

        ModelFileDesc.m_pFileName = "envsphere.obj";
        ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::Nothing;

        Dt::CModel& rSphereModel = Dt::ModelManager::CreateModel(ModelFileDesc);
        
        ModelDescr.m_pModel = &rSphereModel.GetMesh(0);
        
        CMeshPtr CubemapTextureSpherePtr = MeshManager::CreateMesh(ModelDescr);

        // -----------------------------------------------------------------------------

        CMeshPtr QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);

        // -----------------------------------------------------------------------------

        m_SkyboxFromPanorama.m_MeshPtr = CubemapTextureSpherePtr;

        m_SkyboxFromCubemap.m_MeshPtr = CubemapTextureSpherePtr;

        m_SkyboxFromTexture.m_MeshPtr = QuadModelPtr;

        m_BackgroundFromSkybox.m_MeshPtr = SkyboxBoxPtr;

        m_BackgroundFromTexture.m_MeshPtr = QuadModelPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupEnd()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::Render()
    {
        bool HasEnvironmentSkybox = m_SkyboxRenderJobs.size() != 0;
        bool HasMainCamera        = m_CameraRenderJobs.size() != 0;

        if (HasEnvironmentSkybox)
        {
            SSkyboxRenderJob& rRenderJob = m_SkyboxRenderJobs[0];

            Performance::BeginEvent("Sky");

#if 0
            RenderSkyboxFromTexture();
#else
            if (rRenderJob.m_pDataSkybox->GetType() == Dt::CSkyboxFacet::Panorama)
            {
                RenderSkyboxFromPanorama();
            }
            else if (rRenderJob.m_pDataSkybox->GetType() == Dt::CSkyboxFacet::Cubemap)
            {
                RenderSkyboxFromCubemap();
            }
#endif

            Performance::EndEvent();
        }

        if (HasMainCamera)
        {
            SCameraRenderJob& rRenderJob = m_CameraRenderJobs[0];

            Performance::BeginEvent("Background");

            if (HasEnvironmentSkybox != 0 && rRenderJob.m_pDataCamera->GetClearFlag() == Dt::CCameraActorFacet::Skybox)
            {
                RenderBackgroundFromSkybox();
            }
            else if (rRenderJob.m_pDataCamera->GetClearFlag() == Dt::CCameraActorFacet::Texture)
            {
                RenderBackgroundFromTexture();
            }
            else
            {
                // RenderBackgroundFromColor();
            }

            Performance::EndEvent();
        }

        
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CGfxSkyRenderer::GetCubemap()
    {
        return m_CubemapTextureSetPtr;
    }
        
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::RenderSkyboxFromPanorama()
    {
        SSkyboxRenderJob& rRenderJob = m_SkyboxRenderJobs[0];

        if (rRenderJob.m_pGraphicSkybox->GetTimeStamp() != Core::Time::GetNumberOfFrame())
        {
            return;
        }

        CRenderContextPtr RenderContextPtr = m_SkyboxFromPanorama.m_RenderContextPtr;
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

        pPSBuffer->m_HDRFactor = rRenderJob.m_pDataSkybox->GetIntensity();
        pPSBuffer->m_IsHDR     = rRenderJob.m_pDataSkybox->GetPanorama()->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

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

        ContextManager::SetTextureSetPS(rRenderJob.m_pGraphicSkybox->GetPanoramaTextureSet());

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
        TextureManager::UpdateMipmap(m_CubemapTexture2DPtr);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyRenderer::RenderSkyboxFromCubemap()
    {
        SSkyboxRenderJob& rRenderJob = m_SkyboxRenderJobs[0];

        if (!rRenderJob.m_pDataSkybox->GetHasCubemap() || rRenderJob.m_pDataSkybox->GetCubemap()->GetDirtyTime() != Core::Time::GetNumberOfFrame())
        {
            return;
        }

        CRenderContextPtr RenderContextPtr = m_SkyboxFromCubemap.m_RenderContextPtr;
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

        pPSBuffer->m_HDRFactor = rRenderJob.m_pDataSkybox->GetIntensity();
        pPSBuffer->m_IsHDR     = rRenderJob.m_pDataSkybox->GetCubemap()->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

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

        ContextManager::SetTextureSetPS(rRenderJob.m_pGraphicSkybox->GetCubemapTextureSet());

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
        TextureManager::UpdateMipmap(m_CubemapTexture2DPtr);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyRenderer::RenderSkyboxFromTexture()
    {
        SCameraRenderJob& rCameraRenderJob = m_CameraRenderJobs[0];
        SSkyboxRenderJob& rRenderJob       = m_SkyboxRenderJobs[0];

        if (rCameraRenderJob.m_pDataCamera->GetTexture() != nullptr && rCameraRenderJob.m_pDataCamera->GetTexture()->GetDirtyTime() != Core::Time::GetNumberOfFrame())
        {
            return;
        }

        if (rCameraRenderJob.m_pGraphicCamera->GetBackgroundTexture2D() == nullptr)
        {
            return;
        }

        CRenderContextPtr RenderContextPtr = m_SkyboxFromTexture.m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_SkyboxFromTexture.m_VSPtr;
        CShaderPtr        GSPtr            = m_SkyboxFromTexture.m_GSPtr;
        CShaderPtr        PSPtr            = m_SkyboxFromTexture.m_PSPtr;
        CBufferSetPtr     VSBufferSetPtr   = m_SkyboxFromTexture.m_VSBufferSetPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_SkyboxFromTexture.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_SkyboxFromTexture.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr   = m_SkyboxFromTexture.m_InputLayoutPtr;
        CMeshPtr          MeshPtr          = m_SkyboxFromTexture.m_MeshPtr;
        CTextureSetPtr    TextureSetPtr    = m_SkyboxFromTexture.m_TextureSetPtr;
        CSamplerSetPtr    SamplerSetPtr    = m_SkyboxFromTexture.m_SamplerSetPtr;

        Performance::BeginEvent("Skybox from Texture");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SSkyboxFromTextureVSBuffer* pViewBuffer = static_cast<SSkyboxFromTextureVSBuffer*>(BufferManager::MapConstantBuffer(VSBufferSetPtr->GetBuffer(0)));

//         Base::Float4x4 Test;
// 
//         Test  = Base::Float4x4::s_Identity;
//         Test *= ViewManager::GetMainCamera()->GetView()->GetRotationMatrix();
//         Test *= Base::Float4x4().SetTranslation(0.0f, 1.0f, 0.0f);
// 
//         float D = Base::Sqrt(Test[0][0] * Test[0][0] + Test[1][0] * Test[1][0] + Test[2][0] * Test[2][0]);
// 
//         pViewBuffer->m_ModelMatrix = Base::Float4x4::s_Identity;
//         pViewBuffer->m_ModelMatrix.InjectScale(D);
//         pViewBuffer->m_ModelMatrix.InjectTranslation(Test[0][3], Test[1][3], Test[2][3]);

//         //pViewBuffer->m_ModelMatrix *= ViewManager::GetMainCamera()->GetView()->GetRotationMatrix();
//         //pViewBuffer->m_ModelMatrix *= Base::Float4x4().SetRotationX(Base::DegreesToRadians(-90.0f));
//         pViewBuffer->m_ModelMatrix *= Base::Float4x4().SetTranslation(0.0f, 1.0f, 0.0f);

//         pViewBuffer->m_ModelMatrix *= Base::Float4x4().SetRotationX(Base::DegreesToRadians(-90.0f));

        pViewBuffer->m_ModelMatrix  = Base::Float4x4::s_Identity;
        pViewBuffer->m_ModelMatrix *= ViewManager::GetMainCamera()->GetView()->GetRotationMatrix();
        pViewBuffer->m_ModelMatrix *= Base::Float4x4().SetTranslation(0.0f, 1.0f, 0.0f);
        pViewBuffer->m_ModelMatrix *= Base::Float4x4().SetRotationX(Base::DegreesToRadians(-90.0f));

        BufferManager::UnmapConstantBuffer(VSBufferSetPtr->GetBuffer(0));

        // -----------------------------------------------------------------------------

        SCubemapBufferPS* pPSBuffer = static_cast<SCubemapBufferPS*>(BufferManager::MapConstantBuffer(PSBufferSetPtr->GetBuffer(0)));

        pPSBuffer->m_HDRFactor = rRenderJob.m_pDataSkybox->GetIntensity();
        pPSBuffer->m_IsHDR     = rCameraRenderJob.m_pDataCamera->GetTexture()->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

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

        ContextManager::SetConstantBufferSetVS(VSBufferSetPtr);

        ContextManager::SetConstantBufferSetGS(GSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(PSBufferSetPtr);

        ContextManager::SetTextureSetPS(rCameraRenderJob.m_pGraphicCamera->GetBackgroundTextureSet());

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

        ContextManager::ResetConstantBufferSetVS();

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
        TextureManager::UpdateMipmap(m_CubemapTexture2DPtr);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxSkyRenderer::RenderBackgroundFromSkybox()
    {
        CRenderContextPtr RenderContextPtr = m_BackgroundFromSkybox.m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_BackgroundFromSkybox.m_VSPtr;
        CShaderPtr        GSPtr            = m_BackgroundFromSkybox.m_GSPtr;
        CShaderPtr        PSPtr            = m_BackgroundFromSkybox.m_PSPtr;
        CBufferSetPtr     VSBufferSetPtr   = m_BackgroundFromSkybox.m_VSBufferSetPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_BackgroundFromSkybox.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_BackgroundFromSkybox.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr   = m_BackgroundFromSkybox.m_InputLayoutPtr;
        CMeshPtr          MeshPtr          = m_BackgroundFromSkybox.m_MeshPtr;
        CTextureSetPtr    TextureSetPtr    = m_BackgroundFromSkybox.m_TextureSetPtr;
        CSamplerSetPtr    SamplerSetPtr    = m_BackgroundFromSkybox.m_SamplerSetPtr;

        // -----------------------------------------------------------------------------
        // Render skybox
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Background From Skybox");

        // -----------------------------------------------------------------------------
        // Upload dynamic data
        // -----------------------------------------------------------------------------
        CCameraPtr CameraPtr = RenderContextPtr->GetCamera();

        SSkyboxVSBuffer* pViewBuffer = static_cast<SSkyboxVSBuffer*>(BufferManager::MapConstantBuffer(VSBufferSetPtr->GetBuffer(0)));

        pViewBuffer->m_View       = CameraPtr->GetView()->GetViewMatrix();
        pViewBuffer->m_Projection = CameraPtr->GetProjectionMatrix();

        pViewBuffer->m_View.InjectTranslation(0.0f, 0.0f, 0.0f);

        BufferManager::UnmapConstantBuffer(VSBufferSetPtr->GetBuffer(0));

        // -----------------------------------------------------------------------------

        SSkyboxBufferPS* pPSBuffer = static_cast<SSkyboxBufferPS*>(BufferManager::MapConstantBuffer(PSBufferSetPtr->GetBuffer(0)));
        
        assert(pPSBuffer != nullptr);
    
        pPSBuffer->m_InvertedScreenSize   = Base::Float4(1.0f / Main::GetActiveWindowSize()[0], 1.0f / Main::GetActiveWindowSize()[1], 0, 0);
        pPSBuffer->m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();
        
        BufferManager::UnmapConstantBuffer(PSBufferSetPtr->GetBuffer(0));
        
        // -----------------------------------------------------------------------------
        // Render sky box in background
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = {0, 0};
        
        ContextManager::SetRenderContext(RenderContextPtr);
        
        ContextManager::SetSamplerSetPS(SamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(InputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(VSPtr);
        
        ContextManager::SetShaderPS(PSPtr);
        
        ContextManager::SetConstantBufferSetVS(VSBufferSetPtr);
        
        ContextManager::SetConstantBufferSetPS(PSBufferSetPtr);
        
        ContextManager::SetTextureSetPS(m_CubemapTextureSetPtr);

        ContextManager::SetTextureSetPS(TextureSetPtr);
        
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTextureSetPS();
        
        ContextManager::ResetConstantBufferSetPS();
        
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

    void CGfxSkyRenderer::RenderBackgroundFromTexture()
    {
        SCameraRenderJob& rRenderJob         = m_CameraRenderJobs[0];

        if (rRenderJob.m_pGraphicCamera->GetBackgroundTexture2D() == nullptr)
        {
            return;
        }

        // -----------------------------------------------------------------------------
        // Prepare value from sky / environment
        // -----------------------------------------------------------------------------
        float HDRIntensity = 1.0f;

        if (m_SkyboxRenderJobs.size() > 0)
        {
            SSkyboxRenderJob& rEnvironmentSkyJob = m_SkyboxRenderJobs[0];

            HDRIntensity = rEnvironmentSkyJob.m_pDataSkybox->GetIntensity();
        }

        CRenderContextPtr RenderContextPtr = m_BackgroundFromTexture.m_RenderContextPtr;
        CShaderPtr        VSPtr            = m_BackgroundFromTexture.m_VSPtr;
        CShaderPtr        GSPtr            = m_BackgroundFromTexture.m_GSPtr;
        CShaderPtr        PSPtr            = m_BackgroundFromTexture.m_PSPtr;
        CBufferSetPtr     VSBufferSetPtr   = m_BackgroundFromTexture.m_VSBufferSetPtr;
        CBufferSetPtr     GSBufferSetPtr   = m_BackgroundFromTexture.m_GSBufferSetPtr;
        CBufferSetPtr     PSBufferSetPtr   = m_BackgroundFromTexture.m_PSBufferSetPtr;
        CInputLayoutPtr   InputLayoutPtr   = m_BackgroundFromTexture.m_InputLayoutPtr;
        CMeshPtr          MeshPtr          = m_BackgroundFromTexture.m_MeshPtr;
        CTextureSetPtr    TextureSetPtr    = m_BackgroundFromTexture.m_TextureSetPtr;
        CSamplerSetPtr    SamplerSetPtr    = m_BackgroundFromTexture.m_SamplerSetPtr;

        // -----------------------------------------------------------------------------
        // Render sky texture
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Background from Texture");
        
        // -----------------------------------------------------------------------------
        // Data
        // -----------------------------------------------------------------------------
        SSkytextureBufferPS* pPSBuffer = static_cast<SSkytextureBufferPS*>(BufferManager::MapConstantBuffer(PSBufferSetPtr->GetBuffer(0)));

        pPSBuffer->m_HDRFactor     = HDRIntensity;
        pPSBuffer->m_IsHDR         = rRenderJob.m_pGraphicCamera->GetBackgroundTexture2D()->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;
        pPSBuffer->m_ExposureIndex = static_cast<float>(HistogramRenderer::GetLastExposureHistoryIndex());

        BufferManager::UnmapConstantBuffer(PSBufferSetPtr->GetBuffer(0));

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = { 0, 0 };

        ContextManager::SetRenderContext(RenderContextPtr);

        ContextManager::SetSamplerSetPS(SamplerSetPtr);

        ContextManager::SetVertexBufferSet(MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(InputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetConstantBufferSetVS(VSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(PSBufferSetPtr);

        ContextManager::SetTextureSetPS(rRenderJob.m_pGraphicCamera->GetBackgroundTextureSet());

        ContextManager::SetTextureSetPS(TextureSetPtr);

        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTextureSetPS();

        ContextManager::ResetConstantBufferSetPS();

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

    void CGfxSkyRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_SkyboxRenderJobs.clear();
        m_CameraRenderJobs.clear();

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
            if (rCurrentEntity.GetType() == Dt::SLightType::Skybox)
            {
                Dt::CSkyboxFacet*  pDataSkyboxFacet    = static_cast<Dt::CSkyboxFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                Gfx::CSkyboxFacet* pGraphicSkyboxFacet = static_cast<Gfx::CSkyboxFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                // -----------------------------------------------------------------------------
                // Set sun into a new render job
                // -----------------------------------------------------------------------------
                SSkyboxRenderJob NewRenderJob;

                NewRenderJob.m_pDataSkybox    = pDataSkyboxFacet;
                NewRenderJob.m_pGraphicSkybox = pGraphicSkyboxFacet;

                m_SkyboxRenderJobs.push_back(NewRenderJob);
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }

        CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Actor);
        EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() == Dt::SActorType::Camera)
            {
                Dt::CCameraActorFacet* pDataCameraFacet = static_cast<Dt::CCameraActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));
                Gfx::CCameraActorFacet* pGraphicCameraFacet = static_cast<Gfx::CCameraActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                if (pDataCameraFacet->IsMainCamera())
                {
                    SCameraRenderJob NewRenderJob;

                    NewRenderJob.m_pDataCamera    = pDataCameraFacet;
                    NewRenderJob.m_pGraphicCamera = pGraphicCameraFacet;

                    m_CameraRenderJobs.push_back(NewRenderJob);
                }
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);
        }
    }
} // namespace

namespace Gfx
{
namespace SkyRenderer
{
    void OnStart()
    {
        CGfxSkyRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxSkyRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxSkyRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxSkyRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxSkyRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxSkyRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxSkyRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxSkyRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxSkyRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxSkyRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxSkyRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxSkyRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxSkyRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxSkyRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxSkyRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxSkyRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr GetCubemap()
    {
        return CGfxSkyRenderer::GetInstance().GetCubemap();
    }
} // namespace SkyRenderer
} // namespace Gfx

