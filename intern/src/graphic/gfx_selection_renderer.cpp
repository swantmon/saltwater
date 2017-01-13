
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_type.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_actor_facet.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_selection_renderer.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include <map>

using namespace Gfx;

namespace
{
    class CGfxSelectionRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxSelectionRenderer)
        
    public:
        
        CGfxSelectionRenderer();
        ~CGfxSelectionRenderer();
        
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
        void OnResize(unsigned int _Width, unsigned int _Height);
        
        void Update();
        void Render();

        void SelectEntity(unsigned int _EntityID);
        void UnselectEntity();

        unsigned int AddPickingJob(const Base::Float2& _rUV, bool _IsHomogeneous = true);
        const SPickingInfo* GetInfoOfPickingJob(unsigned int _JobID);
        void RemovePickingJob(unsigned int _JobID);
        
    private:

        struct SPickingSettings
        {
            Base::UInt2  m_UV;
            Base::Float2 m_HomogeneousUV;
        };

        struct SPickingOutput
        {
            Base::Float4 m_WSPosition;
            Base::Float4 m_WSNormal;
            float        m_Depth;
        };

        struct SPerDrawCallConstantBufferVS
        {
            Base::Float4x4 m_ModelMatrix;
        };
        
        struct SSelectionSettings
        {
            Base::Float4 m_ColorAlpha;
        };

        struct SRenderJob
        {
            CSurfacePtr    m_SurfacePtr;
            Base::Float4x4 m_ModelMatrix;
        };

        struct SPickingJob
        {
            Base::UInt2  m_UV;
            Base::Float2 m_HomogeneousUV;
        };

    private:

        typedef std::vector<SRenderJob>  CRenderJobs;
        typedef std::vector<SPickingJob> CPickingJobs;
        typedef std::map<unsigned int, SPickingInfo> CPickingInfos;
        
    private:
        
        CBufferSetPtr     m_ViewModelVSBuffer;
        CBufferSetPtr     m_SelectionPSBufferSetPtr;
        CRenderContextPtr m_RenderContextPtr;
        CShaderPtr        m_SelectionPSPtr;
        CShaderPtr        m_PickingCSPtr;
        CBufferSetPtr     m_PickingBufferSetPtr;
        CTextureSetPtr    m_GBufferTextureSetPtr;

        CRenderJobs       m_RenderJobs;
        CPickingJobs      m_PickingJobs;
        CPickingInfos     m_PickingInfos;

        Dt::CEntity* m_pSelectedEntity;
        
    private:
    
        void RenderSelection();

        void RenderPickingJobs();

        void BuildRenderJobs();
    };
} // namespace

