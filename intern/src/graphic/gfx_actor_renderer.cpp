
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_map.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_actor_facet.h"
#include "graphic/gfx_actor_renderer.h"
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

using namespace Gfx;

namespace
{
    class CGfxActorRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxActorRenderer)

    public:

        CGfxActorRenderer();
        ~CGfxActorRenderer();

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

        struct SForwardLightProperties
        {
            Base::Float4 m_ViewDirection;
            Base::Float4 m_LightDirection;
            Base::Float4 m_LightColor;
        };

        struct SRenderJob
        {
            unsigned int     m_SurfaceAttributes;
            CSurfacePtr      m_SurfacePtr;
            CMaterialPtr     m_SurfaceMaterialPtr;
            Base::Float4x4   m_ModelMatrix;
        };

    private:

        typedef std::vector<SRenderJob> CRenderJobs;

    private:

        CBufferSetPtr     m_ViewVSBuffer;
        CBufferSetPtr     m_DeferredPassPSBuffer;
        CBufferSetPtr     m_HSBuffer;
        CBufferSetPtr     m_DSBuffer;
        CBufferSetPtr     m_ViewPSBuffer;
        CBufferSetPtr     m_ForwardPassPSBuffer;
        CSamplerSetPtr    m_PSSamplerSet;
        CRenderContextPtr m_DeferredContextPtr;
        CRenderContextPtr m_ForwardContextPtr;
        CTextureSetPtr    m_DeferredTextureSetPtrs;
        CTextureSetPtr    m_LightingTextureSetPtr;

        CRenderJobs       m_DeferredRenderJobs;
        CRenderJobs       m_ForwardRenderJobs;

    private:

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxActorRenderer::CGfxActorRenderer()
        : m_ViewVSBuffer         ()
        , m_DeferredPassPSBuffer ()
        , m_HSBuffer             ()
        , m_DSBuffer             ()
        , m_ViewPSBuffer         ()
        , m_ForwardPassPSBuffer  ()
        , m_PSSamplerSet         ()
        , m_DeferredContextPtr   ()
        , m_ForwardContextPtr    ()
        , m_LightingTextureSetPtr()
        , m_DeferredRenderJobs   ()
        , m_ForwardRenderJobs    ()
    {
        // -----------------------------------------------------------------------------
        // Register resize delegate
        // -----------------------------------------------------------------------------
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxActorRenderer::OnResize));

        // -----------------------------------------------------------------------------
        // Reserve some jobs
        // -----------------------------------------------------------------------------
        m_DeferredRenderJobs.reserve(1024);
        m_ForwardRenderJobs .reserve(16);
    }

    // -----------------------------------------------------------------------------

    CGfxActorRenderer::~CGfxActorRenderer()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnStart()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnExit()
    {
        m_ViewVSBuffer           = 0;
        m_DeferredPassPSBuffer   = 0;
        m_HSBuffer               = 0;
        m_DSBuffer               = 0;
        m_ViewPSBuffer           = 0;
        m_ForwardPassPSBuffer    = 0;
        m_PSSamplerSet           = 0;
        m_DeferredContextPtr     = 0;
        m_ForwardContextPtr      = 0;
        m_DeferredTextureSetPtrs = 0;
        m_LightingTextureSetPtr  = 0;

        // -----------------------------------------------------------------------------
        // Iterate throw render jobs to release managed pointer
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator EndOfRenderJobs;

        EndOfRenderJobs  = m_DeferredRenderJobs.end();

        for (CRenderJobs::iterator CurrentRenderJob = m_DeferredRenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++ CurrentRenderJob)
        {
            CurrentRenderJob->m_SurfacePtr = nullptr;
        }

        m_DeferredRenderJobs.clear();

        // -----------------------------------------------------------------------------

        EndOfRenderJobs  = m_ForwardRenderJobs.end();

        for (CRenderJobs::iterator CurrentRenderJob = m_ForwardRenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++ CurrentRenderJob)
        {
            CurrentRenderJob->m_SurfacePtr = nullptr;
        }

        m_ForwardRenderJobs.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupShader()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupKernels()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupRenderTargets()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupStates()
    {
        CCameraPtr          CameraPtr              = ViewManager     ::GetMainCamera ();
        CViewPortSetPtr     ViewPortSetPtr         = ViewManager     ::GetViewPortSet();
        CRenderStatePtr     DeferredRenderStatePtr = StateManager    ::GetRenderState(0);
        CRenderStatePtr     ForwardRenderStatePtr  = StateManager    ::GetRenderState(CRenderState::AlphaBlend);
        CTargetSetPtr       DeferredTargetSetPtr   = TargetSetManager::GetDeferredTargetSet();
        CTargetSetPtr       SystemTargetSetPtr     = TargetSetManager::GetDefaultTargetSet();

        CRenderContextPtr RenderContextPtr = ContextManager::CreateRenderContext();

        RenderContextPtr->SetCamera(CameraPtr);
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(DeferredTargetSetPtr);
        RenderContextPtr->SetRenderState(DeferredRenderStatePtr);

        m_DeferredContextPtr = RenderContextPtr;

        // -----------------------------------------------------------------------------

        RenderContextPtr = ContextManager::CreateRenderContext();

        RenderContextPtr->SetCamera(CameraPtr);
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(SystemTargetSetPtr);
        RenderContextPtr->SetRenderState(ForwardRenderStatePtr);

        m_ForwardContextPtr = RenderContextPtr;

        // -----------------------------------------------------------------------------

        CSamplerPtr LinearFilter = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        CSamplerPtr PointFilter  = SamplerManager::GetSampler(CSampler::MinMagMipPointWrap);

        m_PSSamplerSet = SamplerManager::CreateSamplerSet(LinearFilter, LinearFilter, LinearFilter, LinearFilter);
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupTextures()
    {
        m_DeferredTextureSetPtrs = TextureManager::CreateTextureSet(m_DeferredContextPtr->GetTargetSet()->GetRenderTarget(0), m_DeferredContextPtr->GetTargetSet()->GetRenderTarget(1), m_DeferredContextPtr->GetTargetSet()->GetRenderTarget(2));

        // -----------------------------------------------------------------------------

        CTextureBasePtr LightAccumulationTexturePtr = TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0);

        m_LightingTextureSetPtr = TextureManager::CreateTextureSet(LightAccumulationTexturePtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupBuffers()
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

        CBufferPtr ModelBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        m_ViewVSBuffer = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS(), ModelBuffer);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(CMaterial::SMaterialAttributes);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr MaterialBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SForwardLightProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr LightPropertiesBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        m_DeferredPassPSBuffer = BufferManager::CreateBufferSet(MaterialBuffer);

        m_ViewPSBuffer         = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferPS(), MaterialBuffer);
        
        m_HSBuffer             = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferHS());
        
        m_DSBuffer             = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferDS());

        m_ForwardPassPSBuffer  = BufferManager::CreateBufferSet(MaterialBuffer, LightPropertiesBuffer);
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupResources()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupModels()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupEnd()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnReload()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnNewMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnUnloadMap()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {
        BASE_UNUSED(_Width);
        BASE_UNUSED(_Height);
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::Update()
    {
        // -----------------------------------------------------------------------------
        // Set render jobs depending on camera. At the end we have to iterate throw
        // the map only once. Then we can order the render jobs and we get as less
        // state changes as possible.
        // -----------------------------------------------------------------------------
        BuildRenderJobs();
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::Render()
    {
        if (m_DeferredRenderJobs.size() == 0) return;

        Performance::BeginEvent("Actors");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = { 0, 0 };

        ContextManager::SetRenderContext(m_DeferredContextPtr);

        // -----------------------------------------------------------------------------
        // First pass: iterate throw render jobs and compute all meshes
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator EndOfRenderJobs = m_DeferredRenderJobs.end();

        for (CRenderJobs::const_iterator CurrentRenderJob = m_DeferredRenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            CSurfacePtr  SurfacePtr  = CurrentRenderJob->m_SurfacePtr;
            CMaterialPtr MaterialPtr = CurrentRenderJob->m_SurfaceMaterialPtr;

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS* pModelBuffer = static_cast<SPerDrawCallConstantBufferVS*>(BufferManager::MapConstantBuffer(m_ViewVSBuffer->GetBuffer(1)));

            assert(pModelBuffer != nullptr);

            pModelBuffer->m_ModelMatrix = CurrentRenderJob->m_ModelMatrix;

            BufferManager::UnmapConstantBuffer(m_ViewVSBuffer->GetBuffer(1));

            CMaterial::SMaterialAttributes* pMaterialBuffer = static_cast<CMaterial::SMaterialAttributes*>(BufferManager::MapConstantBuffer(m_DeferredPassPSBuffer->GetBuffer(0)));

            Base::CMemory::Copy(pMaterialBuffer, &MaterialPtr->GetMaterialAttributes(), sizeof(CMaterial::SMaterialAttributes));

            BufferManager::UnmapConstantBuffer(m_DeferredPassPSBuffer->GetBuffer(0));

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetTopology(STopology::TriangleList);

            // TODO by tschwandt
            // MATERIAL COMMENT
            // ContextManager::SetShaderVS(MaterialPtr->GetShaderVS());

            ContextManager::SetShaderGS(MaterialPtr->GetShaderGS());

            ContextManager::SetShaderPS(MaterialPtr->GetShaderPS());

            ContextManager::SetTextureSetPS(MaterialPtr->GetTextureSetPS());

            ContextManager::SetSamplerSetPS(m_PSSamplerSet);

            ContextManager::SetConstantBufferSetVS(m_ViewVSBuffer);

            ContextManager::SetConstantBufferSetPS(m_DeferredPassPSBuffer);

            // -----------------------------------------------------------------------------
            // If we have a bump map we use tessellation
            // TODO: we have to set buffer to shader on model loading
            // TODO: remove GetHasBump()
            // -----------------------------------------------------------------------------
            if (MaterialPtr->GetHasBump())
            {
                ContextManager::SetShaderDS(MaterialPtr->GetShaderDS());

                ContextManager::SetShaderHS(MaterialPtr->GetShaderHS());

                ContextManager::SetConstantBufferSetHS(m_HSBuffer);

                ContextManager::SetConstantBufferSetDS(m_DSBuffer);

                ContextManager::SetTextureSetDS(MaterialPtr->GetTextureSetPS());

                ContextManager::SetTopology(STopology::Patches);
            }

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBufferSet(SurfacePtr->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            // TODO by tschwandt
            // MATERIAL COMMENT
            // ContextManager::SetInputLayout(MaterialPtr->GetShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBufferSet();

            ContextManager::ResetConstantBufferSetPS();

            ContextManager::ResetConstantBufferSetDS();

            ContextManager::ResetConstantBufferSetHS();

            ContextManager::ResetConstantBufferSetVS();
        }

        ContextManager::ResetTextureSetDS();

        ContextManager::ResetTextureSetHS();

        ContextManager::ResetSamplerSetPS();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderDS();

        ContextManager::ResetShaderHS();

        ContextManager::ResetShaderGS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        ContextManager::ResetTopology();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // That is an sort object for sorting render jobs. They will be sorted from
        // lower number to higher number. The surface attribute is symbol for
        // complexity.
        // -----------------------------------------------------------------------------
        struct SSortObject
        {
            bool operator() (SRenderJob& _rLeft, SRenderJob& _rRight)
            {
                return (_rLeft.m_SurfaceAttributes < _rRight.m_SurfaceAttributes);
            }
        } SortObject;

        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_DeferredRenderJobs.clear();
        m_ForwardRenderJobs .clear();

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
            if (rCurrentEntity.GetType() != Dt::SActorType::Mesh)
            {
                CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);

                continue;
            }

            CMeshActorFacet* pGraphicModelActorFacet = static_cast<CMeshActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic));

            CMeshPtr MeshPtr = pGraphicModelActorFacet->GetMesh();

            // -----------------------------------------------------------------------------
            // Set every surface of this entity into a new render job
            // -----------------------------------------------------------------------------
            unsigned int NumberOfSurfaces = MeshPtr->GetLOD(0)->GetNumberOfSurfaces();

            for (unsigned int IndexOfSurface = 0; IndexOfSurface < NumberOfSurfaces; ++ IndexOfSurface)
            {
                CSurfacePtr SurfacePtr = MeshPtr->GetLOD(0)->GetSurface(IndexOfSurface);

                if (SurfacePtr == nullptr)
                {
                    break;
                }

                CMaterialPtr MaterialPtr;

                if (pGraphicModelActorFacet->GetMaterial(IndexOfSurface) != 0)
                {
                    MaterialPtr = pGraphicModelActorFacet->GetMaterial(IndexOfSurface);
                }
                else
                {
                    MaterialPtr = SurfacePtr->GetMaterial();
                }

                // -----------------------------------------------------------------------------
                // Set informations to render job
                // -----------------------------------------------------------------------------
                SRenderJob NewRenderJob;

                NewRenderJob.m_SurfaceAttributes  = SurfacePtr->GetKey().m_Key;
                NewRenderJob.m_SurfacePtr         = SurfacePtr;
                NewRenderJob.m_SurfaceMaterialPtr = MaterialPtr;
                NewRenderJob.m_ModelMatrix        = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();

                if (!MaterialPtr->GetHasAlpha())
                {
                    m_DeferredRenderJobs.push_back(NewRenderJob);
                }
                else
                {
                    m_ForwardRenderJobs.push_back(NewRenderJob);
                }
            }

            // -----------------------------------------------------------------------------
            // Next entity
            // -----------------------------------------------------------------------------
            CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor);
        }

        // -----------------------------------------------------------------------------
        // Now we order our render jobs
        // -----------------------------------------------------------------------------
        std::sort(m_DeferredRenderJobs.begin(), m_DeferredRenderJobs.end(), SortObject);
        std::sort(m_ForwardRenderJobs .begin(), m_ForwardRenderJobs .end(), SortObject);
    }
} // namespace


