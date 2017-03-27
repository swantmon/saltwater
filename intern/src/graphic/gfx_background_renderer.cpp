
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_vector4.h"

#include "camera/cam_control_manager.h"

#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_camera_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"
#include "data/data_sky_facet.h"

#include "graphic/gfx_background_renderer.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_camera_actor_facet.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_sky_facet.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sun_facet.h"
#include "graphic/gfx_sun_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include <string>

using namespace Gfx;

namespace
{
    class CGfxBackgroundRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxBackgroundRenderer)
        
    public:

        CGfxBackgroundRenderer();
        ~CGfxBackgroundRenderer();
        
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

    private:

        typedef std::vector<SCameraRenderJob> CCameraRenderJobs;
        
    private:

        SRenderContext m_BackgroundFromSkybox;
        SRenderContext m_BackgroundFromTexture;

        CCameraRenderJobs m_CameraRenderJobs;
        
    private:

        void RenderBackgroundFromSkybox();

        void RenderBackgroundFromTexture();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxBackgroundRenderer::CGfxBackgroundRenderer()
        : m_BackgroundFromSkybox       ()
        , m_BackgroundFromTexture      ()
        , m_CameraRenderJobs           ()
    {
        // -----------------------------------------------------------------------------
        // Register for resizing events
        // -----------------------------------------------------------------------------
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxBackgroundRenderer::OnResize));
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxBackgroundRenderer::~CGfxBackgroundRenderer()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnExit()
    {
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

        m_CameraRenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupShader()
    {
        CShaderPtr SkytextureVSPtr = ShaderManager::CompileVS("vs_screen_p_quad.glsl", "main");;

        CShaderPtr SkytexturePSPtr = ShaderManager::CompilePS("fs_atmosphere_texture.glsl", "main");

        CShaderPtr SkyboxVSPtr  = ShaderManager::CompileVS("vs_cubemap.glsl"           , "main");
        
        CShaderPtr SkyboxPSPtr  = ShaderManager::CompilePS("fs_atmosphere_cubemap.glsl", "main");

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
    
    void CGfxBackgroundRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupRenderTargets()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupStates()
    {
        CCameraPtr          CameraPtr          = ViewManager     ::GetMainCamera();
        CCameraPtr          QuadCameraPtr      = ViewManager     ::GetFullQuadCamera();
        CViewPortSetPtr     ViewPortSetPtr     = ViewManager     ::GetViewPortSet();
        CRenderStatePtr     NoDepthStatePtr    = StateManager    ::GetRenderState(CRenderState::NoDepth | CRenderState::NoCull);
        CTargetSetPtr       TargetSetPtr       = TargetSetManager::GetLightAccumulationTargetSet();
                
        CRenderContextPtr SkyRenderContextPtr = ContextManager::CreateRenderContext();
        
        SkyRenderContextPtr->SetCamera(CameraPtr);
        SkyRenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        SkyRenderContextPtr->SetTargetSet(TargetSetPtr);
        SkyRenderContextPtr->SetRenderState(NoDepthStatePtr);

        // -----------------------------------------------------------------------------
        
        m_BackgroundFromSkybox.m_RenderContextPtr = SkyRenderContextPtr;

        m_BackgroundFromTexture.m_RenderContextPtr = SkyRenderContextPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupTextures()
    {
        CTextureSetPtr DepthTextureSetPtr = TextureManager::CreateTextureSet(TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());

        m_BackgroundFromSkybox.m_TextureSetPtr = DepthTextureSetPtr;

        m_BackgroundFromTexture.m_TextureSetPtr = DepthTextureSetPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupBuffers()
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
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSkyboxBufferPS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr SkyboxPSBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

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
        
        CBufferSetPtr SkytextureVSBufferSetPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBuffer());
        CBufferSetPtr SkytexturePSBufferSetPtr = BufferManager::CreateBufferSet(SkytexturePSBuffer);

        CBufferSetPtr SkyboxVSBufferSetPtr = BufferManager::CreateBufferSet(SkyboxVSBuffer);
        CBufferSetPtr SkyboxPSBufferSetPtr = BufferManager::CreateBufferSet(SkyboxPSBuffer);

        m_BackgroundFromSkybox.m_VSBufferSetPtr = SkyboxVSBufferSetPtr;
        m_BackgroundFromSkybox.m_GSBufferSetPtr = 0;
        m_BackgroundFromSkybox.m_PSBufferSetPtr = SkyboxPSBufferSetPtr;

        m_BackgroundFromTexture.m_VSBufferSetPtr = SkytextureVSBufferSetPtr;
        m_BackgroundFromTexture.m_GSBufferSetPtr = 0;
        m_BackgroundFromTexture.m_PSBufferSetPtr = SkytexturePSBufferSetPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupModels()
    {
        CMeshPtr SkyboxBoxPtr = MeshManager::CreateBox(2.0f, 2.0f, 2.0f);
               
        // -----------------------------------------------------------------------------

        CMeshPtr QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);

        // -----------------------------------------------------------------------------

        m_BackgroundFromSkybox.m_MeshPtr = SkyboxBoxPtr;

        m_BackgroundFromTexture.m_MeshPtr = QuadModelPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnSetupEnd()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::OnUnloadMap()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CGfxBackgroundRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {
        BASE_UNUSED(_Width);
        BASE_UNUSED(_Height);

        OnSetupRenderTargets();
        OnSetupStates();
        OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxBackgroundRenderer::Render()
    {
        bool HasMainCamera = m_CameraRenderJobs.size() != 0;

        if (HasMainCamera)
        {
            SCameraRenderJob& rRenderJob = m_CameraRenderJobs[0];

            Performance::BeginEvent("Background");

            if (rRenderJob.m_pDataCamera->GetClearFlag() == Dt::CCameraActorFacet::Skybox)
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

    void CGfxBackgroundRenderer::RenderBackgroundFromSkybox()
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

        // -----------------------------------------------------------------------------
        // Render sky box
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Background From Skybox");

        // -----------------------------------------------------------------------------
        // Upload dynamic data
        // -----------------------------------------------------------------------------
        CCameraPtr CameraPtr = RenderContextPtr->GetCamera();

        SSkyboxVSBuffer ViewBuffer;

        ViewBuffer.m_View       = CameraPtr->GetView()->GetViewMatrix();
        ViewBuffer.m_Projection = CameraPtr->GetProjectionMatrix();

        ViewBuffer.m_View.InjectTranslation(0.0f, 0.0f, 0.0f);

        BufferManager::UploadConstantBufferData(VSBufferSetPtr->GetBuffer(0), &ViewBuffer);

        // -----------------------------------------------------------------------------

        SSkyboxBufferPS PSBuffer;
 
        PSBuffer.m_InvertedScreenSize   = Base::Float4(1.0f / Main::GetActiveWindowSize()[0], 1.0f / Main::GetActiveWindowSize()[1], 0, 0);
        PSBuffer.m_ExposureHistoryIndex = HistogramRenderer::GetLastExposureHistoryIndex();
        
        BufferManager::UploadConstantBufferData(PSBufferSetPtr->GetBuffer(0), &PSBuffer);
        
        // -----------------------------------------------------------------------------
        // Render sky box in background
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = {0, 0};
        
        ContextManager::SetRenderContext(RenderContextPtr);
                
        ContextManager::SetVertexBufferSet(MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
        
        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);
        
        ContextManager::SetInputLayout(InputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(VSPtr);
        
        ContextManager::SetShaderPS(PSPtr);
        
        ContextManager::SetConstantBuffer(0, VSBufferSetPtr->GetBuffer(0));
        
        ContextManager::SetConstantBuffer(8, PSBufferSetPtr->GetBuffer(0));

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        
        ContextManager::SetTexture(0, pSkyFacet->GetCubemapSetPtr()->GetTexture(0));
        ContextManager::SetTexture(1, TextureSetPtr->GetTexture(0));
        
        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        
        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);

        ContextManager::ResetConstantBuffer(0);
        
        ContextManager::ResetConstantBuffer(8);

        ContextManager::ResetResourceBuffer(0);
        
        ContextManager::ResetTopology();
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBufferSet();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxBackgroundRenderer::RenderBackgroundFromTexture()
    {
        SCameraRenderJob& rRenderJob = m_CameraRenderJobs[0];

        if (rRenderJob.m_pGraphicCamera->GetBackgroundTexture2D() == nullptr)
        {
            return;
        }

        // -----------------------------------------------------------------------------
        // Find sky entity
        // -----------------------------------------------------------------------------
        float HDRIntensity = 1.0f;

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
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Light);
        }

        if (pSkyEntity != 0)
        {
            Dt::CSkyFacet* pSkyFacet = static_cast<Dt::CSkyFacet*>(pSkyEntity->GetDetailFacet(Dt::SFacetCategory::Data));

            assert(pSkyFacet);

            HDRIntensity = pSkyFacet->GetIntensity();
        }

        // -----------------------------------------------------------------------------
        // Prepare value from sky / environment
        // -----------------------------------------------------------------------------
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

        // -----------------------------------------------------------------------------
        // Render sky texture
        // -----------------------------------------------------------------------------
        Performance::BeginEvent("Background from Texture");
        
        // -----------------------------------------------------------------------------
        // Data
        // -----------------------------------------------------------------------------
        SSkytextureBufferPS PSBuffer;

        PSBuffer.m_HDRFactor     = HDRIntensity;
        PSBuffer.m_IsHDR         = rRenderJob.m_pGraphicCamera->GetBackgroundTexture2D()->GetSemantic() == Dt::CTextureBase::HDR ? 1.0f : 0.0f;
        PSBuffer.m_ExposureIndex = static_cast<float>(HistogramRenderer::GetLastExposureHistoryIndex());

        BufferManager::UploadConstantBufferData(PSBufferSetPtr->GetBuffer(0), &PSBuffer);

        // -----------------------------------------------------------------------------
        // Rendering
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = { 0, 0 };

        ContextManager::SetRenderContext(RenderContextPtr);

        ContextManager::SetVertexBufferSet(MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);

        ContextManager::SetIndexBuffer(MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(InputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(VSPtr);

        ContextManager::SetShaderPS(PSPtr);

        ContextManager::SetConstantBuffer(0, VSBufferSetPtr->GetBuffer(0));

        ContextManager::SetConstantBuffer(8, PSBufferSetPtr->GetBuffer(0));

        ContextManager::SetResourceBuffer(0, HistogramRenderer::GetExposureHistoryBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, rRenderJob.m_pGraphicCamera->GetBackgroundTextureSet()->GetTexture(0));
        ContextManager::SetTexture(1, TextureSetPtr->GetTexture(0));

        ContextManager::DrawIndexed(MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTexture(0);
        ContextManager::ResetTexture(1);

        ContextManager::ResetSampler(0);
        ContextManager::ResetSampler(1);

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetConstantBuffer(8);

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxBackgroundRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_CameraRenderJobs.clear();

        // -----------------------------------------------------------------------------
        // Iterate throw every entity inside this map
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity;
        Dt::Map::CEntityIterator EndOfEntities;

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
namespace BackgroundRenderer
{
    void OnStart()
    {
        CGfxBackgroundRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxBackgroundRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxBackgroundRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxBackgroundRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxBackgroundRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxBackgroundRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxBackgroundRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxBackgroundRenderer::GetInstance().Render();
    }
} // namespace BackgroundRenderer
} // namespace Gfx