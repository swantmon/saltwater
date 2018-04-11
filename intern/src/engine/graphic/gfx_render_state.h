
#pragma once

#include "base/base_managed_pool.h"

#include "engine/graphic/gfx_blend_state.h"
#include "engine/graphic/gfx_depth_stencil_state.h"
#include "engine/graphic/gfx_rasterizer_state.h"

namespace Gfx
{
    class CRenderState : public Base::CManagedPoolItemBase
    {
    public:

        enum EFlag
        {
            AlphaBlend       =   1,
            NoDepth          =   2,
            Wireframe        =   4,
            NoCull           =   8,
            EqualDepth       =  16,
            LessEqualDepth   =  32,
            AdditionBlend    =  64,
            DestinationBlend = 128,
        };
    
    public:
        
        void SetBlendState(CBlendStatePtr _BlendStatePtr);
        CBlendStatePtr GetBlendState();
        
        void SetDepthStencilState(CDepthStencilStatePtr _DepthStencilStatePtr);
        CDepthStencilStatePtr GetDepthStencilState();
        
        void SetRasterizerState(CRasterizerStatePtr _RasterizerStatePtr);
        CRasterizerStatePtr GetRasterizerState();
    
        unsigned int GetFlags() const;
    
    protected:
        
        CRenderState();
        ~CRenderState();
    
    protected:
    
        CBlendStatePtr        m_BlendStatePtr;
        CDepthStencilStatePtr m_DepthStencilStatePtr;
        CRasterizerStatePtr   m_RasterizerStatePtr;
        unsigned int          m_Flags;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CRenderState> CRenderStatePtr;
} // namespace Gfx