
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
    
        CBufferSetPtr     m_SkyboxVSBufferSetPtr;
        CBufferSetPtr     m_CubemapGSBufferSetPtr;
        CBufferSetPtr     m_CubemapPSBufferSetPtr;
        CBufferSetPtr     m_SkyboxPSBufferSetPtr;
        CBufferSetPtr     m_SkytextureVSBufferSetPtr;
        CBufferSetPtr     m_SkytexturePSBufferSetPtr;
        CInputLayoutPtr   m_P3SkyboxLayoutPtr;
        CInputLayoutPtr   m_P3N3T2CubemapInputLayoutPtr;
        CInputLayoutPtr   m_P2SkytextureLayoutPtr;
        CMeshPtr          m_SkyboxBoxPtr;
        CMeshPtr          m_CubemapTextureSpherePtr;
        CMeshPtr          m_QuadModelPtr;
        CRenderContextPtr m_SkyRenderContextPtr;
        CRenderContextPtr m_CubemapRenderContextPtr;
        CShaderPtr        m_SkyboxVSPtr;
        CShaderPtr        m_SkyboxPSPtr;
        CShaderPtr        m_CubemapVSPtr;
        CShaderPtr        m_CubemapGSPtr;
        CShaderPtr        m_CubemapTexturePSPtr;
        CShaderPtr        m_CubemapCubemapPSPtr;
        CShaderPtr        m_SkytextureVSPtr;
        CShaderPtr        m_SkytexturePSPtr;
        CTargetSetPtr     m_CubemapTargetSetPtr;
        CTexture2DPtr     m_CubemapTexture2DPtr;
        CTextureSetPtr    m_DepthTextureSetPtr;
        CTextureSetPtr    m_CubemapTextureSetPtr;
        CSamplerSetPtr    m_PSSamplerSetPtr;
        CViewPortSetPtr   m_CubemapViewPortSetPtr;

        CSkyboxRenderJobs m_SkyboxRenderJobs;
        CCameraRenderJobs m_CameraRenderJobs;
        
    private:
        
        void RenderSkyboxFromPanorama();

        void RenderSkyboxFromCubemap();

        void RenderBackgroundFromSkybox();

        void RenderBackgroundFromTexture();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxSkyRenderer::CGfxSkyRenderer()
        : m_SkyboxVSBufferSetPtr       ()
        , m_CubemapGSBufferSetPtr      ()
        , m_CubemapPSBufferSetPtr      ()
        , m_SkyboxPSBufferSetPtr       ()
        , m_SkytextureVSBufferSetPtr   ()
        , m_SkytexturePSBufferSetPtr   ()
        , m_P3SkyboxLayoutPtr          ()
        , m_P3N3T2CubemapInputLayoutPtr()
        , m_P2SkytextureLayoutPtr      ()
        , m_SkyboxBoxPtr               ()
        , m_CubemapTextureSpherePtr    ()
        , m_QuadModelPtr               ()
        , m_SkyRenderContextPtr        ()
        , m_CubemapRenderContextPtr    ()
        , m_SkyboxVSPtr                ()
        , m_SkyboxPSPtr                ()
        , m_CubemapVSPtr               ()
        , m_CubemapGSPtr               ()
        , m_CubemapTexturePSPtr        ()
        , m_CubemapCubemapPSPtr        ()
        , m_SkytextureVSPtr            ()
        , m_SkytexturePSPtr            ()
        , m_CubemapTargetSetPtr        ()
        , m_CubemapTexture2DPtr        ()
        , m_DepthTextureSetPtr         ()
        , m_CubemapTextureSetPtr       ()
        , m_PSSamplerSetPtr            ()
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
        m_SkyboxVSBufferSetPtr        = 0;
        m_CubemapGSBufferSetPtr       = 0;
        m_CubemapPSBufferSetPtr       = 0;
        m_SkyboxPSBufferSetPtr        = 0;
        m_SkytextureVSBufferSetPtr    = 0;
        m_SkytexturePSBufferSetPtr    = 0;
        m_P3SkyboxLayoutPtr           = 0;
        m_P3N3T2CubemapInputLayoutPtr = 0;
        m_P2SkytextureLayoutPtr       = 0;
        m_SkyboxBoxPtr                = 0;
        m_CubemapTextureSpherePtr     = 0;
        m_QuadModelPtr                = 0;
        m_SkyRenderContextPtr         = 0;
        m_CubemapRenderContextPtr     = 0;
        m_SkyboxVSPtr                 = 0;
        m_SkyboxPSPtr                 = 0;
        m_CubemapVSPtr                = 0;
        m_CubemapGSPtr                = 0;
        m_CubemapTexturePSPtr         = 0;
        m_CubemapCubemapPSPtr         = 0;
        m_SkytextureVSPtr             = 0;
        m_SkytexturePSPtr             = 0;
        m_CubemapTargetSetPtr         = 0;
        m_CubemapTexture2DPtr         = 0;
        m_DepthTextureSetPtr          = 0;
        m_CubemapTextureSetPtr        = 0;
        m_PSSamplerSetPtr             = 0;
        m_CubemapViewPortSetPtr       = 0;

        m_SkyboxRenderJobs.clear();
        m_CameraRenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupShader()
    {
        m_SkytextureVSPtr = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");;

        m_SkytexturePSPtr = ShaderManager::CompilePS("fs_atmosphere_texture.glsl", "main");


        m_SkyboxVSPtr  = ShaderManager::CompileVS("vs_cubemap.glsl"           , "main");
        
        m_SkyboxPSPtr  = ShaderManager::CompilePS("fs_atmosphere_cubemap.glsl", "main");
        
        
        m_CubemapVSPtr        = ShaderManager::CompileVS("vs_spherical_env_cubemap_generation.glsl", "main");
        
        m_CubemapGSPtr        = ShaderManager::CompileGS("gs_spherical_env_cubemap_generation.glsl", "main");
        
        m_CubemapTexturePSPtr = ShaderManager::CompilePS("fs_spherical_env_cubemap_generation.glsl", "main");

        m_CubemapCubemapPSPtr = ShaderManager::CompilePS("fs_cubemap_env_cubemap_generation.glsl", "main");

        // -----------------------------------------------------------------------------

        const SInputElementDescriptor InputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };

        m_P2SkytextureLayoutPtr = ShaderManager::CreateInputLayout(InputLayout, 1, m_SkytextureVSPtr);

        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor TriangleInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0,  0, 12, CInputLayout::PerVertex, 0, },
        };
        
        m_P3SkyboxLayoutPtr = ShaderManager::CreateInputLayout(TriangleInputLayout, 1, m_SkyboxVSPtr);
        
        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor PositionInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0, 0 , 32, CInputLayout::PerVertex, 0 },
            { "NORMAL"  , 0, CInputLayout::Float3Format, 0, 12, 32, CInputLayout::PerVertex, 0 },
            { "TEXCOORD", 0, CInputLayout::Float2Format, 0, 24, 32, CInputLayout::PerVertex, 0 },
        };
        
        m_P3N3T2CubemapInputLayoutPtr = ShaderManager::CreateInputLayout(PositionInputLayout, 3, m_CubemapVSPtr);
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
        STextureDescriptor          TextureDescriptor;

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

        m_CubemapRenderContextPtr = ContextManager::CreateRenderContext();

        m_CubemapRenderContextPtr->SetCamera(CameraPtr);
        m_CubemapRenderContextPtr->SetViewPortSet(m_CubemapViewPortSetPtr);
        m_CubemapRenderContextPtr->SetTargetSet(m_CubemapTargetSetPtr);
        m_CubemapRenderContextPtr->SetRenderState(NoDepthStatePtr);
        
        // -----------------------------------------------------------------------------
        
        m_SkyRenderContextPtr = ContextManager::CreateRenderContext();
        
        m_SkyRenderContextPtr->SetCamera(CameraPtr);
        m_SkyRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_SkyRenderContextPtr->SetTargetSet(TargetSetPtr);
        m_SkyRenderContextPtr->SetRenderState(NoDepthStatePtr);
        
        // -----------------------------------------------------------------------------
        
        CSamplerPtr LinearFilter = SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp);
        
        m_PSSamplerSetPtr = SamplerManager::CreateSamplerSet(LinearFilter);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupTextures()
    {
        m_DepthTextureSetPtr = TextureManager::CreateTextureSet(TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());
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
        
        Base::Float3 EyePosition = Base::Float3(0.0f);
        Base::Float3 UpDirection;
        Base::Float3 LookDirection;
        
        float lookAt =  1.5f;
        
        SCubemapBufferGS DefaultGSValues;
        
        DefaultGSValues.m_CubeProjectionMatrix.SetRHFieldOfView(Base::RadiansToDegree(Base::SConstants<float>::s_Pi * 0.5f), 1.0f, 1.0f, 20000.0f);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(lookAt, 0.0f, 0.0f);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[0].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(-lookAt, 0.0f, 0.0f);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[1].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, lookAt, 0.0f);
        UpDirection = Base::Float3(0.0f, 0.0f, 1.0f);
        
        DefaultGSValues.m_CubeViewMatrix[3].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, -lookAt, 0.0f);
        UpDirection = Base::Float3(0.0f, 0.0f, -1.0f);
        
        DefaultGSValues.m_CubeViewMatrix[2].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, 0.0f, lookAt);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[4].LookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + Base::Float3(0.0f, 0.0f, -lookAt);
        UpDirection   = Base::Float3(0.0f, -1.0f, 0.0f);
        
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
        
        m_SkytextureVSBufferSetPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS());
        m_SkytexturePSBufferSetPtr = BufferManager::CreateBufferSet(SkytexturePSBuffer, HistogramExposureHistoryBufferPtr);

        m_CubemapGSBufferSetPtr = BufferManager::CreateBufferSet(CubemapGSBuffer);
        m_CubemapPSBufferSetPtr = BufferManager::CreateBufferSet(CubemapPSBuffer);

        m_SkyboxVSBufferSetPtr = BufferManager::CreateBufferSet(SkyboxVSBuffer);
        m_SkyboxPSBufferSetPtr = BufferManager::CreateBufferSet(SkyboxPSBuffer, HistogramExposureHistoryBufferPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSkyRenderer::OnSetupModels()
    {
        SMeshDescriptor ModelDescr;
               
        m_SkyboxBoxPtr = MeshManager::CreateBox(2.0f, 2.0f, 2.0f);
               
        // -----------------------------------------------------------------------------
        
        Dt::SModelFileDescriptor ModelFileDesc;

        ModelFileDesc.m_pFileName = "envsphere.obj";
        ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::Nothing;

        Dt::CModel& rSphereModel = Dt::ModelManager::CreateModel(ModelFileDesc);
        
        ModelDescr.m_pModel = &rSphereModel.GetMesh(0);
        
        m_CubemapTextureSpherePtr = MeshManager::CreateMesh(ModelDescr);

        // -----------------------------------------------------------------------------

        m_QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
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

            if (rRenderJob.m_pDataSkybox->GetType() == Dt::CSkyboxFacet::Panorama)
            {
                RenderSkyboxFromPanorama();
            }
            else if (rRenderJob.m_pDataSkybox->GetType() == Dt::CSkyboxFacet::Cubemap)
            {
                RenderSkyboxFromCubemap();
            }

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

        Performance::BeginEvent("Skybox from Panorama");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SCubemapBufferPS* pPSBuffer = static_cast<SCubemapBufferPS*>(BufferManager::MapConstantBuffer(m_CubemapPSBufferSetPtr->GetBuffer(0)));

        pPSBuffer->m_HDRFactor = rRenderJob.m_pDataSkybox->GetIntensity();
        pPSBuffer->m_IsHDR     = rRenderJob.m_pDataSkybox->GetPanorama()->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

        BufferManager::UnmapConstantBuffer(m_CubemapPSBufferSetPtr->GetBuffer(0));

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           
        const unsigned int pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_CubemapRenderContextPtr);

        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_CubemapVSPtr);

        ContextManager::SetShaderGS(m_CubemapGSPtr);

        ContextManager::SetShaderPS(m_CubemapTexturePSPtr);

        ContextManager::SetVertexBufferSet(m_CubemapTextureSpherePtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(m_CubemapTextureSpherePtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_P3N3T2CubemapInputLayoutPtr);

        ContextManager::SetConstantBufferSetGS(m_CubemapGSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(m_CubemapPSBufferSetPtr);

        ContextManager::SetTextureSetPS(rRenderJob.m_pGraphicSkybox->GetPanoramaTextureSet());

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(m_CubemapTextureSpherePtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

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

        Performance::BeginEvent("Skybox from Cubemap");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SCubemapBufferPS* pPSBuffer = static_cast<SCubemapBufferPS*>(BufferManager::MapConstantBuffer(m_CubemapPSBufferSetPtr->GetBuffer(0)));

        pPSBuffer->m_HDRFactor = rRenderJob.m_pDataSkybox->GetIntensity();
        pPSBuffer->m_IsHDR     = rRenderJob.m_pDataSkybox->GetCubemap()->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;

        BufferManager::UnmapConstantBuffer(m_CubemapPSBufferSetPtr->GetBuffer(0));

        // -----------------------------------------------------------------------------
        // Environment to cube map
        // -----------------------------------------------------------------------------           
        const unsigned int pOffset[] = { 0, 0 };

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_CubemapRenderContextPtr);

        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_CubemapVSPtr);

        ContextManager::SetShaderGS(m_CubemapGSPtr);

        ContextManager::SetShaderPS(m_CubemapCubemapPSPtr);

        ContextManager::SetVertexBufferSet(m_CubemapTextureSpherePtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(m_CubemapTextureSpherePtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_P3N3T2CubemapInputLayoutPtr);

        ContextManager::SetConstantBufferSetGS(m_CubemapGSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(m_CubemapPSBufferSetPtr);

        ContextManager::SetTextureSetPS(rRenderJob.m_pGraphicSkybox->GetCubemapTextureSet());

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        ContextManager::DrawIndexed(m_CubemapTextureSpherePtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

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

    void CGfxSkyRenderer::RenderBackgroundFromSkybox()
    {
        // -----------------------------------------------------------------------------
        // Render skybox
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Background From Skybox");

        // -----------------------------------------------------------------------------
        // Upload dynamic data
        // -----------------------------------------------------------------------------
        CCameraPtr CameraPtr = m_SkyRenderContextPtr->GetCamera();

        SSkyboxVSBuffer* pViewBuffer = static_cast<SSkyboxVSBuffer*>(BufferManager::MapConstantBuffer(m_SkyboxVSBufferSetPtr->GetBuffer(0)));

        pViewBuffer->m_View       = CameraPtr->GetView()->GetViewMatrix();
        pViewBuffer->m_Projection = CameraPtr->GetProjectionMatrix();

        pViewBuffer->m_View.InjectTranslation(0.0f, 0.0f, 0.0f);

        BufferManager::UnmapConstantBuffer(m_SkyboxVSBufferSetPtr->GetBuffer(0));

        // -----------------------------------------------------------------------------

        SSkyboxBufferPS* pPSBuffer = static_cast<SSkyboxBufferPS*>(BufferManager::MapConstantBuffer(m_SkyboxPSBufferSetPtr->GetBuffer(0)));
        
        assert(pPSBuffer != nullptr);
    
        pPSBuffer->m_InvertedScreenSize   = Base::Float4(1.0f / Main::GetActiveWindowSize()[0], 1.0f / Main::GetActiveWindowSize()[1], 0, 0);
        pPSBuffer->m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();
        
        BufferManager::UnmapConstantBuffer(m_SkyboxPSBufferSetPtr->GetBuffer(0));
        
        // -----------------------------------------------------------------------------
        // Render sky box in background
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = {0, 0};
        
        ContextManager::SetRenderContext(m_SkyRenderContextPtr);
        
        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);
        
        ContextManager::SetVertexBufferSet(m_SkyboxBoxPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(m_SkyboxBoxPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(m_P3SkyboxLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_SkyboxVSPtr);
        
        ContextManager::SetShaderPS(m_SkyboxPSPtr);
        
        ContextManager::SetConstantBufferSetVS(m_SkyboxVSBufferSetPtr);
        
        ContextManager::SetConstantBufferSetPS(m_SkyboxPSBufferSetPtr);
        
        ContextManager::SetTextureSetPS(m_CubemapTextureSetPtr);

        ContextManager::SetTextureSetPS(m_DepthTextureSetPtr);
        
        ContextManager::DrawIndexed(m_SkyboxBoxPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
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

        // -----------------------------------------------------------------------------
        // Render sky texture
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Background from Texture");
        
        // -----------------------------------------------------------------------------
        // Data
        // -----------------------------------------------------------------------------
        SSkytextureBufferPS* pPSBuffer = static_cast<SSkytextureBufferPS*>(BufferManager::MapConstantBuffer(m_SkytexturePSBufferSetPtr->GetBuffer(0)));

        pPSBuffer->m_HDRFactor     = HDRIntensity;
        pPSBuffer->m_IsHDR         = rRenderJob.m_pGraphicCamera->GetBackgroundTexture2D()->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;
        pPSBuffer->m_ExposureIndex = static_cast<float>(HistogramRenderer::GetLastExposureHistoryIndex());

        BufferManager::UnmapConstantBuffer(m_SkytexturePSBufferSetPtr->GetBuffer(0));

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = { 0, 0 };

        ContextManager::SetRenderContext(m_SkyRenderContextPtr);

        ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_P2SkytextureLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_SkytextureVSPtr);

        ContextManager::SetShaderPS(m_SkytexturePSPtr);

        ContextManager::SetConstantBufferSetVS(m_SkytextureVSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(m_SkytexturePSBufferSetPtr);

        ContextManager::SetTextureSetPS(rRenderJob.m_pGraphicCamera->GetBackgroundTextureSet());

        ContextManager::SetTextureSetPS(m_DepthTextureSetPtr);

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

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