namespace Gfx
{
namespace ActorRenderer
{
    void OnStart()
    {
        CGfxActorRenderer::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxActorRenderer::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void OnSetupShader()
    {
        CGfxActorRenderer::GetInstance().OnSetupShader();
    }

    // -----------------------------------------------------------------------------

    void OnSetupKernels()
    {
        CGfxActorRenderer::GetInstance().OnSetupKernels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupRenderTargets()
    {
        CGfxActorRenderer::GetInstance().OnSetupRenderTargets();
    }

    // -----------------------------------------------------------------------------

    void OnSetupStates()
    {
        CGfxActorRenderer::GetInstance().OnSetupStates();
    }

    // -----------------------------------------------------------------------------

    void OnSetupTextures()
    {
        CGfxActorRenderer::GetInstance().OnSetupTextures();
    }

    // -----------------------------------------------------------------------------

    void OnSetupBuffers()
    {
        CGfxActorRenderer::GetInstance().OnSetupBuffers();
    }

    // -----------------------------------------------------------------------------

    void OnSetupResources()
    {
        CGfxActorRenderer::GetInstance().OnSetupResources();
    }

    // -----------------------------------------------------------------------------

    void OnSetupModels()
    {
        CGfxActorRenderer::GetInstance().OnSetupModels();
    }

    // -----------------------------------------------------------------------------

    void OnSetupEnd()
    {
        CGfxActorRenderer::GetInstance().OnSetupEnd();
    }

    // -----------------------------------------------------------------------------

    void OnReload()
    {
        CGfxActorRenderer::GetInstance().OnReload();
    }

    // -----------------------------------------------------------------------------

    void OnNewMap()
    {
        CGfxActorRenderer::GetInstance().OnNewMap();
    }

    // -----------------------------------------------------------------------------

    void OnUnloadMap()
    {
        CGfxActorRenderer::GetInstance().OnUnloadMap();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxActorRenderer::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void Render()
    {
        CGfxActorRenderer::GetInstance().Render();
    }
} // namespace ActorRenderer
} // namespace Gfx
