
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "data/data_component_facet.h"
#include "data/data_component.h"
#include "data/data_entity.h"
#include "data/data_map.h"
#include "data/data_material_component.h"
#include "data/data_mesh_component.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_ar_renderer.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_material.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

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

        void Update();
        void Render();
        void RenderHitProxy();

    private:

        struct SPerDrawCallConstantBufferVS
        {
            glm::mat4 m_ModelMatrix;
        };

        struct SBilateralBlurConstantBufferCS
        {
            glm::uvec4 m_Direction;
        };

        struct SHitProxyProperties
        {
            unsigned int m_ID;
        };

        struct SRenderJob
        {
            Base::ID         m_EntityID;
            CSurfacePtr      m_SurfacePtr;
            const CMaterial* m_SurfaceMaterialPtr;
            glm::mat4        m_ModelMatrix;
        };

    private:

        typedef std::vector<SRenderJob> CRenderJobs;

    private:

        CMeshPtr m_QuadModelPtr;

        CInputLayoutPtr m_FullQuadInputLayoutPtr;

        CBufferPtr m_ModelBufferPtr;
        CBufferPtr m_MaterialPSBufferPtr;
        CBufferPtr m_BilateralBlurCSBufferPtr;
        CBufferPtr m_HitProxyPassPSBufferPtr;

        CRenderContextPtr m_DeferredRenderContextPtr;
        CRenderContextPtr m_HitProxyContextPtr;

        CShaderPtr m_RectangleShaderVSPtr;
        CShaderPtr m_BilateralBlurShaderCSPtr;
        CShaderPtr m_CopyToGBufferShaderPSPtr;
        CShaderPtr m_DifferentialGBufferShaderPSPtr;
        CShaderPtr m_HitProxyShaderPtr;

        CTexturePtr m_BackgroundTexturePtr;
        CTexturePtr m_VSPositionTexturePtr;
        CTexturePtr m_VSPositionTempTexturePtr;
        CTexturePtr m_WebcamTexturePtr;

        CRenderJobs m_RenderJobs;

    private:

        void BuildRenderJobs();     
    };
} // namespace

