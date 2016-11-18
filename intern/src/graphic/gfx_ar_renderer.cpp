
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "data/data_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_light_facet.h"
#include "data/data_map.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_actor_facet.h"
#include "graphic/gfx_ar_renderer.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_kinect_control.h"
#include "mr/mr_webcam_control.h"

using namespace Gfx;

namespace
{
    class CGfxARRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxARRenderer)

    public:

        CGfxARRenderer();
        ~CGfxARRenderer();

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

    private:

        struct SPerDrawCallConstantBufferVS
        {
            Base::Float4x4 m_ModelMatrix;
        };

        struct SBilateralBlurConstantBufferCS
        {
            Base::UInt4 m_Direction;
        };

        struct SRenderJob
        {
            CSurfacePtr      m_SurfacePtr;
            CMaterialPtr     m_SurfaceMaterialPtr;
            Base::Float4x4   m_ModelMatrix;
        };

    private:

        typedef std::vector<SRenderJob> CRenderJobs;

    private:

        CMeshPtr m_QuadModelPtr;

        CInputLayoutPtr m_FullQuadInputLayoutPtr;

        CBufferSetPtr m_ViewVSBufferSetPtr;
        CBufferSetPtr m_BaseVSBufferSetPtr;
        CBufferSetPtr m_BasePSBufferSetPtr;
        CBufferSetPtr m_MaterialPSBufferSetPtr;
        CBufferSetPtr m_BilateralBlurCSBufferSetPtr;

        CRenderContextPtr m_DeferredRenderContextPtr;

        CShaderPtr m_RectangleShaderVSPtr;
        CShaderPtr m_BilateralBlurShaderCSPtr;
        CShaderPtr m_CopyToGBufferShaderPSPtr;
        CShaderPtr m_DifferentialGBufferShaderPSPtr;

        CTexture2DPtr m_BackgroundTexturePtr;
        CTexture2DPtr m_VSPositionTexturePtr;
        CTexture2DPtr m_VSPositionTempTexturePtr;
        CTexture2DPtr m_CubemapTexturePtr;
        CTexture2DPtr m_WebcamTexturePtr;

        CTextureSetPtr m_BilateralBlurTextureSetPtr;
        CTextureSetPtr m_BilateralBlurTempTextureSetPtr;
        CTextureSetPtr m_CopyToGBufferTextureSetPtr;
        CTextureSetPtr m_DifferentualGBufferTextureSetPtr;

        CSamplerSetPtr m_PSSamplerSetPtr;

        CRenderJobs m_RenderJobs;

    private:

        void BuildRenderJobs();     
    };
} // namespace