namespace
{
    CGfxSelectionRenderer::CGfxSelectionRenderer()
        : m_ViewModelVSBuffer      ()
        , m_SelectionPSBufferSetPtr()
        , m_RenderContextPtr       ()
        , m_SelectionPSPtr         ()
        , m_PickingBufferSetPtr    ()
        , m_GBufferTextureSetPtr   ()
        , m_RenderJobs             ()
        , m_PickingJobs            ()
        , m_PickingInfos           ()
        , m_pSelectedEntity        (0)
    {
        // -----------------------------------------------------------------------------
        // Register resize delegate
        // -----------------------------------------------------------------------------
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxSelectionRenderer::OnResize));
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxSelectionRenderer::~CGfxSelectionRenderer()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnStart()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnExit()
    {
        m_ViewModelVSBuffer       = 0;
        m_SelectionPSBufferSetPtr = 0;
        m_RenderContextPtr        = 0;
        m_SelectionPSPtr          = 0;
        m_PickingCSPtr            = 0;
        m_PickingBufferSetPtr     = 0;
        m_GBufferTextureSetPtr    = 0;
        m_pSelectedEntity         = 0;

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
        m_PickingJobs.clear();
        m_PickingInfos.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupShader()
    {
        m_SelectionPSPtr = ShaderManager::CompilePS("fs_selection.glsl", "main");
        m_PickingCSPtr   = ShaderManager::CompileCS("cs_picking.glsl", "main");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupRenderTargets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupStates()
    {
        CCameraPtr          CameraPtr        = ViewManager     ::GetMainCamera ();
        CViewPortSetPtr     ViewPortSetPtr   = ViewManager     ::GetViewPortSet();
        CRenderStatePtr     RenderStatePtr   = StateManager    ::GetRenderState(CRenderState::Wireframe);
        CTargetSetPtr       TargetSetPtr     = TargetSetManager::GetSystemTargetSet();
        
        CRenderContextPtr RenderContextPtr = ContextManager::CreateRenderContext();
        
        RenderContextPtr->SetCamera(CameraPtr);
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(TargetSetPtr);
        RenderContextPtr->SetRenderState(RenderStatePtr);
        
        m_RenderContextPtr = RenderContextPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupTextures()
    {
        CTargetSetPtr DeferredTargetSetPtr = TargetSetManager::GetDeferredTargetSet();

        m_GBufferTextureSetPtr = TextureManager::CreateTextureSet(DeferredTargetSetPtr->GetRenderTarget(0), DeferredTargetSetPtr->GetRenderTarget(1), DeferredTargetSetPtr->GetRenderTarget(2), DeferredTargetSetPtr->GetDepthStencilTarget());
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBufferVS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr ViewBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSelectionSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr SelectionBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPickingSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr PickingBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUToCPU;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPURead;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPickingOutput);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr PickingOuputBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        m_PickingBufferSetPtr     = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferCS(), PickingBufferPtr, PickingOuputBufferPtr);

        m_ViewModelVSBuffer       = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS(), ViewBuffer);

        m_SelectionPSBufferSetPtr = BufferManager::CreateBufferSet(SelectionBufferPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupModels()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {
        BASE_UNUSED(_Width);
        BASE_UNUSED(_Height);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::Update()
    {
        BuildRenderJobs();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::Render()
    {
        Performance::BeginEvent("Selection");

        RenderSelection();

        RenderPickingJobs();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::SelectEntity(unsigned int _EntityID)
    {
        m_pSelectedEntity = &Dt::EntityManager::GetEntityByID(_EntityID);

        assert(m_pSelectedEntity != nullptr);
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::UnselectEntity()
    {
        m_pSelectedEntity = nullptr;
    }

    // -----------------------------------------------------------------------------

    unsigned int CGfxSelectionRenderer::AddPickingJob(const Base::Float2& _rUV, bool _IsHomogeneous)
    {
        SPickingJob NewPickingJob;

        Base::Int2 ActiveWindowSize = Gfx::Main::GetActiveWindowSize();

        NewPickingJob.m_UV            = Base::UInt2(static_cast<unsigned int>(_rUV[0] * ActiveWindowSize[0]), static_cast<unsigned int>(_rUV[1] * ActiveWindowSize[1]));
        NewPickingJob.m_HomogeneousUV = _rUV;

        m_PickingJobs.push_back(NewPickingJob);

        return m_PickingJobs.size() - 1;
    }

    // -----------------------------------------------------------------------------

    const SPickingInfo* CGfxSelectionRenderer::GetInfoOfPickingJob(unsigned int _JobID)
    {
        if (m_PickingInfos.find(_JobID) == m_PickingInfos.end())
        {
            return 0;
        }

        return &m_PickingInfos[_JobID];
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::RemovePickingJob(unsigned int _JobID)
    {
        m_PickingInfos.erase(_JobID);
        m_PickingJobs.erase(m_PickingJobs.begin() + _JobID);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::RenderSelection()
    {
        if (m_RenderJobs.size() == 0) return;

        Performance::BeginEvent("Actors");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = { 0, 0 };

        ContextManager::SetRenderContext(m_RenderContextPtr);

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
            SPerDrawCallConstantBufferVS* pModelBuffer = static_cast<SPerDrawCallConstantBufferVS*>(BufferManager::MapConstantBuffer(m_ViewModelVSBuffer->GetBuffer(1)));

            assert(pModelBuffer != nullptr);

            pModelBuffer->m_ModelMatrix = CurrentRenderJob->m_ModelMatrix;

            BufferManager::UnmapConstantBuffer(m_ViewModelVSBuffer->GetBuffer(1));

            SSelectionSettings* pSelectionSettings = static_cast<SSelectionSettings*>(BufferManager::MapConstantBuffer(m_SelectionPSBufferSetPtr->GetBuffer(0)));

            pSelectionSettings->m_ColorAlpha = Base::Float4(0.31f, 0.45f, 0.64f, 0.4f);

            BufferManager::UnmapConstantBuffer(m_SelectionPSBufferSetPtr->GetBuffer(0));

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            ContextManager::SetShaderPS(m_SelectionPSPtr);

            ContextManager::SetConstantBufferSetVS(m_ViewModelVSBuffer);

            ContextManager::SetConstantBufferSetPS(m_SelectionPSBufferSetPtr);

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBufferSet(SurfacePtr->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

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

    void CGfxSelectionRenderer::RenderPickingJobs()
    {
        CPickingJobs::const_iterator EndOfJobs = m_PickingJobs.end();

        unsigned int RenderIndex = 0;

        for (CPickingJobs::const_iterator CurrentJob = m_PickingJobs.begin(); CurrentJob != EndOfJobs; ++CurrentJob)
        {
             // -----------------------------------------------------------------------------
            // Upload input data
            // -----------------------------------------------------------------------------
            Base::Int2 ActiveWindowSize = Gfx::Main::GetActiveWindowSize();

            SPickingSettings* pSettings = static_cast<SPickingSettings*>(BufferManager::MapConstantBuffer(m_PickingBufferSetPtr->GetBuffer(1)));

            pSettings->m_UV            = CurrentJob->m_UV;
            pSettings->m_HomogeneousUV = CurrentJob->m_HomogeneousUV;

            BufferManager::UnmapConstantBuffer(m_PickingBufferSetPtr->GetBuffer(1));

            // -----------------------------------------------------------------------------
            // Execute
            // -----------------------------------------------------------------------------
            Performance::BeginEvent("Picking");

            ContextManager::SetShaderCS(m_PickingCSPtr);

            ContextManager::SetConstantBufferSetCS(m_PickingBufferSetPtr);

            ContextManager::SetTextureSetCS(m_GBufferTextureSetPtr);

            ContextManager::Dispatch(1, 1, 1);

            ContextManager::ResetTextureSetCS();

            ContextManager::ResetConstantBufferSetCS();

            ContextManager::ResetShaderCS();

            Performance::EndEvent();

            // -----------------------------------------------------------------------------
            // Read values from GPU to CPU and fill data
            // -----------------------------------------------------------------------------
            SPickingInfo NewPickingInfo;

            NewPickingInfo.m_Flags      = SPickingInfo::Nothing;
            NewPickingInfo.m_WSPosition = Base::Float3::s_Zero;
            NewPickingInfo.m_WSNormal   = Base::Float3::s_Zero;
            NewPickingInfo.m_Depth      = 1.0f;
            NewPickingInfo.m_pEntity    = 0;
            NewPickingInfo.m_pRegion    = 0;

            SPickingOutput* pOutput = static_cast<SPickingOutput*>(BufferManager::MapConstantBuffer(m_PickingBufferSetPtr->GetBuffer(2)));

            NewPickingInfo.m_WSPosition = Base::Float3(pOutput->m_WSPosition[0], pOutput->m_WSPosition[1], pOutput->m_WSPosition[2]);
            NewPickingInfo.m_WSNormal   = Base::Float3(pOutput->m_WSNormal[0], pOutput->m_WSNormal[1], pOutput->m_WSNormal[2]);
            NewPickingInfo.m_Depth      = pOutput->m_Depth;

            BufferManager::UnmapConstantBuffer(m_PickingBufferSetPtr->GetBuffer(2));

            // -----------------------------------------------------------------------------
            // Put new info into map
            // -----------------------------------------------------------------------------
            m_PickingInfos[RenderIndex] = NewPickingInfo;

            // -----------------------------------------------------------------------------
            // Increase index
            // -----------------------------------------------------------------------------
            ++RenderIndex;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::BuildRenderJobs()
    {
        // -----------------------------------------------------------------------------
        // Clear current render jobs
        // -----------------------------------------------------------------------------
        m_RenderJobs.clear();

        // -----------------------------------------------------------------------------
        // Is an entity selected
        // -----------------------------------------------------------------------------
        if (m_pSelectedEntity == nullptr) return;

        // -----------------------------------------------------------------------------
        // Add current entity depending on type
        // -----------------------------------------------------------------------------
        auto AddEntityToJobs = [&](Dt::CEntity* _pEntity)
        {
            assert(_pEntity);

            if (_pEntity->GetCategory() == Dt::SEntityCategory::Actor && _pEntity->GetType() == Dt::SActorType::Mesh)
            {
                CMeshActorFacet* pGraphicModelActorFacet = static_cast<CMeshActorFacet*>(_pEntity->GetDetailFacet(Dt::SFacetCategory::Graphic));

                CMeshPtr MeshPtr = pGraphicModelActorFacet->GetMesh();

                assert(pGraphicModelActorFacet != nullptr);
                assert(MeshPtr.IsValid());

                // -----------------------------------------------------------------------------
                // Set every surface of this entity into a new render job
                // -----------------------------------------------------------------------------
                unsigned int NumberOfSurfaces = MeshPtr->GetLOD(0)->GetNumberOfSurfaces();

                for (unsigned int IndexOfSurface = 0; IndexOfSurface < NumberOfSurfaces; ++IndexOfSurface)
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

                    NewRenderJob.m_SurfacePtr  = SurfacePtr;
                    NewRenderJob.m_ModelMatrix = _pEntity->GetTransformationFacet()->GetWorldMatrix();

                    m_RenderJobs.push_back(NewRenderJob);
                }
            }
        };
        
        std::function<void(Dt::CEntity*)> NavigateToNextEntity = [&](Dt::CEntity* _pRootEntity) -> void
        {
            Dt::CEntity* pSiblingEntity = _pRootEntity;

            for (; pSiblingEntity != nullptr; )
            {
                assert(pSiblingEntity->GetHierarchyFacet());

                Dt::CEntity* pFirstChild = pSiblingEntity->GetHierarchyFacet()->GetFirstChild();

                if (pFirstChild != nullptr)
                {
                    NavigateToNextEntity(pFirstChild);
                }

                AddEntityToJobs(pSiblingEntity);

                pSiblingEntity = pSiblingEntity->GetHierarchyFacet()->GetSibling();
            }
        };

        if (m_pSelectedEntity != nullptr)
        {
            Dt::CHierarchyFacet* pHierarchyFacet = m_pSelectedEntity->GetHierarchyFacet();

            if (pHierarchyFacet)
            {
                Dt::CEntity* pFirstChild = pHierarchyFacet->GetFirstChild();

                if (pFirstChild != nullptr)
                {
                    NavigateToNextEntity(pFirstChild);
                }
            }

            AddEntityToJobs(m_pSelectedEntity);
        }
        
    }
} // namespace


namespace Gfx
{
namespace SelectionRenderer
{
    void OnStart()
    {
        CGfxSelectionRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxSelectionRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    
    void OnSetupShader()
    {
        CGfxSelectionRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxSelectionRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxSelectionRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxSelectionRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxSelectionRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxSelectionRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxSelectionRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxSelectionRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxSelectionRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxSelectionRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxSelectionRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxSelectionRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void SelectEntity(unsigned int _EntityID)
    {
        CGfxSelectionRenderer::GetInstance().SelectEntity(_EntityID);
    }

    // -----------------------------------------------------------------------------

    void UnselectEntity()
    {
        CGfxSelectionRenderer::GetInstance().UnselectEntity();
    }

    // -----------------------------------------------------------------------------

    unsigned int AddPickingJob(const Base::Float2& _rUV, bool _IsHomogeneous)
    {
        return CGfxSelectionRenderer::GetInstance().AddPickingJob(_rUV, _IsHomogeneous);
    }

    // -----------------------------------------------------------------------------

    const SPickingInfo* GetInfoOfPickingJob(unsigned int _JobID)
    {
        return CGfxSelectionRenderer::GetInstance().GetInfoOfPickingJob(_JobID);
    }

    // -----------------------------------------------------------------------------

    void RemovePickingJob(unsigned int _JobID)
    {
        CGfxSelectionRenderer::GetInstance().RemovePickingJob(_JobID);
    }
} // namespace SelectionRenderer
} // namespace Gfx
