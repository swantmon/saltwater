
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

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

        CSelectionTicket& AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY);
        void ReleaseTicket(CSelectionTicket& _rTicket);

        void PushPick(CSelectionTicket& _rTicket, const Base::UInt2& _rCursor);
        bool PopPick(CSelectionTicket& _rTicket);

        void Clear(CSelectionTicket& _rTicket);

        bool IsEmpty(const CSelectionTicket& _rTicket);

        bool IsValid(const CSelectionTicket& _rTicket);

    private:

        static const unsigned int s_MaxNumberOfTickets = 16;
        static const unsigned int s_MaxSizeX           = 16;
        static const unsigned int s_MaxSizeY           = 16;

    private:

        struct SRequest
        {
            Base::UInt2   m_Cursor;
            unsigned int  m_Extra;
            Base::U64     m_TimeStamp;
            CBufferSetPtr m_BufferSetPtr;
        };

        struct SSelectionRequest
        {
            unsigned int m_MinX;
            unsigned int m_MinY;
            unsigned int m_MaxX;
            unsigned int m_MaxY;
        };

        struct SSelectionOutput
        {
            Base::Float4 m_WSPosition;
            Base::Float4 m_WSNormal;
            float        m_Depth;
        };

        class CInternSelectionTicket : CSelectionTicket
        {
        public:

            static const unsigned int s_MaxNumberOfRequests = 0 + 2;

        public:

            CInternSelectionTicket();
            ~CInternSelectionTicket();

        public:

            void Reset();

        private:

            bool         m_IsValid;
            int          m_OffsetX;
            int          m_OffsetY;
            int          m_SizeX;
            int          m_SizeY;
            unsigned int m_NumberOfRequests;
            unsigned int m_IndexOfPushRequest;
            unsigned int m_IndexOfPopRequest;
            Base::U64    m_Frame;
            SRequest     m_Requests[s_MaxNumberOfRequests];

        private:

            friend class CGfxSelectionRenderer;
        };
        
    private:

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

    private:

        typedef std::vector<SRenderJob> CRenderJobs;
        
    private:
        
        CBufferSetPtr          m_ViewModelVSBuffer;
        CBufferSetPtr          m_SelectionPSBufferSetPtr;
        CBufferPtr             m_PickingBufferPtr;
        CRenderContextPtr      m_RenderContextPtr;
        CShaderPtr             m_SelectionPSPtr;
        CShaderPtr             m_PickingCSPtr;
        CTextureSetPtr         m_GBufferTextureSetPtr;
        CRenderJobs            m_RenderJobs;
        CInternSelectionTicket m_SelectionTickets[s_MaxNumberOfTickets];

        Dt::CEntity* m_pSelectedEntity;
        
    private:

        void ResetTickets();
    
        void RenderSelection();

        void RenderPickingJobs();

        void BuildRenderJobs();
    };
} // namespace

namespace 
{
    CGfxSelectionRenderer::CInternSelectionTicket::CInternSelectionTicket()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxSelectionRenderer::CInternSelectionTicket::~CInternSelectionTicket()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::CInternSelectionTicket::Reset()
    {

    }
} // namespace 

namespace
{
    CGfxSelectionRenderer::CGfxSelectionRenderer()
        : m_ViewModelVSBuffer      ()
        , m_SelectionPSBufferSetPtr()
        , m_PickingBufferPtr       ()
        , m_RenderContextPtr       ()
        , m_SelectionPSPtr         ()
        , m_GBufferTextureSetPtr   ()
        , m_RenderJobs             ()
        , m_pSelectedEntity        (0)
    {
        ResetTickets();

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
        m_PickingBufferPtr        = 0;
        m_PickingBufferPtr        = 0;
        m_RenderContextPtr        = 0;
        m_SelectionPSPtr          = 0;
        m_PickingCSPtr            = 0;
        m_GBufferTextureSetPtr    = 0;
        m_pSelectedEntity         = 0;

        ResetTickets();

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
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSelectionRequest);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_PickingBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

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

