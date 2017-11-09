
#include "graphic/gfx_precompiled.h"

#include "base/base_exception.h"
#include "base/base_managed_pool.h"
#include "base/base_typedef.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_blend_description.h"
#include "graphic/gfx_depth_description.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_rasterizer_description.h"
#include "graphic/gfx_state_manager.h"

#include <assert.h>
#include <exception>

using namespace Gfx;

namespace
{
    class CGfxStateManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxStateManager)
    
    public:

        CGfxStateManager();
       ~CGfxStateManager();

    public:

        void OnStart();
        void OnExit();

    public:

        CBlendStatePtr GetBlendState(unsigned int _Flags);
        CDepthStencilStatePtr GetDepthStencilState(unsigned int _Flags);
        CRasterizerStatePtr GetRasterizerState(unsigned int _Flags);
        CRenderStatePtr GetRenderState(unsigned int _Flags);

    private:

        // -----------------------------------------------------------------------------
        // Represents a unique blend state.
        // -----------------------------------------------------------------------------
        class CInternBlendState : public CBlendState
        {
            public:

                CInternBlendState();
               ~CInternBlendState();

            private:

                friend class CGfxStateManager;
        };

        // -----------------------------------------------------------------------------
        // Represents a unique depth stencil state.
        // -----------------------------------------------------------------------------
        class CInternDepthStencilState : public CDepthStencilState
        {
            public:

                CInternDepthStencilState();
               ~CInternDepthStencilState();

            private:

                friend class CGfxStateManager;
        };

        // -----------------------------------------------------------------------------
        // Represents a unique rasterizer state.
        // -----------------------------------------------------------------------------
        class CInternRasterizerState : public CRasterizerState
        {
            public:

                CInternRasterizerState();
               ~CInternRasterizerState();

            private:

                friend class CGfxStateManager;
        };

        // -----------------------------------------------------------------------------
        // Represents a unique render state.
        // -----------------------------------------------------------------------------
        class CInternRenderState : public CRenderState
        {
            public:

                CInternRenderState();
               ~CInternRenderState();

            private:

                friend class CGfxStateManager;
        };

        // -----------------------------------------------------------------------------
        // The state pools are driven by the user policy because the states cannot be
        // created from outside and the manager already holds a fixed size array of
        // smart pointers to the states.
        // -----------------------------------------------------------------------------
        typedef Base::CManagedPool<CInternBlendState>        CBlendStates;
        typedef Base::CManagedPool<CInternDepthStencilState> CDepthStencilStates;
        typedef Base::CManagedPool<CInternRasterizerState>   CRasterizerStates;
        typedef Base::CManagedPool<CInternRenderState>       CRenderStates;

    private:

        static const unsigned int s_NumberOfBlendStates        = 1 + Gfx::CBlendState::DestinationBlend;
        static const unsigned int s_NumberOfDepthStencilStates = 1 + Gfx::CDepthStencilState::LessEqualDepth;
        static const unsigned int s_NumberOfRasterizerStates   = 1 + Gfx::CRasterizerState::Wireframe + CRasterizerState::NoCull;
        static const unsigned int s_NumberOfRenderStates       = 1 + Gfx::CRenderState::AlphaBlend + Gfx::CRenderState::NoDepth + CRenderState::Wireframe + CRenderState::NoCull + CRenderState::EqualDepth + CRenderState::LessEqualDepth + CRenderState::AdditionBlend + CRenderState::DestinationBlend;

        static SBlendDescription      s_NativeBlendStateDescriptors       [s_NumberOfBlendStates];
        static SDepthDescription      s_NativeDepthStencilStateDescriptors[s_NumberOfDepthStencilStates];
        static SRasterizerDescription s_NativeRasterizerStateDescriptors  [s_NumberOfRasterizerStates];

    private:

        CBlendStates        m_BlendStates;
        CDepthStencilStates m_DepthStencilStates;
        CRasterizerStates   m_RasterizerStates;
        CRenderStates       m_RenderStates;
    
        CBlendStatePtr        m_BlendStatePtrs       [s_NumberOfBlendStates];
        CDepthStencilStatePtr m_DepthStencilStatePtrs[s_NumberOfDepthStencilStates];
        CRasterizerStatePtr   m_RasterizerStatePtrs  [s_NumberOfRasterizerStates];
        CRenderStatePtr       m_RenderStatePtrs      [s_NumberOfRenderStates];
    };
} // namespace