namespace
{
    CGfxARRenderer::CGfxARRenderer()
        : m_QuadModelPtr                    ()
        , m_FullQuadInputLayoutPtr          ()
        , m_ModelBufferPtr                  ()
        , m_MaterialPSBufferPtr             ()
        , m_BilateralBlurCSBufferPtr        ()
        , m_HitProxyPassPSBufferPtr         ()
        , m_DeferredRenderContextPtr        ()
        , m_HitProxyContextPtr              ()
        , m_RectangleShaderVSPtr            ()
        , m_BilateralBlurShaderCSPtr        ()
        , m_CopyToGBufferShaderPSPtr        ()
        , m_DifferentialGBufferShaderPSPtr  ()
        , m_HitProxyShaderPtr               ()
        , m_BackgroundTexturePtr            ()
        , m_VSPositionTexturePtr            ()
        , m_VSPositionTempTexturePtr        ()
        , m_WebcamTexturePtr                ()
        , m_RenderJobs                      ()
    {
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
        m_ModelBufferPtr                   = 0;
        m_MaterialPSBufferPtr              = 0;
        m_BilateralBlurCSBufferPtr         = 0;
        m_HitProxyPassPSBufferPtr          = 0;
        m_DeferredRenderContextPtr         = 0;
        m_HitProxyContextPtr               = 0;
        m_RectangleShaderVSPtr             = 0;
        m_BilateralBlurShaderCSPtr         = 0;
        m_CopyToGBufferShaderPSPtr         = 0;
        m_DifferentialGBufferShaderPSPtr   = 0;
        m_HitProxyShaderPtr                = 0;
        m_BackgroundTexturePtr             = 0;
        m_VSPositionTexturePtr             = 0;
        m_VSPositionTempTexturePtr         = 0;
        m_WebcamTexturePtr                 = 0;

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

        m_HitProxyShaderPtr = ShaderManager::CompilePS("fs_hitproxy.glsl", "main");

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
        CCameraPtr      CameraPtr              = ViewManager     ::GetMainCamera ();
        CViewPortSetPtr ViewPortSetPtr         = ViewManager     ::GetViewPortSet();
        CRenderStatePtr DeferredRenderStatePtr = StateManager    ::GetRenderState(0);
        CRenderStatePtr HitProxyRenderStatePtr = StateManager    ::GetRenderState(CRenderState::EqualDepth);
        CTargetSetPtr   DeferredTargetSetPtr   = TargetSetManager::GetDeferredTargetSet();
        CTargetSetPtr   HitProxyTargetSetPtr   = TargetSetManager::GetHitProxyTargetSet();

        // -----------------------------------------------------------------------------

        m_DeferredRenderContextPtr = ContextManager::CreateRenderContext();

        m_DeferredRenderContextPtr->SetCamera(CameraPtr);
        m_DeferredRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_DeferredRenderContextPtr->SetTargetSet(DeferredTargetSetPtr);
        m_DeferredRenderContextPtr->SetRenderState(DeferredRenderStatePtr);

        // -----------------------------------------------------------------------------

        m_HitProxyContextPtr = ContextManager::CreateRenderContext();

        m_HitProxyContextPtr->SetCamera(CameraPtr);
        m_HitProxyContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_HitProxyContextPtr->SetTargetSet(HitProxyTargetSetPtr);
        m_HitProxyContextPtr->SetRenderState(HitProxyRenderStatePtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupTextures()
    {
        STextureDescriptor TextureDescriptor;
        
        TextureDescriptor.m_NumberOfPixelsU  = 1280;
        TextureDescriptor.m_NumberOfPixelsV  = 720;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::Unknown;
        TextureDescriptor.m_Usage            = CTexture::GPURead;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = CTexture::R8G8B8_UBYTE;
        
        m_WebcamTexturePtr     = TextureManager::CreateTexture2D(TextureDescriptor);
        
        m_BackgroundTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_Format = CTexture::R32G32B32_FLOAT;

        m_VSPositionTexturePtr     = TextureManager::CreateTexture2D(TextureDescriptor);

        m_VSPositionTempTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBufferVS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_ModelBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(CMaterial::SMaterialAttributes);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_MaterialPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SHitProxyProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_HitProxyPassPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SBilateralBlurConstantBufferCS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_BilateralBlurCSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
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

    void CGfxARRenderer::Update()
    {
        /*if (!MR::ControlManager::IsActive()) return;*/

        // -----------------------------------------------------------------------------
        // Set render jobs depending on camera. At the end we have to iterate throw
        // the map only once. Then we can order the render jobs and we get as less
        // state changes as possible.
        // -----------------------------------------------------------------------------
        //BuildRenderJobs();

        // -----------------------------------------------------------------------------
        // Check if camera has video output
        // -----------------------------------------------------------------------------
//         MR::CControl* pControl = MR::ControlManager::GetActiveControl();
// 
//         if (pControl != nullptr && pControl->GetType() == MR::CControl::Webcam)
//         {
//             MR::CWebcamControl& rWebcamControl = static_cast<MR::CWebcamControl&>(*pControl);
// 
//             if (rWebcamControl.GetConvertedFrame()->GetPixels() != nullptr)
//             {
//                 // -----------------------------------------------------------------------------
//                 // Upload to texture on graphic card
//                 // TODO: Target rectangle should be variable
//                 // TODO: Use image from main camera?
//                 // -----------------------------------------------------------------------------
//                 Base::AABB2UInt TargetRect(glm::uvec2(0), glm::uvec2(1280, 720));
// 
//                 TextureManager::CopyToTexture2D(m_WebcamTexturePtr, TargetRect, TargetRect[1][0], rWebcamControl.GetConvertedFrame()->GetPixels());
//             }
//         }
//         else if (pControl != nullptr && pControl->GetType() == MR::CControl::Kinect)
//         {
//             // -----------------------------------------------------------------------------
//             // Copy data from kinect
//             // TODO: It is generally outdated because of the branch of ckunert and should
//             // be edited according to his work.
//             // -----------------------------------------------------------------------------
//             MR::CKinectControl& rKinectControl = static_cast<MR::CKinectControl&>(*pControl);
// 
//             Base::AABB2UInt TargetRect(glm::uvec2(0), glm::uvec2(1280, 720));
// 
//             TextureManager::CopyToTexture2D(m_BackgroundTexturePtr, TargetRect, TargetRect[1][0], rKinectControl.GetConvertedFrame()->GetPixels());
// 
//             TextureManager::CopyToTexture2D(m_VSPositionTexturePtr, TargetRect, TargetRect[1][0], rKinectControl.GetConvertedDepthFrame());
// 
//             TextureManager::CopyToTexture2D(m_WebcamTexturePtr, TargetRect, TargetRect[1][0], rKinectControl.GetConvertedFrame()->GetPixels());
//         }
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::Render()
    {
//         if (!MR::ControlManager::IsActive()) return;
// 
//         Performance::BeginEvent("AR");
// 
//         
// 
//         MR::CControl* pControl = MR::ControlManager::GetActiveControl();
// 
//         // -----------------------------------------------------------------------------
// 
//         if (m_RenderJobs.size() > 0)
//         {
//             Performance::BeginEvent("Local Scene to GBuffer");
// 
//             // -----------------------------------------------------------------------------
//             // Prepare renderer
//             // -----------------------------------------------------------------------------
//             ContextManager::SetRenderContext(m_DeferredRenderContextPtr);
// 
//             // -----------------------------------------------------------------------------
//             // First pass: iterate throw render jobs and compute all meshes
//             // -----------------------------------------------------------------------------
//             CRenderJobs::const_iterator EndOfRenderJobs = m_RenderJobs.end();
// 
//             for (CRenderJobs::const_iterator CurrentRenderJob = m_RenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
//             {
//                 CSurfacePtr SurfacePtr = CurrentRenderJob->m_SurfacePtr;
// 
//                 // -----------------------------------------------------------------------------
//                 // Upload data to buffer
//                 // -----------------------------------------------------------------------------
//                 SPerDrawCallConstantBufferVS ModelBuffer;
// 
//                 ModelBuffer.m_ModelMatrix = CurrentRenderJob->m_ModelMatrix;
// 
//                 BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);
// 
//                 BufferManager::UploadBufferData(m_MaterialPSBufferPtr, &CurrentRenderJob->m_SurfaceMaterialPtr->GetMaterialAttributes());
// 
//                 // -----------------------------------------------------------------------------
//                 // Render
//                 // -----------------------------------------------------------------------------
//                 ContextManager::SetTopology(STopology::TriangleList);
// 
//                 ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());
// 
//                 ContextManager::SetShaderPS(m_DifferentialGBufferShaderPSPtr);
// 
//                 ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
//                 ContextManager::SetConstantBuffer(1, m_ModelBufferPtr);
//                 ContextManager::SetConstantBuffer(2, m_MaterialPSBufferPtr);
// 
//                 // -----------------------------------------------------------------------------
//                 // Set items to context manager
//                 // -----------------------------------------------------------------------------
//                 ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());
// 
//                 ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);
// 
//                 ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());
// 
//                 ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
// 
//                 ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(m_WebcamTexturePtr));
// 
//                 ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
// 
//                 ContextManager::ResetTexture(0);
// 
//                 ContextManager::ResetSampler(0);
// 
//                 ContextManager::ResetInputLayout();
// 
//                 ContextManager::ResetIndexBuffer();
// 
//                 ContextManager::ResetVertexBuffer();
// 
//                 ContextManager::ResetConstantBuffer(0);
//                 ContextManager::ResetConstantBuffer(1);
//                 ContextManager::ResetConstantBuffer(2);
//             }
// 
//             ContextManager::ResetSampler(0);
// 
//             ContextManager::ResetShaderVS();
// 
//             ContextManager::ResetShaderPS();
// 
//             ContextManager::ResetRenderContext();
// 
//             ContextManager::ResetTopology();
// 
//             Performance::EndEvent();
//         }
// 
//         // -----------------------------------------------------------------------------
// 
//         if (pControl->GetType() == MR::CControl::Kinect)
//         {
//             // TODO: Remove texture set
//             //m_BilateralBlurTempTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_VSPositionTexturePtr), static_cast<CTextureBasePtr>(m_VSPositionTempTexturePtr));
//             //m_BilateralBlurTextureSetPtr = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(m_VSPositionTempTexturePtr), static_cast<CTextureBasePtr>(m_VSPositionTexturePtr));
// 
// //             SBilateralBlurConstantBufferCS* pBilateralBlurConstantBuffer;
// //             
// //             pBilateralBlurConstantBuffer = static_cast<SBilateralBlurConstantBufferCS*>(BufferManager::MapConstantBuffer(m_BilateralBlurCSBufferSetPtr->GetBuffer(0)));
// //             
// //             pBilateralBlurConstantBuffer->m_Direction = Base::UInt4(1, 0, 0, 0);
// //             
// //             BufferManager::UnmapConstantBuffer(m_BilateralBlurCSBufferSetPtr->GetBuffer(0));
// //             
// //             ContextManager::SetShaderCS(m_BilateralBlurShaderCSPtr);
// //             
// //             ContextManager::SetConstantBufferSetCS(m_BilateralBlurCSBufferSetPtr);
// //             
// //             ContextManager::SetTextureSetCS(m_BilateralBlurTempTextureSetPtr);
// //             
// //             ContextManager::Dispatch(1280 / 16, 720 / 16, 1);
// //             
// //             ContextManager::ResetShaderCS();
// //             
// //             ContextManager::ResetTextureSetCS();
// //             
// //             ContextManager::ResetConstantBufferSetCS();
// //             
// //             // -----------------------------------------------------------------------------
// //             
// //             pBilateralBlurConstantBuffer = static_cast<SBilateralBlurConstantBufferCS*>(BufferManager::MapConstantBuffer(m_BilateralBlurCSBufferSetPtr->GetBuffer(0)));
// //             
// //             pBilateralBlurConstantBuffer->m_Direction = Base::UInt4(0, 1, 0, 0);
// //             
// //             BufferManager::UnmapConstantBuffer(m_BilateralBlurCSBufferSetPtr->GetBuffer(0));
// //             
// //             ContextManager::SetShaderCS(m_BilateralBlurShaderCSPtr);
// //             
// //             ContextManager::SetConstantBufferSetCS(m_BilateralBlurCSBufferSetPtr);
// //             
// //             ContextManager::SetTextureSetCS(m_BilateralBlurTextureSetPtr);
// //             
// //             ContextManager::Dispatch(1280 / 16, 720 / 16, 1);
// //             
// //             ContextManager::ResetShaderCS();
// //             
// //             ContextManager::ResetTextureSetCS();
// //             
// //             ContextManager::ResetConstantBufferSetCS();
// 
//             // -----------------------------------------------------------------------------
// 
//             Performance::BeginEvent("Copy Kinect to G-Buffer");
// 
//             ContextManager::SetRenderContext(m_DeferredRenderContextPtr);
// 
//             // -----------------------------------------------------------------------------
//             // Render
//             // -----------------------------------------------------------------------------
//             ContextManager::SetTopology(STopology::TriangleList);
// 
//             ContextManager::SetShaderVS(m_RectangleShaderVSPtr);
// 
//             ContextManager::SetShaderPS(m_CopyToGBufferShaderPSPtr);
// 
//             ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
// 
//             // -----------------------------------------------------------------------------
//             // Set items to context manager
//             // -----------------------------------------------------------------------------
//             ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
// 
//             ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), 0);
// 
//             ContextManager::SetInputLayout(m_FullQuadInputLayoutPtr);
// 
//             ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
//             ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
//             ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
//             ContextManager::SetSampler(3, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));
// 
//             ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(m_BackgroundTexturePtr));
//             ContextManager::SetTexture(1, static_cast<CTextureBasePtr>(m_VSPositionTexturePtr));
//             ContextManager::SetTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
//             ContextManager::SetTexture(3, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
// 
//             ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);
// 
//             ContextManager::ResetTexture(0);
//             ContextManager::ResetTexture(1);
//             ContextManager::ResetTexture(2);
//             ContextManager::ResetTexture(3);
// 
//             ContextManager::ResetSampler(0);
//             ContextManager::ResetSampler(1);
//             ContextManager::ResetSampler(2);
//             ContextManager::ResetSampler(3);
// 
//             ContextManager::ResetInputLayout();
// 
//             ContextManager::ResetIndexBuffer();
// 
//             ContextManager::ResetVertexBuffer();
// 
//             ContextManager::ResetConstantBuffer(0);
// 
//             ContextManager::ResetShaderVS();
// 
//             ContextManager::ResetShaderPS();
// 
//             ContextManager::ResetRenderContext();
// 
//             ContextManager::ResetTopology();
// 
//             Performance::EndEvent();
//         }
// 
//         // -----------------------------------------------------------------------------
// 
//         Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::RenderHitProxy()
    {
        if (m_RenderJobs.size() == 0) return;

        Performance::BeginEvent("AR Hit Proxy");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        

        ContextManager::SetRenderContext(m_HitProxyContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        // -----------------------------------------------------------------------------
        // First pass: iterate throw render jobs and compute all meshes
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator EndOfRenderJobs = m_RenderJobs.end();

        for (CRenderJobs::const_iterator CurrentRenderJob = m_RenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            CSurfacePtr  SurfacePtr = CurrentRenderJob->m_SurfacePtr;

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = CurrentRenderJob->m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            SHitProxyProperties HitProxyProperties;

            HitProxyProperties.m_ID = static_cast<unsigned int>(CurrentRenderJob->m_EntityID);

            BufferManager::UploadBufferData(m_HitProxyPassPSBufferPtr, &HitProxyProperties);

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            ContextManager::SetShaderPS(m_HitProxyShaderPtr);

            ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
            ContextManager::SetConstantBuffer(1, m_ModelBufferPtr);
            ContextManager::SetConstantBuffer(2, m_HitProxyPassPSBufferPtr);

            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBuffer();

            ContextManager::ResetConstantBuffer(0);
            ContextManager::ResetConstantBuffer(1);
            ContextManager::ResetConstantBuffer(2);

            ContextManager::ResetShaderPS();

            ContextManager::ResetShaderVS();
        }

        ContextManager::ResetTopology();

        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_RenderJobs.clear();

        auto DataMeshComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CMeshComponent>();

        for (auto Component : DataMeshComponents)
        {
            Dt::CMeshComponent* pDtComponent = static_cast<Dt::CMeshComponent*>(Component);

            if (pDtComponent->IsActiveAndUsable() == false) continue;

            const Dt::CEntity& rCurrentEntity = *pDtComponent->GetHostEntity();

            // -----------------------------------------------------------------------------
            // Get graphic facet
            // -----------------------------------------------------------------------------
            if (rCurrentEntity.GetLayer() == Dt::SEntityLayer::AR)
            {
                Gfx::CMesh* pGfxComponent = static_cast<Gfx::CMesh*>(pDtComponent->GetFacet(Dt::CMeshComponent::Graphic));

                // -----------------------------------------------------------------------------
                // Surface
                // -----------------------------------------------------------------------------
                CSurfacePtr SurfacePtr = pGfxComponent->GetLOD(0)->GetSurface();

                if (SurfacePtr == nullptr)
                {
                    break;
                }

                const CMaterial* pMaterial = SurfacePtr->GetMaterial();
                
                if (pDtComponent->GetHostEntity()->GetComponentFacet()->HasComponent<Dt::CMaterialComponent>())
                {
                    auto pDtMaterialComponent = pDtComponent->GetHostEntity()->GetComponentFacet()->GetComponent<Dt::CMaterialComponent>();

                    pMaterial = static_cast<const Gfx::CMaterial*>(pDtMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));
                }

                // -----------------------------------------------------------------------------
                // Set informations to render job
                // -----------------------------------------------------------------------------
                SRenderJob NewRenderJob;

                NewRenderJob.m_EntityID           = rCurrentEntity.GetID();
                NewRenderJob.m_SurfacePtr         = SurfacePtr;
                NewRenderJob.m_SurfaceMaterialPtr = pMaterial;
                NewRenderJob.m_ModelMatrix        = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();

                m_RenderJobs.push_back(NewRenderJob);
            }
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

    // -----------------------------------------------------------------------------

    void RenderHitProxy()
    {
        CGfxARRenderer::GetInstance().RenderHitProxy();
    }
} // namespace ARRenderer
} // namespace Gfx
