
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_native_buffer.h"
#include "graphic/gfx_native_buffer_set.h"
#include "graphic/gfx_native_sampler.h"
#include "graphic/gfx_native_shader.h"
#include "graphic/gfx_native_target_set.h"
#include "graphic/gfx_native_texture_2d.h"
#include "graphic/gfx_state_manager.h"

#include <assert.h>

#include "GL/glew.h"

using namespace Gfx;

namespace
{
    class CGfxContextManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxContextManager)
        
    public:

        CGfxContextManager();
       ~CGfxContextManager();

    public:

        void OnStart();
        void OnExit();
    
    public:
    
        CRenderContextPtr CreateRenderContext();
        
        void ResetRenderContext();
        void SetRenderContext(CRenderContextPtr _RenderContextPtr);
        
        void SetRenderFlags(unsigned int _Flags);
        unsigned int GetRenderFlags();

    public:

        void ResetBlendState();
        void SetBlendState(CBlendStatePtr _StatePtr);
        CBlendStatePtr GetBlendState();

        void ResetDepthStencilState();
        void SetDepthStencilState(CDepthStencilStatePtr _StatePtr);
        CDepthStencilStatePtr GetDepthStencilState();

        void ResetRasterizerState();
        void SetRasterizerState(CRasterizerStatePtr _StatePtr);
        CRasterizerStatePtr GetRasterizerState();

    public:

        void ResetTopology();
        void SetTopology(STopology::Enum _Topology);
        STopology::Enum GetTopology() const;

        void ResetIndexBuffer();
        void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Offset);
        void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Stride, unsigned int _Offset);
        CBufferPtr GetIndexBuffer();

        void ResetVertexBufferSet();
        void SetVertexBufferSet(CBufferSetPtr _BufferSetPtr, const unsigned int* _pOffsets);
        void SetVertexBufferSet(CBufferSetPtr _BufferSetPtr, const unsigned int* _pStrides, const unsigned int* _pOffsets);
        CBufferSetPtr GetVertexBufferSet();

        void ResetConstantBufferSetVS();
        void SetConstantBufferSetVS(CBufferSetPtr _BufferSetPtr);
        CBufferSetPtr GetConstantBufferSetVS();
        void ResetConstantBufferSetHS();
        void SetConstantBufferSetHS(CBufferSetPtr _BufferSetPtr);
        CBufferSetPtr GetConstantBufferSetHS();
        void ResetConstantBufferSetDS();
        void SetConstantBufferSetDS(CBufferSetPtr _BufferSetPtr);
        CBufferSetPtr GetConstantBufferSetDS();
        void ResetConstantBufferSetGS();
        void SetConstantBufferSetGS(CBufferSetPtr _BufferSetPtr);
        CBufferSetPtr GetConstantBufferSetGS();
        void ResetConstantBufferSetPS();
        void SetConstantBufferSetPS(CBufferSetPtr _BufferSetPtr);
        CBufferSetPtr GetConstantBufferSetPS();
        void ResetConstantBufferSetCS();
        void SetConstantBufferSetCS(CBufferSetPtr _BufferSetPtr);
        CBufferSetPtr GetConstantBufferSetCS();

    public:

        void ResetInputLayout();
        void SetInputLayout(CInputLayoutPtr _InputLayoutPtr);
        CInputLayoutPtr GetInputLayout();

    public:

        void ResetShaderVS();
        void SetShaderVS(CShaderPtr _ShaderSetPtr);
        CShaderPtr GetShaderVS();
        void ResetShaderHS();
        void SetShaderHS(CShaderPtr _ShaderSetPtr);
        CShaderPtr GetShaderHS();
        void ResetShaderDS();
        void SetShaderDS(CShaderPtr _ShaderSetPtr);
        CShaderPtr GetShaderDS();
        void ResetShaderGS();
        void SetShaderGS(CShaderPtr _ShaderSetPtr);
        CShaderPtr GetShaderGS();
        void ResetShaderPS();
        void SetShaderPS(CShaderPtr _ShaderSetPtr);
        CShaderPtr GetShaderPS();
        void ResetShaderCS();
        void SetShaderCS(CShaderPtr _ShaderSetPtr);
        CShaderPtr GetShaderCS();

        void ResetSamplerSetVS();
        void SetSamplerSetVS(CSamplerSetPtr _SamplerSetPtr);
        CSamplerSetPtr GetSamplerSetVS();
        void ResetSamplerSetHS();
        void SetSamplerSetHS(CSamplerSetPtr _SamplerSetPtr);
        CSamplerSetPtr GetSamplerSetHS();
        void ResetSamplerSetDS();
        void SetSamplerSetDS(CSamplerSetPtr _SamplerSetPtr);
        CSamplerSetPtr GetSamplerSetDS();
        void ResetSamplerSetGS();
        void SetSamplerSetGS(CSamplerSetPtr _SamplerSetPtr);
        CSamplerSetPtr GetSamplerSetGS();
        void ResetSamplerSetPS();
        void SetSamplerSetPS(CSamplerSetPtr _SamplerSetPtr);
        CSamplerSetPtr GetSamplerSetPS();
        void ResetSamplerSetCS();
        void SetSamplerSetCS(CSamplerSetPtr _SamplerSetPtr);
        CSamplerSetPtr GetSamplerSetCS();

        void ResetTextureSetVS();
        void SetTextureSetVS(CTextureSetPtr _TextureSetPtr);
        CTextureSetPtr GetTextureSetVS();
        void ResetTextureSetHS();
        void SetTextureSetHS(CTextureSetPtr _TextureSetPtr);
        CTextureSetPtr GetTextureSetHS();
        void ResetTextureSetDS();
        void SetTextureSetDS(CTextureSetPtr _TextureSetPtr);
        CTextureSetPtr GetTextureSetDS();
        void ResetTextureSetGS();
        void SetTextureSetGS(CTextureSetPtr _TextureSetPtr);
        CTextureSetPtr GetTextureSetGS();
        void ResetTextureSetPS();
        void SetTextureSetPS(CTextureSetPtr _TextureSetPtr);
        CTextureSetPtr GetTextureSetPS();
        void ResetTextureSetCS();
        void SetTextureSetCS(CTextureSetPtr _TextureSetPtr);
        CTextureSetPtr GetTextureSetCS();

        void ResetTargetSet();
        void SetTargetSet(CTargetSetPtr _TargetSetPtr);
        CTargetSetPtr GetTargetSet();

    public:

        void ResetViewPortSet();
        void SetViewPortSet(CViewPortSetPtr _ViewPortSetPtr);
        CViewPortSetPtr GetViewPortSet();

    public:

        void Draw(unsigned int _NumberOfVertices, unsigned int _IndexOfFirstVertex);
        void DrawIndexed(unsigned int _NumberOfIndices, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation);
        void DrawIndexedInstanced(unsigned int _NumberOfIndices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation, unsigned int _StartInstanceLocation);
        
    public:
        
        void Dispatch(unsigned int _NumberOfThreadGroupsX, unsigned int _NumberOfThreadGroupsY, unsigned int _NumberOfThreadGroupsZ);
    
    private:
    
        static const GLenum s_NativeTopologies[];
        
    private:
        
        class CInternRenderContext : public Gfx::CRenderContext
        {
        public:
            CInternRenderContext();
            ~CInternRenderContext();
            
        private:
            
            friend class CGfxContextManager;
        };
        
    private:
        
        typedef Base::CManagedPool<CInternRenderContext> CRenderContexts;
        typedef CRenderContexts::CIterator               CRenderContextIterator;

    private:

        unsigned int           m_RenderFlags;
        CBlendStatePtr         m_BlendStatePtr;
        CDepthStencilStatePtr  m_DepthStencilStatePtr;
        CRasterizerStatePtr    m_RasterizerStatePtr;
        int                    m_Topology;
        CBufferPtr             m_IndexBufferPtr;
        unsigned int           m_IndexBufferStride;
        unsigned int           m_IndexBufferOffset;
        unsigned int           m_NumberOfVertexBuffers;
        unsigned int           m_VertexBufferStrides[CBufferSet::s_MaxNumberOfBuffers];
        unsigned int           m_VertexBufferOffsets[CBufferSet::s_MaxNumberOfBuffers];
        CBufferSetPtr          m_VertexBufferSetPtr;
        CBufferSetPtr          m_ConstantBufferSetVSPtr;
        CBufferSetPtr          m_ConstantBufferSetGSPtr;
        CBufferSetPtr          m_ConstantBufferSetDSPtr;
        CBufferSetPtr          m_ConstantBufferSetHSPtr;
        CBufferSetPtr          m_ConstantBufferSetPSPtr;
        CBufferSetPtr          m_ConstantBufferSetCSPtr;
        CInputLayoutPtr        m_InputLayoutPtr;
        CShaderPtr             m_ShaderVSPtr;
        CShaderPtr             m_ShaderGSPtr;
        CShaderPtr             m_ShaderDSPtr;
        CShaderPtr             m_ShaderHSPtr;
        CShaderPtr             m_ShaderPSPtr;
        CShaderPtr             m_ShaderCSPtr;
        CSamplerSetPtr         m_SamplerSetVSPtr;
        CSamplerSetPtr         m_SamplerSetGSPtr;
        CSamplerSetPtr         m_SamplerSetDSPtr;
        CSamplerSetPtr         m_SamplerSetHSPtr;
        CSamplerSetPtr         m_SamplerSetPSPtr;
        CSamplerSetPtr         m_SamplerSetCSPtr;
        CTextureSetPtr         m_TextureSetVSPtr;
        CTextureSetPtr         m_TextureSetGSPtr;
        CTextureSetPtr         m_TextureSetDSPtr;
        CTextureSetPtr         m_TextureSetHSPtr;
        CTextureSetPtr         m_TextureSetPSPtr;
        CTextureSetPtr         m_TextureSetCSPtr;
        CTargetSetPtr          m_TargetSetPtr;;
        CViewPortSetPtr        m_ViewPortSetPtr;
        CRenderContexts        m_RenderContexts;
        
        GLuint                 m_NativeShaderPipeline;
        
        unsigned int           m_IndexOfTextureBinding;

    private:

        bool ContainsNewStridesOrOffsets(const unsigned int* _pStrides, const unsigned int* _pOffsets) const;
    
        int ConvertInputLayoutFormat(Gfx::CInputLayout::EFormat _Format) const;
        int ConvertInputLayoutFormatSize(Gfx::CInputLayout::EFormat _Format) const;

        void CreateShaderPipeline();
        void DestroyShaderPipeline();
    };
} // namespace

