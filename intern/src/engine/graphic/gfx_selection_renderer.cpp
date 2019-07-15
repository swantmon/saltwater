
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_math_limits.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/data/data_entity_manager.h"

#include "engine/graphic/gfx_ar_renderer.h"
#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_debug_renderer.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_material.h"
#include "engine/graphic/gfx_mesh.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_mesh_renderer.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_reconstruction_renderer.h"
#include "engine/graphic/gfx_selection_renderer.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

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

        CSelectionTicket& AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY, unsigned int _Flags = SPickFlag::Nothing);
        void ReleaseTicket(CSelectionTicket& _rTicket);

        void PushPick(CSelectionTicket& _rTicket, const glm::ivec2& _rCursor);
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
            glm::vec4    m_WSPosition;
            glm::vec4    m_WSNormal;
            float        m_Depth;
            unsigned int m_EntityID;
        };

        struct SRequest
        {
            glm::ivec2   m_Cursor;
            Base::U64    m_TimeStamp;
            CScissorRect m_ScissorRect;
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

        struct SSelectionSettingsBuffer
        {
            glm::ivec4 m_PositionSize;
        };
        
    private:
        
        std::array<CBufferPtr, s_MaxNumberOfBuffer> m_SelectionSettingsBufferPtrs;
        std::array<CBufferPtr, s_MaxNumberOfBuffer> m_SelectionOutputBufferPtrs;

        CShaderPtr m_SelectionCSPtr;

        CTextureSetPtr m_GBufferTextureSetPtr;

        std::array<CInternSelectionTicket, s_MaxNumberOfTickets> m_SelectionTickets;
        
    private:

        void ResetTickets();
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
        : m_SelectionSettingsBufferPtrs()
        , m_SelectionOutputBufferPtrs  ()
        , m_GBufferTextureSetPtr       ()
        , m_SelectionTickets           ()
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
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnExit()
    {
        m_SelectionCSPtr       = nullptr;
        m_GBufferTextureSetPtr = nullptr;

        ResetTickets();

        // -----------------------------------------------------------------------------
        // Reset buffer
        // -----------------------------------------------------------------------------
        for (auto& rBuffer : m_SelectionOutputBufferPtrs) rBuffer = nullptr;
        for (auto& rBuffer : m_SelectionSettingsBufferPtrs) rBuffer = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::OnSetupShader()
    {
        m_SelectionCSPtr = ShaderManager::CompileCS("picking/cs_selection.glsl", "main");
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

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSelectionSettingsBuffer);
        ConstanteBufferDesc.m_pBytes        = nullptr;
        ConstanteBufferDesc.m_pClassKey     = nullptr;

        for (auto& rBuffer : m_SelectionSettingsBufferPtrs)
        {
            rBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        }

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUToCPU;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPURead;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SSelectionOutput);
        ConstanteBufferDesc.m_pBytes        = nullptr;
        ConstanteBufferDesc.m_pClassKey     = nullptr;

        for (auto& rBuffer : m_SelectionOutputBufferPtrs)
        {
            rBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        }
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
    
    void CGfxSelectionRenderer::Update()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxSelectionRenderer::Render()
    {
        Performance::BeginEvent("Picking");

        int IndexOfTicket = 0;

        for (auto& rTicket : m_SelectionTickets)
        {
            if (rTicket.m_IsValid && (rTicket.m_NumberOfRequests > 0) && (rTicket.m_Frame <= Core::Time::GetNumberOfFrame()))
            {
                int IndexOfLastRequest = (rTicket.m_IndexOfPushRequest > 0) ? rTicket.m_IndexOfPushRequest - 1 : CInternSelectionTicket::s_MaxNumberOfRequests - 1;
                int IndexOfBuffer      = IndexOfTicket * CInternSelectionTicket::s_MaxNumberOfRequests + IndexOfLastRequest;

                assert (IndexOfBuffer < s_MaxNumberOfBuffer);

                SRequest& rRequest = rTicket.m_Requests[IndexOfLastRequest];

                // -----------------------------------------------------------------------------
                // Render hit proxies depending on flag
                // -----------------------------------------------------------------------------
                Performance::BeginEvent("Renderer");

                ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());
                ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::EqualDepth));
                ContextManager::SetTargetSet(TargetSetManager::GetHitProxyTargetSet());
                ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));
                ContextManager::SetScissorRect(rRequest.m_ScissorRect);

                if (rTicket.m_Flags & SPickFlag::Mesh)  MeshRenderer::RenderHitProxy();
                if (rTicket.m_Flags & SPickFlag::AR)    ARRenderer::RenderHitProxy();
                if (rTicket.m_Flags & SPickFlag::Voxel) ReconstructionRenderer::RenderHitProxy();

                ContextManager::ResetRenderContext();

                ContextManager::ResetShaderVS();
                ContextManager::ResetShaderPS();

                Performance::EndEvent();

                // -----------------------------------------------------------------------------
                // Result
                // -----------------------------------------------------------------------------
                Performance::BeginEvent("Result");

                glm::ivec2 ActiveWindowSize = Gfx::Main::GetActiveNativeWindowSize();

                SSelectionSettingsBuffer Settings;

                Settings.m_PositionSize.x = rRequest.m_Cursor.x + rTicket.m_OffsetX;
                Settings.m_PositionSize.y = ActiveWindowSize.y - rRequest.m_Cursor.y + rTicket.m_OffsetY;
                Settings.m_PositionSize.z = rTicket.m_SizeX;
                Settings.m_PositionSize.w = rTicket.m_SizeY;

                BufferManager::UploadBufferData(m_SelectionSettingsBufferPtrs[IndexOfBuffer], &Settings);

                ContextManager::SetShaderCS(m_SelectionCSPtr);

                ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

                ContextManager::SetResourceBuffer(0, m_SelectionSettingsBufferPtrs[IndexOfBuffer]);
                ContextManager::SetResourceBuffer(1, m_SelectionOutputBufferPtrs[IndexOfBuffer]);

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

                Performance::EndEvent();

                // -----------------------------------------------------------------------------
                // Set request
                // -----------------------------------------------------------------------------
                rRequest.m_TimeStamp = Core::Time::GetNumberOfFrame();
            }

            ++IndexOfTicket;
        }

        Performance::EndEvent();
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

        Clear(rTicket);

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
        rTicket.m_pObject       = nullptr;
        rTicket.m_Frame         = Core::Time::GetNumberOfFrame() - 1;

        return rTicket;
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::ReleaseTicket(CSelectionTicket& _rTicket)
    {
        auto& rTicket = static_cast<CInternSelectionTicket&>(_rTicket);

        rTicket.m_IsValid = false;
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::PushPick(CSelectionTicket& _rTicket, const glm::ivec2& _rCursor)
    {
        auto& rTicket = static_cast<CInternSelectionTicket&>(_rTicket);

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

        if ((_rCursor[0] < 0) || (_rCursor[1] < 0) || (_rCursor[0] >= ActiveWindowSize[0]) || (_rCursor[1] >= ActiveWindowSize[1]))
        {
            return;
        }

        // -----------------------------------------------------------------------------
        // Setup request
        // -----------------------------------------------------------------------------
        SRequest& rRequest = rTicket.m_Requests[rTicket.m_IndexOfPushRequest];

        rRequest.m_Cursor      = _rCursor;
        rRequest.m_TimeStamp   = Base::SLimits<Base::U64>::s_Max;

        rRequest.m_ScissorRect.SetTopLeftX(_rCursor.x + rTicket.m_OffsetX);
        rRequest.m_ScissorRect.SetTopLeftY(ActiveWindowSize.y - _rCursor.y + rTicket.m_OffsetY);
        rRequest.m_ScissorRect.SetWidth(rTicket.m_SizeX);
        rRequest.m_ScissorRect.SetHeight(rTicket.m_SizeY);

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

        auto& rTicket = static_cast<CInternSelectionTicket&>(_rTicket);

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

        SSelectionOutput* pOutput = static_cast<SSelectionOutput*>(BufferManager::MapBuffer(m_SelectionOutputBufferPtrs[IndexOfBuffer], CBuffer::Read));

        rTicket.m_WSPosition = glm::vec3(pOutput->m_WSPosition[0], pOutput->m_WSPosition[1], pOutput->m_WSPosition[2]);
        rTicket.m_WSNormal   = glm::vec3(pOutput->m_WSNormal[0], pOutput->m_WSNormal[1], pOutput->m_WSNormal[2]);
        rTicket.m_Depth      = pOutput->m_Depth;
        rTicket.m_HitFlag    = SHitFlag::Nothing;
        rTicket.m_pObject    = nullptr;

        if (pOutput->m_EntityID > 0)
        {
            rTicket.m_HitFlag = SHitFlag::Entity;

            rTicket.m_pObject = Dt::CEntityManager::GetInstance().GetEntityByID(pOutput->m_EntityID);
        }

        BufferManager::UnmapBuffer(m_SelectionOutputBufferPtrs[IndexOfBuffer]);

        return true;
    }

    // -----------------------------------------------------------------------------

    void CGfxSelectionRenderer::Clear(CSelectionTicket& _rTicket)
    {
        auto& rTicket = static_cast<CInternSelectionTicket&>(_rTicket);

        rTicket.m_IsValid            = false;
        rTicket.m_IndexOfPushRequest = 0;
        rTicket.m_IndexOfPopRequest  = 0;
        rTicket.m_NumberOfRequests   = 0;
    }

    // -----------------------------------------------------------------------------

    bool CGfxSelectionRenderer::IsEmpty(const CSelectionTicket& _rTicket)
    {
        const auto& rTicket = static_cast<const CInternSelectionTicket&>(_rTicket);

        return rTicket.m_NumberOfRequests == 0;
    }

    // -----------------------------------------------------------------------------

    bool CGfxSelectionRenderer::IsValid(const CSelectionTicket& _rTicket)
    {
        const auto& rTicket = static_cast<const CInternSelectionTicket&>(_rTicket);

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
} // namespace SelectionRenderer
} // namespace Gfx
