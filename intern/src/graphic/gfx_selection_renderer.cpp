
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_include_glm.h"
#include "base/base_math_limits.h"
#include "base/base_program_parameters.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_component_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_light_probe_component.h"
#include "data/data_mesh_component.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_ar_renderer.h"
#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_component_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_light_probe_component.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_component.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_mesh_renderer.h"
#include "graphic/gfx_performance.h"
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
        
        void Update();
        void Render();

        void SelectEntity(Base::ID _EntityID);
        void UnselectEntity();

        CSelectionTicket& AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY, unsigned int _Flags = SPickFlag::Nothing);
        void ReleaseTicket(CSelectionTicket& _rTicket);

        void PushPick(CSelectionTicket& _rTicket, const glm::ivec2& _rCursor);
        bool PopPick(CSelectionTicket& _rTicket);

        void Clear(CSelectionTicket& _rTicket);

        bool IsEmpty(const CSelectionTicket& _rTicket);

        bool IsValid(const CSelectionTicket& _rTicket);

        void ResetSettings();
        void SetSettings(const SSelectionSettings& _rSettings);
        const SSelectionSettings& GetSettings();

    private:

        static const unsigned int s_MaxNumberOfTickets = 16;
        static const unsigned int s_MaxSizeX           = 16;
        static const unsigned int s_MaxSizeY           = 16;

    private:

        struct SSelectionOutput
        {
            glm::vec4 m_WSPosition;
            glm::vec4 m_WSNormal;
            float        m_Depth;
            unsigned int m_EntityID;
        };

        struct SRequest
        {
            glm::ivec2 m_Cursor;
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
            glm::mat4 m_ModelMatrix;
        };

        struct SSelectionSettingsBuffer
        {
            glm::vec4 m_MinMaxUV;
        };
        
        struct SHighlightSettingsBuffer
        {
            glm::vec4 m_ColorAlpha;
        };

        struct SSurfaceRenderJob
        {
            CSurfacePtr    m_SurfacePtr;
            glm::mat4 m_ModelMatrix;
        };

        struct SProbeRenderJob
        {
            glm::mat4        m_ModelMatrix;
            Gfx::CTexturePtr  m_TextureCubePtr;
            Dt::CLightProbeComponent* m_pDtProbeFacet;
        };

    private:

        typedef std::vector<SSurfaceRenderJob> CSurfaceRenderJobs;
        typedef std::vector<SProbeRenderJob>   CProbeRenderJobs;
        
    private:
        
        CBufferPtr             m_ModelBufferPtr;
        CBufferPtr             m_HighlightPSBufferPtr;
        CBufferSetPtr          m_SelectionBufferSetPtrs[s_MaxNumberOfBuffer];
        CShaderPtr             m_HighlightPSPtr;
        CShaderPtr             m_TextureCubePSPtr;
        CShaderPtr             m_SelectionCSPtr;
        CTextureSetPtr         m_GBufferTextureSetPtr;
        CMeshPtr               m_SphereMeshPtr;
        CMeshPtr               m_BoxMeshPtr;
        CSurfaceRenderJobs     m_SurfaceRenderJobs;
        CProbeRenderJobs       m_ProbeRenderJobs;
        CInternSelectionTicket m_SelectionTickets[s_MaxNumberOfTickets];

        Dt::CEntity* m_pSelectedEntity;

        SSelectionSettings m_Settings;
        
    private:

        void ResetTickets();
    
        void RenderHighlight();

        void RenderHighlightSurfaces();

        void RenderHighlightProbes();

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
        : m_ModelBufferPtr        ()
        , m_HighlightPSBufferPtr  ()
        , m_SelectionBufferSetPtrs()
        , m_HighlightPSPtr        ()
        , m_TextureCubePSPtr      ()
        , m_GBufferTextureSetPtr  ()
        , m_SphereMeshPtr         ()
        , m_BoxMeshPtr            ()
        , m_SurfaceRenderJobs     ()
        , m_ProbeRenderJobs       ()
        , m_SelectionTickets      ()
        , m_pSelectedEntity       (0)
    {
        ResetTickets();
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxSelectionRenderer::~CGfxSelectionRenderer()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnStart()
    {
        ResetSettings();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnExit()
    {
        m_ModelBufferPtr       = 0;
        m_HighlightPSBufferPtr = 0;
        m_HighlightPSPtr       = 0;
        m_TextureCubePSPtr     = 0;
        m_SelectionCSPtr       = 0;
        m_GBufferTextureSetPtr = 0;
        m_SphereMeshPtr        = 0;
        m_BoxMeshPtr           = 0;
        m_pSelectedEntity      = 0;

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
        CSurfaceRenderJobs::const_iterator EndOfRenderJobs;

        EndOfRenderJobs = m_SurfaceRenderJobs.end();

        for (CSurfaceRenderJobs::iterator CurrentRenderJob = m_SurfaceRenderJobs.begin(); CurrentRenderJob != EndOfRenderJobs; ++CurrentRenderJob)
        {
            CurrentRenderJob->m_SurfacePtr = nullptr;
        }

        m_SurfaceRenderJobs.clear();

        m_ProbeRenderJobs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupShader()
    {
        m_HighlightPSPtr   = ShaderManager::CompilePS("fs_highlight.glsl", "main");
        m_TextureCubePSPtr = ShaderManager::CompilePS("fs_texture_cube.glsl", "main");
        m_SelectionCSPtr   = ShaderManager::CompileCS("cs_selection.glsl", "main");
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
        
        m_ModelBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SHighlightSettingsBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_HighlightPSBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        for (unsigned int IndexOfBuffer = 0; IndexOfBuffer < s_MaxNumberOfBuffer; ++IndexOfBuffer)
        {
            ConstanteBufferDesc.m_Stride        = 0;
            ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
            ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
            ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
            ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSelectionSettingsBuffer);
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

            m_SelectionBufferSetPtrs[IndexOfBuffer] = BufferManager::CreateBufferSet(SelectionRequestBufferPtr, SelectionOuputBufferPtr);;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupModels()
    {
        m_SphereMeshPtr = MeshManager::CreateSphere(1.0f, 32, 32);

        m_BoxMeshPtr = MeshManager::CreateBox(2.0f, 2.0f, 2.0f);
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

    void CGfxSelectionRenderer::SelectEntity(Base::ID _EntityID)
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
        rTicket.m_WSPosition    = glm::vec3(0.0f);
        rTicket.m_WSNormal      = glm::vec3(0.0f);
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

    void CGfxSelectionRenderer::PushPick(CSelectionTicket& _rTicket, const glm::ivec2& _rCursor)
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
        glm::ivec2 ActiveWindowSize = Gfx::Main::GetActiveNativeWindowSize();

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
        
        rTicket.m_NumberOfRequests = glm::clamp(rTicket.m_NumberOfRequests + 1, 0u, CInternSelectionTicket::s_MaxNumberOfRequests);
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

        SSelectionOutput* pOutput = static_cast<SSelectionOutput*>(BufferManager::MapBuffer(m_SelectionBufferSetPtrs[IndexOfBuffer]->GetBuffer(1), CBuffer::Read));

        rTicket.m_WSPosition = glm::vec3(pOutput->m_WSPosition[0], pOutput->m_WSPosition[1], pOutput->m_WSPosition[2]);
        rTicket.m_WSNormal   = glm::vec3(pOutput->m_WSNormal[0], pOutput->m_WSNormal[1], pOutput->m_WSNormal[2]);
        rTicket.m_Depth      = pOutput->m_Depth;
        rTicket.m_HitFlag    = SHitFlag::Nothing;
        rTicket.m_pObject    = nullptr;

        if (pOutput->m_EntityID > 0)
        {
            rTicket.m_HitFlag = SHitFlag::Entity;

            rTicket.m_pObject = &Dt::EntityManager::GetEntityByID(pOutput->m_EntityID);
        }

        BufferManager::UnmapBuffer(m_SelectionBufferSetPtrs[IndexOfBuffer]->GetBuffer(1));

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

    void CGfxSelectionRenderer::ResetSettings()
    {
        SSelectionSettings Settings;

        Settings.m_HighlightColor = Base::CProgramParameters::GetInstance().Get("graphics:selection:highlight:color", glm::vec4(0.31f, 0.45f, 0.64f, 0.4f));
        Settings.m_HighlightUseDepth = Base::CProgramParameters::GetInstance().Get("graphics:selection:highlight:use_depth", true);
        Settings.m_HighlightUseWireframe = Base::CProgramParameters::GetInstance().Get("graphics:selection:highlight:use_wireframe", true);

        SetSettings(Settings);
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::SetSettings(const SSelectionSettings& _rSettings)
    {
        m_Settings = _rSettings;
    }

    // -----------------------------------------------------------------------------

    const SSelectionSettings& CGfxSelectionRenderer::GetSettings()
    {
        return m_Settings;
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
        if (m_SurfaceRenderJobs.size() == 0 && m_ProbeRenderJobs.size() == 0) return;

        Performance::BeginEvent("Highlight");

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        ContextManager::SetTargetSet(TargetSetManager::GetDefaultTargetSet());

        ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());

        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::AlphaBlend));

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(m_Settings.m_HighlightUseDepth == true ? CDepthStencilState::LessEqualDepth : 0));

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_ModelBufferPtr);

        ContextManager::SetConstantBuffer(2, m_HighlightPSBufferPtr);

        // -----------------------------------------------------------------------------
        // Upload settings
        // -----------------------------------------------------------------------------
        SHighlightSettingsBuffer SelectionSettings;

        SelectionSettings.m_ColorAlpha = m_Settings.m_HighlightColor;

        BufferManager::UploadBufferData(m_HighlightPSBufferPtr, &SelectionSettings);

        // -----------------------------------------------------------------------------
        // Render
        // -----------------------------------------------------------------------------
        RenderHighlightSurfaces();

        RenderHighlightProbes();

        // -----------------------------------------------------------------------------
        // Exit renderer
        // -----------------------------------------------------------------------------
        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetConstantBuffer(2);

        ContextManager::ResetTexture(0);

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

    void CGfxSelectionRenderer::RenderHighlightSurfaces()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(m_Settings.m_HighlightUseWireframe == true ? CRasterizerState::Wireframe : 0));

        ContextManager::SetShaderPS(m_HighlightPSPtr);

        CSurfaceRenderJobs::const_iterator CurrentSurfaceRenderJob = m_SurfaceRenderJobs.begin();
        CSurfaceRenderJobs::const_iterator EndOfSurfaceRenderJobs = m_SurfaceRenderJobs.end();

        for (; CurrentSurfaceRenderJob != EndOfSurfaceRenderJobs; ++CurrentSurfaceRenderJob)
        {
            CSurfacePtr  SurfacePtr = CurrentSurfaceRenderJob->m_SurfacePtr;

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = CurrentSurfaceRenderJob->m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::RenderHighlightProbes()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(0));

        ContextManager::SetShaderPS(m_TextureCubePSPtr);

        CProbeRenderJobs::const_iterator CurrentProbeRenderJob = m_ProbeRenderJobs.begin();
        CProbeRenderJobs::const_iterator EndOfProbeRenderJobs  = m_ProbeRenderJobs.end();

        for (; CurrentProbeRenderJob != EndOfProbeRenderJobs; ++CurrentProbeRenderJob)
        {
            CSurfacePtr SurfacePtr = m_SphereMeshPtr->GetLOD(0)->GetSurface();

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix = CurrentProbeRenderJob->m_ModelMatrix;

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

            ContextManager::SetTexture(0, CurrentProbeRenderJob->m_TextureCubePtr);

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
        }

        // -----------------------------------------------------------------------------

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Wireframe));

        ContextManager::SetShaderPS(m_HighlightPSPtr);

        CurrentProbeRenderJob = m_ProbeRenderJobs.begin();
        EndOfProbeRenderJobs  = m_ProbeRenderJobs.end();

        for (; CurrentProbeRenderJob != EndOfProbeRenderJobs; ++CurrentProbeRenderJob)
        {
            CSurfacePtr SurfacePtr = m_BoxMeshPtr->GetLOD(0)->GetSurface();

            // -----------------------------------------------------------------------------
            // Upload data to buffer
            // -----------------------------------------------------------------------------
            SPerDrawCallConstantBufferVS ModelBuffer;

            ModelBuffer.m_ModelMatrix  = CurrentProbeRenderJob->m_ModelMatrix;
            ModelBuffer.m_ModelMatrix *= glm::scale(CurrentProbeRenderJob->m_pDtProbeFacet->GetBoxSize());

            BufferManager::UploadBufferData(m_ModelBufferPtr, &ModelBuffer);

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            ContextManager::SetShaderVS(SurfacePtr->GetShaderVS());

            // -----------------------------------------------------------------------------
            // Set items to context manager
            // -----------------------------------------------------------------------------
            ContextManager::SetVertexBuffer(SurfacePtr->GetVertexBuffer());

            ContextManager::SetIndexBuffer(SurfacePtr->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(SurfacePtr->GetShaderVS()->GetInputLayout());

            ContextManager::DrawIndexed(SurfacePtr->GetNumberOfIndices(), 0, 0);
        }
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
                if (rTicket.m_Flags & SPickFlag::Actor) MeshRenderer::RenderHitProxy();
                if (rTicket.m_Flags & SPickFlag::AR)    ARRenderer   ::RenderHitProxy();

                // -----------------------------------------------------------------------------
                // Setup buffer
                // -----------------------------------------------------------------------------
                glm::ivec2 ActiveWindowSize = Gfx::Main::GetActiveNativeWindowSize();

                SSelectionSettingsBuffer Settings;

                MinX = rRequest.m_Cursor[0] + rTicket.m_OffsetX;
                MinY = rRequest.m_Cursor[1] + rTicket.m_OffsetY;
                MaxX = MinX + rTicket.m_SizeX;
                MaxY = MinY + rTicket.m_SizeY;

                if (MaxX > static_cast<unsigned int>(ActiveWindowSize[0])) MaxX = ActiveWindowSize[0];
                if (MaxY > static_cast<unsigned int>(ActiveWindowSize[1])) MaxY = ActiveWindowSize[1];
                if (MinX > MaxX) MinX = 0;
                if (MinY > MaxY) MinY = 0;

                Settings.m_MinMaxUV.x = static_cast<float>(MinX) / static_cast<float>(ActiveWindowSize[0]);
                Settings.m_MinMaxUV.y = static_cast<float>(MinY) / static_cast<float>(ActiveWindowSize[1]);
                Settings.m_MinMaxUV.z = static_cast<float>(MaxX) / static_cast<float>(ActiveWindowSize[0]);
                Settings.m_MinMaxUV.w = static_cast<float>(MaxY) / static_cast<float>(ActiveWindowSize[1]);

                BufferManager::UploadBufferData(m_SelectionBufferSetPtrs[IndexOfBuffer]->GetBuffer(0), &Settings);

                // -----------------------------------------------------------------------------
                // Execute
                // -----------------------------------------------------------------------------
                ContextManager::SetShaderCS(m_SelectionCSPtr);

                ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

                ContextManager::SetResourceBuffer(0, m_SelectionBufferSetPtrs[IndexOfBuffer]->GetBuffer(0));
                ContextManager::SetResourceBuffer(1, m_SelectionBufferSetPtrs[IndexOfBuffer]->GetBuffer(1));

                ContextManager::SetTexture(0, TargetSetManager::GetDeferredTargetSet()->GetDepthStencilTarget());

                ContextManager::SetImageTexture(0, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(0));
                ContextManager::SetImageTexture(1, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(1));
                ContextManager::SetImageTexture(2, TargetSetManager::GetDeferredTargetSet()->GetRenderTarget(2));
                ContextManager::SetImageTexture(3, TargetSetManager::GetHitProxyTargetSet()->GetRenderTarget(0));

                ContextManager::Dispatch(rTicket.m_SizeX, rTicket.m_SizeY, 1);

                ContextManager::ResetImageTexture(0);
                ContextManager::ResetImageTexture(1);
                ContextManager::ResetImageTexture(2);
                ContextManager::ResetImageTexture(3);

                ContextManager::ResetTexture(0);

                ContextManager::ResetResourceBuffer(0);
                ContextManager::ResetResourceBuffer(1);

                ContextManager::ResetConstantBuffer(0);

                ContextManager::ResetShaderCS();

                // -----------------------------------------------------------------------------
                // Clear hit proxy target set
                // -----------------------------------------------------------------------------
                TargetSetManager::ClearTargetSet(TargetSetManager::GetHitProxyTargetSet(), glm::vec4(0.0f));

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
        m_SurfaceRenderJobs.clear();
        m_ProbeRenderJobs  .clear();

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

            if (_pEntity->GetComponentFacet()->HasComponent<Dt::CMeshComponent>())
            {
                CMeshComponent* pGfxComponent = CComponentManager::GetInstance().GetComponent<CMeshComponent>(_pEntity->GetComponentFacet()->GetComponent<Dt::CMeshComponent>()->GetID());

                assert(pGfxComponent != nullptr);

                CMeshPtr MeshPtr = pGfxComponent->GetMesh();

                assert(MeshPtr.IsValid());

                // -----------------------------------------------------------------------------
                // Set every surface of this entity into a new render job
                // -----------------------------------------------------------------------------
                CSurfacePtr SurfacePtr = MeshPtr->GetLOD(0)->GetSurface();

                CMaterialPtr MaterialPtr;

                if (pGfxComponent->GetMaterial() != 0)
                {
                    MaterialPtr = pGfxComponent->GetMaterial();
                }
                else
                {
                    MaterialPtr = SurfacePtr->GetMaterial();
                }

                assert(MaterialPtr != 0 && MaterialPtr.IsValid());

                // -----------------------------------------------------------------------------
                // Set informations to render job
                // -----------------------------------------------------------------------------
                SSurfaceRenderJob NewRenderJob;

                NewRenderJob.m_SurfacePtr  = SurfacePtr;
                NewRenderJob.m_ModelMatrix = _pEntity->GetTransformationFacet()->GetWorldMatrix();

                m_SurfaceRenderJobs.push_back(NewRenderJob);
            }
            else if (_pEntity->GetComponentFacet()->HasComponent<Dt::CLightProbeComponent>())
            {
                Dt::CLightProbeComponent*  pDataComponent = _pEntity->GetComponentFacet()->GetComponent<Dt::CLightProbeComponent>();
                Gfx::CLightProbeComponent* pGfxComponent  = CComponentManager::GetInstance().GetComponent<Gfx::CLightProbeComponent>(pDataComponent->GetID());

                assert(pGfxComponent != nullptr);

                // -----------------------------------------------------------------------------
                // Set informations to render job
                // -----------------------------------------------------------------------------
                SProbeRenderJob NewRenderJob;

                NewRenderJob.m_ModelMatrix    = _pEntity->GetTransformationFacet()->GetWorldMatrix();
                NewRenderJob.m_TextureCubePtr = pGfxComponent->GetSpecularPtr();
                NewRenderJob.m_pDtProbeFacet  = pDataComponent;

                m_ProbeRenderJobs.push_back(NewRenderJob);
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

                if (pSiblingEntity->IsActive())
                {
                    AddEntityToJobs(pSiblingEntity);
                }

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

    void SelectEntity(Base::ID _EntityID)
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

    void PushPick(CSelectionTicket& _rTicket, const glm::ivec2& _rCursor)
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

    // -----------------------------------------------------------------------------

    void ResetSettings()
    {
        CGfxSelectionRenderer::GetInstance().ResetSettings();
    }

    // -----------------------------------------------------------------------------

    void SetSettings(const SSelectionSettings& _rSettings)
    {
        CGfxSelectionRenderer::GetInstance().SetSettings(_rSettings);
    }

    // -----------------------------------------------------------------------------

    const SSelectionSettings& GetSettings()
    {
        return CGfxSelectionRenderer::GetInstance().GetSettings();
    }
} // namespace SelectionRenderer
} // namespace Gfx