namespace
{
    const GLenum CGfxContextManager::s_NativeTopologies[] =
    {
        GL_POINTS,
        GL_LINES,
        GL_LINE_STRIP,
        GL_LINE_LOOP,
        GL_TRIANGLES,
        GL_TRIANGLE_STRIP,
        GL_TRIANGLE_FAN,
        GL_QUADS,
        GL_QUAD_STRIP,
        GL_POLYGON,
        GL_PATCHES,
    };
} // namespace

namespace
{
    CGfxContextManager::CGfxContextManager()
        : m_RenderFlags           (0)
        , m_BlendStatePtr         ()
        , m_DepthStencilStatePtr  ()
        , m_RasterizerStatePtr    ()
        , m_Topology              (STopology::Undefined)
        , m_IndexBufferPtr        ()
        , m_IndexBufferStride     (0)
        , m_IndexBufferOffset     (0)
        , m_NumberOfVertexBuffers (0)
        , m_VertexBufferSetPtr    ()
        , m_ConstantBufferSetVSPtr()
        , m_ConstantBufferSetGSPtr()
        , m_ConstantBufferSetDSPtr()
        , m_ConstantBufferSetHSPtr()
        , m_ConstantBufferSetPSPtr()
        , m_ConstantBufferSetCSPtr()
        , m_InputLayoutPtr        ()
        , m_ShaderVSPtr           ()
        , m_ShaderGSPtr           ()
        , m_ShaderDSPtr           ()
        , m_ShaderHSPtr           ()
        , m_ShaderPSPtr           ()
        , m_ShaderCSPtr           ()
        , m_SamplerSetVSPtr       ()
        , m_SamplerSetGSPtr       ()
        , m_SamplerSetDSPtr       ()
        , m_SamplerSetHSPtr       ()
        , m_SamplerSetPSPtr       ()
        , m_SamplerSetCSPtr       ()
        , m_TextureSetVSPtr       ()
        , m_TextureSetGSPtr       ()
        , m_TextureSetDSPtr       ()
        , m_TextureSetHSPtr       ()
        , m_TextureSetPSPtr       ()
        , m_TextureSetCSPtr       ()
        , m_TargetSetPtr          ()
        , m_ViewPortSetPtr        ()
        , m_RenderContexts        ()
        , m_NativeShaderPipeline  (0)
        , m_IndexOfTextureBinding (0)
    {
        unsigned int IndexOfVertexBuffer;

        for (IndexOfVertexBuffer = 0; IndexOfVertexBuffer < CBufferSet::s_MaxNumberOfBuffers; ++ IndexOfVertexBuffer)
        {
            m_VertexBufferStrides[IndexOfVertexBuffer] = 0;
            m_VertexBufferOffsets[IndexOfVertexBuffer] = 0;
        }
    }

    // -----------------------------------------------------------------------------

    CGfxContextManager::~CGfxContextManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Setup program pipeline for shader
        // -----------------------------------------------------------------------------
        CreateShaderPipeline();

        // -----------------------------------------------------------------------------
        // Set default states
        // -----------------------------------------------------------------------------
        CBlendStatePtr        BlendStatePtr        = StateManager::GetBlendState       (0);
        CDepthStencilStatePtr DepthStencilStatePtr = StateManager::GetDepthStencilState(0);
        CRasterizerStatePtr   RasterizerStatePtr   = StateManager::GetRasterizerState  (0);

        SetBlendState       (BlendStatePtr);
        SetDepthStencilState(DepthStencilStatePtr);
        SetRasterizerState  (RasterizerStatePtr);

        // -----------------------------------------------------------------------------
        // Bind a default simple vertex shader to program pipeline (prevents from 
        // errors executing an dispatch on pipeline without vertex shader).
        // -----------------------------------------------------------------------------        
        unsigned int ProgramHandle = 0;

        const char* pShader = "#version 430 \n void main(void) { }";

