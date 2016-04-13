
#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_texture_base.h"

namespace Gfx
{
    class CTargetSet : public Base::CManagedPoolItemBase
    {
    public:

        static const unsigned int s_MaxNumberOfRenderTargets = 8;

    public:

        CTextureBasePtr GetRenderTarget(unsigned int _Index);
        CTextureBasePtr GetDepthStencilTarget();
        unsigned int GetNumberOfRenderTargets() const;
    
    protected:

        unsigned int            m_NumberOfRenderTargets;
        CTextureBasePtr         m_RenderTargetPtrs[s_MaxNumberOfRenderTargets];
        CTextureBasePtr         m_DepthStencilTargetPtr;

    protected:

        CTargetSet();
       ~CTargetSet();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CTargetSet> CTargetSetPtr;
} // namespace Gfx
