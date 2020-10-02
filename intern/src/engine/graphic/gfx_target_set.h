
#pragma once

#include "base/base_managed_pool.h"

#include "engine/graphic/gfx_texture.h"

namespace Gfx
{
    class ENGINE_API CTargetSet : public Base::CManagedPoolItemBase
    {
    public:

        static const unsigned int s_MaxNumberOfRenderTargets = 8;

    public:

        CTexturePtr GetRenderTarget(unsigned int _Index);
        CTexturePtr GetDepthStencilTarget();
        unsigned int GetNumberOfRenderTargets() const;
    
    protected:

        unsigned int m_NumberOfRenderTargets;
        CTexturePtr  m_RenderTargetPtrs[s_MaxNumberOfRenderTargets];
        CTexturePtr  m_DepthStencilTargetPtr;

    protected:

        CTargetSet();
       ~CTargetSet();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CTargetSet> CTargetSetPtr;
} // namespace Gfx