namespace
{
    SBlendDescription CGfxStateManager::s_NativeBlendStateDescriptors[s_NumberOfBlendStates] =
    {
        // -----------------------------------------------------------------------------
        // State                                 | Default Value                |
        // -----------------------------------------------------------------------------
        // AlphaToCoverageEnable                 | FALSE                        |
        // IndependentBlendEnable                | FALSE                        |
        // RenderTarget[0].BlendEnable           | FALSE                        |
        // RenderTarget[0].SrcBlend              | D3D11_BLEND_ONE              |
        // RenderTarget[0].DestBlend             | D3D11_BLEND_AxisZERO             |
        // RenderTarget[0].BlendOp               | D3D11_BLEND_OP_ADD           |
        // RenderTarget[0].SrcBlendAlpha         | D3D11_BLEND_ONE              |
        // RenderTarget[0].DestBlendAlpha        | D3D11_BLEND_AxisZERO             |
        // RenderTarget[0].BlendOpAlpha          | D3D11_BLEND_OP_ADD           |
        // RenderTarget[0].RenderTargetWriteMask | D3D11_COLOR_WRITE_ENABLE_ALL |
        // -----------------------------------------------------------------------------
        // Good Tool:
        // http://www.andersriggelsen.dk/glblendfunc.php
        // -----------------------------------------------------------------------------
        { // Default
            GL_FALSE, GL_FALSE,
            GL_FALSE, GL_ONE, GL_ZERO, GL_FUNC_ADD, GL_ONE, GL_ZERO, GL_FUNC_ADD, 0,
            GL_FALSE, GL_ONE, GL_ZERO, GL_FUNC_ADD, GL_ONE, GL_ZERO, GL_FUNC_ADD, 0,
            GL_FALSE, GL_ONE, GL_ZERO, GL_FUNC_ADD, GL_ONE, GL_ZERO, GL_FUNC_ADD, 0,
            GL_FALSE, GL_ONE, GL_ZERO, GL_FUNC_ADD, GL_ONE, GL_ZERO, GL_FUNC_ADD, 0,
            GL_FALSE, GL_ONE, GL_ZERO, GL_FUNC_ADD, GL_ONE, GL_ZERO, GL_FUNC_ADD, 0,
            GL_FALSE, GL_ONE, GL_ZERO, GL_FUNC_ADD, GL_ONE, GL_ZERO, GL_FUNC_ADD, 0,
            GL_FALSE, GL_ONE, GL_ZERO, GL_FUNC_ADD, GL_ONE, GL_ZERO, GL_FUNC_ADD, 0,
            GL_FALSE, GL_ONE, GL_ZERO, GL_FUNC_ADD, GL_ONE, GL_ZERO, GL_FUNC_ADD, 0,
        },
        { // Alpha Blending
            GL_FALSE, GL_FALSE,
            GL_TRUE , GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, 0,
            GL_TRUE , GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, 0,
            GL_TRUE , GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, 0,
            GL_TRUE , GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, 0,
            GL_TRUE , GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, 0,
            GL_TRUE , GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, 0,
            GL_TRUE , GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, 0,
            GL_TRUE , GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, 0,
        },
        { // Addition Blending
            GL_FALSE, GL_FALSE,
            GL_TRUE , GL_ONE, GL_ONE, GL_FUNC_ADD, GL_ONE, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE, GL_ONE, GL_FUNC_ADD, GL_ONE, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE, GL_ONE, GL_FUNC_ADD, GL_ONE, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE, GL_ONE, GL_FUNC_ADD, GL_ONE, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE, GL_ONE, GL_FUNC_ADD, GL_ONE, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE, GL_ONE, GL_FUNC_ADD, GL_ONE, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE, GL_ONE, GL_FUNC_ADD, GL_ONE, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE, GL_ONE, GL_FUNC_ADD, GL_ONE, GL_ONE, GL_FUNC_ADD, 0,
        },
        { // Destination Blending
            GL_FALSE, GL_FALSE,
            GL_TRUE , GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, 0,
            GL_TRUE , GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_FUNC_ADD, 0,
        },
    };
} // namespace

