
#include "engine/engine_precompiled.h"

#include "engine/graphic/gfx_target_set.h"

namespace Gfx
{
    CTargetSet::CTargetSet()
        : m_NumberOfRenderTargets  (0)
        , m_DepthStencilTargetPtr  ()
    {
    }

    // -----------------------------------------------------------------------------

    CTargetSet::~CTargetSet()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CTexturePtr CTargetSet::GetRenderTarget(unsigned int _Index)
    {
        assert(_Index < m_NumberOfRenderTargets);
        
        return m_RenderTargetPtrs[_Index];
    }
    
    // -----------------------------------------------------------------------------
    
    CTexturePtr CTargetSet::GetDepthStencilTarget()
    {
        return m_DepthStencilTargetPtr;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTargetSet::GetNumberOfRenderTargets() const
    {
        return m_NumberOfRenderTargets;
    }
} // namespace Gfx