        ProgramHandle = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &pShader);

        glUseProgramStages(m_NativeShaderPipeline, GL_VERTEX_SHADER_BIT, ProgramHandle);

        glDeleteProgram(ProgramHandle);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::OnExit()
    {
        // -----------------------------------------------------------------------------
        // Clear render context
        // -----------------------------------------------------------------------------
        m_RenderContexts.Clear();

        m_RenderFlags = 0;
        
        // -----------------------------------------------------------------------------
        // The context holds a reference to the states so release them.
        // -----------------------------------------------------------------------------
        ResetBlendState();
        ResetDepthStencilState();
        ResetRasterizerState();
        ResetTopology();
        ResetIndexBuffer();
        ResetVertexBufferSet();
        ResetConstantBufferSetVS();
        ResetConstantBufferSetGS();
        ResetConstantBufferSetDS();
        ResetConstantBufferSetHS();
        ResetConstantBufferSetPS();
        ResetConstantBufferSetCS();
        ResetInputLayout();
        ResetShaderVS();
        ResetShaderGS();
        ResetShaderDS();
        ResetShaderHS();
        ResetShaderPS();
        ResetShaderCS();
        ResetSamplerSetVS();
        ResetSamplerSetGS();
        ResetSamplerSetDS();
        ResetSamplerSetHS();
        ResetSamplerSetPS();
        ResetSamplerSetCS();
        ResetTextureSetVS();
        ResetTextureSetGS();
        ResetTextureSetDS();
        ResetTextureSetHS();
        ResetTextureSetPS();
        ResetTextureSetCS();
        ResetTargetSet();
        ResetViewPortSet();

        m_BlendStatePtr        = nullptr;
        m_DepthStencilStatePtr = nullptr;
        m_RasterizerStatePtr   = nullptr;

        DestroyShaderPipeline();
    }
    
    // -----------------------------------------------------------------------------
    
    CRenderContextPtr CGfxContextManager::CreateRenderContext()
    {
        CRenderContexts::CPtr RenderContext = m_RenderContexts.Allocate();
        
        return CRenderContextPtr(RenderContext);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetRenderContext()
    {
        ResetTargetSet();
        ResetViewPortSet();
        ResetBlendState();
        ResetDepthStencilState();
        ResetRasterizerState();
    }
    
    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetRenderContext(CRenderContextPtr _RenderContextPtr)
    {
        assert(_RenderContextPtr != nullptr);
        
        SetTargetSet(_RenderContextPtr->GetTargetSet());
        SetViewPortSet(_RenderContextPtr->GetViewPortSet());
        
        CRenderStatePtr RenderStatePtr = _RenderContextPtr->GetRenderState();
        
        if (m_RenderFlags != 0)
        {
            int ModifiedFlag = RenderStatePtr->GetFlags() | m_RenderFlags;
            
            RenderStatePtr = StateManager::GetRenderState(ModifiedFlag);
        }
        
        assert(RenderStatePtr != nullptr);
        
        SetBlendState       (RenderStatePtr->GetBlendState());
        SetDepthStencilState(RenderStatePtr->GetDepthStencilState());
        SetRasterizerState  (RenderStatePtr->GetRasterizerState());
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetRenderFlags(unsigned int _Flags)
    {
        m_RenderFlags = _Flags;
    }
    
    // -----------------------------------------------------------------------------

    unsigned int CGfxContextManager::GetRenderFlags()
    {
        return m_RenderFlags;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetBlendState()
    {
        m_BlendStatePtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetBlendState(CBlendStatePtr _StatePtr)
    {
        assert(_StatePtr != nullptr);
        
        if (m_BlendStatePtr != _StatePtr)
        {
            const float* pBlendFactors = _StatePtr->GetBlendFactors();
            const SBlendDescription& rDescription = _StatePtr->GetDescription();
            
            for (unsigned int IndexOfRendertarget = 0; IndexOfRendertarget < s_MaxNumberOfRendertargets; ++ IndexOfRendertarget)
            {
                SRenderTargetBlendDescription RTDescription = rDescription.RenderTarget[IndexOfRendertarget];
                
                if (RTDescription.BlendEnable == GL_TRUE)
                {
                    // -----------------------------------------------------------------------------
                    // Informations from:
                    // https://www.opengl.org/wiki/Blending
                    // https://www.opengl.org/wiki/GLAPI/glBlendEquationSeparate
                    // https://www.opengl.org/wiki/GLAPI/glBlendFuncSeparate
                    // -----------------------------------------------------------------------------
                    
                    glEnablei(GL_BLEND, IndexOfRendertarget);
                    
                    glBlendEquationSeparatei(IndexOfRendertarget, RTDescription.BlendOp, RTDescription.BlendOp);
                    
                    glBlendFuncSeparatei(IndexOfRendertarget, RTDescription.SrcBlend, RTDescription.DestBlend, RTDescription.SrcBlendAlpha, RTDescription.DestBlendAlpha);
                }
                else
                {
                    glDisablei(GL_BLEND, IndexOfRendertarget);
                }
            }
            
            glBlendColor(pBlendFactors[0], pBlendFactors[1], pBlendFactors[2], pBlendFactors[3]);
            
            m_BlendStatePtr = _StatePtr;
        }
    }

    // -----------------------------------------------------------------------------

    CBlendStatePtr CGfxContextManager::GetBlendState()
    {
        return m_BlendStatePtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetDepthStencilState()
    {
        m_DepthStencilStatePtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetDepthStencilState(CDepthStencilStatePtr _StatePtr)
    {
        assert(_StatePtr != nullptr);
        
        if (m_DepthStencilStatePtr != _StatePtr)
        {
            const SDepthDescription& rDescription = _StatePtr->GetDescription();
        
            // -----------------------------------------------------------------------------
            // Set depth
            // -----------------------------------------------------------------------------
            if (rDescription.DepthEnable == GL_TRUE)
            {
                glEnable(GL_DEPTH_TEST);
                
                GLboolean DepthMask = static_cast<GLboolean>(rDescription.DepthWriteMask);
                GLenum    DepthFunc = rDescription.DepthFunc;
                
                glDepthFunc(DepthFunc);
                
                glDepthMask(DepthMask);
            }
            else
            {
                glDisable(GL_DEPTH_TEST);
            }

            // -----------------------------------------------------------------------------
            // Set stencil
            //
            // The glStencilFunc call is used to specify the conditions under which a fragment passes the stencil test. Its parameters are discussed below.
            //
            // func: The test function, can be GL_NEVER, GL_LESS, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_EQUAL, GL_NOTEQUAL, and GL_ALWAYS.
            // ref: A value to compare the stencil value to using the test function.
            // mask: A bitwise AND operation is performed on the stencil value and reference value with this mask value before comparing them.
            // -----------------------------------------------------------------------------
            if (rDescription.StencilEnable == GL_TRUE)
            {
                glEnable(GL_STENCIL_TEST);
                glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
                
                // -----------------------------------------------------------------------------
                
                GLenum StencilFunc;
                int    Mask;
                int    OpSFail;
                int    OpDpFail;
                int    OpDpPass;
                
                // -----------------------------------------------------------------------------
                
                glActiveStencilFaceEXT(GL_FRONT);
                
                StencilFunc = rDescription.FrontFace.StencilFunc;
                Mask        = rDescription.StencilReadMask;
                OpSFail     = rDescription.FrontFace.StencilFailOp;
                OpDpFail    = rDescription.FrontFace.StencilDepthFailOp;
                OpDpPass    = rDescription.FrontFace.StencilPassOp;
                
                glStencilOp(OpSFail, OpDpFail, OpDpPass);
                
                glStencilFunc(StencilFunc, 0, 0u);
                
                glStencilMask(Mask);
                
                // -----------------------------------------------------------------------------
                
                glActiveStencilFaceEXT(GL_BACK);
                
                StencilFunc = rDescription.BackFace.StencilFunc;
                Mask        = rDescription.StencilWriteMask;
                OpSFail     = rDescription.BackFace.StencilFailOp;
                OpDpFail    = rDescription.BackFace.StencilDepthFailOp;
                OpDpPass    = rDescription.BackFace.StencilPassOp;
                
                glStencilOp(OpSFail, OpDpFail, OpDpPass);
                
                glStencilFunc(StencilFunc, 0, 0u);
                
                glStencilMask(Mask);
            }
            else
            {
                glDisable(GL_STENCIL_TEST);
            }
            
            m_DepthStencilStatePtr = _StatePtr;
        }
    }

    // -----------------------------------------------------------------------------

    CDepthStencilStatePtr CGfxContextManager::GetDepthStencilState()
    {
        return m_DepthStencilStatePtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetRasterizerState()
    {
        glDisable(GL_CULL_FACE);
        
        m_RasterizerStatePtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetRasterizerState(CRasterizerStatePtr _StatePtr)
    {
        assert(_StatePtr != nullptr);
        
        if (m_RasterizerStatePtr != _StatePtr)
        {
            const SRasterizerDescription& rDescription = _StatePtr->GetDescription();
            
            if (rDescription.CullEnable == GL_TRUE)
            {
                int FillMode  = rDescription.FillMode;
                int CullMode  = rDescription.CullMode;
                int FrontFace = rDescription.FrontCounterClockwise;
                
                glEnable(GL_CULL_FACE);
                
                glCullFace(CullMode);
                
                glPolygonMode(GL_FRONT_AND_BACK, FillMode);
                
                glFrontFace(FrontFace);
            }
            else
            {
                glDisable(GL_CULL_FACE);
            }

            m_RasterizerStatePtr = _StatePtr;
        }
    }

    // -----------------------------------------------------------------------------

    CRasterizerStatePtr CGfxContextManager::GetRasterizerState()
    {
        return m_RasterizerStatePtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetTopology()
    {
        m_Topology = STopology::Undefined;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetTopology(STopology::Enum _Topology)
    {
        assert(_Topology != STopology::Undefined);

        if (m_Topology != _Topology)
        {
            m_Topology = _Topology;
        }
    }

    // -----------------------------------------------------------------------------

    STopology::Enum CGfxContextManager::GetTopology() const
    {
        return static_cast<STopology::Enum>(m_Topology);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetIndexBuffer()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        m_IndexBufferPtr    = nullptr;
        m_IndexBufferStride = 0;
        m_IndexBufferOffset = 0;
        
        m_IndexBufferPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Offset)
    {
        assert(_BufferPtr != nullptr);
        
        Gfx::CNativeBuffer& rNativeBuffer = *static_cast<Gfx::CNativeBuffer*>(_BufferPtr.GetPtr());
        
        const unsigned int Stride = rNativeBuffer.GetStride();
        
        if (m_IndexBufferPtr != _BufferPtr)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rNativeBuffer.m_NativeBuffer);

            m_IndexBufferPtr = _BufferPtr;
        }
        
        if (m_IndexBufferStride != Stride || m_IndexBufferOffset != _Offset)
        {
            m_IndexBufferStride =  Stride;
            m_IndexBufferOffset = _Offset;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Stride, unsigned int _Offset)
    {
        assert(_BufferPtr != nullptr);
        
        Gfx::CNativeBuffer& rNativeBuffer = *static_cast<Gfx::CNativeBuffer*>(_BufferPtr.GetPtr());
        
        if (m_IndexBufferPtr != _BufferPtr)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rNativeBuffer.m_NativeBuffer);

            m_IndexBufferPtr = _BufferPtr;
        }
        
        if (m_IndexBufferStride != _Stride || m_IndexBufferOffset != _Offset)
        {
            m_IndexBufferStride = _Stride;
            m_IndexBufferOffset = _Offset;
        }
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CGfxContextManager::GetIndexBuffer()
    {
        return m_IndexBufferPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetVertexBufferSet()
    {
        if (m_VertexBufferSetPtr == nullptr) return;
        
        glBindVertexArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        for (unsigned int IndexOfVertexBuffer = 0; IndexOfVertexBuffer < CBufferSet::s_MaxNumberOfBuffers; ++ IndexOfVertexBuffer)
        {
            m_VertexBufferStrides[IndexOfVertexBuffer] = 0;
            m_VertexBufferOffsets[IndexOfVertexBuffer] = 0;
        }
        
        m_VertexBufferSetPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetVertexBufferSet(CBufferSetPtr _BufferSetPtr, const unsigned int* _pOffsets)
    {
        BASE_UNUSED(_pOffsets);

        assert(_BufferSetPtr != nullptr);
        
        Gfx::CNativeBufferSet& rNativeBuffer = *static_cast<Gfx::CNativeBufferSet*>(_BufferSetPtr.GetPtr());
        
        unsigned int NumberOfBuffers = rNativeBuffer.GetNumberOfBuffers();
        
        if (m_VertexBufferSetPtr != _BufferSetPtr || m_NumberOfVertexBuffers != NumberOfBuffers)
        {            
            glBindVertexArray(rNativeBuffer.m_NativeBufferArrayHandle);

            for (unsigned int IndexOfBuffer = 0; IndexOfBuffer < NumberOfBuffers; ++ IndexOfBuffer)
            {
                Gfx::CNativeBuffer& rNativeSingleBuffer = *static_cast<Gfx::CNativeBuffer*>(_BufferSetPtr->GetBuffer(IndexOfBuffer).GetPtr());
                
                glBindBuffer(GL_ARRAY_BUFFER, rNativeSingleBuffer.m_NativeBuffer);
            }
            
            m_VertexBufferSetPtr    = _BufferSetPtr;
            m_NumberOfVertexBuffers = NumberOfBuffers;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetVertexBufferSet(CBufferSetPtr _BufferSetPtr, const unsigned int* _pStrides, const unsigned int* _pOffsets)
    {
        BASE_UNUSED(_pStrides);
        BASE_UNUSED(_pOffsets);

        assert(_BufferSetPtr != nullptr);
        
        Gfx::CNativeBufferSet& rNativeBuffer = *static_cast<Gfx::CNativeBufferSet*>(_BufferSetPtr.GetPtr());
        
        unsigned int NumberOfBuffers = rNativeBuffer.GetNumberOfBuffers();
        
        if (m_VertexBufferSetPtr != _BufferSetPtr || m_NumberOfVertexBuffers != NumberOfBuffers)
        {            
            glBindVertexArray(rNativeBuffer.m_NativeBufferArrayHandle);

            for (unsigned int IndexOfBuffer = 0; IndexOfBuffer < NumberOfBuffers; ++ IndexOfBuffer)
            {
                Gfx::CNativeBuffer& rNativeSingleBuffer = *static_cast<Gfx::CNativeBuffer*>(_BufferSetPtr->GetBuffer(IndexOfBuffer).GetPtr());
                
                glBindBuffer(GL_ARRAY_BUFFER, rNativeSingleBuffer.m_NativeBuffer);
            }
            
            m_VertexBufferSetPtr    = _BufferSetPtr;
            m_NumberOfVertexBuffers = NumberOfBuffers;
        }
    }

    // -----------------------------------------------------------------------------

    bool CGfxContextManager::ContainsNewStridesOrOffsets(const unsigned int* _pStrides, const unsigned int* _pOffsets) const
    {
        unsigned int IndexOfVertexBuffer;

        assert((_pStrides != nullptr) && (_pOffsets != nullptr));

        for (IndexOfVertexBuffer = 0; IndexOfVertexBuffer < m_NumberOfVertexBuffers; ++ IndexOfVertexBuffer)
        {
            if (m_VertexBufferStrides[IndexOfVertexBuffer] != _pStrides[IndexOfVertexBuffer]) return true;
            if (m_VertexBufferOffsets[IndexOfVertexBuffer] != _pOffsets[IndexOfVertexBuffer]) return true;
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr CGfxContextManager::GetVertexBufferSet()
    {
        return m_VertexBufferSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetConstantBufferSetVS()
    {
        m_ConstantBufferSetVSPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetConstantBufferSetVS(CBufferSetPtr _BufferSetPtr)
    {
        if (_BufferSetPtr == nullptr) return;
        
        if (m_ConstantBufferSetVSPtr != _BufferSetPtr && m_ShaderVSPtr != nullptr)
        {
            CNativeShader* pNativeShaderSet = static_cast<CNativeShader*>(m_ShaderVSPtr.GetPtr());
            
            Gfx::CNativeBufferHandle BufferHandle = 0;

            unsigned int IndexOfConstantBuffer = 0;
            unsigned int IndexOfResourceBuffer = 0;
            
            for (unsigned int IndexOfBuffer = 0; IndexOfBuffer < _BufferSetPtr->GetNumberOfBuffers(); ++IndexOfBuffer)
            {
                CNativeBuffer& rNativeBuffer = *static_cast<CNativeBuffer*>(_BufferSetPtr->GetBuffer(IndexOfBuffer).GetPtr());
                
                BufferHandle = rNativeBuffer.m_NativeBuffer;

                switch (rNativeBuffer.GetBinding())
                {
                case CBuffer::ConstantBuffer:

                    glUniformBlockBinding(pNativeShaderSet->m_NativeShader, IndexOfConstantBuffer, BufferHandle);

                    glBindBuffer(GL_UNIFORM_BUFFER, BufferHandle);

                    glBindBufferRange(GL_UNIFORM_BUFFER, BufferHandle, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_UNIFORM_BUFFER, 0);

                    ++IndexOfConstantBuffer;
                    break;
                case CBuffer::ResourceBuffer:
                    glShaderStorageBlockBinding(pNativeShaderSet->m_NativeShader, IndexOfResourceBuffer, BufferHandle);

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferHandle);

                    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, BufferHandle, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                    ++IndexOfResourceBuffer;
                    break;
                default:
                    BASE_CONSOLE_STREAMWARNING("Unsupported constant buffer type");
                    break;
                }
            }
            
            m_ConstantBufferSetVSPtr = _BufferSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr CGfxContextManager::GetConstantBufferSetVS()
    {
        return m_ConstantBufferSetVSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetConstantBufferSetHS()
    {
        m_ConstantBufferSetHSPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetConstantBufferSetHS(CBufferSetPtr _BufferSetPtr)
    {
        if (_BufferSetPtr == nullptr) return;
        
        if (m_ConstantBufferSetHSPtr != _BufferSetPtr && m_ShaderHSPtr != nullptr)
        {
            CNativeShader* pNativeShaderSet = static_cast<CNativeShader*>(m_ShaderHSPtr.GetPtr());
            
            Gfx::CNativeBufferHandle BufferHandle = 0;

            unsigned int IndexOfConstantBuffer = 0;
            unsigned int IndexOfResourceBuffer = 0;
            
            for (unsigned int IndexOfBuffer = 0; IndexOfBuffer < _BufferSetPtr->GetNumberOfBuffers(); ++IndexOfBuffer)
            {
                CNativeBuffer& rNativeBuffer = *static_cast<CNativeBuffer*>(_BufferSetPtr->GetBuffer(IndexOfBuffer).GetPtr());
                
                BufferHandle = rNativeBuffer.m_NativeBuffer;
                
                switch (rNativeBuffer.GetBinding())
                {
                case CBuffer::ConstantBuffer:

                    glUniformBlockBinding(pNativeShaderSet->m_NativeShader, IndexOfConstantBuffer, BufferHandle);

                    glBindBuffer(GL_UNIFORM_BUFFER, BufferHandle);

                    glBindBufferRange(GL_UNIFORM_BUFFER, BufferHandle, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_UNIFORM_BUFFER, 0);

                    ++IndexOfConstantBuffer;
                    break;
                case CBuffer::ResourceBuffer:
                    glShaderStorageBlockBinding(pNativeShaderSet->m_NativeShader, IndexOfResourceBuffer, BufferHandle);

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferHandle);

                    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, BufferHandle, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                    ++IndexOfResourceBuffer;
                    break;
                default:
                    BASE_CONSOLE_STREAMWARNING("Unsupported constant buffer type");
                    break;
                }
            }
            
            m_ConstantBufferSetHSPtr = _BufferSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr CGfxContextManager::GetConstantBufferSetHS()
    {
        return m_ConstantBufferSetHSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetConstantBufferSetDS()
    {        
        m_ConstantBufferSetDSPtr = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetConstantBufferSetDS(CBufferSetPtr _BufferSetPtr)
    {
        if (_BufferSetPtr == nullptr) return;
        
        if (m_ConstantBufferSetDSPtr != _BufferSetPtr && m_ShaderDSPtr != nullptr)
        {
            CNativeShader* pNativeShaderSet = static_cast<CNativeShader*>(m_ShaderDSPtr.GetPtr());
            
            Gfx::CNativeBufferHandle BufferHandle = 0;

            unsigned int IndexOfConstantBuffer = 0;
            unsigned int IndexOfResourceBuffer = 0;
            
            for (unsigned int IndexOfBuffer = 0; IndexOfBuffer < _BufferSetPtr->GetNumberOfBuffers(); ++IndexOfBuffer)
            {
                CNativeBuffer& rNativeBuffer = *static_cast<CNativeBuffer*>(_BufferSetPtr->GetBuffer(IndexOfBuffer).GetPtr());
                
                BufferHandle = rNativeBuffer.m_NativeBuffer;
                
                switch (rNativeBuffer.GetBinding())
                {
                case CBuffer::ConstantBuffer:

                    glUniformBlockBinding(pNativeShaderSet->m_NativeShader, IndexOfConstantBuffer, BufferHandle);

                    glBindBuffer(GL_UNIFORM_BUFFER, BufferHandle);

                    glBindBufferRange(GL_UNIFORM_BUFFER, BufferHandle, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_UNIFORM_BUFFER, 0);

                    ++IndexOfConstantBuffer;
                    break;
                case CBuffer::ResourceBuffer:
                    glShaderStorageBlockBinding(pNativeShaderSet->m_NativeShader, IndexOfResourceBuffer, BufferHandle);

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferHandle);

                    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, BufferHandle, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                    ++IndexOfResourceBuffer;
                    break;
                default:
                    BASE_CONSOLE_STREAMWARNING("Unsupported constant buffer type");
                    break;
                }
            }
            
            m_ConstantBufferSetDSPtr = _BufferSetPtr;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferSetPtr CGfxContextManager::GetConstantBufferSetDS()
    {
        return m_ConstantBufferSetDSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetConstantBufferSetGS()
    {
        m_ConstantBufferSetGSPtr = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetConstantBufferSetGS(CBufferSetPtr _BufferSetPtr)
    {
        if (_BufferSetPtr == nullptr) return;
        
        if (m_ConstantBufferSetGSPtr != _BufferSetPtr && m_ShaderGSPtr != nullptr)
        {
            CNativeShader* pNativeShaderSet = static_cast<CNativeShader*>(m_ShaderGSPtr.GetPtr());
            
            Gfx::CNativeBufferHandle BufferHandle = 0;

            unsigned int IndexOfConstantBuffer = 0;
            unsigned int IndexOfResourceBuffer = 0;
            
            for (unsigned int IndexOfBuffer = 0; IndexOfBuffer < _BufferSetPtr->GetNumberOfBuffers(); ++IndexOfBuffer)
            {
                CNativeBuffer& rNativeBuffer = *static_cast<CNativeBuffer*>(_BufferSetPtr->GetBuffer(IndexOfBuffer).GetPtr());
                
                BufferHandle = rNativeBuffer.m_NativeBuffer;
                
                switch (rNativeBuffer.GetBinding())
                {
                case CBuffer::ConstantBuffer:

                    glUniformBlockBinding(pNativeShaderSet->m_NativeShader, IndexOfConstantBuffer, BufferHandle);

                    glBindBuffer(GL_UNIFORM_BUFFER, BufferHandle);

                    glBindBufferRange(GL_UNIFORM_BUFFER, BufferHandle, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_UNIFORM_BUFFER, 0);

                    ++IndexOfConstantBuffer;
                    break;
                case CBuffer::ResourceBuffer:
                    glShaderStorageBlockBinding(pNativeShaderSet->m_NativeShader, IndexOfResourceBuffer, BufferHandle);

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferHandle);

                    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, BufferHandle, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                    ++IndexOfResourceBuffer;
                    break;
                default:
                    BASE_CONSOLE_STREAMWARNING("Unsupported constant buffer type");
                    break;
                }
            }
            
            m_ConstantBufferSetGSPtr = _BufferSetPtr;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferSetPtr CGfxContextManager::GetConstantBufferSetGS()
    {
        return m_ConstantBufferSetGSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetConstantBufferSetPS()
    {
        m_ConstantBufferSetPSPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetConstantBufferSetPS(CBufferSetPtr _BufferSetPtr)
    {
        if (_BufferSetPtr == nullptr) return;
        
        if (m_ConstantBufferSetPSPtr != _BufferSetPtr && m_ShaderPSPtr != nullptr)
        {
            CNativeShader* pNativeShaderSet = static_cast<CNativeShader*>(m_ShaderPSPtr.GetPtr());
            
            Gfx::CNativeBufferHandle BufferHandle = 0;

            unsigned int IndexOfConstantBuffer = 0;
            unsigned int IndexOfResourceBuffer = 0;
            
            for (unsigned int IndexOfBuffer = 0; IndexOfBuffer < _BufferSetPtr->GetNumberOfBuffers(); ++IndexOfBuffer)
            {
                CNativeBuffer& rNativeBuffer = *static_cast<CNativeBuffer*>(_BufferSetPtr->GetBuffer(IndexOfBuffer).GetPtr());
                
                BufferHandle = rNativeBuffer.m_NativeBuffer;
                
                switch (rNativeBuffer.GetBinding())
                {
                case CBuffer::ConstantBuffer:

                    glUniformBlockBinding(pNativeShaderSet->m_NativeShader, IndexOfConstantBuffer, BufferHandle);

                    glBindBuffer(GL_UNIFORM_BUFFER, BufferHandle);

                    glBindBufferRange(GL_UNIFORM_BUFFER, BufferHandle, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_UNIFORM_BUFFER, 0);

                    ++IndexOfConstantBuffer;
                    break;
                case CBuffer::ResourceBuffer:
                    glShaderStorageBlockBinding(pNativeShaderSet->m_NativeShader, IndexOfResourceBuffer, BufferHandle);

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferHandle);

                    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, BufferHandle, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                    ++IndexOfResourceBuffer;
                    break;
                default:
                    BASE_CONSOLE_STREAMWARNING("Unsupported constant buffer type");
                    break;
                }
            }
            
            m_ConstantBufferSetPSPtr = _BufferSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr CGfxContextManager::GetConstantBufferSetPS()
    {
        return m_ConstantBufferSetPSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetConstantBufferSetCS()
    {
        m_ConstantBufferSetCSPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetConstantBufferSetCS(CBufferSetPtr _BufferSetPtr)
    {
        assert(_BufferSetPtr != nullptr);
        
        if (m_ConstantBufferSetCSPtr != _BufferSetPtr)
        {
            CNativeShader* pNativeShaderSet = static_cast<CNativeShader*>(m_ShaderCSPtr.GetPtr());
            
            Gfx::CNativeBufferHandle BufferHandle = 0;

            unsigned int IndexOfConstantBuffer = 0;
            unsigned int IndexOfResourceBuffer = 0;
            
            for (unsigned int IndexOfBuffer = 0; IndexOfBuffer < _BufferSetPtr->GetNumberOfBuffers(); ++IndexOfBuffer)
            {
                CNativeBuffer& rNativeBuffer = *static_cast<CNativeBuffer*>(_BufferSetPtr->GetBuffer(IndexOfBuffer).GetPtr());
                
                BufferHandle = rNativeBuffer.m_NativeBuffer;
                
                switch (rNativeBuffer.GetBinding())
                {
                case CBuffer::ConstantBuffer:

                    glUniformBlockBinding(pNativeShaderSet->m_NativeShader, IndexOfConstantBuffer, BufferHandle);

                    glBindBuffer(GL_UNIFORM_BUFFER, BufferHandle);

                    glBindBufferRange(GL_UNIFORM_BUFFER, BufferHandle, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_UNIFORM_BUFFER, 0);

                    ++IndexOfConstantBuffer;
                    break;
                case CBuffer::ResourceBuffer:
                {
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferHandle);

                    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, IndexOfResourceBuffer, BufferHandle, 0, rNativeBuffer.GetNumberOfBytes());

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                    ++IndexOfResourceBuffer;
                }
                    break;
                default:
                    BASE_CONSOLE_STREAMWARNING("Unsupported constant buffer type");
                    break;
                }
            }
            
            m_ConstantBufferSetCSPtr = _BufferSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr CGfxContextManager::GetConstantBufferSetCS()
    {
        return m_ConstantBufferSetCSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetInputLayout()
    {
        if (m_InputLayoutPtr == nullptr) return;
        
        unsigned int NumberOfElements = m_InputLayoutPtr->GetNumberOfElements();
        
        for (unsigned int IndexOfElement = 0; IndexOfElement < NumberOfElements; ++ IndexOfElement)
        {
            glDisableVertexAttribArray(IndexOfElement);
        }
        
        m_InputLayoutPtr = nullptr;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetInputLayout(CInputLayoutPtr _InputLayoutPtr)
    {
        assert(_InputLayoutPtr      != nullptr);
        assert(m_VertexBufferSetPtr != nullptr);
        
        if (m_InputLayoutPtr != _InputLayoutPtr)
        {
            unsigned int LastIndexOfVB    = static_cast<unsigned int>(-1);
            unsigned int NumberOfElements = _InputLayoutPtr->GetNumberOfElements();
            
            for (unsigned int IndexOfElement = 0; IndexOfElement < NumberOfElements; ++ IndexOfElement)
            {
                const Gfx::CInputLayout::CElement& rElement = _InputLayoutPtr->GetElement(IndexOfElement);
                
                unsigned int IndexOfVB         = rElement.GetInputSlot();
                int NativeFormat               = ConvertInputLayoutFormat(rElement.GetFormat());
                int FormatSize                 = ConvertInputLayoutFormatSize(rElement.GetFormat());
                unsigned int Stride            = rElement.GetStride();
                unsigned int AlignedByteOffset = rElement.GetAlignedByteOffset();
                
                if (IndexOfVB != LastIndexOfVB)
                {
                    Gfx::CNativeBuffer& rNativeSingleBuffer = *static_cast<Gfx::CNativeBuffer*>(m_VertexBufferSetPtr->GetBuffer(IndexOfVB).GetPtr());
                    
                    glBindBuffer(GL_ARRAY_BUFFER, rNativeSingleBuffer.m_NativeBuffer);
                    
                    LastIndexOfVB = IndexOfVB;
                }
                
                glEnableVertexAttribArray(IndexOfElement);

                glVertexAttribPointer(IndexOfElement, FormatSize, NativeFormat, GL_FALSE, Stride, (char *)NULL + AlignedByteOffset);
                
                if (rElement.GetInputClassification() == CInputLayout::PerInstance)
                {
                    glVertexAttribDivisor(IndexOfElement, rElement.GetInstanceDataStepRate());
                } 
                
                // -----------------------------------------------------------------------------
                // Interleaved:
                // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24,  0);
                // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, 12);
                // 
                // Byte	Content
                // 0	Position
                // 12	Normal
                // 24	Position
                // 36	Normal
                // 48	Position
                // 60	Normal
                // 
                // 
                // Non-Interleaved:
                // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12,  0);  // alternativ auch stride=0
                // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12, 36);  // alternativ auch stride=0
                // 
                // Byte	Content
                // 0	Position
                // 12	Position
                // 24	Position
                // 36	Normal
                // 48	Normal
                // 60	Normal
                //
                // From: http://wiki.delphigl.com/index.php/glVertexAttribPointer
                //
                // -----------------------------------------------------------------------------
            }
            
            m_InputLayoutPtr = _InputLayoutPtr;
        }        
    }

    // -----------------------------------------------------------------------------

    CInputLayoutPtr CGfxContextManager::GetInputLayout()
    {
        return m_InputLayoutPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetShaderVS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_VERTEX_SHADER_BIT, 0);
        
        m_ShaderVSPtr = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetShaderVS(CShaderPtr _ShaderSetPtr)
    {
        assert(_ShaderSetPtr != nullptr);
        
        if (m_ShaderVSPtr != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());
            
            glUseProgramStages(m_NativeShaderPipeline, GL_VERTEX_SHADER_BIT, rShaderSet.m_NativeShader);
            
            m_ShaderVSPtr = _ShaderSetPtr;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr CGfxContextManager::GetShaderVS()
    {
        return m_ShaderVSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetShaderHS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_TESS_CONTROL_SHADER_BIT, 0);
        
        m_ShaderHSPtr = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetShaderHS(CShaderPtr _ShaderSetPtr)
    {
        if (_ShaderSetPtr == nullptr) return;
        
        if (m_ShaderHSPtr != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());
            
            glUseProgramStages(m_NativeShaderPipeline, GL_TESS_CONTROL_SHADER_BIT, rShaderSet.m_NativeShader);
            
            m_ShaderHSPtr = _ShaderSetPtr;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr CGfxContextManager::GetShaderHS()
    {
        return m_ShaderHSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetShaderDS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_TESS_EVALUATION_SHADER_BIT, 0);
        
        m_ShaderDSPtr = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetShaderDS(CShaderPtr _ShaderSetPtr)
    {
        if (_ShaderSetPtr == nullptr) return;
        
        if (m_ShaderDSPtr != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());
            
            glUseProgramStages(m_NativeShaderPipeline, GL_TESS_EVALUATION_SHADER_BIT, rShaderSet.m_NativeShader);
            
            m_ShaderDSPtr = _ShaderSetPtr;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr CGfxContextManager::GetShaderDS()
    {
        return m_ShaderDSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetShaderGS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_GEOMETRY_SHADER_BIT, 0);
        
        m_ShaderGSPtr = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetShaderGS(CShaderPtr _ShaderSetPtr)
    {
        if (_ShaderSetPtr == nullptr) return;
        
        if (m_ShaderGSPtr != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());
            
            glUseProgramStages(m_NativeShaderPipeline, GL_GEOMETRY_SHADER_BIT, rShaderSet.m_NativeShader);

            m_ShaderGSPtr = _ShaderSetPtr;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr CGfxContextManager::GetShaderGS()
    {
        return m_ShaderGSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetShaderPS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_FRAGMENT_SHADER_BIT, 0);
        
        m_ShaderPSPtr = nullptr;
    }
    
    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetShaderPS(CShaderPtr _ShaderSetPtr)
    {
        assert(_ShaderSetPtr != nullptr);
        
        if (m_ShaderPSPtr != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());
            
            glUseProgramStages(m_NativeShaderPipeline, GL_FRAGMENT_SHADER_BIT, rShaderSet.m_NativeShader);

            m_ShaderPSPtr = _ShaderSetPtr;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr CGfxContextManager::GetShaderPS()
    {
        return m_ShaderPSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetShaderCS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_COMPUTE_SHADER_BIT, 0);
        
        m_ShaderCSPtr = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetShaderCS(CShaderPtr _ShaderSetPtr)
    {
        assert(_ShaderSetPtr != nullptr);
        
        if (m_ShaderCSPtr != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());
            
            glUseProgramStages(m_NativeShaderPipeline, GL_COMPUTE_SHADER_BIT, rShaderSet.m_NativeShader);

            m_ShaderCSPtr = _ShaderSetPtr;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr CGfxContextManager::GetShaderCS()
    {
        return m_ShaderCSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetSamplerSetVS()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetSamplerSetVS(CSamplerSetPtr _SamplerSetPtr)
    {
        assert(_SamplerSetPtr != nullptr);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CGfxContextManager::GetSamplerSetVS()
    {
        return m_SamplerSetVSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetSamplerSetHS()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetSamplerSetHS(CSamplerSetPtr _SamplerSetPtr)
    {
        assert(_SamplerSetPtr != nullptr);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CGfxContextManager::GetSamplerSetHS()
    {
        return m_SamplerSetHSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetSamplerSetDS()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetSamplerSetDS(CSamplerSetPtr _SamplerSetPtr)
    {
        assert(_SamplerSetPtr != nullptr);
    }
    
    // -----------------------------------------------------------------------------
    
    CSamplerSetPtr CGfxContextManager::GetSamplerSetDS()
    {
        return m_SamplerSetDSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetSamplerSetGS()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetSamplerSetGS(CSamplerSetPtr _SamplerSetPtr)
    {
        assert(_SamplerSetPtr != nullptr);
    }
    
    // -----------------------------------------------------------------------------
    
    CSamplerSetPtr CGfxContextManager::GetSamplerSetGS()
    {
        return m_SamplerSetGSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetSamplerSetPS()
    {
        if (m_SamplerSetPSPtr == nullptr) return;
        
        for (unsigned int IndexOfSampler = 0; IndexOfSampler < m_SamplerSetPSPtr->GetNumberOfSamplers(); ++IndexOfSampler)
        {
            glBindSampler(IndexOfSampler, 0);
        }
        
        m_SamplerSetPSPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetSamplerSetPS(CSamplerSetPtr _SamplerSetPtr)
    {
        assert(_SamplerSetPtr != nullptr);
        
        Gfx::CNativeSamplerHandle SamplerHandle = 0;
        
        for (unsigned int IndexOfSampler = 0; IndexOfSampler < _SamplerSetPtr->GetNumberOfSamplers(); ++IndexOfSampler)
        {
            CNativeSampler& rNativeSampler = *static_cast<CNativeSampler*>(_SamplerSetPtr->GetSampler(IndexOfSampler).GetPtr());
            
            SamplerHandle = rNativeSampler.m_NativeSampler;
            
            glBindSampler(IndexOfSampler, SamplerHandle);

            m_SamplerSetPSPtr = _SamplerSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CGfxContextManager::GetSamplerSetPS()
    {
        return m_SamplerSetPSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetSamplerSetCS()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetSamplerSetCS(CSamplerSetPtr _SamplerSetPtr)
    {
        assert(_SamplerSetPtr != nullptr);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CGfxContextManager::GetSamplerSetCS()
    {
        return m_SamplerSetCSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetTextureSetVS()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetTextureSetVS(CTextureSetPtr _TextureSetPtr)
    {
        BASE_UNUSED(_TextureSetPtr);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CGfxContextManager::GetTextureSetVS()
    {
        return m_TextureSetVSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetTextureSetHS()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetTextureSetHS(CTextureSetPtr _TextureSetPtr)
    {
        BASE_UNUSED(_TextureSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CTextureSetPtr CGfxContextManager::GetTextureSetHS()
    {
        return m_TextureSetHSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetTextureSetDS()
    {
        if (m_TextureSetDSPtr == nullptr) return;
        
        for (unsigned int IndexOfTexture = 0; IndexOfTexture < m_TextureSetDSPtr->GetNumberOfTextures(); ++IndexOfTexture)
        {
            CNativeTexture2D* pNativeTexture = static_cast<CNativeTexture2D*>(m_TextureSetDSPtr->GetTexture(IndexOfTexture).GetPtr());

            if (pNativeTexture != 0)
            {
                CNativeTexture2D& rNativeTexture = *pNativeTexture;

                GLuint TextureBinding = rNativeTexture.m_NativeDimension;

                if (rNativeTexture.IsCube())
                {
                    TextureBinding = GL_TEXTURE_CUBE_MAP;
                }

                glActiveTexture(GL_TEXTURE0 + IndexOfTexture);

                glBindTexture(TextureBinding, 0);
            }
        }
        
        m_IndexOfTextureBinding = 0;

        m_TextureSetDSPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetTextureSetDS(CTextureSetPtr _TextureSetPtr)
    {
        if(_TextureSetPtr == nullptr) return;
        
        if (m_TextureSetDSPtr != _TextureSetPtr)
        {
            Gfx::CNativeTextureHandle TextureHandle  = 0;
            GLuint                    TextureBinding = 0;
            
            for (unsigned int IndexOfTexture = 0; IndexOfTexture < _TextureSetPtr->GetNumberOfTextures(); ++IndexOfTexture)
            {
                CNativeTexture2D* pNativeTexture = static_cast<CNativeTexture2D*>(_TextureSetPtr->GetTexture(IndexOfTexture).GetPtr());

                if (pNativeTexture != 0)
                {
                    CNativeTexture2D& rNativeTexture = *pNativeTexture;

                    TextureHandle = rNativeTexture.m_NativeTexture;

                    TextureBinding = rNativeTexture.m_NativeDimension;

                    if (rNativeTexture.IsCube())
                    {
                        TextureBinding = GL_TEXTURE_CUBE_MAP;
                    }

                    glActiveTexture(GL_TEXTURE0 + m_IndexOfTextureBinding);

                    glBindTexture(TextureBinding, TextureHandle);
                }

                ++m_IndexOfTextureBinding;
            }
            
            m_TextureSetDSPtr = _TextureSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CGfxContextManager::GetTextureSetDS()
    {
        return m_TextureSetDSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ResetTextureSetGS()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::SetTextureSetGS(CTextureSetPtr _TextureSetPtr)
    {
        BASE_UNUSED(_TextureSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CTextureSetPtr CGfxContextManager::GetTextureSetGS()
    {
        return m_TextureSetGSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetTextureSetPS()
    {
        if (m_TextureSetPSPtr == nullptr) return;
        
        for (unsigned int IndexOfTexture = 0; IndexOfTexture < m_TextureSetPSPtr->GetNumberOfTextures(); ++IndexOfTexture)
        {
            CNativeTexture2D* pNativeTexture = static_cast<CNativeTexture2D*>(m_TextureSetPSPtr->GetTexture(IndexOfTexture).GetPtr());

            if (pNativeTexture != 0)
            {
                CNativeTexture2D& rNativeTexture = *pNativeTexture;

                GLuint TextureBinding = rNativeTexture.m_NativeDimension;

                if (rNativeTexture.IsCube())
                {
                    TextureBinding = GL_TEXTURE_CUBE_MAP;
                }

                glActiveTexture(GL_TEXTURE0 + IndexOfTexture);

                glBindTexture(TextureBinding, 0);
            }
        }
        
        m_IndexOfTextureBinding = 0;

        m_TextureSetPSPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetTextureSetPS(CTextureSetPtr _TextureSetPtr)
    {
        if(_TextureSetPtr == nullptr) return;

        if (m_TextureSetPSPtr != _TextureSetPtr)
        {
            Gfx::CNativeTextureHandle TextureHandle  = 0;
            GLuint                    TextureBinding = 0;
            
            for (unsigned int IndexOfTexture = 0; IndexOfTexture < _TextureSetPtr->GetNumberOfTextures(); ++IndexOfTexture)
            {
                CNativeTexture2D* pNativeTexture = static_cast<CNativeTexture2D*>(_TextureSetPtr->GetTexture(IndexOfTexture).GetPtr());

                if (pNativeTexture != 0)
                {
                    CNativeTexture2D& rNativeTexture = *pNativeTexture;

                    TextureHandle = rNativeTexture.m_NativeTexture;

                    TextureBinding = rNativeTexture.m_NativeDimension;

                    if (rNativeTexture.IsCube())
                    {
                        TextureBinding = GL_TEXTURE_CUBE_MAP;
                    }

                    glActiveTexture(GL_TEXTURE0 + m_IndexOfTextureBinding);

                    glBindTexture(TextureBinding, TextureHandle);
                }

                ++m_IndexOfTextureBinding;
            }
            
            m_TextureSetPSPtr = _TextureSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CGfxContextManager::GetTextureSetPS()
    {
        return m_TextureSetPSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetTextureSetCS()
    {
        m_IndexOfTextureBinding = 0;

        m_TextureSetCSPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetTextureSetCS(CTextureSetPtr _TextureSetPtr)
    {
        if (_TextureSetPtr == nullptr) return;

        if (m_TextureSetCSPtr != _TextureSetPtr)
        {
            Gfx::CNativeTextureHandle TextureHandle  = 0;
            GLenum                    Access         = 0;
            GLenum                    Format         = 0;
            GLuint                    TextureBinding = 0;

            for (unsigned int IndexOfTexture = 0; IndexOfTexture < _TextureSetPtr->GetNumberOfTextures(); ++IndexOfTexture)
            {
                CNativeTexture2D& rNativeTexture = *static_cast<CNativeTexture2D*>(_TextureSetPtr->GetTexture(IndexOfTexture).GetPtr());

                TextureHandle = rNativeTexture.m_NativeTexture;

                // -----------------------------------------------------------------------------
                // 'Access' for binding the texture is depending on the usage of this certain
                // texture.
                // 'Format' is the internal format.
                // -----------------------------------------------------------------------------
                Access = rNativeTexture.m_NativeUsage;
                Format = rNativeTexture.m_NativeInternalFormat;

                if ((rNativeTexture.GetBinding() & CTextureBase::EBinding::DepthStencilTarget) == CTextureBase::EBinding::DepthStencilTarget)
                {
                    TextureHandle = rNativeTexture.m_NativeTexture;

                    TextureBinding = rNativeTexture.m_NativeDimension;

                    if (rNativeTexture.IsCube())
                    {
                        TextureBinding = GL_TEXTURE_CUBE_MAP;
                    }

                    glActiveTexture(GL_TEXTURE0 + m_IndexOfTextureBinding);

                    glBindTexture(TextureBinding, TextureHandle);
                }
                else if (rNativeTexture.IsCube())
                {
                    // layered
                }
                else
                {
                    glBindImageTexture(m_IndexOfTextureBinding, TextureHandle, 0, GL_FALSE, 0, Access, Format);
                }

                ++ m_IndexOfTextureBinding;
            }

            m_TextureSetCSPtr = _TextureSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CGfxContextManager::GetTextureSetCS()
    {
        return m_TextureSetCSPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetTargetSet()
    {
        m_TargetSetPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetTargetSet(CTargetSetPtr _TargetSetPtr)
    {
        assert(_TargetSetPtr != nullptr);
        
        if (m_TargetSetPtr != _TargetSetPtr)
        {
            Gfx::CNativeTargetSet& rNativeTargetSet = *static_cast<Gfx::CNativeTargetSet*>(_TargetSetPtr.GetPtr());
            
            glBindFramebuffer(GL_FRAMEBUFFER, rNativeTargetSet.m_NativeTargetSet);
            
            unsigned int NumberOfRenderTargets = rNativeTargetSet.GetNumberOfRenderTargets();
            
            if (NumberOfRenderTargets != 0)
            {
                GLenum DrawBuffers[8] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7};
                
                glDrawBuffers(NumberOfRenderTargets, DrawBuffers);
            }
            
            m_TargetSetPtr = _TargetSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CTargetSetPtr CGfxContextManager::GetTargetSet()
    {
        return m_TargetSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetViewPortSet()
    {
        m_ViewPortSetPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetViewPortSet(CViewPortSetPtr _ViewPortSetPtr)
    {
        assert(_ViewPortSetPtr != nullptr);
        
        if (_ViewPortSetPtr != m_ViewPortSetPtr)
        {
            unsigned int NumberOfViewports = _ViewPortSetPtr->GetNumberOfViewPorts();
            CViewPortPtr* pViewportPtrs    = _ViewPortSetPtr->GetViewPorts();
            
            for (unsigned int IndexOfViewport = 0; IndexOfViewport < NumberOfViewports; ++ IndexOfViewport)
            {
                CViewPortPtr CurrentViewportPtr = pViewportPtrs[IndexOfViewport];
                
                GLint TopX = static_cast<GLint>(CurrentViewportPtr->GetTopLeftX());
                GLint TopY = static_cast<GLint>(CurrentViewportPtr->GetTopLeftY());
                
                GLsizei Width  = static_cast<GLsizei>(CurrentViewportPtr->GetWidth());
                GLsizei Height = static_cast<GLsizei>(CurrentViewportPtr->GetHeight());
                
                glViewport(TopX, TopY, Width, Height);
            }
            
            m_ViewPortSetPtr = _ViewPortSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CViewPortSetPtr CGfxContextManager::GetViewPortSet()
    {
        return m_ViewPortSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::Draw(unsigned int _NumberOfVertices, unsigned int _IndexOfFirstVertex)
    {
        glDrawArrays(s_NativeTopologies[m_Topology], _IndexOfFirstVertex, _NumberOfVertices);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::DrawIndexed(unsigned int _NumberOfIndices, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation)
    {
        BASE_UNUSED(_IndexOfFirstIndex);
        BASE_UNUSED(_BaseVertexLocation);
        
        glDrawElements(s_NativeTopologies[m_Topology], _NumberOfIndices, GL_UNSIGNED_INT, 0);
        
        m_IndexOfTextureBinding = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::DrawIndexedInstanced(unsigned int _NumberOfIndices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation, unsigned int _StartInstanceLocation)
    {
        BASE_UNUSED(_IndexOfFirstIndex);
        BASE_UNUSED(_BaseVertexLocation);
        BASE_UNUSED(_StartInstanceLocation);

        glDrawElementsInstanced(s_NativeTopologies[m_Topology], _NumberOfIndices, GL_UNSIGNED_INT, 0, _NumberOfInstances);
        
        m_IndexOfTextureBinding = 0;
    }
    
    // -----------------------------------------------------------------------------


    
    void CGfxContextManager::Dispatch(unsigned int _NumberOfThreadGroupsX, unsigned int _NumberOfThreadGroupsY, unsigned int _NumberOfThreadGroupsZ)
    {
        assert(_NumberOfThreadGroupsX > 0 && _NumberOfThreadGroupsY > 0 && _NumberOfThreadGroupsZ > 0);

        glDispatchCompute(_NumberOfThreadGroupsX, _NumberOfThreadGroupsY, _NumberOfThreadGroupsZ);

        m_IndexOfTextureBinding = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxContextManager::ConvertInputLayoutFormat(Gfx::CInputLayout::EFormat _Format) const
    {
        static int s_NativeFormat[] =
        {
            GL_BYTE,
            GL_BYTE,
            GL_BYTE,
            GL_BYTE,
            GL_BYTE,
            GL_BYTE,
            GL_SHORT,
            GL_SHORT,
            GL_SHORT,
            GL_SHORT,
            GL_SHORT,
            GL_SHORT,
            GL_INT,
            GL_INT,
            GL_INT,
            GL_INT,
            GL_INT,
            GL_INT,
            GL_INT,
            GL_INT,
            GL_HALF_FLOAT,
            GL_HALF_FLOAT,
            GL_HALF_FLOAT,
            GL_FLOAT,
            GL_FLOAT,
            GL_FLOAT,
            GL_FLOAT,
        };
        
        return s_NativeFormat[_Format];
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxContextManager::ConvertInputLayoutFormatSize(Gfx::CInputLayout::EFormat _Format) const
    {
        static int s_NativeFormatSize[] =
        {
            1,
            2,
            4,
            1,
            2,
            4,
            1,
            2,
            4,
            1,
            2,
            4,
            1,
            2,
            3,
            4,
            1,
            2,
            3,
            4,
            1,
            2,
            4,
            1,
            2,
            3,
            4,
        };
        
        return s_NativeFormatSize[_Format];
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::CreateShaderPipeline()
    {
        glUseProgram(0);

        glGenProgramPipelines(1, &m_NativeShaderPipeline);

        glBindProgramPipeline(m_NativeShaderPipeline);

        assert(glIsProgramPipeline(m_NativeShaderPipeline));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::DestroyShaderPipeline()
    {
        glDeleteProgramPipelines(1, &m_NativeShaderPipeline);
    }
} // namespace

namespace
{
    CGfxContextManager::CInternRenderContext::CInternRenderContext()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxContextManager::CInternRenderContext::~CInternRenderContext()
    {
        
    }
} // namespace

namespace Gfx
{
namespace ContextManager
{
    void OnStart()
    {
        CGfxContextManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxContextManager::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    CRenderContextPtr CreateRenderContext()
    {
        return CGfxContextManager::GetInstance().CreateRenderContext();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetRenderContext()
    {
        CGfxContextManager::GetInstance().ResetRenderContext();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetRenderContext(CRenderContextPtr _RenderContextPtr)
    {
        CGfxContextManager::GetInstance().SetRenderContext(_RenderContextPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void SetRenderFlags(unsigned int _Flags)
    {
        CGfxContextManager::GetInstance().SetRenderFlags(_Flags);
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int GetRenderFlags()
    {
        return CGfxContextManager::GetInstance().GetRenderFlags();
    }

    // -----------------------------------------------------------------------------

    void ResetBlendState()
    {
        CGfxContextManager::GetInstance().ResetBlendState();
    }

    // -----------------------------------------------------------------------------

    void SetBlendState(CBlendStatePtr _StatePtr)
    {
        CGfxContextManager::GetInstance().SetBlendState(_StatePtr);
    }

    // -----------------------------------------------------------------------------

    CBlendStatePtr GetBlendState()
    {
        return CGfxContextManager::GetInstance().GetBlendState();
    }

    // -----------------------------------------------------------------------------

    void ResetDepthStencilState()
    {
        CGfxContextManager::GetInstance().ResetDepthStencilState();
    }

    // -----------------------------------------------------------------------------

    void SetDepthStencilState(CDepthStencilStatePtr _StatePtr)
    {
        CGfxContextManager::GetInstance().SetDepthStencilState(_StatePtr);
    }

    // -----------------------------------------------------------------------------

    CDepthStencilStatePtr GetDepthStencilState()
    {
        return CGfxContextManager::GetInstance().GetDepthStencilState();
    }

    // -----------------------------------------------------------------------------

    void ResetRasterizerState()
    {
        CGfxContextManager::GetInstance().ResetRasterizerState();
    }

    // -----------------------------------------------------------------------------

    void SetRasterizerState(CRasterizerStatePtr _StatePtr)
    {
        CGfxContextManager::GetInstance().SetRasterizerState(_StatePtr);
    }

    // -----------------------------------------------------------------------------

    CRasterizerStatePtr GetRasterizerState()
    {
        return CGfxContextManager::GetInstance().GetRasterizerState();
    }

    // -----------------------------------------------------------------------------

    void ResetTopology()
    {
        CGfxContextManager::GetInstance().ResetTopology();
    }

    // -----------------------------------------------------------------------------

    void SetTopology(STopology::Enum _Topology)
    {
        CGfxContextManager::GetInstance().SetTopology(_Topology);
    }

    // -----------------------------------------------------------------------------

    STopology::Enum GetTopology()
    {
        return CGfxContextManager::GetInstance().GetTopology();
    }

    // -----------------------------------------------------------------------------

    void ResetIndexBuffer()
    {
        CGfxContextManager::GetInstance().ResetIndexBuffer();
    }

    // -----------------------------------------------------------------------------

    void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Offset)
    {
        CGfxContextManager::GetInstance().SetIndexBuffer(_BufferPtr, _Offset);
    }

    // -----------------------------------------------------------------------------

    void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Stride, unsigned int _Offset)
    {
        CGfxContextManager::GetInstance().SetIndexBuffer(_BufferPtr, _Stride, _Offset);
    }

    // -----------------------------------------------------------------------------

    CBufferPtr GetIndexBuffer()
    {
        return CGfxContextManager::GetInstance().GetIndexBuffer();
    }

    // -----------------------------------------------------------------------------

    void ResetVertexBufferSet()
    {
        CGfxContextManager::GetInstance().ResetVertexBufferSet();
    }

    // -----------------------------------------------------------------------------

    void SetVertexBufferSet(CBufferSetPtr _BufferSetPtr, const unsigned int* _pOffsets)
    {
        CGfxContextManager::GetInstance().SetVertexBufferSet(_BufferSetPtr, _pOffsets);
    }

    // -----------------------------------------------------------------------------

    void SetVertexBufferSet(CBufferSetPtr _BufferSetPtr, const unsigned int* _pStrides, const unsigned int* _pOffsets)
    {
        CGfxContextManager::GetInstance().SetVertexBufferSet(_BufferSetPtr, _pStrides, _pOffsets);
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr GetVertexBufferSet()
    {
        return CGfxContextManager::GetInstance().GetVertexBufferSet();
    }

    // -----------------------------------------------------------------------------

    void ResetConstantBufferSetVS()
    {
        CGfxContextManager::GetInstance().ResetConstantBufferSetVS();
    }

    // -----------------------------------------------------------------------------

    void SetConstantBufferSetVS(CBufferSetPtr _BufferSetPtr)
    {
        CGfxContextManager::GetInstance().SetConstantBufferSetVS(_BufferSetPtr);
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr GetConstantBufferSetVS()
    {
        return CGfxContextManager::GetInstance().GetConstantBufferSetVS();
    }

    // -----------------------------------------------------------------------------

    void ResetConstantBufferSetHS()
    {
        CGfxContextManager::GetInstance().ResetConstantBufferSetHS();
    }

    // -----------------------------------------------------------------------------

    void SetConstantBufferSetHS(CBufferSetPtr _BufferSetPtr)
    {
        CGfxContextManager::GetInstance().SetConstantBufferSetHS(_BufferSetPtr);
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr GetConstantBufferSetHS()
    {
        return CGfxContextManager::GetInstance().GetConstantBufferSetHS();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetConstantBufferSetDS()
    {
        CGfxContextManager::GetInstance().ResetConstantBufferSetDS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetConstantBufferSetDS(CBufferSetPtr _BufferSetPtr)
    {
        CGfxContextManager::GetInstance().SetConstantBufferSetDS(_BufferSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferSetPtr GetConstantBufferSetDS()
    {
        return CGfxContextManager::GetInstance().GetConstantBufferSetDS();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetConstantBufferSetGS()
    {
        CGfxContextManager::GetInstance().ResetConstantBufferSetGS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetConstantBufferSetGS(CBufferSetPtr _BufferSetPtr)
    {
        CGfxContextManager::GetInstance().SetConstantBufferSetGS(_BufferSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferSetPtr GetConstantBufferSetGS()
    {
        return CGfxContextManager::GetInstance().GetConstantBufferSetGS();
    }

    // -----------------------------------------------------------------------------

    void ResetConstantBufferSetPS()
    {
        CGfxContextManager::GetInstance().ResetConstantBufferSetPS();
    }

    // -----------------------------------------------------------------------------

    void SetConstantBufferSetPS(CBufferSetPtr _BufferSetPtr)
    {
        CGfxContextManager::GetInstance().SetConstantBufferSetPS(_BufferSetPtr);
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr GetConstantBufferSetPS()
    {
        return CGfxContextManager::GetInstance().GetConstantBufferSetPS();
    }

    // -----------------------------------------------------------------------------

    void ResetConstantBufferSetCS()
    {
        CGfxContextManager::GetInstance().ResetConstantBufferSetCS();
    }

    // -----------------------------------------------------------------------------

    void SetConstantBufferSetCS(CBufferSetPtr _BufferSetPtr)
    {
        CGfxContextManager::GetInstance().SetConstantBufferSetCS(_BufferSetPtr);
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr GetConstantBufferSetCS()
    {
        return CGfxContextManager::GetInstance().GetConstantBufferSetCS();
    }

    // -----------------------------------------------------------------------------

    void ResetInputLayout()
    {
        CGfxContextManager::GetInstance().ResetInputLayout();
    }

    // -----------------------------------------------------------------------------

    void SetInputLayout(CInputLayoutPtr _InputLayoutPtr)
    {
        CGfxContextManager::GetInstance().SetInputLayout(_InputLayoutPtr);
    }

    // -----------------------------------------------------------------------------

    CInputLayoutPtr GetInputLayout()
    {
        return CGfxContextManager::GetInstance().GetInputLayout();
    }

    // -----------------------------------------------------------------------------

    void ResetShaderVS()
    {
        CGfxContextManager::GetInstance().ResetShaderVS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetShaderVS(CShaderPtr _ShaderSetPtr)
    {
        CGfxContextManager::GetInstance().SetShaderVS(_ShaderSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr GetShaderVS()
    {
        return CGfxContextManager::GetInstance().GetShaderVS();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetShaderHS()
    {
        CGfxContextManager::GetInstance().ResetShaderHS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetShaderHS(CShaderPtr _ShaderSetPtr)
    {
        CGfxContextManager::GetInstance().SetShaderHS(_ShaderSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr GetShaderHS()
    {
        return CGfxContextManager::GetInstance().GetShaderHS();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetShaderDS()
    {
        CGfxContextManager::GetInstance().ResetShaderDS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetShaderDS(CShaderPtr _ShaderSetPtr)
    {
        CGfxContextManager::GetInstance().SetShaderDS(_ShaderSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr GetShaderDS()
    {
        return CGfxContextManager::GetInstance().GetShaderDS();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetShaderGS()
    {
        CGfxContextManager::GetInstance().ResetShaderGS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetShaderGS(CShaderPtr _ShaderSetPtr)
    {
        CGfxContextManager::GetInstance().SetShaderGS(_ShaderSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr GetShaderGS()
    {
        return CGfxContextManager::GetInstance().GetShaderGS();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetShaderPS()
    {
        CGfxContextManager::GetInstance().ResetShaderPS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetShaderPS(CShaderPtr _ShaderSetPtr)
    {
        CGfxContextManager::GetInstance().SetShaderPS(_ShaderSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr GetShaderSetPS()
    {
        return CGfxContextManager::GetInstance().GetShaderPS();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetShaderCS()
    {
        CGfxContextManager::GetInstance().ResetShaderCS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetShaderCS(CShaderPtr _ShaderSetPtr)
    {
        CGfxContextManager::GetInstance().SetShaderCS(_ShaderSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr GetShaderCS()
    {
        return CGfxContextManager::GetInstance().GetShaderCS();
    }

    // -----------------------------------------------------------------------------

    void ResetSamplerSetVS()
    {
        CGfxContextManager::GetInstance().ResetSamplerSetVS();
    }

    // -----------------------------------------------------------------------------

    void SetSamplerSetVS(CSamplerSetPtr _SamplerSetPtr)
    {
        CGfxContextManager::GetInstance().SetSamplerSetVS(_SamplerSetPtr);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr GetSamplerSetVS()
    {
        return CGfxContextManager::GetInstance().GetSamplerSetVS();
    }

    // -----------------------------------------------------------------------------

    void ResetSamplerSetHS()
    {
        CGfxContextManager::GetInstance().ResetSamplerSetHS();
    }

    // -----------------------------------------------------------------------------

    void SetSamplerSetHS(CSamplerSetPtr _SamplerSetPtr)
    {
        CGfxContextManager::GetInstance().SetSamplerSetHS(_SamplerSetPtr);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr GetSamplerSetHS()
    {
        return CGfxContextManager::GetInstance().GetSamplerSetHS();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetSamplerSetDS()
    {
        CGfxContextManager::GetInstance().ResetSamplerSetDS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetSamplerSetDS(CSamplerSetPtr _SamplerSetPtr)
    {
        CGfxContextManager::GetInstance().SetSamplerSetDS(_SamplerSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CSamplerSetPtr GetSamplerSetDS()
    {
        return CGfxContextManager::GetInstance().GetSamplerSetDS();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetSamplerSetGS()
    {
        CGfxContextManager::GetInstance().ResetSamplerSetGS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetSamplerSetGS(CSamplerSetPtr _SamplerSetPtr)
    {
        CGfxContextManager::GetInstance().SetSamplerSetGS(_SamplerSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CSamplerSetPtr GetSamplerSetGS()
    {
        return CGfxContextManager::GetInstance().GetSamplerSetGS();
    }

    // -----------------------------------------------------------------------------

    void ResetSamplerSetPS()
    {
        CGfxContextManager::GetInstance().ResetSamplerSetPS();
    }

    // -----------------------------------------------------------------------------

    void SetSamplerSetPS(CSamplerSetPtr _SamplerSetPtr)
    {
        CGfxContextManager::GetInstance().SetSamplerSetPS(_SamplerSetPtr);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr GetSamplerSetPS()
    {
        return CGfxContextManager::GetInstance().GetSamplerSetPS();
    }

    // -----------------------------------------------------------------------------

    void ResetSamplerSetCS()
    {
        CGfxContextManager::GetInstance().ResetSamplerSetCS();
    }

    // -----------------------------------------------------------------------------

    void SetSamplerSetCS(CSamplerSetPtr _SamplerSetPtr)
    {
        CGfxContextManager::GetInstance().SetSamplerSetCS(_SamplerSetPtr);
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr GetSamplerSetCS()
    {
        return CGfxContextManager::GetInstance().GetSamplerSetCS();
    }

    // -----------------------------------------------------------------------------

    void ResetTextureSetVS()
    {
        CGfxContextManager::GetInstance().ResetTextureSetVS();
    }

    // -----------------------------------------------------------------------------

    void SetTextureSetVS(CTextureSetPtr _TextureSetPtr)
    {
        CGfxContextManager::GetInstance().SetTextureSetVS(_TextureSetPtr);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr GetTextureSetVS()
    {
        return CGfxContextManager::GetInstance().GetTextureSetVS();
    }

    // -----------------------------------------------------------------------------

    void ResetTextureSetHS()
    {
        CGfxContextManager::GetInstance().ResetTextureSetHS();
    }

    // -----------------------------------------------------------------------------

    void SetTextureSetHS(CTextureSetPtr _TextureSetPtr)
    {
        CGfxContextManager::GetInstance().SetTextureSetHS(_TextureSetPtr);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr GetTextureSetHS()
    {
        return CGfxContextManager::GetInstance().GetTextureSetHS();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetTextureSetDS()
    {
        CGfxContextManager::GetInstance().ResetTextureSetDS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetTextureSetDS(CTextureSetPtr _TextureSetPtr)
    {
        CGfxContextManager::GetInstance().SetTextureSetDS(_TextureSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CTextureSetPtr GetTextureSetDS()
    {
        return CGfxContextManager::GetInstance().GetTextureSetDS();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetTextureSetGS()
    {
        CGfxContextManager::GetInstance().ResetTextureSetGS();
    }
    
    // -----------------------------------------------------------------------------
    
    void SetTextureSetGS(CTextureSetPtr _TextureSetPtr)
    {
        CGfxContextManager::GetInstance().SetTextureSetGS(_TextureSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CTextureSetPtr GetTextureSetGS()
    {
        return CGfxContextManager::GetInstance().GetTextureSetGS();
    }

    // -----------------------------------------------------------------------------

    void ResetTextureSetPS()
    {
        CGfxContextManager::GetInstance().ResetTextureSetPS();
    }

    // -----------------------------------------------------------------------------

    void SetTextureSetPS(CTextureSetPtr _TextureSetPtr)
    {
        CGfxContextManager::GetInstance().SetTextureSetPS(_TextureSetPtr);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr GetTextureSetPS()
    {
        return CGfxContextManager::GetInstance().GetTextureSetPS();
    }

    // -----------------------------------------------------------------------------

    void ResetTextureSetCS()
    {
        CGfxContextManager::GetInstance().ResetTextureSetCS();
    }

    // -----------------------------------------------------------------------------

    void SetTextureSetCS(CTextureSetPtr _TextureSetPtr)
    {
        CGfxContextManager::GetInstance().SetTextureSetCS(_TextureSetPtr);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr GetTextureSetCS()
    {
        return CGfxContextManager::GetInstance().GetTextureSetCS();
    }

    // -----------------------------------------------------------------------------

    void ResetTargetSet()
    {
        CGfxContextManager::GetInstance().ResetTargetSet();
    }

    // -----------------------------------------------------------------------------

    void SetTargetSet(CTargetSetPtr _TargetSetPtr)
    {
        CGfxContextManager::GetInstance().SetTargetSet(_TargetSetPtr);
    }

    // -----------------------------------------------------------------------------

    CTargetSetPtr GetTargetSet()
    {
        return CGfxContextManager::GetInstance().GetTargetSet();
    }

    // -----------------------------------------------------------------------------

    void ResetViewPortSet()
    {
        return CGfxContextManager::GetInstance().ResetViewPortSet();
    }

    // -----------------------------------------------------------------------------

    void SetViewPortSet(CViewPortSetPtr _ViewPortSetPtr)
    {
        return CGfxContextManager::GetInstance().SetViewPortSet(_ViewPortSetPtr);
    }

    // -----------------------------------------------------------------------------

    CViewPortSetPtr GetViewPortSet()
    {
        return CGfxContextManager::GetInstance().GetViewPortSet();
    }

    // -----------------------------------------------------------------------------

    void Draw(unsigned int _NumberOfVertices, unsigned int _IndexOfFirstVertex)
    {
        CGfxContextManager::GetInstance().Draw(_NumberOfVertices, _IndexOfFirstVertex);
    }

    // -----------------------------------------------------------------------------

    void DrawIndexed(unsigned int _NumberOfIndices, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation)
    {
        CGfxContextManager::GetInstance().DrawIndexed(_NumberOfIndices, _IndexOfFirstIndex, _BaseVertexLocation);
    }

    // -----------------------------------------------------------------------------

    void DrawIndexedInstanced(unsigned int _NumberOfIndices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation, unsigned int _StartInstanceLocation)
    {
        CGfxContextManager::GetInstance().DrawIndexedInstanced(_NumberOfIndices, _NumberOfInstances, _IndexOfFirstIndex, _BaseVertexLocation, _StartInstanceLocation);
    }
    
    // -----------------------------------------------------------------------------
    
    void Dispatch(unsigned int _NumberOfThreadGroupsX, unsigned int _NumberOfThreadGroupsY, unsigned int _NumberOfThreadGroupsZ)
    {
        CGfxContextManager::GetInstance().Dispatch(_NumberOfThreadGroupsX, _NumberOfThreadGroupsY, _NumberOfThreadGroupsZ);
    }
} // ContextManager
} // namespace Gfx