namespace
{
    SDepthDescription CGfxStateManager::s_NativeDepthStencilStateDescriptors[s_NumberOfDepthStencilStates] =
    {
        // -----------------------------------------------------------------------------
        // State              | Default Value                    |
        // -----------------------------------------------------------------------------
        // DepthEnable        | TRUE                             | glDepth
        // DepthWriteMask     | D3D11_DEPTH_WRITE_MASK_ALL       | glDepthMask
        // DepthFunc          | D3D11_COMPARISON_LESS            | glDepthFunc
        // StencilEnable      | FALSE                            |
        // StencilReadMask    | D3D11_DEFAULT_STENCIL_READ_MASK  |
        // StencilWriteMask   | D3D11_DEFAULT_STENCIL_WRITE_MASK |
        // StencilFunc        | D3D11_STENCIL_OP_KEEP            |
        // StencilDepthFailOp | D3D11_STENCIL_OP_KEEP            |
        // StencilPassOp      | D3D11_STENCIL_OP_KEEP            |
        // StencilFailOp      | D3D11_COMPARISON_ALWAYS          |
        //
        //
        // More information:
        // https://open.gl/depthstencils
        // -----------------------------------------------------------------------------
        { GL_TRUE , 1, GL_LESS  , GL_FALSE, 1, 1, GL_KEEP, GL_KEEP, GL_KEEP, GL_ALWAYS, },
        { GL_FALSE, 1, GL_LESS  , GL_FALSE, 1, 1, GL_KEEP, GL_KEEP, GL_KEEP, GL_ALWAYS, },
        { GL_TRUE , 1, GL_EQUAL , GL_FALSE, 1, 1, GL_KEEP, GL_KEEP, GL_KEEP, GL_ALWAYS, },
        { GL_TRUE , 1, GL_LEQUAL, GL_FALSE, 1, 1, GL_KEEP, GL_KEEP, GL_KEEP, GL_ALWAYS, },
    };
} // namespace

namespace
{
    SRasterizerDescription CGfxStateManager::s_NativeRasterizerStateDescriptors[s_NumberOfRasterizerStates] =
    {
        // -----------------------------------------------------------------------------
        // State                 | Default Value    |
        // -----------------------------------------------------------------------------
        // FillMode              | D3D11_FILL_SOLID | glPolygoneMode
        // CullMode              | D3D11_CULL_BACK  | glCullFace
        // FrontCounterClockwise | GL_CCW           |
        // DepthBias             | 0                |
        // SlopeScaledDepthBias  | 0.0f             |
        // DepthBiasClamp        | 0.0f             |
        // DepthClipEnable       | TRUE             |
        // ScissorEnable         | FALSE            |
        // MultisampleEnable     | FALSE            |
        // AntialiasedLineEnable | FALSE            |
        //
        // More information:
        // https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glCullFace.xml
        // https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glPolygonMode.xml
        // -----------------------------------------------------------------------------
#ifdef __ANDROID__
        { GL_TRUE , GL_NONE, GL_BACK, GL_CCW, 0, 0.0f, 0.0f, GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE, },
        { GL_TRUE , GL_NONE, GL_BACK, GL_CCW, 0, 0.0f, 0.0f, GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE, },
        { GL_FALSE, GL_NONE, GL_BACK, GL_CCW, 0, 0.0f, 0.0f, GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE, },
        { GL_FALSE, GL_NONE, GL_BACK, GL_CCW, 0, 0.0f, 0.0f, GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE, },
#else
        { GL_TRUE , GL_FILL, GL_BACK, GL_CCW, 0, 0.0f, 0.0f, GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE, },
        { GL_TRUE , GL_LINE, GL_BACK, GL_CCW, 0, 0.0f, 0.0f, GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE, },
        { GL_FALSE, GL_FILL, GL_BACK, GL_CCW, 0, 0.0f, 0.0f, GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE, },
        { GL_FALSE, GL_LINE, GL_BACK, GL_CCW, 0, 0.0f, 0.0f, GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE, },
#endif // __ANDROID__
    };
} // namespace