namespace
{
    CGfxARRenderer::CGfxARRenderer()
        : m_QuadModelPtr                     ()
        , m_FullQuadInputLayoutPtr           ()
        , m_ViewVSBufferSetPtr               ()
        , m_BaseVSBufferSetPtr               ()
        , m_BasePSBufferSetPtr               ()
        , m_MaterialPSBufferSetPtr           ()
        , m_BilateralBlurCSBufferSetPtr      ()
        , m_DeferredRenderContextPtr         ()
        , m_RectangleShaderVSPtr             ()
        , m_BilateralBlurShaderCSPtr         ()
        , m_CopyToGBufferShaderPSPtr         ()
        , m_DifferentialGBufferShaderPSPtr   ()
        , m_BackgroundTexturePtr             ()
        , m_VSPositionTexturePtr             ()
        , m_VSPositionTempTexturePtr         ()
        , m_CubemapTexturePtr                ()
        , m_WebcamTexturePtr                 ()
        , m_BilateralBlurTextureSetPtr       ()
        , m_BilateralBlurTempTextureSetPtr   ()
        , m_CopyToGBufferTextureSetPtr       ()
        , m_DifferentualGBufferTextureSetPtr ()
        , m_PSSamplerSetPtr                  ()
        , m_RenderJobs               ()
    {
        // -----------------------------------------------------------------------------
        // Register resize delegate
        // -----------------------------------------------------------------------------
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxARRenderer::OnResize));

        // -----------------------------------------------------------------------------
        // Reserve some jobs
        // -----------------------------------------------------------------------------
        m_RenderJobs.reserve(2);
    }

    // -----------------------------------------------------------------------------

    CGfxARRenderer::~CGfxARRenderer()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnExit()
    {
        m_QuadModelPtr                     = 0;
        m_FullQuadInputLayoutPtr           = 0;
        m_ViewVSBufferSetPtr               = 0;
        m_BaseVSBufferSetPtr               = 0;
        m_BasePSBufferSetPtr               = 0;
        m_MaterialPSBufferSetPtr           = 0;
        m_BilateralBlurCSBufferSetPtr      = 0;
        m_DeferredRenderContextPtr         = 0;
        m_RectangleShaderVSPtr             = 0;
        m_BilateralBlurShaderCSPtr         = 0;
        m_CopyToGBufferShaderPSPtr         = 0;
        m_DifferentialGBufferShaderPSPtr   = 0;
        m_BackgroundTexturePtr             = 0;
        m_VSPositionTexturePtr             = 0;
        m_VSPositionTempTexturePtr         = 0;
        m_CubemapTexturePtr                = 0;
        m_WebcamTexturePtr                 = 0;
        m_BilateralBlurTextureSetPtr       = 0;
        m_BilateralBlurTempTextureSetPtr   = 0;
        m_CopyToGBufferTextureSetPtr       = 0;
        m_DifferentualGBufferTextureSetPtr = 0;
        m_PSSamplerSetPtr                  = 0;

        // -----------------------------------------------------------------------------
        // Iterate throw render jobs to release managed pointer
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator EndOfRenderJobs;

        EndOfRenderJobs = m_RenderJobs.end();

        for (CRenderJobs::iterator CurrentRenderJob = m_RenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            CurrentRenderJob->m_SurfacePtr = nullptr;
        }

        m_RenderJobs.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupShader()
    {
        m_RectangleShaderVSPtr = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");

        m_BilateralBlurShaderCSPtr = ShaderManager::CompileCS("cs_bilateral_blur.glsl", "main");

        m_CopyToGBufferShaderPSPtr = ShaderManager::CompilePS("fs_copy_to_gbuffer.glsl", "main");

        m_DifferentialGBufferShaderPSPtr = ShaderManager::CompilePS("fs_differential_gbuffer.glsl", "main");

        // -----------------------------------------------------------------------------

        const SInputElementDescriptor InputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };

        m_FullQuadInputLayoutPtr = ShaderManager::CreateInputLayout(InputLayout, 1, m_RectangleShaderVSPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupKernels()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupRenderTargets()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupStates()
    {
        CCameraPtr          CameraPtr              = ViewManager     ::GetMainCamera ();
        CViewPortSetPtr     ViewPortSetPtr         = ViewManager     ::GetViewPortSet();
        CRenderStatePtr     DeferredRenderStatePtr = StateManager    ::GetRenderState(0);
        CTargetSetPtr       DeferredTargetSetPtr   = TargetSetManager::GetDeferredTargetSet();

        // -----------------------------------------------------------------------------

        m_DeferredRenderContextPtr = ContextManager::CreateRenderContext();

        m_DeferredRenderContextPtr->SetCamera(CameraPtr);
        m_DeferredRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_DeferredRenderContextPtr->SetTargetSet(DeferredTargetSetPtr);
        m_DeferredRenderContextPtr->SetRenderState(DeferredRenderStatePtr);

        // -----------------------------------------------------------------------------

        CSamplerPtr LinearFilter = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);

        m_PSSamplerSetPtr = SamplerManager::CreateSamplerSet(LinearFilter, LinearFilter, LinearFilter, LinearFilter);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupTextures()
    {
        Base::Int2 Size = Main::GetActiveWindowSize();

        STextureDescriptor TextureDescriptor;
        
        TextureDescriptor.m_NumberOfPixelsU  = Size[0];
        TextureDescriptor.m_NumberOfPixelsV  = Size[1];
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPURead;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTextureBase::R8G8B8_UBYTE;
        
        m_WebcamTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        m_DifferentualGBufferTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_WebcamTexturePtr));

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = 512;
        TextureDescriptor.m_NumberOfPixelsV  = 512;
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
        TextureDescriptor.m_Format           = CTextureBase::R8G8B8_UBYTE;

        m_CubemapTexturePtr = TextureManager::CreateCubeTexture(TextureDescriptor);

        for (unsigned int IndexOfCubemapLayer = 0; IndexOfCubemapLayer < 6; ++IndexOfCubemapLayer)
        {
            CTexture2DPtr CubemapFaceTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

            TextureManager::CopyToTextureArray2D(m_CubemapTexturePtr, IndexOfCubemapLayer, CubemapFaceTexturePtr, false);
        }

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = 1280;
        TextureDescriptor.m_NumberOfPixelsV  = 720;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = CTextureBase::GPURead;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTextureBase::R8G8B8_UBYTE;
        
        m_BackgroundTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureDescriptor.m_Format           = CTextureBase::R32_FLOAT;

        m_VSPositionTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        m_VSPositionTempTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        // -----------------------------------------------------------------------------

        CTextureBasePtr GBuffer0texturePtr = TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0);
        CTextureBasePtr GBuffer1texturePtr = TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1);

        // -----------------------------------------------------------------------------

        m_BilateralBlurTempTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_VSPositionTexturePtr), static_cast<CTextureBasePtr>(m_VSPositionTempTexturePtr));

        m_BilateralBlurTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_VSPositionTempTexturePtr), static_cast<CTextureBasePtr>(m_VSPositionTexturePtr));

        m_CopyToGBufferTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_BackgroundTexturePtr), static_cast<CTextureBasePtr>(m_VSPositionTexturePtr), GBuffer0texturePtr, GBuffer1texturePtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;

        // -----------------------------------------------------------------------------
        // Vertex buffer
        // -----------------------------------------------------------------------------
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBufferVS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr ModelBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(CMaterial::SMaterialAttributes);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr MaterialBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        m_ViewVSBufferSetPtr     = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS(), ModelBufferPtr);

        m_BaseVSBufferSetPtr     = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS());

        m_BasePSBufferSetPtr     = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS());

        m_MaterialPSBufferSetPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), MaterialBuffer);

        // -----------------------------------------------------------------------------
        // Bilateral blur
        // -----------------------------------------------------------------------------
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SBilateralBlurConstantBufferCS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr BilateralBlurBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        m_BilateralBlurCSBufferSetPtr = BufferManager::CreateBufferSet(BilateralBlurBufferPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupResources()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupModels()
    {
        m_QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupEnd()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnReload()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnNewMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnUnloadMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {
        BASE_UNUSED(_Width);
        BASE_UNUSED(_Height);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::Update()
    {
        if (!MR::ControlManager::IsActive()) return;

        // -----------------------------------------------------------------------------
        // Set render jobs depending on camera. At the end we have to iterate throw
        // the map only once. Then we can order the render jobs and we get as less
        // state changes as possible.
        // -----------------------------------------------------------------------------
        BuildRenderJobs();

        // -----------------------------------------------------------------------------
        // Check if camera has video output
        // -----------------------------------------------------------------------------
        MR::CControl* pControl = MR::ControlManager::GetActiveControl();

        if (pControl != nullptr && pControl->GetType() == MR::CControl::Webcam)
        {
            MR::CWebcamControl& rWebcamControl = static_cast<MR::CWebcamControl&>(*pControl);

            if (rWebcamControl.GetConvertedFrame()->GetPixels() != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Upload to texture on graphic card
                // TODO: Target rectangle should be variable
                // TODO: Use image from main camera?
                // -----------------------------------------------------------------------------
                Base::AABB2UInt TargetRect(Base::UInt2(0), Base::UInt2(1280, 720));

                TextureManager::CopyToTexture2D(m_WebcamTexturePtr, TargetRect, TargetRect[1][0], rWebcamControl.GetConvertedFrame()->GetPixels());
            }
        }
        else if (pControl != nullptr && pControl->GetType() == MR::CControl::Kinect)
        {
            MR::CKinectControl& rKinectControl = static_cast<MR::CKinectControl&>(*pControl);

            Base::AABB2UInt TargetRect(Base::UInt2(0), Base::UInt2(1280, 720));

            TextureManager::CopyToTexture2D(m_BackgroundTexturePtr, TargetRect, TargetRect[1][0], rKinectControl.GetConvertedFrame()->GetPixels());

            TextureManager::CopyToTexture2D(m_VSPositionTexturePtr, TargetRect, TargetRect[1][0], rKinectControl.GetConvertedDepthFrame());

            TextureManager::CopyToTexture2D(m_WebcamTexturePtr, TargetRect, TargetRect[1][0], rKinectControl.GetConvertedFrame()->GetPixels());
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::Render()
    {
        if (!MR::ControlManager::IsActive()) return;

        Performance::BeginEvent("AR");

        const unsigned int pOffset[] = { 0, 0 };

        MR::CControl* pControl = MR::ControlManager::GetActiveControl();

        // -----------------------------------------------------------------------------

        if (m_RenderJobs.size() > 0)
        {
            Performance::BeginEvent("Local Scene to GBuffer");

            // -----------------------------------------------------------------------------
            // Prepare renderer
            // -----------------------------------------------------------------------------
            ContextManager::SetRenderContext(m_DeferredRenderContextPtr);

            // -----------------------------------------------------------------------------
            // First pass: iterate throw render jobs and compute all meshes
            // -----------------------------------------------------------------------------
            CRenderJobs::const_iterator EndOfRenderJobs = m_RenderJobs.end();

            for (CRenderJobs::const_iterator CurrentRenderJob = m_RenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
            {
                CSurfacePtr SurfacePtr = CurrentRenderJob->m_SurfacePtr;

                // -----------------------------------------------------------------------------
                // Upload data to buffer
                // -----------------------------------------------------------------------------
                SPerDrawCallConstantBufferVS* pModelBuffer = static_cast<SPerDrawCallConstantBufferVS*>(BufferManager::MapConstantBuffer(m_ViewVSBufferSetPtr->GetBuffer(1)));

                assert(pModelBuffer != nullptr);

                pModelBuffer->m_ModelMatrix = CurrentRenderJob->m_ModelMatrix;

                BufferManager::UnmapConstantBuffer(m_ViewVSBufferSetPtr->GetBuffer(1));

                CMaterial::SMaterialAttributes* pMaterialBuffer = static_cast<CMaterial::SMaterialAttributes*>(BufferManager::MapConstantBuffer(m_MaterialPSBufferSetPtr->GetBuffer(1)));

                Base::CMemory::Copy(pMaterialBuffer, &CurrentRenderJob->m_SurfaceMaterialPtr->GetMaterialAttributes(), sizeof(CMaterial::SMaterialAttributes));

                BufferManager::UnmapConstantBuffer(m_MaterialPSBufferSetPtr->GetBuffer(1));

                // -----------------------------------------------------------------------------
                // Render
                // -----------------------------------------------------------------------------
                ContextManager::SetTopology(STopology::TriangleList);

                ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

                ContextManager::SetShaderPS(m_DifferentialGBufferShaderPSPtr);

                ContextManager::SetConstantBufferSetVS(m_ViewVSBufferSetPtr);

                ContextManager::SetConstantBufferSetPS(m_MaterialPSBufferSetPtr);

                // -----------------------------------------------------------------------------
                // Set items to context manager
                // -----------------------------------------------------------------------------
                ContextManager::SetVertexBufferSet(SurfacePtr->GetVertexBuffer(), pOffset);

                ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

                ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

                ContextManager::SetTextureSetPS(m_DifferentualGBufferTextureSetPtr);

                ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);

                ContextManager::ResetTextureSetPS();

                ContextManager::ResetInputLayout();

                ContextManager::ResetIndexBuffer();

                ContextManager::ResetVertexBufferSet();

                ContextManager::ResetConstantBufferSetVS();

                ContextManager::ResetConstantBufferSetPS();
            }

            ContextManager::ResetSamplerSetPS();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetRenderContext();

            ContextManager::ResetTopology();

            Performance::EndEvent();
        }

        // -----------------------------------------------------------------------------

        if (pControl->GetType() == MR::CControl::Kinect)
        {
//             SBilateralBlurConstantBufferCS* pBilateralBlurConstantBuffer;
//             
//             pBilateralBlurConstantBuffer = static_cast<SBilateralBlurConstantBufferCS*>(BufferManager::MapConstantBuffer(m_BilateralBlurCSBufferSetPtr->GetBuffer(0)));
//             
//             pBilateralBlurConstantBuffer->m_Direction = Base::UInt4(1, 0, 0, 0);
//             
//             BufferManager::UnmapConstantBuffer(m_BilateralBlurCSBufferSetPtr->GetBuffer(0));
//             
//             ContextManager::SetShaderCS(m_BilateralBlurShaderCSPtr);
//             
//             ContextManager::SetConstantBufferSetCS(m_BilateralBlurCSBufferSetPtr);
//             
//             ContextManager::SetTextureSetCS(m_BilateralBlurTempTextureSetPtr);
//             
//             ContextManager::Dispatch(1280 / 16, 720 / 16, 1);
//             
//             ContextManager::ResetShaderCS();
//             
//             ContextManager::ResetTextureSetCS();
//             
//             ContextManager::ResetConstantBufferSetCS();
//             
//             // -----------------------------------------------------------------------------
//             
//             pBilateralBlurConstantBuffer = static_cast<SBilateralBlurConstantBufferCS*>(BufferManager::MapConstantBuffer(m_BilateralBlurCSBufferSetPtr->GetBuffer(0)));
//             
//             pBilateralBlurConstantBuffer->m_Direction = Base::UInt4(0, 1, 0, 0);
//             
//             BufferManager::UnmapConstantBuffer(m_BilateralBlurCSBufferSetPtr->GetBuffer(0));
//             
//             ContextManager::SetShaderCS(m_BilateralBlurShaderCSPtr);
//             
//             ContextManager::SetConstantBufferSetCS(m_BilateralBlurCSBufferSetPtr);
//             
//             ContextManager::SetTextureSetCS(m_BilateralBlurTextureSetPtr);
//             
//             ContextManager::Dispatch(1280 / 16, 720 / 16, 1);
//             
//             ContextManager::ResetShaderCS();
//             
//             ContextManager::ResetTextureSetCS();
//             
//             ContextManager::ResetConstantBufferSetCS();

            // -----------------------------------------------------------------------------

            Performance::BeginEvent("Copy Kinect to G-Buffer");

            ContextManager::SetRenderContext(m_DeferredRenderContextPtr);

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

            ContextManager::SetShaderPS(m_CopyToGBufferShaderPSPtr);

            ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);

            ContextManager::SetConstantBufferSetPS(m_BasePSBufferSetPtr);

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);

            ContextManager::SetTextureSetPS(m_CopyToGBufferTextureSetPtr);

            ContextManager::SetSamplerSetPS(m_PSSamplerSetPtr);

            ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

            ContextManager::ResetSamplerSetPS();

            ContextManager::ResetTextureSetPS();

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBufferSet();

            ContextManager::ResetConstantBufferSetVS();

            ContextManager::ResetConstantBufferSetPS();

            ContextManager::ResetSamplerSetPS();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetRenderContext();

            ContextManager::ResetTopology();

            Performance::EndEvent();
        }

        // -----------------------------------------------------------------------------

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_RenderJobs.clear();

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // TODO: Visibility culling!, Sort objects by deferred and forward rendering
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Actor);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; )
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetType() != Dt::SActorType::AR)
            {
                CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);

                continue;
            }

            CARActorFacet* pGraphicARActorFacet = static_cast<CARActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

            CMeshPtr ModelPtr = pGraphicARActorFacet->GetMesh();

            // -----------------------------------------------------------------------------
            // Set every surface of this entity into a new render job
            // -----------------------------------------------------------------------------
            unsigned int NumberOfSurfaces = ModelPtr->GetLOD(0)->GetNumberOfSurfaces();

            for (unsigned int IndexOfSurface = 0; IndexOfSurface < NumberOfSurfaces; ++ IndexOfSurface)
            {
                CSurfacePtr SurfacePtr = ModelPtr->GetLOD(0)->GetSurface(IndexOfSurface);

                if (SurfacePtr == nullptr)
                {
                    break;
                }

                CMaterialPtr MaterialPtr;

                if (pGraphicARActorFacet->GetMaterial(IndexOfSurface) != 0)
                {
                    MaterialPtr = pGraphicARActorFacet->GetMaterial(IndexOfSurface);
                }
                else
                {
                    MaterialPtr = SurfacePtr->GetMaterial();
                }

                 // -----------------------------------------------------------------------------
                // Set informations to render job
                // -----------------------------------------------------------------------------
                SRenderJob NewRenderJob;

                NewRenderJob.m_SurfacePtr         = SurfacePtr;
                NewRenderJob.m_SurfaceMaterialPtr = MaterialPtr;
                NewRenderJob.m_ModelMatrix        = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();

                m_RenderJobs.push_back(NewRenderJob);
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
namespace ARRenderer
{
    void OnStart()
    {
        CGfxARRenderer::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxARRenderer::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void OnSetupShader()
    {
        CGfxARRenderer::GetInstance().OnSetupShader();
    }

    // -----------------------------------------------------------------------------

    void OnSetupKernels()
    {
        CGfxARRenderer::GetInstance().OnSetupKernels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupRenderTargets()
    {
        CGfxARRenderer::GetInstance().OnSetupRenderTargets();
    }

    // -----------------------------------------------------------------------------

    void OnSetupStates()
    {
        CGfxARRenderer::GetInstance().OnSetupStates();
    }

    // -----------------------------------------------------------------------------

    void OnSetupTextures()
    {
        CGfxARRenderer::GetInstance().OnSetupTextures();
    }

    // -----------------------------------------------------------------------------

    void OnSetupBuffers()
    {
        CGfxARRenderer::GetInstance().OnSetupBuffers();
    }

    // -----------------------------------------------------------------------------

    void OnSetupResources()
    {
        CGfxARRenderer::GetInstance().OnSetupResources();
    }

    // -----------------------------------------------------------------------------

    void OnSetupModels()
    {
        CGfxARRenderer::GetInstance().OnSetupModels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupEnd()
    {
        CGfxARRenderer::GetInstance().OnSetupEnd();
    }

    // -----------------------------------------------------------------------------

    void OnReload()
    {
        CGfxARRenderer::GetInstance().OnReload();
    }

    // -----------------------------------------------------------------------------

    void OnNewMap()
    {
        CGfxARRenderer::GetInstance().OnNewMap();
    }

    // -----------------------------------------------------------------------------

    void OnUnloadMap()
    {
        CGfxARRenderer::GetInstance().OnUnloadMap();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxARRenderer::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void Render()
    {
        CGfxARRenderer::GetInstance().Render();
    }
} // namespace ARRenderer
} // namespace Gfx
