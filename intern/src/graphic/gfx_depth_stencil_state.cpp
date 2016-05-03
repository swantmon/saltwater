
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_depth_stencil_state.h"

namespace Gfx
{
    CDepthStencilState::CDepthStencilState()
        : m_StencilRef (0)
        , m_Flags      (0)
        , m_Description()
    {
    }

    // -----------------------------------------------------------------------------

    CDepthStencilState::~CDepthStencilState()
    {
    }

    // -----------------------------------------------------------------------------

    void CDepthStencilState::SetStencilRef(unsigned int _StencilRef)
    { 
        m_StencilRef = _StencilRef; 
    }

    // -----------------------------------------------------------------------------

    unsigned int CDepthStencilState::GetStencilRef() const 
    { 
        return m_StencilRef; 
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CDepthStencilState::GetFlags() const
    {
        return m_Flags;
    }
    
    // -----------------------------------------------------------------------------
    
    const SDepthDescription& CDepthStencilState::GetDescription() const
    {
        return m_Description;
    }
} // namespace Gfx