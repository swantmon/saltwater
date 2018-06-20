
#pragma once

#include "base/base_managed_pool.h"

#include "engine/graphic/gfx_depth_description.h"

namespace Gfx
{
    class CDepthStencilState : public Base::CManagedPoolItemBase
    {
    public:

        enum EState
        {
            Default,
            NoDepth,
            EqualDepth,
            LessEqualDepth,
            NoWriteDepth,
            NumberOfStates,
            UndefinedState = -1,
        };

    public:

        void SetStencilRef(unsigned int _StencilRef);
        unsigned int GetStencilRef() const;
    
        unsigned int GetFlags() const;
        const SDepthDescription& GetDescription() const;

    protected:

        CDepthStencilState();
       ~CDepthStencilState();

    protected:

        unsigned int m_StencilRef;
        unsigned int m_Flags;
        SDepthDescription m_Description;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CDepthStencilState> CDepthStencilStatePtr;
} // namespace Gfx
