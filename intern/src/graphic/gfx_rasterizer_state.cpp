
#include "graphic/gfx_rasterizer_state.h"

namespace Gfx
{
    CRasterizerState::CRasterizerState()
        : m_Flags      (0)
        , m_Description()
    {
    }

    // -----------------------------------------------------------------------------

    CRasterizerState::~CRasterizerState()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CRasterizerState::GetFlags() const
    {
        return m_Flags;
    }
    
    // -----------------------------------------------------------------------------
    
    const SRasterizerDescription& CRasterizerState::GetDescription() const
    {
        return m_Description;
    }
} // namespace Gfx