    CSelectionTicket& CGfxSelectionRenderer::AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY)
    {
        unsigned int IndexOfTicket;

        assert((_SizeX <= s_MaxSizeX) && (_SizeY <= s_MaxSizeY));

        for (IndexOfTicket = 0; IndexOfTicket < s_MaxNumberOfTickets; ++IndexOfTicket)
        {
            CInternSelectionTicket& rTicket = m_SelectionTickets[IndexOfTicket];

            if ((!rTicket.m_IsValid) && (rTicket.m_NumberOfRequests == 0))
            {
                break;
            }
        }

        assert(IndexOfTicket < s_MaxNumberOfTickets);

        CInternSelectionTicket& rTicket = m_SelectionTickets[IndexOfTicket];

        rTicket.m_IsValid  = true;
        rTicket.m_OffsetX  = _OffsetX;
        rTicket.m_OffsetY  = _OffsetY;
        rTicket.m_SizeX    = _SizeX;
        rTicket.m_SizeY    = _SizeY;
        rTicket.m_Frame    = Core::Time::GetNumberOfFrame() - 1;

        Clear(rTicket);

        return rTicket;
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::ReleaseTicket(CSelectionTicket& _rTicket)
    {
        CInternSelectionTicket& rTicket = static_cast<CInternSelectionTicket&>(_rTicket);

        rTicket.m_IsValid = false;
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::PushPick(CSelectionTicket& _rTicket, const Base::UInt2& _rCursor)
    {
        CInternSelectionTicket& rTicket = static_cast<CInternSelectionTicket&>(_rTicket);

        assert(rTicket.m_IsValid);

        // -----------------------------------------------------------------------------
        // Only allow one push request per frame.
        // -----------------------------------------------------------------------------
        if (rTicket.m_Frame == Core::Time::GetNumberOfFrame())
        {
            return;
        }

        rTicket.m_Frame = Core::Time::GetNumberOfFrame();

        // -----------------------------------------------------------------------------
        // Take care that the cursor position is within the window.
        // -----------------------------------------------------------------------------
        Base::Int2 ActiveWindowSize = Gfx::Main::GetActiveWindowSize();

        if ((_rCursor[0] < 0) || (_rCursor[1] < 0) || (_rCursor[0] >= static_cast<short>(ActiveWindowSize[0])) || (_rCursor[1] >= static_cast<short>(ActiveWindowSize[1])))
        {
            return;
        }

        // -----------------------------------------------------------------------------
        // Avoid CPU-GPU stalls by resolving to a circular set of resources.
        // -----------------------------------------------------------------------------
        assert(rTicket.m_NumberOfRequests < CInternSelectionTicket::s_MaxNumberOfRequests);

        // -----------------------------------------------------------------------------
        // Setup request
        // -----------------------------------------------------------------------------
        SRequest& rRequest = rTicket.m_Requests[rTicket.m_IndexOfPushRequest];

        rRequest.m_Cursor    = _rCursor;
        rRequest.m_TimeStamp = Core::Time::GetNumberOfFrame() + 1;

        Gfx::SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUToCPU;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPURead;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSelectionOutput);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr PickingOuputBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        rRequest.m_BufferSetPtr = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferCS(), m_PickingBufferPtr, PickingOuputBufferPtr);

        // -----------------------------------------------------------------------------
        // Set push index
        // -----------------------------------------------------------------------------
        rTicket.m_IndexOfPushRequest = (rTicket.m_IndexOfPushRequest + 1) % CInternSelectionTicket::s_MaxNumberOfRequests;
        
        ++ rTicket.m_NumberOfRequests;
    }

    // -----------------------------------------------------------------------------

    bool CGfxSelectionRenderer::PopPick(CSelectionTicket& _rTicket)
    {
        Base::Size   ResultBegin;
        Base::Size   ResultEnd;
        unsigned int IndexOfTicket;
        unsigned int IndexOfResult;

        CInternSelectionTicket& rTicket = static_cast<CInternSelectionTicket&>(_rTicket);

        if (rTicket.m_NumberOfRequests == 0)
        {
            return false;
        }

        // ----------------------------------------------------------------------------- 
        // The request should be at least 2 frames old to avoid stalls.
        // -----------------------------------------------------------------------------
        SRequest& rRequest = rTicket.m_Requests[rTicket.m_IndexOfPopRequest];

        if (Core::Time::GetNumberOfFrame() < rRequest.m_TimeStamp)
        {
            return false;
        }

        IndexOfTicket = static_cast<unsigned int>(&rTicket - &m_SelectionTickets[0]);
        IndexOfResult = IndexOfTicket * CInternSelectionTicket::s_MaxNumberOfRequests + rTicket.m_IndexOfPopRequest;

        // -----------------------------------------------------------------------------
        // Forward to the next request.
        // -----------------------------------------------------------------------------
        rTicket.m_IndexOfPopRequest = (rTicket.m_IndexOfPopRequest + 1) % CInternSelectionTicket::s_MaxNumberOfRequests;
        
        -- rTicket.m_NumberOfRequests;

        // -----------------------------------------------------------------------------
        // Read values from GPU to CPU and fill data
        // -----------------------------------------------------------------------------
        rTicket.m_HitFlag    = SHitFlag::Nothing;
        rTicket.m_WSPosition = Base::Float3::s_Zero;
        rTicket.m_WSNormal   = Base::Float3::s_Zero;
        rTicket.m_Depth      = 1.0f;
        rTicket.m_pObject    = 0;

        SSelectionOutput* pOutput = static_cast<SSelectionOutput*>(BufferManager::MapConstantBuffer(rRequest.m_BufferSetPtr->GetBuffer(2)));

        rTicket.m_WSPosition = Base::Float3(pOutput->m_WSPosition[0], pOutput->m_WSPosition[1], pOutput->m_WSPosition[2]);
        rTicket.m_WSNormal   = Base::Float3(pOutput->m_WSNormal[0], pOutput->m_WSNormal[1], pOutput->m_WSNormal[2]);
        rTicket.m_Depth      = pOutput->m_Depth;

        BufferManager::UnmapConstantBuffer(rRequest.m_BufferSetPtr->GetBuffer(2));

        // -----------------------------------------------------------------------------
        // Release data
        // -----------------------------------------------------------------------------
        rRequest.m_BufferSetPtr = 0;

        return true;
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::Clear(CSelectionTicket& _rTicket)
    {
        CInternSelectionTicket& rTicket = static_cast<CInternSelectionTicket&>(_rTicket);

        rTicket.m_IndexOfPushRequest = 0;
        rTicket.m_IndexOfPopRequest  = 0;
        rTicket.m_NumberOfRequests   = 0;
    }

    // -----------------------------------------------------------------------------

    bool CGfxSelectionRenderer::IsEmpty(const CSelectionTicket& _rTicket)
    {
        const CInternSelectionTicket& rTicket = static_cast<const CInternSelectionTicket&>(_rTicket);

        return rTicket.m_NumberOfRequests == 0;
    }

    // -----------------------------------------------------------------------------

    bool CGfxSelectionRenderer::IsValid(const CSelectionTicket& _rTicket)
    {
        const CInternSelectionTicket& rTicket = static_cast<const CInternSelectionTicket&>(_rTicket);

        return rTicket.m_IsValid;
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::ResetTickets()
    {
        unsigned int IndexOfTicket;

        for (IndexOfTicket = 0; IndexOfTicket < CInternSelectionTicket::s_MaxNumberOfRequests; ++IndexOfTicket)
        {
            CInternSelectionTicket& rTicket = m_SelectionTickets[IndexOfTicket];

            rTicket.Reset();
        }
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
        unsigned int IndexOfResult;
        unsigned int IndexOfLastRequest;
        unsigned int IndexOfTicket;
        unsigned int MinX;
        unsigned int MinY;
        unsigned int MaxX;
        unsigned int MaxY;

        for (IndexOfTicket = 0; IndexOfTicket < s_MaxNumberOfTickets; ++IndexOfTicket)
        {
            CInternSelectionTicket& rTicket = m_SelectionTickets[IndexOfTicket];

            if (rTicket.m_IsValid && (rTicket.m_NumberOfRequests > 0) && (rTicket.m_Frame == Core::Time::GetNumberOfFrame()))
            {
                IndexOfLastRequest = (rTicket.m_IndexOfPushRequest > 0) ? rTicket.m_IndexOfPushRequest - 1 : CInternSelectionTicket::s_MaxNumberOfRequests - 1;

                SRequest& rRequest = rTicket.m_Requests[IndexOfLastRequest];

                IndexOfResult = IndexOfTicket * CInternSelectionTicket::s_MaxNumberOfRequests + IndexOfLastRequest;

                // -----------------------------------------------------------------------------
                // Setup buffer
                // -----------------------------------------------------------------------------
                Base::Int2 ActiveWindowSize = Gfx::Main::GetActiveWindowSize();

                SSelectionRequest* pSettings = static_cast<SSelectionRequest*>(BufferManager::MapConstantBuffer(rRequest.m_BufferSetPtr->GetBuffer(1)));

                MinX = rRequest.m_Cursor[0] + rTicket.m_OffsetX;
                MinY = rRequest.m_Cursor[1] + rTicket.m_OffsetY;
                MaxX = MinX + rTicket.m_SizeX;
                MaxY = MinY + rTicket.m_SizeY;

                if (MinX < 0) MinX = 0;
                if (MinY < 0) MinY = 0;
                if (MaxX > static_cast<int>(ActiveWindowSize[0])) MaxX = ActiveWindowSize[0];
                if (MaxY > static_cast<int>(ActiveWindowSize[1])) MaxY = ActiveWindowSize[1];

                pSettings->m_MinX = MinX;
                pSettings->m_MinY = MinY;
                pSettings->m_MaxX = MaxX;
                pSettings->m_MaxY = MaxY;

                BufferManager::UnmapConstantBuffer(rRequest.m_BufferSetPtr->GetBuffer(1));

                // -----------------------------------------------------------------------------
                // Execute
                // -----------------------------------------------------------------------------
                Performance::BeginEvent("Picking");

                ContextManager::SetShaderCS(m_PickingCSPtr);

                ContextManager::SetConstantBufferSetCS(rRequest.m_BufferSetPtr);

                ContextManager::SetTextureSetCS(m_GBufferTextureSetPtr);

                ContextManager::Dispatch(1, 1, 1);

                ContextManager::ResetTextureSetCS();

                ContextManager::ResetConstantBufferSetCS();

                ContextManager::ResetShaderCS();

                Performance::EndEvent();

                // -----------------------------------------------------------------------------
                // Set request
                // -----------------------------------------------------------------------------
                rRequest.m_TimeStamp = Core::Time::GetNumberOfFrame();
            }
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

    CSelectionTicket& AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY)
    {
        return CGfxSelectionRenderer::GetInstance().AcquireTicket(_OffsetX, _OffsetY, _SizeX, _SizeY);
    }

    // -----------------------------------------------------------------------------

    void ReleaseTicket(CSelectionTicket& _rTicket)
    {
        return CGfxSelectionRenderer::GetInstance().ReleaseTicket(_rTicket);
    }

    // -----------------------------------------------------------------------------

    void PushPick(CSelectionTicket& _rTicket, const Base::UInt2& _rCursor)
    {
        CGfxSelectionRenderer::GetInstance().PushPick(_rTicket, _rCursor);
    }

    // -----------------------------------------------------------------------------

    bool PopPick(CSelectionTicket& _rTicket)
    {
        return CGfxSelectionRenderer::GetInstance().PopPick(_rTicket);
    }

    // -----------------------------------------------------------------------------

    void Clear(CSelectionTicket& _rTicket)
    {
        CGfxSelectionRenderer::GetInstance().Clear(_rTicket);
    }

    // -----------------------------------------------------------------------------

    bool IsEmpty(const CSelectionTicket& _rTicket)
    {
        return CGfxSelectionRenderer::GetInstance().IsEmpty(_rTicket);
    }

    // -----------------------------------------------------------------------------

    bool IsValid(const CSelectionTicket& _rTicket)
    {
        return CGfxSelectionRenderer::GetInstance().IsValid(_rTicket);
    }
} // namespace SelectionRenderer
} // namespace Gfx