namespace
{
    CGfxStateManager::CInternBlendState::CInternBlendState()
        : CBlendState()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxStateManager::CInternBlendState::~CInternBlendState()
    {
    }
} // namespace

namespace
{
    CGfxStateManager::CInternDepthStencilState::CInternDepthStencilState()
        : CDepthStencilState()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxStateManager::CInternDepthStencilState::~CInternDepthStencilState()
    {
    }
} // namespace

namespace
{
    CGfxStateManager::CInternRasterizerState::CInternRasterizerState()
        : CRasterizerState()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxStateManager::CInternRasterizerState::~CInternRasterizerState()
    {
    }
} // namespace

namespace
{
    CGfxStateManager::CInternRenderState::CInternRenderState()
        : CRenderState()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxStateManager::CInternRenderState::~CInternRenderState()
    {
    }
} // namespace

namespace
{
    CGfxStateManager::CGfxStateManager()
        : m_BlendStates       ()
        , m_DepthStencilStates()
        , m_RasterizerStates  ()
        , m_RenderStates      ()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxStateManager::~CGfxStateManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxStateManager::OnStart()
    {
        unsigned int IndexOfState;

        try
        {
            // -----------------------------------------------------------------------------
            // Setup the blend states.
            // -----------------------------------------------------------------------------
            for (IndexOfState = 0; IndexOfState < s_NumberOfBlendStates; ++ IndexOfState)
            {
                Base::CManagedPoolItemPtr<CInternBlendState> BlendStatePtr = m_BlendStates.Allocate();
                
                BlendStatePtr->m_Flags       = IndexOfState;
                BlendStatePtr->m_Description = s_NativeBlendStateDescriptors[IndexOfState];
                
                m_BlendStatePtrs[IndexOfState] = BlendStatePtr;
            }

            // -----------------------------------------------------------------------------
            // Create the depth stencil states.
            // -----------------------------------------------------------------------------
            for (IndexOfState = 0; IndexOfState < s_NumberOfDepthStencilStates; ++ IndexOfState)
            {
                Base::CManagedPoolItemPtr<CInternDepthStencilState> DepthStencilStatePtr = m_DepthStencilStates.Allocate();
                
                DepthStencilStatePtr->m_Flags       = IndexOfState;
                DepthStencilStatePtr->m_Description = s_NativeDepthStencilStateDescriptors[IndexOfState];
                
                m_DepthStencilStatePtrs[IndexOfState] = DepthStencilStatePtr;
            }

            // -----------------------------------------------------------------------------
            // Create the rasterizer states.
            // -----------------------------------------------------------------------------
            for (IndexOfState = 0; IndexOfState < s_NumberOfRasterizerStates; ++ IndexOfState)
            {
                Base::CManagedPoolItemPtr<CInternRasterizerState> RasterizerStatePtr = m_RasterizerStates.Allocate();
                
                RasterizerStatePtr->m_Flags       = IndexOfState;
                RasterizerStatePtr->m_Description = s_NativeRasterizerStateDescriptors[IndexOfState];
                
                m_RasterizerStatePtrs[IndexOfState] = RasterizerStatePtr;
            }

            // -----------------------------------------------------------------------------
            // Create the render states.
            // -----------------------------------------------------------------------------
            for (IndexOfState = 0; IndexOfState < s_NumberOfRenderStates; ++ IndexOfState)
            {
                Base::CManagedPoolItemPtr<CInternRenderState> RenderStatePtr = m_RenderStates.Allocate();
                
                switch (IndexOfState & (CRenderState::AlphaBlend | CRenderState::AdditionBlend | CRenderState::DestinationBlend))
                {
                    case 0:                              RenderStatePtr->SetBlendState(GetBlendState(0                      )); break;
                    case CRenderState::AlphaBlend:       RenderStatePtr->SetBlendState(GetBlendState(CBlendState::AlphaBlend)); break;
                    case CRenderState::AdditionBlend:    RenderStatePtr->SetBlendState(GetBlendState(CBlendState::AdditionBlend)); break;
                    case CRenderState::DestinationBlend: RenderStatePtr->SetBlendState(GetBlendState(CBlendState::DestinationBlend)); break;
                }
                
                switch (IndexOfState & (CRenderState::NoDepth | CRenderState::EqualDepth | CRenderState::LessEqualDepth))
                {
                    case 0:                            RenderStatePtr->SetDepthStencilState(GetDepthStencilState(0                                 )); break;
                    case CRenderState::NoDepth:        RenderStatePtr->SetDepthStencilState(GetDepthStencilState(CDepthStencilState::NoDepth       )); break;
                    case CRenderState::EqualDepth:     RenderStatePtr->SetDepthStencilState(GetDepthStencilState(CDepthStencilState::EqualDepth    )); break;
                    case CRenderState::LessEqualDepth: RenderStatePtr->SetDepthStencilState(GetDepthStencilState(CDepthStencilState::LessEqualDepth)); break;
                }
                
                switch (IndexOfState & (CRenderState::Wireframe | CRenderState::NoCull))
                {
                    case 0:                                              RenderStatePtr->SetRasterizerState(GetRasterizerState(0                                                     )); break;
                    case CRenderState::Wireframe:                        RenderStatePtr->SetRasterizerState(GetRasterizerState(CRasterizerState::Wireframe                           )); break;
                    case CRenderState::NoCull:                           RenderStatePtr->SetRasterizerState(GetRasterizerState(CRasterizerState::NoCull                              )); break;
                    case CRenderState::Wireframe | CRenderState::NoCull: RenderStatePtr->SetRasterizerState(GetRasterizerState(CRasterizerState::Wireframe | CRasterizerState::NoCull)); break;
                }
                
                RenderStatePtr->m_Flags = IndexOfState;
                
                m_RenderStatePtrs[IndexOfState] = RenderStatePtr;
            }
        }
        catch (...)
        {
            // -----------------------------------------------------------------------------
            // Clear the blend states.
            // -----------------------------------------------------------------------------
            for (IndexOfState = 0; IndexOfState < s_NumberOfBlendStates; ++ IndexOfState)
            {
                m_BlendStatePtrs[IndexOfState] = nullptr;
            }
            
            // -----------------------------------------------------------------------------
            // Clear the depth stencil states.
            // -----------------------------------------------------------------------------
            for (IndexOfState = 0; IndexOfState < s_NumberOfDepthStencilStates; ++ IndexOfState)
            {
                m_DepthStencilStatePtrs[IndexOfState] = nullptr;
            }
            
            // -----------------------------------------------------------------------------
            // Clear the rasterizer states.
            // -----------------------------------------------------------------------------
            for (IndexOfState = 0; IndexOfState < s_NumberOfRasterizerStates; ++ IndexOfState)
            {
                m_RasterizerStatePtrs[IndexOfState] = nullptr;
            }
            
            // -----------------------------------------------------------------------------
            // Clear the render states.
            // -----------------------------------------------------------------------------
            for (IndexOfState = 0; IndexOfState < s_NumberOfRenderStates; ++ IndexOfState)
            {
                m_RenderStatePtrs[IndexOfState] = nullptr;
            }
            
            // -----------------------------------------------------------------------------
            // Clear all container with pointer to different states
            // -----------------------------------------------------------------------------
            m_BlendStates       .Clear();
            m_DepthStencilStates.Clear();
            m_RasterizerStates  .Clear();
            m_RenderStates      .Clear();

            BASE_THROWM("Error creating states");
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxStateManager::OnExit()
    {
        unsigned int IndexOfState;
        
        // -----------------------------------------------------------------------------
        // Clear the blend states.
        // -----------------------------------------------------------------------------
        for (IndexOfState = 0; IndexOfState < s_NumberOfBlendStates; ++ IndexOfState)
        {
            m_BlendStatePtrs[IndexOfState] = nullptr;
        }
        
        // -----------------------------------------------------------------------------
        // Clear the depth stencil states.
        // -----------------------------------------------------------------------------
        for (IndexOfState = 0; IndexOfState < s_NumberOfDepthStencilStates; ++ IndexOfState)
        {
            m_DepthStencilStatePtrs[IndexOfState] = nullptr;
        }
        
        // -----------------------------------------------------------------------------
        // Clear the rasterizer states.
        // -----------------------------------------------------------------------------
        for (IndexOfState = 0; IndexOfState < s_NumberOfRasterizerStates; ++ IndexOfState)
        {
            m_RasterizerStatePtrs[IndexOfState] = nullptr;
        }
        
        // -----------------------------------------------------------------------------
        // Clear the render states.
        // -----------------------------------------------------------------------------
        for (IndexOfState = 0; IndexOfState < s_NumberOfRenderStates; ++ IndexOfState)
        {
            m_RenderStatePtrs[IndexOfState] = nullptr;
        }
        
        // -----------------------------------------------------------------------------
        // Release the state resources.
        // -----------------------------------------------------------------------------
        m_RenderStates      .Clear();
        m_BlendStates       .Clear();
        m_DepthStencilStates.Clear();
        m_RasterizerStates  .Clear();
    }

    // -----------------------------------------------------------------------------

    CBlendStatePtr CGfxStateManager::GetBlendState(unsigned int _Flags)
    {
        return CBlendStatePtr(&(*m_BlendStatePtrs[_Flags]));
    }

    // -----------------------------------------------------------------------------

    CDepthStencilStatePtr CGfxStateManager::GetDepthStencilState(unsigned int _Flags)
    {
        return CDepthStencilStatePtr(&(*m_DepthStencilStatePtrs[_Flags]));
    }

    // -----------------------------------------------------------------------------

    CRasterizerStatePtr CGfxStateManager::GetRasterizerState(unsigned int _Flags)
    {
        return CRasterizerStatePtr(&(*m_RasterizerStatePtrs[_Flags]));
    }

    // -----------------------------------------------------------------------------

    CRenderStatePtr CGfxStateManager::GetRenderState(unsigned int _Flags)
    {
        return CRenderStatePtr(&(*m_RenderStatePtrs[_Flags]));
    }
} // namespace

namespace Gfx
{
namespace StateManager
{
    void OnStart()
    {
        CGfxStateManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxStateManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    CBlendStatePtr GetBlendState(unsigned int _Flags)
    {
        return CGfxStateManager::GetInstance().GetBlendState(_Flags);
    }

    // -----------------------------------------------------------------------------

    CDepthStencilStatePtr GetDepthStencilState(unsigned int _Flags)
    {
        return CGfxStateManager::GetInstance().GetDepthStencilState(_Flags);
    }

    // -----------------------------------------------------------------------------

    CRasterizerStatePtr GetRasterizerState(unsigned int _Flags)
    {
        return CGfxStateManager::GetInstance().GetRasterizerState(_Flags);
    }

    // -----------------------------------------------------------------------------

    CRenderStatePtr GetRenderState(unsigned int _Flags)
    {
        return CGfxStateManager::GetInstance().GetRenderState(_Flags);
    }
} // StateManager
} // namespace Gfx
