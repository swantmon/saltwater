
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "data/data_actor_type.h"
#include "data/data_entity.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_sun_facet.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_ar_renderer.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_actor_facet.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sun_facet.h"
#include "graphic/gfx_sun_manager.h"
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

        void OnResize(unsigned int _Width, unsigned int _Height);

        void Update();
        void Render();

    private:

        struct SSunLightProperties
        {
            Base::Float4x4 m_LightViewProjection;
            Base::Float4   m_LightDirection;
            Base::Float4   m_LightColor;
            float          m_SunAngularRadius;
            unsigned int   m_ExposureHistoryIndex;
        };

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
            CSurfacePtr    m_SurfacePtr;
            CMaterialPtr   m_SurfaceMaterialPtr;
            Base::Float4x4 m_ModelMatrix;
        };

    private:

        typedef std::vector<SRenderJob> CRenderJobs;

    private:

        CMeshPtr m_QuadModelPtr;

        CInputLayoutPtr m_P2InputLayoutPtr;

        CBufferSetPtr m_ViewVSBufferSetPtr;
        CBufferSetPtr m_BaseVSBufferSetPtr;
        CBufferSetPtr m_BasePSBufferSetPtr;
        CBufferSetPtr m_MaterialPSBufferSetPtr;
        CBufferSetPtr m_SunLightPSBufferPtr;

        CTargetSetPtr m_DifferentialTargetSetPtr;
        CTargetSetPtr m_DifferentialLightTargetSetPtr;

        CRenderContextPtr m_DifferentialRenderContextPtr;
        CRenderContextPtr m_DifferentialLightRenderContextPtr;
        CRenderContextPtr m_ApplyDeferredRenderContextPtr;

        CShaderPtr m_RectangleShaderVSPtr;
        CShaderPtr m_ApplyDifferentialGBuffer;
        CShaderPtr m_DifferentialGBufferShaderPSPtr;
        CShaderPtr m_SunLightShaderPSPtr;

        CTexture2DPtr m_BackgroundTexturePtr;
        CTexture2DPtr m_VSPositionTexturePtr;
        CTexture2DPtr m_VSPositionTempTexturePtr;
        CTexture2DPtr m_CubemapTexturePtr;
        CTexture2DPtr m_WebcamTexturePtr;

        CTextureBasePtr m_DifferentialLightTexturePtr;

        CTextureSetPtr m_ApplyDifferentialTextureSetPtr;
        CTextureSetPtr m_DifferentualGBufferTextureSetPtr;
        CTextureSetPtr m_GBufferTextureSetPtr;

        CSamplerSetPtr m_PSPointSamplerSetPtr;
        CSamplerSetPtr m_PSLinearSamplerSetPtr;

        CRenderJobs m_RenderJobs;

    private:

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxARRenderer::CGfxARRenderer()
        : m_QuadModelPtr                     ()
        , m_P2InputLayoutPtr                 ()
        , m_ViewVSBufferSetPtr               ()
        , m_BaseVSBufferSetPtr               ()
        , m_BasePSBufferSetPtr               ()
        , m_MaterialPSBufferSetPtr           ()
        , m_SunLightPSBufferPtr              ()
        , m_DifferentialTargetSetPtr         ()
        , m_DifferentialLightTargetSetPtr    ()
        , m_DifferentialRenderContextPtr     ()
        , m_DifferentialLightRenderContextPtr()
        , m_ApplyDeferredRenderContextPtr    ()
        , m_RectangleShaderVSPtr             ()
        , m_ApplyDifferentialGBuffer         ()
        , m_DifferentialGBufferShaderPSPtr   ()
        , m_SunLightShaderPSPtr              ()
        , m_BackgroundTexturePtr             ()
        , m_VSPositionTexturePtr             ()
        , m_VSPositionTempTexturePtr         ()
        , m_CubemapTexturePtr                ()
        , m_WebcamTexturePtr                 ()
        , m_DifferentialLightTexturePtr      ()
        , m_ApplyDifferentialTextureSetPtr   ()
        , m_DifferentualGBufferTextureSetPtr ()
        , m_GBufferTextureSetPtr             ()
        , m_PSPointSamplerSetPtr             ()
        , m_PSLinearSamplerSetPtr            ()
        , m_RenderJobs                       ()
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
        m_QuadModelPtr                      = 0;
        m_P2InputLayoutPtr                  = 0;
        m_ViewVSBufferSetPtr                = 0;
        m_BaseVSBufferSetPtr                = 0;
        m_BasePSBufferSetPtr                = 0;
        m_MaterialPSBufferSetPtr            = 0;
        m_SunLightPSBufferPtr               = 0;
        m_DifferentialTargetSetPtr          = 0;
        m_DifferentialLightTargetSetPtr     = 0;
        m_DifferentialRenderContextPtr      = 0;
        m_DifferentialLightRenderContextPtr = 0;
        m_ApplyDeferredRenderContextPtr     = 0;
        m_RectangleShaderVSPtr              = 0;
        m_ApplyDifferentialGBuffer          = 0;
        m_DifferentialGBufferShaderPSPtr    = 0;
        m_SunLightShaderPSPtr               = 0;
        m_BackgroundTexturePtr              = 0;
        m_VSPositionTexturePtr              = 0;
        m_VSPositionTempTexturePtr          = 0;
        m_CubemapTexturePtr                 = 0;
        m_WebcamTexturePtr                  = 0;
        m_DifferentialLightTexturePtr       = 0;
        m_ApplyDifferentialTextureSetPtr    = 0;
        m_DifferentualGBufferTextureSetPtr  = 0;
        m_GBufferTextureSetPtr              = 0;
        m_PSPointSamplerSetPtr              = 0;
        m_PSLinearSamplerSetPtr             = 0;

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

        m_ApplyDifferentialGBuffer = ShaderManager::CompilePS("fs_apply_differential_gbuffer.glsl", "main");

        m_DifferentialGBufferShaderPSPtr = ShaderManager::CompilePS("fs_differential_gbuffer.glsl", "main");

        m_SunLightShaderPSPtr = ShaderManager::CompilePS("fs_light_sunlight.glsl", "main");

        // -----------------------------------------------------------------------------

        const SInputElementDescriptor InputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };

        m_P2InputLayoutPtr = ShaderManager::CreateInputLayout(InputLayout, 1, m_RectangleShaderVSPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupKernels()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupRenderTargets()
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        Base::Int2 Size = Main::GetActiveWindowSize();
        
        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::Unknown;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr AlbedoPtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // RGB Albedo
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr GBuffer1Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 1
        
        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr GBuffer2Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 2
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr GBuffer3Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 3
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R16G16B16A16_FLOAT;
        
        m_DifferentialLightTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Light Accumulation (HDR)
        
        // -----------------------------------------------------------------------------
        // Create deferred target set
        // -----------------------------------------------------------------------------
        CTextureBasePtr DeferredRenderbuffer[4];
        
        DeferredRenderbuffer[0] = GBuffer1Ptr;
        DeferredRenderbuffer[1] = GBuffer2Ptr;
        DeferredRenderbuffer[2] = GBuffer3Ptr;
        DeferredRenderbuffer[3] = TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget();
        
        m_DifferentialTargetSetPtr = TargetSetManager::CreateTargetSet(DeferredRenderbuffer, 4);
        
        // -----------------------------------------------------------------------------
        // Create light accumulation target set
        // -----------------------------------------------------------------------------
        m_DifferentialLightTargetSetPtr = TargetSetManager::CreateTargetSet(m_DifferentialLightTexturePtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupStates()
    {
        CCameraPtr      CameraPtr                = ViewManager     ::GetMainCamera ();
        CViewPortSetPtr ViewPortSetPtr           = ViewManager     ::GetViewPortSet();
        CRenderStatePtr DefaultRenderStatePtr    = StateManager    ::GetRenderState(0);
        CRenderStatePtr EqualDepthRenderStatePtr = StateManager    ::GetRenderState(CRenderState::EqualDepth);
        CTargetSetPtr   DeferredTargetSetPtr     = TargetSetManager::GetLightAccumulationTargetSet();

        // -----------------------------------------------------------------------------

        m_ApplyDeferredRenderContextPtr = ContextManager::CreateRenderContext();

        m_ApplyDeferredRenderContextPtr->SetCamera(CameraPtr);
        m_ApplyDeferredRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_ApplyDeferredRenderContextPtr->SetTargetSet(DeferredTargetSetPtr);
        m_ApplyDeferredRenderContextPtr->SetRenderState(DefaultRenderStatePtr);

        // -----------------------------------------------------------------------------

        m_DifferentialRenderContextPtr = ContextManager::CreateRenderContext();

        m_DifferentialRenderContextPtr->SetCamera(CameraPtr);
        m_DifferentialRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_DifferentialRenderContextPtr->SetTargetSet(m_DifferentialTargetSetPtr);
        m_DifferentialRenderContextPtr->SetRenderState(EqualDepthRenderStatePtr);

        // -----------------------------------------------------------------------------

        m_DifferentialLightRenderContextPtr = ContextManager::CreateRenderContext();

        m_DifferentialLightRenderContextPtr->SetCamera(CameraPtr);
        m_DifferentialLightRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        m_DifferentialLightRenderContextPtr->SetTargetSet(m_DifferentialLightTargetSetPtr);
        m_DifferentialLightRenderContextPtr->SetRenderState(EqualDepthRenderStatePtr);

        // -----------------------------------------------------------------------------

        CSamplerPtr LinearFilter = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        CSamplerPtr PointFilter  = SamplerManager::GetSampler(CSampler::MinMagMipPointClamp);

        m_PSPointSamplerSetPtr = SamplerManager::CreateSamplerSet(PointFilter, PointFilter, PointFilter, PointFilter);

        m_PSLinearSamplerSetPtr = SamplerManager::CreateSamplerSet(LinearFilter, LinearFilter, LinearFilter, LinearFilter);
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::OnSetupTextures()
    {
        CTextureBasePtr GBuffer0TexturePtr = m_DifferentialTargetSetPtr->GetRenderTarget(0);
        CTextureBasePtr GBuffer1TexturePtr = m_DifferentialTargetSetPtr->GetRenderTarget(1);
        CTextureBasePtr GBuffer2TexturePtr = m_DifferentialTargetSetPtr->GetRenderTarget(2);
        CTextureBasePtr DepthTexturePtr    = m_DifferentialTargetSetPtr->GetDepthStencilTarget();

        m_ApplyDifferentialTextureSetPtr = TextureManager::CreateTextureSet(m_DifferentialLightTexturePtr, TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0));

        m_GBufferTextureSetPtr = TextureManager::CreateTextureSet(GBuffer0TexturePtr, GBuffer1TexturePtr, GBuffer2TexturePtr, DepthTexturePtr);
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

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(CMaterial::SMaterialAttributes);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr MaterialBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSunLightProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SunLightBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        CBufferPtr HistogramExposureHistoryBufferPtr = HistogramRenderer::GetExposureHistoryBuffer();

        // -----------------------------------------------------------------------------

        m_ViewVSBufferSetPtr      = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS(), ModelBufferPtr);

        m_BaseVSBufferSetPtr      = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS());

        m_BasePSBufferSetPtr      = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS());

        m_MaterialPSBufferSetPtr  = BufferManager::CreateBufferSet(MaterialBuffer);

        m_SunLightPSBufferPtr     = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), SunLightBuffer, HistogramExposureHistoryBufferPtr);
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
        // -----------------------------------------------------------------------------
        // Set render jobs depending on camera. At the end we have to iterate throw
        // the map only once. Then we can order the render jobs and we get as less
        // state changes as possible.
        // -----------------------------------------------------------------------------
        BuildRenderJobs();
    }

    // -----------------------------------------------------------------------------

    void CGfxARRenderer::Render()
    {
        if (m_RenderJobs.size() == 0) return;

        Performance::BeginEvent("AR");

        const unsigned int pOffset[] = { 0, 0 };

        Performance::BeginEvent("Clear");

        TargetSetManager::ClearTargetSet(m_DifferentialTargetSetPtr, -1.0f);

        TargetSetManager::ClearTargetSet(m_DifferentialLightTargetSetPtr, -1.0f);

        Performance::EndEvent(); // Clear

        Performance::BeginEvent("Creation");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_DifferentialRenderContextPtr);

        // -----------------------------------------------------------------------------
        // First pass: iterate throw render jobs and compute all meshes
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator EndOfRenderJobs = m_RenderJobs.end();

        for (CRenderJobs::const_iterator CurrentRenderJob = m_RenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            CSurfacePtr  SurfacePtr  = CurrentRenderJob->m_SurfacePtr;
            CMaterialPtr MaterialPtr = CurrentRenderJob->m_SurfaceMaterialPtr;

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS* pModelBuffer = static_cast<SPerDrawCallConstantBufferVS*>(BufferManager::MapConstantBuffer(m_ViewVSBufferSetPtr->GetBuffer(1)));

            assert(pModelBuffer != nullptr);

            pModelBuffer->m_ModelMatrix = CurrentRenderJob->m_ModelMatrix;

            BufferManager::UnmapConstantBuffer(m_ViewVSBufferSetPtr->GetBuffer(1));

            CMaterial::SMaterialAttributes* pMaterialBuffer = static_cast<CMaterial::SMaterialAttributes*>(BufferManager::MapConstantBuffer(m_MaterialPSBufferSetPtr->GetBuffer(0)));

            Base::CMemory::Copy(pMaterialBuffer, &CurrentRenderJob->m_SurfaceMaterialPtr->GetMaterialAttributes(), sizeof(CMaterial::SMaterialAttributes));

            BufferManager::UnmapConstantBuffer(m_MaterialPSBufferSetPtr->GetBuffer(0));

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            ContextManager::SetShaderPS(MaterialPtr->GetShaderPS());

            ContextManager::SetTextureSetPS(MaterialPtr->GetTextureSetPS());

            ContextManager::SetSamplerSetPS(m_PSLinearSamplerSetPtr);

            ContextManager::SetConstantBufferSetVS(m_ViewVSBufferSetPtr);

            ContextManager::SetConstantBufferSetPS(m_MaterialPSBufferSetPtr);

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBufferSet(SurfacePtr->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

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

        Performance::EndEvent(); // Creation

        // -----------------------------------------------------------------------------

        Performance::BeginEvent("Lighting");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_DifferentialLightRenderContextPtr);

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
            if (rCurrentEntity.GetType() == Dt::SLightType::Sun)
            {
                ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

                ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

                ContextManager::SetInputLayout(m_P2InputLayoutPtr);

                ContextManager::SetTopology(STopology::TriangleList);

                ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

                ContextManager::SetShaderPS(m_SunLightShaderPSPtr);

                ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);

                ContextManager::SetSamplerSetPS(m_PSPointSamplerSetPtr);

                Dt::CSunLightFacet* pDataSunFacet    = static_cast<Dt::CSunLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));;
                Gfx::CSunFacet*     pGraphicSunFacet = static_cast<Gfx::CSunFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

                // -----------------------------------------------------------------------------
                // Upload buffer data
                // -----------------------------------------------------------------------------
                SSunLightProperties* pLightBuffer = static_cast<SSunLightProperties*>(BufferManager::MapConstantBuffer(m_SunLightPSBufferPtr->GetBuffer(1)));

                assert(pLightBuffer != nullptr);

                pLightBuffer->m_LightViewProjection  = pGraphicSunFacet->GetCamera()->GetViewProjectionMatrix();
                pLightBuffer->m_LightDirection       = Base::Float4(pDataSunFacet->GetDirection(), 0.0f).Normalize();
                pLightBuffer->m_LightColor           = Base::Float4(pDataSunFacet->GetLightness(), 1.0f);
                pLightBuffer->m_SunAngularRadius     = 0.27f * Base::SConstants<float>::s_Pi / 180.0f;
                pLightBuffer->m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();

                BufferManager::UnmapConstantBuffer(m_SunLightPSBufferPtr->GetBuffer(1));

                // -----------------------------------------------------------------------------

                ContextManager::SetConstantBufferSetPS(m_SunLightPSBufferPtr);

                ContextManager::SetTextureSetPS(m_GBufferTextureSetPtr);

                ContextManager::SetTextureSetPS(pGraphicSunFacet->GetTextureSMSet());

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
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }

        Performance::EndEvent(); // Lighting

        // -----------------------------------------------------------------------------

        Performance::BeginEvent("Apply");

        ContextManager::SetRenderContext(m_ApplyDeferredRenderContextPtr);

        // -----------------------------------------------------------------------------
        // Render
        // -----------------------------------------------------------------------------
        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_RectangleShaderVSPtr);

        ContextManager::SetShaderPS(m_ApplyDifferentialGBuffer);

        ContextManager::SetConstantBufferSetVS(m_BaseVSBufferSetPtr);

        ContextManager::SetConstantBufferSetPS(m_BasePSBufferSetPtr);

        // -----------------------------------------------------------------------------
        // Set items to context manager
        // -----------------------------------------------------------------------------
        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_P2InputLayoutPtr);

        ContextManager::SetTextureSetPS(m_ApplyDifferentialTextureSetPtr);

        ContextManager::SetSamplerSetPS(m_PSPointSamplerSetPtr);

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

        Performance::EndEvent(); // Apply

        Performance::EndEvent(); // AR
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
            if (rCurrentEntity.GetLayer() != Dt::SEntityLayer::AR)
            {
                CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);

                continue;
            }

            CMeshActorFacet* pGraphicARActorFacet = static_cast<CMeshActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

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
