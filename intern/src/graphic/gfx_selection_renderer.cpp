
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_math_limits.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_actor_renderer.h"
#include "graphic/gfx_ar_renderer.h"
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

        CSelectionTicket& AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY, unsigned int _Flags = SPickFlag::Nothing);
        void ReleaseTicket(CSelectionTicket& _rTicket);

        void PushPick(CSelectionTicket& _rTicket, const Base::Int2& _rCursor);
        bool PopPick(CSelectionTicket& _rTicket);

        void Clear(CSelectionTicket& _rTicket);

        bool IsEmpty(const CSelectionTicket& _rTicket);

        bool IsValid(const CSelectionTicket& _rTicket);

    private:

        static const unsigned int s_MaxNumberOfTickets = 16;
        static const unsigned int s_MaxSizeX           = 16;
        static const unsigned int s_MaxSizeY           = 16;

    private:

        struct SSelectionOutput
        {
            Base::Float4 m_WSPosition;
            Base::Float4 m_WSNormal;
            float        m_Depth;
            unsigned int m_EntityID;
        };

        struct SRequest
        {
            Base::Int2 m_Cursor;
            Base::U64  m_TimeStamp;
        };
        
        class CInternSelectionTicket : CSelectionTicket
        {
        public:

            static const unsigned int s_MaxNumberOfRequests = 2;

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
            unsigned int m_IndexOfTicket;
            unsigned int m_Flags;
            Base::U64    m_Frame;
            SRequest     m_Requests[s_MaxNumberOfRequests];

        private:

            friend class CGfxSelectionRenderer;
        };
        
    private:

        static const unsigned int s_MaxNumberOfBuffer = s_MaxNumberOfTickets * CInternSelectionTicket::s_MaxNumberOfRequests;

    private:

        struct SPerDrawCallConstantBufferVS
        {
            Base::Float4x4 m_ModelMatrix;
        };

        struct SSelectionSettings
        {
            unsigned int m_MinX;
            unsigned int m_MinY;
            unsigned int m_MaxX;
            unsigned int m_MaxY;
        };
        
        struct SHighlightSettings
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
        CBufferSetPtr          m_HighlightPSBufferSetPtr;
        CBufferSetPtr          m_SelectionBufferSetPtrs[s_MaxNumberOfBuffer];
        CRenderContextPtr      m_RenderContextPtr;
        CShaderPtr             m_HighlightPSPtr;
        CShaderPtr             m_SelectionCSPtr;
        CTextureSetPtr         m_GBufferTextureSetPtr;
        CRenderJobs            m_RenderJobs;
        CInternSelectionTicket m_SelectionTickets[s_MaxNumberOfTickets];

        Dt::CEntity* m_pSelectedEntity;
        
    private:

        void ResetTickets();
    
        void RenderHighlight();

        void RenderSelection();

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
        , m_HighlightPSBufferSetPtr()
        , m_SelectionBufferSetPtrs ()
        , m_RenderContextPtr       ()
        , m_HighlightPSPtr         ()
        , m_GBufferTextureSetPtr   ()
        , m_RenderJobs             ()
        , m_SelectionTickets       ()
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
        m_HighlightPSBufferSetPtr = 0;
        m_RenderContextPtr        = 0;
        m_HighlightPSPtr          = 0;
        m_SelectionCSPtr          = 0;
        m_GBufferTextureSetPtr    = 0;
        m_pSelectedEntity         = 0;

        ResetTickets();

        // -----------------------------------------------------------------------------
        // Reset buffer
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfBuffer = 0; IndexOfBuffer < s_MaxNumberOfBuffer; ++IndexOfBuffer)
        {
            m_SelectionBufferSetPtrs[IndexOfBuffer] = 0;
        }

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
        m_HighlightPSPtr = ShaderManager::CompilePS("fs_highlight.glsl", "main");
        m_SelectionCSPtr = ShaderManager::CompileCS("cs_selection.glsl", "main");
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
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SHighlightSettings);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr HighlightBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        for (unsigned int IndexOfBuffer = 0; IndexOfBuffer < s_MaxNumberOfBuffer; ++IndexOfBuffer)
        {
            ConstanteBufferDesc.m_Stride        = 0;
            ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
            ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
            ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
            ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSelectionSettings);
            ConstanteBufferDesc.m_pBytes        = 0;
            ConstanteBufferDesc.m_pClassKey     = 0;

            CBufferPtr SelectionRequestBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

            // -----------------------------------------------------------------------------

            ConstanteBufferDesc.m_Stride        = 0;
            ConstanteBufferDesc.m_Usage         = CBuffer::GPUToCPU;
            ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
            ConstanteBufferDesc.m_Access        = CBuffer::CPURead;
            ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSelectionOutput);
            ConstanteBufferDesc.m_pBytes        = 0;
            ConstanteBufferDesc.m_pClassKey     = 0;

            CBufferPtr SelectionOuputBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

            m_SelectionBufferSetPtrs[IndexOfBuffer] = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferCS(), SelectionRequestBufferPtr, SelectionOuputBufferPtr);;
        }

        // -----------------------------------------------------------------------------

        m_ViewModelVSBuffer       = BufferManager::CreateBufferSet(Main::GetPerFrameConstantBufferVS(), ViewBuffer);

        m_HighlightPSBufferSetPtr = BufferManager::CreateBufferSet(HighlightBufferPtr);
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
        RenderHighlight();

        RenderSelection();
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

    CSelectionTicket& CGfxSelectionRenderer::AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY, unsigned int _Flags)
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

        rTicket.m_IsValid       = true;
        rTicket.m_HitFlag       = SHitFlag::Nothing;
        rTicket.m_OffsetX       = _OffsetX;
        rTicket.m_OffsetY       = _OffsetY;
        rTicket.m_SizeX         = _SizeX;
        rTicket.m_SizeY         = _SizeY;
        rTicket.m_IndexOfTicket = IndexOfTicket;
        rTicket.m_Flags         = _Flags;
        rTicket.m_WSPosition    = Base::Float3::s_Zero;
        rTicket.m_WSNormal      = Base::Float3::s_Zero;
        rTicket.m_Depth         = -1.0f;
        rTicket.m_pObject       = 0;
        rTicket.m_Frame         = Core::Time::GetNumberOfFrame() - 1;
        
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

    void CGfxSelectionRenderer::PushPick(CSelectionTicket& _rTicket, const Base::Int2& _rCursor)
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
        // Setup request
        // -----------------------------------------------------------------------------
        SRequest& rRequest = rTicket.m_Requests[rTicket.m_IndexOfPushRequest];

        rRequest.m_Cursor    = _rCursor;
        rRequest.m_TimeStamp = Base::SLimits<Base::U64>::s_Max;

        // -----------------------------------------------------------------------------
        // Set push index
        // -----------------------------------------------------------------------------
        rTicket.m_IndexOfPushRequest = (rTicket.m_IndexOfPushRequest + 1) % CInternSelectionTicket::s_MaxNumberOfRequests;
        
        rTicket.m_NumberOfRequests = Base::Clamp(rTicket.m_NumberOfRequests + 1, 0u, CInternSelectionTicket::s_MaxNumberOfRequests);
    }

    // -----------------------------------------------------------------------------

    bool CGfxSelectionRenderer::PopPick(CSelectionTicket& _rTicket)
    {
        unsigned int IndexOfLastRequest;
        unsigned int IndexOfBuffer;

        CInternSelectionTicket& rTicket = static_cast<CInternSelectionTicket&>(_rTicket);

        if (rTicket.m_NumberOfRequests == 0)
        {
            return false;
        }

        // ----------------------------------------------------------------------------- 
        // The request should be at least 2 frames old to avoid stalls.
        // -----------------------------------------------------------------------------
        SRequest& rRequest = rTicket.m_Requests[rTicket.m_IndexOfPopRequest];

        if (Core::Time::GetNumberOfFrame() >= rRequest.m_TimeStamp + 2)
        {
            return false;
        }

        // -----------------------------------------------------------------------------
        // Forward to the next request.
        // -----------------------------------------------------------------------------
        rTicket.m_IndexOfPopRequest = (rTicket.m_IndexOfPopRequest + 1) % CInternSelectionTicket::s_MaxNumberOfRequests;
        
        -- rTicket.m_NumberOfRequests;

        // -----------------------------------------------------------------------------
        // Read values from GPU to CPU and fill data
        // -----------------------------------------------------------------------------
        IndexOfLastRequest = (rTicket.m_IndexOfPopRequest > 0) ? rTicket.m_IndexOfPopRequest - 1 : CInternSelectionTicket::s_MaxNumberOfRequests - 1;
        IndexOfBuffer      = rTicket.m_IndexOfTicket * s_MaxNumberOfTickets + IndexOfLastRequest;

        SSelectionOutput* pOutput = static_cast<SSelectionOutput*>(BufferManager::MapConstantBuffer(m_SelectionBufferSetPtrs[IndexOfBuffer]->GetBuffer(2), CBuffer::Read));

        rTicket.m_WSPosition = Base::Float3(pOutput->m_WSPosition[0], pOutput->m_WSPosition[1], pOutput->m_WSPosition[2]);
        rTicket.m_WSNormal   = Base::Float3(pOutput->m_WSNormal[0], pOutput->m_WSNormal[1], pOutput->m_WSNormal[2]);
        rTicket.m_Depth      = pOutput->m_Depth;
        rTicket.m_HitFlag    = SHitFlag::Nothing;
        rTicket.m_pObject    = nullptr;

        if (pOutput->m_EntityID > 0)
        {
            rTicket.m_HitFlag = SHitFlag::Entity;

            rTicket.m_pObject = &Dt::EntityManager::GetEntityByID(pOutput->m_EntityID);
        }

        BufferManager::UnmapConstantBuffer(m_SelectionBufferSetPtrs[IndexOfBuffer]->GetBuffer(2));

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
    
    void CGfxSelectionRenderer::RenderHighlight()
    {
        if (m_RenderJobs.size() == 0) return;

        Performance::BeginEvent("Highlight");

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
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = CurrentRenderJob->m_ModelMatrix;

            BufferManager::UploadConstantBufferData(m_ViewModelVSBuffer->GetBuffer(1), &ModelBuffer);

            SHighlightSettings SelectionSettings;

            SelectionSettings.m_ColorAlpha = Base::Float4(0.31f, 0.45f, 0.64f, 0.4f);

            BufferManager::UploadConstantBufferData(m_HighlightPSBufferSetPtr->GetBuffer(0), &SelectionSettings);

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            ContextManager::SetShaderPS(m_HighlightPSPtr);

            ContextManager::SetConstantBufferSetVS(m_ViewModelVSBuffer);

            ContextManager::SetConstantBufferSetPS(m_HighlightPSBufferSetPtr);

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

    void CGfxSelectionRenderer::RenderSelection()
    {
        unsigned int IndexOfLastRequest;
        unsigned int IndexOfTicket;
        unsigned int IndexOfBuffer;
        unsigned int MinX;
        unsigned int MinY;
        unsigned int MaxX;
        unsigned int MaxY;

        Performance::BeginEvent("Picking");

        for (IndexOfTicket = 0; IndexOfTicket < s_MaxNumberOfTickets; ++IndexOfTicket)
        {
            CInternSelectionTicket& rTicket = m_SelectionTickets[IndexOfTicket];

            if (rTicket.m_IsValid && (rTicket.m_NumberOfRequests > 0) && (rTicket.m_Frame <= Core::Time::GetNumberOfFrame()))
            {
                IndexOfLastRequest = (rTicket.m_IndexOfPushRequest > 0) ? rTicket.m_IndexOfPushRequest - 1 : CInternSelectionTicket::s_MaxNumberOfRequests - 1;
                IndexOfBuffer      = IndexOfTicket * s_MaxNumberOfTickets + IndexOfLastRequest;

                SRequest& rRequest = rTicket.m_Requests[IndexOfLastRequest];

                // -----------------------------------------------------------------------------
                // Render hit proxies depending on flag
                // -----------------------------------------------------------------------------
                if (rTicket.m_Flags & SPickFlag::Actor) ActorRenderer::RenderHitProxy();
                if (rTicket.m_Flags & SPickFlag::AR)    ARRenderer   ::RenderHitProxy();

                // -----------------------------------------------------------------------------
                // Setup buffer
                // -----------------------------------------------------------------------------
                Base::Int2 ActiveWindowSize = Gfx::Main::GetActiveWindowSize();

                SSelectionSettings Settings;

                MinX = rRequest.m_Cursor[0] + rTicket.m_OffsetX;
                MinY = rRequest.m_Cursor[1] + rTicket.m_OffsetY;
                MaxX = MinX + rTicket.m_SizeX;
                MaxY = MinY + rTicket.m_SizeY;

                if (MaxX > static_cast<unsigned int>(ActiveWindowSize[0])) MaxX = ActiveWindowSize[0];
                if (MaxY > static_cast<unsigned int>(ActiveWindowSize[1])) MaxY = ActiveWindowSize[1];
                if (MinX > MaxX) MinX = 0;
                if (MinY > MaxY) MinY = 0;

                Settings.m_MinX = MinX;
                Settings.m_MinY = MinY;
                Settings.m_MaxX = MaxX;
                Settings.m_MaxY = MaxY;

                BufferManager::UploadConstantBufferData(m_SelectionBufferSetPtrs[IndexOfBuffer]->GetBuffer(1), &Settings);

                // -----------------------------------------------------------------------------
                // Execute
                // -----------------------------------------------------------------------------
                ContextManager::SetShaderCS(m_SelectionCSPtr);

                ContextManager::SetConstantBufferSetCS(m_SelectionBufferSetPtrs[IndexOfBuffer]);

                ContextManager::SetImageTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
                ContextManager::SetImageTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
                ContextManager::SetImageTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2));
                ContextManager::SetImageTexture(3, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());
                ContextManager::SetImageTexture(4, TargetSetManager::GetHitProxyTargetSet()->GetRenderTarget(0));

                ContextManager::Dispatch(rTicket.m_SizeX, rTicket.m_SizeY, 1);

                ContextManager::ResetImageTexture(0);
                ContextManager::ResetImageTexture(1);
                ContextManager::ResetImageTexture(2);
                ContextManager::ResetImageTexture(3);
                ContextManager::ResetImageTexture(4);

                ContextManager::ResetConstantBufferSetCS();

                ContextManager::ResetShaderCS();

                // -----------------------------------------------------------------------------
                // Clear hit proxy target set
                // -----------------------------------------------------------------------------
                TargetSetManager::ClearTargetSet(TargetSetManager::GetHitProxyTargetSet(), Base::Float4(0.0f));

                // -----------------------------------------------------------------------------
                // Set request
                // -----------------------------------------------------------------------------
                rRequest.m_TimeStamp = Core::Time::GetNumberOfFrame();
            }
        }

        Performance::EndEvent();
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

                assert(pGraphicModelActorFacet != nullptr);

                CMeshPtr MeshPtr = pGraphicModelActorFacet->GetMesh();

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

    CSelectionTicket& AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY, unsigned int _Flags)
    {
        return CGfxSelectionRenderer::GetInstance().AcquireTicket(_OffsetX, _OffsetY, _SizeX, _SizeY, _Flags);
    }

    // -----------------------------------------------------------------------------

    void ReleaseTicket(CSelectionTicket& _rTicket)
    {
        return CGfxSelectionRenderer::GetInstance().ReleaseTicket(_rTicket);
    }

    // -----------------------------------------------------------------------------

    void PushPick(CSelectionTicket& _rTicket, const Base::Int2& _rCursor)
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
