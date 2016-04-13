
#include "graphic/gfx_render_state.h"

namespace Gfx
{
    CRenderState::CRenderState()
        : m_BlendStatePtr       ()
        , m_DepthStencilStatePtr()
        , m_RasterizerStatePtr  ()
        , m_Flags               (AlphaBlend)
    {
    }

    // -----------------------------------------------------------------------------

    CRenderState::~CRenderState()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CRenderState::SetBlendState(CBlendStatePtr _BlendStatePtr)
    {
        m_BlendStatePtr = _BlendStatePtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CBlendStatePtr CRenderState::GetBlendState()
    {
        return m_BlendStatePtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CRenderState::SetDepthStencilState(CDepthStencilStatePtr _DepthStencilStatePtr)
    {
        m_DepthStencilStatePtr = _DepthStencilStatePtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CDepthStencilStatePtr CRenderState::GetDepthStencilState()
    {
        return m_DepthStencilStatePtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CRenderState::SetRasterizerState(CRasterizerStatePtr _RasterizerStatePtr)
    {
        m_RasterizerStatePtr = _RasterizerStatePtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CRasterizerStatePtr CRenderState::GetRasterizerState()
    {
        return m_RasterizerStatePtr;
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CRenderState::GetFlags() const
    {
        return m_Flags;
    }
} // namespace Gfx
