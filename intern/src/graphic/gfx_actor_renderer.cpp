
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_type.h"
#include "data/data_entity.h"
#include "data/data_map.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_actor_renderer.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_actor_facet.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
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
        void RenderHitProxy();

    private:

        struct SPerDrawCallConstantBufferVS
        {
            Base::Float4x4 m_ModelMatrix;
        };

        struct SHitProxyProperties
        {
            unsigned int m_ID;
        };

        struct SRenderJob
        {
            unsigned int   m_SurfaceAttributes;
            unsigned int   m_EntityID;
            CSurfacePtr    m_SurfacePtr;
            CMaterialPtr   m_SurfaceMaterialPtr;
            Base::Float4x4 m_ModelMatrix;
        };

    private:

        typedef std::vector<SRenderJob> CRenderJobs;

    private:

        CBufferPtr        m_ModelBufferPtr;
        CBufferPtr        m_SurfaceMaterialBufferPtr;
        CBufferPtr        m_HitProxyPassPSBufferPtr;
        CShaderPtr        m_HitProxyShaderPtr;
        CRenderContextPtr m_DeferredContextPtr;
        CRenderContextPtr m_HitProxyContextPtr;
        CRenderJobs       m_DeferredRenderJobs;

    private:

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxActorRenderer::CGfxActorRenderer()
        : m_ModelBufferPtr          ()
        , m_SurfaceMaterialBufferPtr()
        , m_HitProxyPassPSBufferPtr ()
        , m_HitProxyShaderPtr       ()
        , m_DeferredContextPtr      ()
        , m_HitProxyContextPtr      ()
        , m_DeferredRenderJobs      ()
    {
        // -----------------------------------------------------------------------------
        // Register resize delegate
        // -----------------------------------------------------------------------------
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxActorRenderer::OnResize));

        // -----------------------------------------------------------------------------
        // Reserve some jobs
        // -----------------------------------------------------------------------------
        m_DeferredRenderJobs.reserve(1024);
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
        m_ModelBufferPtr           = 0;
        m_SurfaceMaterialBufferPtr = 0;
        m_HitProxyPassPSBufferPtr  = 0;
        m_HitProxyShaderPtr        = 0;
        m_DeferredContextPtr       = 0;
        m_HitProxyContextPtr       = 0;

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
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupShader()
    {
        m_HitProxyShaderPtr = ShaderManager::CompilePS("fs_hitproxy.glsl", "main");
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
        CCameraPtr      CameraPtr              = ViewManager     ::GetMainCamera ();
        CViewPortSetPtr ViewPortSetPtr         = ViewManager     ::GetViewPortSet();
        CRenderStatePtr DeferredRenderStatePtr = StateManager    ::GetRenderState(0);
        CRenderStatePtr HitProxyRenderStatePtr = StateManager    ::GetRenderState(CRenderState::EqualDepth);
        CTargetSetPtr   DeferredTargetSetPtr   = TargetSetManager::GetDeferredTargetSet();
        CTargetSetPtr   HitProxyTargetSetPtr   = TargetSetManager::GetHitProxyTargetSet();

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
        RenderContextPtr->SetTargetSet(HitProxyTargetSetPtr);
        RenderContextPtr->SetRenderState(HitProxyRenderStatePtr);

        m_HitProxyContextPtr = RenderContextPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupTextures()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxActorRenderer::OnSetupBuffers()
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

        m_SurfaceMaterialBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SHitProxyProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_HitProxyPassPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
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
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = CurrentRenderJob->m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            BufferManager::UploadBufferData(m_SurfaceMaterialBufferPtr, &MaterialPtr->GetMaterialAttributes());

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            ContextManager::SetShaderGS(MaterialPtr->GetShaderGS());

            ContextManager::SetShaderPS(MaterialPtr->GetShaderPS());

            for (unsigned int IndexOfTexture = 0; IndexOfTexture < MaterialPtr->GetTextureSetPS()->GetNumberOfTextures(); ++IndexOfTexture)
            {
                ContextManager::SetSampler(IndexOfTexture, MaterialPtr->GetSamplerSetPS()->GetSampler(IndexOfTexture));

                ContextManager::SetTexture(IndexOfTexture, MaterialPtr->GetTextureSetPS()->GetTexture(IndexOfTexture));
            }

            ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
            ContextManager::SetConstantBuffer(1, m_ModelBufferPtr);
            ContextManager::SetConstantBuffer(2, m_SurfaceMaterialBufferPtr);

            // -----------------------------------------------------------------------------
            // If we have a bump map we use tessellation
            // TODO: we have to set buffer to shader on model loading
            // TODO: remove GetHasBump()
            // -----------------------------------------------------------------------------
            if (MaterialPtr->GetHasBump())
            {
                ContextManager::SetShaderDS(MaterialPtr->GetShaderDS());

                ContextManager::SetShaderHS(MaterialPtr->GetShaderHS());

                ContextManager::SetTopology(STopology::Patches);
            }

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);

            for (unsigned int IndexOfTexture = 0; IndexOfTexture < MaterialPtr->GetTextureSetPS()->GetNumberOfTextures(); ++IndexOfTexture)
            {
                ContextManager::ResetSampler(IndexOfTexture);

                ContextManager::ResetTexture(IndexOfTexture);
            }

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBuffer();

            ContextManager::ResetConstantBuffer(0);
            ContextManager::ResetConstantBuffer(1);
            ContextManager::ResetConstantBuffer(2);
        }

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

    void CGfxActorRenderer::RenderHitProxy()
    {
        if (m_DeferredRenderJobs.size() == 0) return;

        Performance::BeginEvent("Actors Hit Proxy");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        ContextManager::SetRenderContext(m_HitProxyContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        // -----------------------------------------------------------------------------
        // First pass: iterate throw render jobs and compute all meshes
        // -----------------------------------------------------------------------------
        CRenderJobs::const_iterator EndOfRenderJobs = m_DeferredRenderJobs.end();

        for (CRenderJobs::const_iterator CurrentRenderJob = m_DeferredRenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            CSurfacePtr  SurfacePtr  = CurrentRenderJob->m_SurfacePtr;

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = CurrentRenderJob->m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            SHitProxyProperties HitProxyProperties;

            HitProxyProperties.m_ID = CurrentRenderJob->m_EntityID;

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
            if (rCurrentEntity.GetType() != Dt::SActorType::Mesh || rCurrentEntity.GetLayer() != Dt::SEntityLayer::Default)
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

                assert(MaterialPtr != 0 && MaterialPtr.IsValid());

                // -----------------------------------------------------------------------------
                // Set informations to render job
                // -----------------------------------------------------------------------------
                SRenderJob NewRenderJob;

                NewRenderJob.m_SurfaceAttributes  = SurfacePtr->GetKey().m_Key;
                NewRenderJob.m_EntityID           = CurrentEntity->GetID();
                NewRenderJob.m_SurfacePtr         = SurfacePtr;
                NewRenderJob.m_SurfaceMaterialPtr = MaterialPtr;
                NewRenderJob.m_ModelMatrix        = rCurrentEntity.GetTransformationFacet()->GetWorldMatrix();

                m_DeferredRenderJobs.push_back(NewRenderJob);
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

    // -----------------------------------------------------------------------------

    void RenderHitProxy()
    {
        CGfxActorRenderer::GetInstance().RenderHitProxy();
    }
} // namespace ActorRenderer
} // namespace Gfx
