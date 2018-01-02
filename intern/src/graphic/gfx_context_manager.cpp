
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
#include "graphic/gfx_native_texture_3d.h"
#include "graphic/gfx_state_manager.h"

#include <assert.h>

using namespace Gfx;

#ifndef NDEBUG
#define VALIDATE_PIPELINE
#endif // !NDEBUG

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

        void Reset();
    
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

        void ResetTopology();
        void SetTopology(STopology::Enum _Topology);
        STopology::Enum GetTopology() const;

        void ResetTargetSet();
        void SetTargetSet(CTargetSetPtr _TargetSetPtr);
        CTargetSetPtr GetTargetSet();

        void ResetViewPortSet();
        void SetViewPortSet(CViewPortSetPtr _ViewPortSetPtr);
        CViewPortSetPtr GetViewPortSet();

        void ResetInputLayout();
        void SetInputLayout(CInputLayoutPtr _InputLayoutPtr);
        CInputLayoutPtr GetInputLayout();

        void ResetIndexBuffer();
        void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Offset);
        void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Stride, unsigned int _Offset);
        CBufferPtr GetIndexBuffer();

        void ResetVertexBuffer();
        void SetVertexBuffer(CBufferPtr _BufferPtr, bool _ResetInputLayout);
        CBufferPtr GetVertexBuffer();

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

        void ResetSampler(unsigned int _Unit);
        void SetSampler(unsigned int _Unit, CSamplerPtr _SamplerPtr);
        CSamplerPtr GetSampler(unsigned int _Unit);

        void ResetTexture(unsigned int _Unit);
        void SetTexture(unsigned int _Unit, CTextureBasePtr _TextureBasePtr);
        CTextureBasePtr GetTexture(unsigned int _Unit);

        void ResetImageTexture(unsigned int _Unit);
        void SetImageTexture(unsigned int _Unit, CTextureBasePtr _TextureBasePtr);
        CTextureBasePtr GetImageTexture(unsigned int _Unit);

        void ResetConstantBuffer(unsigned int _Unit);
        void SetConstantBuffer(unsigned int _Unit, CBufferPtr _BufferPtr);
        void SetConstantBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range);
        CBufferPtr GetConstantBuffer(unsigned int _Unit);

        void ResetResourceBuffer(unsigned int _Unit);
        void SetResourceBuffer(unsigned int _Unit, CBufferPtr _BufferPtr);
        void SetResourceBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range);
        CBufferPtr GetResourceBuffer(unsigned int _Unit);

		void ResetAtomicCounterBuffer(unsigned int _Unit);
		void SetAtomicCounterBuffer(unsigned int _Unit, CBufferPtr _BufferPtr);
        void SetAtomicCounterBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range);
		CBufferPtr GetAtomicCounterBuffer(unsigned int _Unit);

        void Flush();

        void Barrier();

        void Draw(unsigned int _NumberOfVertices, unsigned int _IndexOfFirstVertex);
        void DrawIndexed(unsigned int _NumberOfIndices, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation);
        void DrawInstanced(unsigned int _NumberOfVertices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstVertex);
        void DrawIndexedInstanced(unsigned int _NumberOfIndices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation, unsigned int _StartInstanceLocation);
        void DrawIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset);
        void DrawIndexedIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset);
        
        void Dispatch(unsigned int _NumberOfThreadGroupsX, unsigned int _NumberOfThreadGroupsY, unsigned int _NumberOfThreadGroupsZ);
        void DispatchIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset);
 
    private:

        void ValidatePipeline();

    private:
    
        static const unsigned int s_NumberOfTextureUnits  = 16;
        static const unsigned int s_NumberOfImageUnits    = 8;
        static const unsigned int s_NumberOfBufferUnits   = 16;
		static const unsigned int s_NumberOfResourceUnits = 16;
		static const unsigned int s_NumberOfAtomicUnits = 16;

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

        unsigned int          m_RenderFlags;
        CBlendStatePtr        m_BlendStatePtr;
        CDepthStencilStatePtr m_DepthStencilStatePtr;
        CRasterizerStatePtr   m_RasterizerStatePtr;
        int                   m_Topology;
        CBufferPtr            m_IndexBufferPtr;
        unsigned int          m_IndexBufferStride;
        unsigned int          m_IndexBufferOffset;
        unsigned int          m_VertexBufferStrides[CBufferSet::s_MaxNumberOfBuffers];
        unsigned int          m_VertexBufferOffsets[CBufferSet::s_MaxNumberOfBuffers];
        CBufferPtr            m_VertexBufferPtr;
        CInputLayoutPtr       m_InputLayoutPtr;
        CTargetSetPtr         m_TargetSetPtr;;
        CViewPortSetPtr       m_ViewPortSetPtr;
        CRenderContexts       m_RenderContexts;
        GLuint                m_NativeShaderPipeline;
        CShaderPtr            m_ShaderSlots[CShader::NumberOfTypes];
        CTextureBasePtr       m_TextureUnits[s_NumberOfTextureUnits];
        CSamplerPtr           m_SamplerUnits[s_NumberOfTextureUnits];
        CTextureBasePtr       m_ImageUnits[s_NumberOfImageUnits];
        CBufferPtr            m_BufferUnits[s_NumberOfBufferUnits];
        CBufferPtr            m_ResourceUnits[s_NumberOfResourceUnits];
        CBufferPtr            m_AtomicUnits[s_NumberOfAtomicUnits];


        GLuint m_EmptyVertexBuffer;

    private:

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
        GL_NONE, // GL_QUAD_STRIP,
        GL_NONE, // GL_POLYGON,
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
        , m_VertexBufferPtr       ()
        , m_InputLayoutPtr        ()
        , m_TargetSetPtr          ()
        , m_ViewPortSetPtr        ()
        , m_RenderContexts        ()
        , m_NativeShaderPipeline  (0)
    {
/*        Reset();*/
    }

    // -----------------------------------------------------------------------------

    CGfxContextManager::~CGfxContextManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::OnStart()
    {
        assert(s_NumberOfTextureUnits < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);

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

        char* pShader;

        if (Main::GetGraphicsAPI() == GLES32)
        {
            pShader = "#version 320 es \n void main(void) { }";
        }
        else
        {
            pShader = "#version 450 \n void main(void) { }";
        }

        ProgramHandle = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &pShader);

        glUseProgramStages(m_NativeShaderPipeline, GL_VERTEX_SHADER_BIT, ProgramHandle);

        glDeleteProgram(ProgramHandle);

        // -----------------------------------------------------------------------------
        // Create an vertex array object and set this to active. This is currently
        // done for the whole pipeline. in case of a better performance, this should be
        // done for each combination of input layout and vertex buffer combination.
        // However, it will be fine as long as we don't need multiple vertex buffer 
        // objects.
        // -----------------------------------------------------------------------------
        GLuint VertexArrayID;

        glGenVertexArrays(1, &VertexArrayID);

        glBindVertexArray(VertexArrayID);

        glGenBuffers(1, &m_EmptyVertexBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, m_EmptyVertexBuffer);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::OnExit()
    {
        Reset();
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::Reset()
    {
        unsigned int IndexOfVertexBuffer;
        unsigned int IndexOfTextureUnit;
        unsigned int IndexOfImageUnit;
        unsigned int IndexOfBufferSlot;
        unsigned int IndexOfResourceSlot;

        for (IndexOfVertexBuffer = 0; IndexOfVertexBuffer < CBufferSet::s_MaxNumberOfBuffers; ++IndexOfVertexBuffer)
        {
            m_VertexBufferStrides[IndexOfVertexBuffer] = 0;
            m_VertexBufferOffsets[IndexOfVertexBuffer] = 0;
        }

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
        ResetVertexBuffer();
        ResetInputLayout();
        ResetShaderVS();
        ResetShaderGS();
        ResetShaderDS();
        ResetShaderHS();
        ResetShaderPS();
        ResetShaderCS();
        ResetTargetSet();
        ResetViewPortSet();

        for (IndexOfTextureUnit = 0; IndexOfTextureUnit < s_NumberOfTextureUnits; ++IndexOfTextureUnit)
        {
            ResetTexture(IndexOfTextureUnit);
            ResetSampler(IndexOfTextureUnit);
        }

        for (IndexOfImageUnit = 0; IndexOfImageUnit < s_NumberOfImageUnits; ++IndexOfImageUnit)
        {
            ResetImageTexture(IndexOfImageUnit);
        }

        for (IndexOfBufferSlot = 0; IndexOfBufferSlot < s_NumberOfBufferUnits; ++IndexOfBufferSlot)
        {
            ResetConstantBuffer(IndexOfBufferSlot);
        }

        for (IndexOfResourceSlot = 0; IndexOfResourceSlot < s_NumberOfResourceUnits; ++IndexOfResourceSlot)
        {
            ResetResourceBuffer(IndexOfResourceSlot);
        }

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

#ifndef __ANDROID__
                glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
#endif // !__ANDROID__
                
                // -----------------------------------------------------------------------------
                
                GLenum StencilFunc;
                int    Mask;
                int    OpSFail;
                int    OpDpFail;
                int    OpDpPass;
                
                // -----------------------------------------------------------------------------
                
#ifndef __ANDROID__
                glActiveStencilFaceEXT(GL_FRONT);
#endif // !__ANDROID__
                
                StencilFunc = rDescription.FrontFace.StencilFunc;
                Mask        = rDescription.StencilReadMask;
                OpSFail     = rDescription.FrontFace.StencilFailOp;
                OpDpFail    = rDescription.FrontFace.StencilDepthFailOp;
                OpDpPass    = rDescription.FrontFace.StencilPassOp;
                
                glStencilOp(OpSFail, OpDpFail, OpDpPass);
                
                glStencilFunc(StencilFunc, 0, 0u);
                
                glStencilMask(Mask);
                
                // -----------------------------------------------------------------------------
                
#ifndef __ANDROID__
                glActiveStencilFaceEXT(GL_BACK);
#endif // !__ANDROID__
                
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
                
#ifndef __ANDROID__
                glPolygonMode(GL_FRONT_AND_BACK, FillMode);
#endif // !__ANDROID__
                
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
                GLenum DrawBuffers[8] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };

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
            CViewPortPtr* pViewportPtrs = _ViewPortSetPtr->GetViewPorts();

            for (unsigned int IndexOfViewport = 0; IndexOfViewport < NumberOfViewports; ++IndexOfViewport)
            {
                CViewPortPtr CurrentViewportPtr = pViewportPtrs[IndexOfViewport];

                GLint TopX = static_cast<GLint>(CurrentViewportPtr->GetTopLeftX());
                GLint TopY = static_cast<GLint>(CurrentViewportPtr->GetTopLeftY());

                GLsizei Width = static_cast<GLsizei>(CurrentViewportPtr->GetWidth());
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
        if (_InputLayoutPtr == nullptr) return;

        unsigned int NumberOfElements = _InputLayoutPtr->GetNumberOfElements();

        for (unsigned int IndexOfElement = 0; IndexOfElement < NumberOfElements; ++ IndexOfElement)
        {
            const Gfx::CInputLayout::CElement& rElement = _InputLayoutPtr->GetElement(IndexOfElement);

            int NativeFormat               = ConvertInputLayoutFormat(rElement.GetFormat());
            int FormatSize                 = ConvertInputLayoutFormatSize(rElement.GetFormat());
            unsigned int Stride            = rElement.GetStride();
            unsigned int AlignedByteOffset = rElement.GetAlignedByteOffset();

            if (rElement.GetInputSlot() != 0)
            {
                BASE_CONSOLE_WARNING("Multiple vertex buffer objects are currently not supported. Undefined behavior expected.");
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

    // -----------------------------------------------------------------------------

    CInputLayoutPtr CGfxContextManager::GetInputLayout()
    {
        return m_InputLayoutPtr;
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

    void CGfxContextManager::ResetVertexBuffer()
    {
        if (m_VertexBufferPtr == nullptr) return;

        glBindBuffer(GL_ARRAY_BUFFER, m_EmptyVertexBuffer);
        
        for (unsigned int IndexOfVertexBuffer = 0; IndexOfVertexBuffer < CBufferSet::s_MaxNumberOfBuffers; ++ IndexOfVertexBuffer)
        {
            m_VertexBufferStrides[IndexOfVertexBuffer] = 0;
            m_VertexBufferOffsets[IndexOfVertexBuffer] = 0;
        }
        
        m_VertexBufferPtr = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetVertexBuffer(CBufferPtr _BufferPtr, bool _UseActiveInputLayout)
    {
        assert(_BufferPtr != nullptr);
        
        Gfx::CNativeBuffer& rNativeBuffer = *static_cast<Gfx::CNativeBuffer*>(_BufferPtr.GetPtr());
        
        if (m_VertexBufferPtr != _BufferPtr)
        {            
            glBindBuffer(GL_ARRAY_BUFFER, rNativeBuffer.m_NativeBuffer);

            if (_UseActiveInputLayout)
            {
                SetInputLayout(m_InputLayoutPtr);
            }

            m_VertexBufferPtr = _BufferPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CGfxContextManager::GetVertexBuffer()
    {
        return m_VertexBufferPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetShaderVS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_VERTEX_SHADER_BIT, 0);

        m_ShaderSlots[CShader::Vertex] = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetShaderVS(CShaderPtr _ShaderSetPtr)
    {
        assert(_ShaderSetPtr != nullptr);

        if (m_ShaderSlots[CShader::Vertex] != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());

            glUseProgramStages(m_NativeShaderPipeline, GL_VERTEX_SHADER_BIT, rShaderSet.m_NativeShader);

            m_ShaderSlots[CShader::Vertex] = _ShaderSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxContextManager::GetShaderVS()
    {
        return m_ShaderSlots[CShader::Vertex];
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetShaderHS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_TESS_CONTROL_SHADER_BIT, 0);

        m_ShaderSlots[CShader::Hull] = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetShaderHS(CShaderPtr _ShaderSetPtr)
    {
        if (_ShaderSetPtr == nullptr) return;

        if (m_ShaderSlots[CShader::Hull] != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());

            glUseProgramStages(m_NativeShaderPipeline, GL_TESS_CONTROL_SHADER_BIT, rShaderSet.m_NativeShader);

            m_ShaderSlots[CShader::Hull] = _ShaderSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxContextManager::GetShaderHS()
    {
        return m_ShaderSlots[CShader::Hull];
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetShaderDS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_TESS_EVALUATION_SHADER_BIT, 0);

        m_ShaderSlots[CShader::Domain] = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetShaderDS(CShaderPtr _ShaderSetPtr)
    {
        if (_ShaderSetPtr == nullptr) return;

        if (m_ShaderSlots[CShader::Domain] != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());

            glUseProgramStages(m_NativeShaderPipeline, GL_TESS_EVALUATION_SHADER_BIT, rShaderSet.m_NativeShader);

            m_ShaderSlots[CShader::Domain] = _ShaderSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxContextManager::GetShaderDS()
    {
        return m_ShaderSlots[CShader::Domain];
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetShaderGS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_GEOMETRY_SHADER_BIT, 0);

        m_ShaderSlots[CShader::Geometry] = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetShaderGS(CShaderPtr _ShaderSetPtr)
    {
        if (_ShaderSetPtr == nullptr) return;

        if (m_ShaderSlots[CShader::Geometry] != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());

            glUseProgramStages(m_NativeShaderPipeline, GL_GEOMETRY_SHADER_BIT, rShaderSet.m_NativeShader);

            m_ShaderSlots[CShader::Geometry] = _ShaderSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxContextManager::GetShaderGS()
    {
        return m_ShaderSlots[CShader::Geometry];
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetShaderPS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_FRAGMENT_SHADER_BIT, 0);

        m_ShaderSlots[CShader::Pixel] = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetShaderPS(CShaderPtr _ShaderSetPtr)
    {
        assert(_ShaderSetPtr != nullptr);

        if (m_ShaderSlots[CShader::Pixel] != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());

            glUseProgramStages(m_NativeShaderPipeline, GL_FRAGMENT_SHADER_BIT, rShaderSet.m_NativeShader);

            m_ShaderSlots[CShader::Pixel] = _ShaderSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxContextManager::GetShaderPS()
    {
        return m_ShaderSlots[CShader::Pixel];
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetShaderCS()
    {
        glUseProgramStages(m_NativeShaderPipeline, GL_COMPUTE_SHADER_BIT, 0);

        m_ShaderSlots[CShader::Compute] = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetShaderCS(CShaderPtr _ShaderSetPtr)
    {
        assert(_ShaderSetPtr != nullptr);

        if (m_ShaderSlots[CShader::Compute] != _ShaderSetPtr)
        {
            Gfx::CNativeShader& rShaderSet = *static_cast<Gfx::CNativeShader*>(_ShaderSetPtr.GetPtr());

            glUseProgramStages(m_NativeShaderPipeline, GL_COMPUTE_SHADER_BIT, rShaderSet.m_NativeShader);

            m_ShaderSlots[CShader::Compute] = _ShaderSetPtr;
        }
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxContextManager::GetShaderCS()
    {
        return m_ShaderSlots[CShader::Compute];
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetSampler(unsigned int _Unit)
    {
        glBindSampler(_Unit, 0);

        m_SamplerUnits[_Unit] = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetSampler(unsigned int _Unit, CSamplerPtr _SamplerPtr)
    {
        if (_SamplerPtr == nullptr) return;

        CNativeSampler* pNativeSampler = 0;

        assert(_Unit < s_NumberOfTextureUnits);

        if (m_SamplerUnits[_Unit] == _SamplerPtr) return;

        pNativeSampler = static_cast<CNativeSampler*>(_SamplerPtr.GetPtr());

        glBindSampler(_Unit, pNativeSampler->m_NativeSampler);

        m_SamplerUnits[_Unit] = _SamplerPtr;
    }

    // -----------------------------------------------------------------------------

    CSamplerPtr CGfxContextManager::GetSampler(unsigned int _Unit)
    {
        return m_SamplerUnits[_Unit];
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetTexture(unsigned int _Unit)
    {
        assert(_Unit < s_NumberOfTextureUnits);

        GLuint TextureBinding = GL_TEXTURE_2D;

        if (m_TextureUnits[_Unit] != NULL && m_TextureUnits[_Unit]->IsCube())
        {
            TextureBinding = GL_TEXTURE_CUBE_MAP;
        }

        glBindTexture(TextureBinding, 0);

        m_TextureUnits[_Unit] = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetTexture(unsigned int _Unit, CTextureBasePtr _TextureBasePtr)
    {
        if (_TextureBasePtr == nullptr) return;

        CNativeTexture2D* pNativeTexture  = 0;

        assert(_Unit < s_NumberOfTextureUnits);

        if (m_TextureUnits[_Unit] == _TextureBasePtr) return;

        GLuint TextureBinding = 0;
        GLuint TextureHandle = 0;

        if (_TextureBasePtr->GetDimension() == CTextureBase::Dim2D)
        {
            CNativeTexture2D& rNativeTexture = *static_cast<CNativeTexture2D*>(_TextureBasePtr.GetPtr());

            TextureBinding = rNativeTexture.m_NativeDimension;
            TextureHandle  = rNativeTexture.m_NativeTexture;
        }
        else if (_TextureBasePtr->GetDimension() == CTextureBase::Dim3D)
        {
            CNativeTexture3D& rNativeTexture = *static_cast<CNativeTexture3D*>(_TextureBasePtr.GetPtr());

            TextureBinding = rNativeTexture.m_NativeDimension;
            TextureHandle  = rNativeTexture.m_NativeTexture;
        }
        else
        {
            BASE_CONSOLE_ERROR("Unexpected texture type set to context");
        }

        if (_TextureBasePtr->IsCube())
        {
            TextureBinding = GL_TEXTURE_CUBE_MAP;
        }

        glActiveTexture(GL_TEXTURE0 + _Unit);

        glBindTexture(TextureBinding, TextureHandle);

        m_TextureUnits[_Unit] = _TextureBasePtr;
    }

    // -----------------------------------------------------------------------------

    CTextureBasePtr CGfxContextManager::GetTexture(unsigned int _Unit)
    {
        assert(_Unit < s_NumberOfTextureUnits);

        return m_TextureUnits[_Unit];
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetImageTexture(unsigned int _Unit)
    {
        assert(_Unit < s_NumberOfImageUnits);

        glBindImageTexture(_Unit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);

        m_ImageUnits[_Unit] = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetImageTexture(unsigned int _Unit, CTextureBasePtr _TextureBasePtr)
    {
        assert(_Unit < s_NumberOfImageUnits);

        if (_TextureBasePtr == nullptr || m_ImageUnits[_Unit] == _TextureBasePtr) return;

        GLboolean IsLayered  = GL_FALSE;
        GLuint TextureUsage  = 0;
        GLuint TextureHandle = 0;
        GLuint TextureFormat = 0;

        if (_TextureBasePtr->GetDimension() == CTextureBase::Dim2D)
        {
            CNativeTexture2D& rNativeTexture = *static_cast<CNativeTexture2D*>(_TextureBasePtr.GetPtr());

            TextureHandle = rNativeTexture.m_NativeTexture;
            TextureUsage  = rNativeTexture.m_NativeUsage;
            TextureFormat = rNativeTexture.m_NativeInternalFormat;
        }
        else if (_TextureBasePtr->GetDimension() == CTextureBase::Dim3D)
        {
            CNativeTexture3D& rNativeTexture = *static_cast<CNativeTexture3D*>(_TextureBasePtr.GetPtr());

            TextureHandle = rNativeTexture.m_NativeTexture;
            TextureUsage  = rNativeTexture.m_NativeUsage;
            TextureFormat = rNativeTexture.m_NativeInternalFormat;
        }
        else
        {
            BASE_CONSOLE_ERROR("Unexpected image texture type set to context");
        }

        if (_TextureBasePtr->GetDimension() == CTextureBase::Dim3D) IsLayered = GL_TRUE;

        glBindImageTexture(_Unit, TextureHandle, _TextureBasePtr->GetCurrentMipLevel(), IsLayered, 0, TextureUsage, TextureFormat);

        m_ImageUnits[_Unit] = _TextureBasePtr;
    }

    // -----------------------------------------------------------------------------

    CTextureBasePtr CGfxContextManager::GetImageTexture(unsigned int _Unit)
    {
        assert(_Unit < s_NumberOfImageUnits);

        return m_ImageUnits[_Unit];
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetConstantBuffer(unsigned int _Unit)
    {
        assert(_Unit < s_NumberOfBufferUnits);

        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        m_BufferUnits[_Unit] = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetConstantBuffer(unsigned int _Unit, CBufferPtr _BufferPtr)
    {
        if (_BufferPtr == nullptr) return;

        CNativeBuffer* pNativeBuffer = 0;

        assert(_Unit < s_NumberOfBufferUnits);

        if (m_BufferUnits[_Unit] == _BufferPtr) return;

        pNativeBuffer = static_cast<CNativeBuffer*>(_BufferPtr.GetPtr());

        assert(pNativeBuffer->GetBinding() == CBuffer::ConstantBuffer);

        glBindBufferRange(GL_UNIFORM_BUFFER, _Unit, pNativeBuffer->m_NativeBuffer, 0, pNativeBuffer->GetNumberOfBytes());

        m_BufferUnits[_Unit] = _BufferPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetConstantBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range)
    {
        if (_BufferPtr == nullptr) return;

        CNativeBuffer* pNativeBuffer = 0;

        assert(_Unit < s_NumberOfBufferUnits);

        if (m_BufferUnits[_Unit] == _BufferPtr) return;

        pNativeBuffer = static_cast<CNativeBuffer*>(_BufferPtr.GetPtr());

        assert(pNativeBuffer->GetBinding() == CBuffer::ConstantBuffer);

        glBindBufferRange(GL_UNIFORM_BUFFER, _Unit, pNativeBuffer->m_NativeBuffer, _Offset, _Range);

        m_BufferUnits[_Unit] = 0; // TODO: store range binding
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CGfxContextManager::GetConstantBuffer(unsigned int _Unit)
    {
        assert(_Unit < s_NumberOfBufferUnits);

        return m_BufferUnits[_Unit];
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::ResetResourceBuffer(unsigned int _Unit)
    {
        assert(_Unit < s_NumberOfResourceUnits);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        m_ResourceUnits[_Unit] = 0;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetResourceBuffer(unsigned int _Unit, CBufferPtr _BufferPtr)
    {
        if (_BufferPtr == nullptr) return;

        CNativeBuffer* pNativeBuffer = 0;

        assert(_Unit < s_NumberOfResourceUnits);

        if (m_ResourceUnits[_Unit] == _BufferPtr) return;

        pNativeBuffer = static_cast<CNativeBuffer*>(_BufferPtr.GetPtr());

        assert(pNativeBuffer->GetBinding() == CBuffer::ResourceBuffer);

        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, _Unit, pNativeBuffer->m_NativeBuffer, 0, pNativeBuffer->GetNumberOfBytes());
		
        m_ResourceUnits[_Unit] = _BufferPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetResourceBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range)
    {
        if (_BufferPtr == nullptr) return;

        CNativeBuffer* pNativeBuffer = 0;

        assert(_Unit < s_NumberOfResourceUnits);

        if (m_ResourceUnits[_Unit] == _BufferPtr) return;

        pNativeBuffer = static_cast<CNativeBuffer*>(_BufferPtr.GetPtr());

        assert(pNativeBuffer->GetBinding() == CBuffer::ResourceBuffer);

        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, _Unit, pNativeBuffer->m_NativeBuffer, _Offset, _Range);

        m_ResourceUnits[_Unit] = 0; // TODO: store range binding
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CGfxContextManager::GetResourceBuffer(unsigned int _Unit)
    {
        assert(_Unit < s_NumberOfResourceUnits);

        return m_ResourceUnits[_Unit];
    }

	// -----------------------------------------------------------------------------

	void CGfxContextManager::ResetAtomicCounterBuffer(unsigned int _Unit)
	{
		assert(_Unit < s_NumberOfAtomicUnits);

		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

		m_AtomicUnits[_Unit] = 0;
	}

	// -----------------------------------------------------------------------------

	void CGfxContextManager::SetAtomicCounterBuffer(unsigned int _Unit, CBufferPtr _BufferPtr)
	{
		if (_BufferPtr == nullptr) return;

		CNativeBuffer* pNativeBuffer = 0;

		assert(_Unit < s_NumberOfAtomicUnits);

		if (m_AtomicUnits[_Unit] == _BufferPtr) return;

		pNativeBuffer = static_cast<CNativeBuffer*>(_BufferPtr.GetPtr());

		assert(pNativeBuffer->GetBinding() == CBuffer::AtomicCounterBuffer);

		glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, _Unit, pNativeBuffer->m_NativeBuffer, 0, pNativeBuffer->GetNumberOfBytes());

		m_AtomicUnits[_Unit] = _BufferPtr;
	}

    // -----------------------------------------------------------------------------

    void CGfxContextManager::SetAtomicCounterBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range)
    {
        if (_BufferPtr == nullptr) return;

        CNativeBuffer* pNativeBuffer = 0;

        assert(_Unit < s_NumberOfAtomicUnits);

        if (m_AtomicUnits[_Unit] == _BufferPtr) return;

        pNativeBuffer = static_cast<CNativeBuffer*>(_BufferPtr.GetPtr());

        assert(pNativeBuffer->GetBinding() == CBuffer::AtomicCounterBuffer);

        glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, _Unit, pNativeBuffer->m_NativeBuffer, _Offset, _Range);

        m_AtomicUnits[_Unit] = 0; // TODO: store range binding
    }

	// -----------------------------------------------------------------------------

	CBufferPtr CGfxContextManager::GetAtomicCounterBuffer(unsigned int _Unit)
	{
		assert(_Unit < s_NumberOfAtomicUnits);

		return m_AtomicUnits[_Unit];
	}

    // -----------------------------------------------------------------------------

    void CGfxContextManager::Flush()
    {
        glFinish();
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::Barrier()
    {
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::Draw(unsigned int _NumberOfVertices, unsigned int _IndexOfFirstVertex)
    {
        ValidatePipeline();
        glDrawArrays(s_NativeTopologies[m_Topology], _IndexOfFirstVertex, _NumberOfVertices);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::DrawIndexed(unsigned int _NumberOfIndices, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation)
    {
        BASE_UNUSED(_IndexOfFirstIndex);
        BASE_UNUSED(_BaseVertexLocation);

        ValidatePipeline();
        glDrawElements(s_NativeTopologies[m_Topology], _NumberOfIndices, GL_UNSIGNED_INT, 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::DrawInstanced(unsigned int _NumberOfVertices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstVertex)
    {
        ValidatePipeline();
        glDrawArraysInstanced(s_NativeTopologies[m_Topology], _IndexOfFirstVertex, _NumberOfVertices, _NumberOfInstances);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::DrawIndexedInstanced(unsigned int _NumberOfIndices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation, unsigned int _StartInstanceLocation)
    {
        BASE_UNUSED(_IndexOfFirstIndex);
        BASE_UNUSED(_BaseVertexLocation);
        BASE_UNUSED(_StartInstanceLocation);

        ValidatePipeline();
        glDrawElementsInstanced(s_NativeTopologies[m_Topology], _NumberOfIndices, GL_UNSIGNED_INT, 0, _NumberOfInstances);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::DrawIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset)
    {
        Gfx::CNativeBuffer& rNativeBuffer = *static_cast<Gfx::CNativeBuffer*>(_IndirectBufferPtr.GetPtr());

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, rNativeBuffer.m_NativeBuffer);

        ValidatePipeline();
        glDrawArraysIndirect(s_NativeTopologies[m_Topology], reinterpret_cast<void*>(_Offset));

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::DrawIndexedIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset)
    {
        Gfx::CNativeBuffer& rNativeBuffer = *static_cast<Gfx::CNativeBuffer*>(_IndirectBufferPtr.GetPtr());

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, rNativeBuffer.m_NativeBuffer);

        ValidatePipeline();
        glDrawElementsIndirect(s_NativeTopologies[m_Topology], GL_UNSIGNED_INT, reinterpret_cast<void*>(_Offset));

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::Dispatch(unsigned int _NumberOfThreadGroupsX, unsigned int _NumberOfThreadGroupsY, unsigned int _NumberOfThreadGroupsZ)
    {
        assert(_NumberOfThreadGroupsX > 0 && _NumberOfThreadGroupsY > 0 && _NumberOfThreadGroupsZ > 0);

        ValidatePipeline();
        glDispatchCompute(_NumberOfThreadGroupsX, _NumberOfThreadGroupsY, _NumberOfThreadGroupsZ);
    }

    // -----------------------------------------------------------------------------

    void CGfxContextManager::DispatchIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset)
    {
        Gfx::CNativeBuffer& rNativeBuffer = *static_cast<Gfx::CNativeBuffer*>(_IndirectBufferPtr.GetPtr());

        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, rNativeBuffer.m_NativeBuffer);

        ValidatePipeline();
        glDispatchComputeIndirect(_Offset);

        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
    }

    // -----------------------------------------------------------------------------
    
    void CGfxContextManager::ValidatePipeline()
    {
#ifdef VALIDATE_PIPELINE

        glValidateProgramPipeline(m_NativeShaderPipeline);

        GLint IsValid = GL_TRUE;

        glGetProgramPipelineiv(m_NativeShaderPipeline, GL_VALIDATE_STATUS, &IsValid);

        if (IsValid != GL_TRUE)
        {
            GLint LogLength;
            glGetProgramPipelineiv(m_NativeShaderPipeline, GL_INFO_LOG_LENGTH, &LogLength);

            GLchar* pInfoLog = new char[LogLength];
            glGetProgramPipelineInfoLog(m_NativeShaderPipeline, LogLength, &LogLength, pInfoLog);
            BASE_CONSOLE_ERROR(pInfoLog);
            delete[] pInfoLog;
        }
#endif
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
        CGfxContextManager::GetInstance().ResetViewPortSet();
    }

    // -----------------------------------------------------------------------------

    void SetViewPortSet(CViewPortSetPtr _ViewPortSetPtr)
    {
        CGfxContextManager::GetInstance().SetViewPortSet(_ViewPortSetPtr);
    }

    // -----------------------------------------------------------------------------

    CViewPortSetPtr GetViewPortSet()
    {
        return CGfxContextManager::GetInstance().GetViewPortSet();
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

    void ResetVertexBuffer()
    {
        CGfxContextManager::GetInstance().ResetVertexBuffer();
    }

    // -----------------------------------------------------------------------------

    void SetVertexBuffer(CBufferPtr _BufferPtr, bool _ResetInputLayout)
    {
        CGfxContextManager::GetInstance().SetVertexBuffer(_BufferPtr, _ResetInputLayout);
    }

    // -----------------------------------------------------------------------------

    CBufferPtr GetVertexBuffer()
    {
        return CGfxContextManager::GetInstance().GetVertexBuffer();
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

    void ResetSampler(unsigned int _Unit)
    {
        CGfxContextManager::GetInstance().ResetSampler(_Unit);
    }

    // -----------------------------------------------------------------------------

    void SetSampler(unsigned int _Unit, CSamplerPtr _SamplerPtr)
    {
        CGfxContextManager::GetInstance().SetSampler(_Unit, _SamplerPtr);
    }

    // -----------------------------------------------------------------------------


    CSamplerPtr GetSampler(unsigned int _Unit)
    {
        return CGfxContextManager::GetInstance().GetSampler(_Unit);
    }

    // -----------------------------------------------------------------------------

    void ResetTexture(unsigned int _Unit)
    {
        CGfxContextManager::GetInstance().ResetTexture(_Unit);
    }

    // -----------------------------------------------------------------------------

    void SetTexture(unsigned int _Unit, CTextureBasePtr _TextureBasePtr)
    {
        CGfxContextManager::GetInstance().SetTexture(_Unit, _TextureBasePtr);
    }

    // -----------------------------------------------------------------------------

    CTextureBasePtr GetTexture(unsigned int _Unit)
    {
        return CGfxContextManager::GetInstance().GetTexture(_Unit);
    }

    // -----------------------------------------------------------------------------

    void ResetImageTexture(unsigned int _Unit)
    {
        CGfxContextManager::GetInstance().ResetImageTexture(_Unit);
    }

    // -----------------------------------------------------------------------------

    void SetImageTexture(unsigned int _Unit, CTextureBasePtr _TextureBasePtr)
    {
        CGfxContextManager::GetInstance().SetImageTexture(_Unit, _TextureBasePtr);
    }

    // -----------------------------------------------------------------------------

    CTextureBasePtr GetImageTexture(unsigned int _Unit)
    {
        return CGfxContextManager::GetInstance().GetImageTexture(_Unit);
    }

    // -----------------------------------------------------------------------------

    void ResetConstantBuffer(unsigned int _Unit)
    {
        CGfxContextManager::GetInstance().ResetConstantBuffer(_Unit);
    }

    // -----------------------------------------------------------------------------

    void SetConstantBuffer(unsigned int _Unit, CBufferPtr _BufferPtr)
    {
        CGfxContextManager::GetInstance().SetConstantBuffer(_Unit, _BufferPtr);
    }

    // -----------------------------------------------------------------------------

    void SetConstantBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range)
    {
        CGfxContextManager::GetInstance().SetConstantBufferRange(_Unit, _BufferPtr, _Offset, _Range);
    }

    // -----------------------------------------------------------------------------

    CBufferPtr GetConstantBuffer(unsigned int _Unit)
    {
        return CGfxContextManager::GetInstance().GetConstantBuffer(_Unit);
    }

    // -----------------------------------------------------------------------------

    void ResetResourceBuffer(unsigned int _Unit)
    {
        CGfxContextManager::GetInstance().ResetResourceBuffer(_Unit);
    }

    // -----------------------------------------------------------------------------

    void SetResourceBuffer(unsigned int _Unit, CBufferPtr _BufferPtr)
    {
        CGfxContextManager::GetInstance().SetResourceBuffer(_Unit, _BufferPtr);
    }

    // -----------------------------------------------------------------------------

    void SetResourceBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range)
    {
        CGfxContextManager::GetInstance().SetResourceBufferRange(_Unit, _BufferPtr, _Offset, _Range);
    }

    // -----------------------------------------------------------------------------

    CBufferPtr GetResourceBuffer(unsigned int _Unit)
    {
        return CGfxContextManager::GetInstance().GetResourceBuffer(_Unit);
    }

	// -----------------------------------------------------------------------------

	void ResetAtomicCounterBuffer(unsigned int _Unit)
	{
		CGfxContextManager::GetInstance().ResetAtomicCounterBuffer(_Unit);
	}

	// -----------------------------------------------------------------------------

	void SetAtomicCounterBuffer(unsigned int _Unit, CBufferPtr _BufferPtr)
	{
		CGfxContextManager::GetInstance().SetAtomicCounterBuffer(_Unit, _BufferPtr);
	}

    // -----------------------------------------------------------------------------

    void SetAtomicCounterBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range)
    {
        CGfxContextManager::GetInstance().SetAtomicCounterBufferRange(_Unit, _BufferPtr, _Offset, _Range);
    }

	// -----------------------------------------------------------------------------

	CBufferPtr GetAtomicCounterBuffer(unsigned int _Unit)
	{
		return CGfxContextManager::GetInstance().GetAtomicCounterBuffer(_Unit);
	}

    // -----------------------------------------------------------------------------

    void Flush()
    {
        CGfxContextManager::GetInstance().Flush();
    }

    // -----------------------------------------------------------------------------

    void Barrier()
    {
        CGfxContextManager::GetInstance().Barrier();
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

    void DrawInstanced(unsigned int _NumberOfVertices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstVertex)
    {
        CGfxContextManager::GetInstance().DrawInstanced(_NumberOfVertices, _NumberOfInstances, _IndexOfFirstVertex);
    }

    // -----------------------------------------------------------------------------

    void DrawIndexedInstanced(unsigned int _NumberOfIndices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation, unsigned int _StartInstanceLocation)
    {
        CGfxContextManager::GetInstance().DrawIndexedInstanced(_NumberOfIndices, _NumberOfInstances, _IndexOfFirstIndex, _BaseVertexLocation, _StartInstanceLocation);
    }
    
    // -----------------------------------------------------------------------------

    void DrawIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset)
    {
        CGfxContextManager::GetInstance().DrawIndirect(_IndirectBufferPtr, _Offset);
    }

    // -----------------------------------------------------------------------------

    void DrawIndexedIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset)
    {
        CGfxContextManager::GetInstance().DrawIndexedIndirect(_IndirectBufferPtr, _Offset);
    }

    // -----------------------------------------------------------------------------
    
    void Dispatch(unsigned int _NumberOfThreadGroupsX, unsigned int _NumberOfThreadGroupsY, unsigned int _NumberOfThreadGroupsZ)
    {
        CGfxContextManager::GetInstance().Dispatch(_NumberOfThreadGroupsX, _NumberOfThreadGroupsY, _NumberOfThreadGroupsZ);
    }

    // -----------------------------------------------------------------------------

    void DispatchIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset)
    {
        CGfxContextManager::GetInstance().DispatchIndirect(_IndirectBufferPtr, _Offset);
    }
} // ContextManager
} // namespace Gfx
