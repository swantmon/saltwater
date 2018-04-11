
#pragma once

#include "base/base_managed_pool.h"

#include "engine/graphic/gfx_rasterizer_description.h"

namespace Gfx
{
    class CRasterizerState : public Base::CManagedPoolItemBase
    {
    public:

        enum EState
        {
            Default,
            Wireframe,
            NoCull,
            NumberOfStates,
            UndefinedState = -1,
        };
    
    public:
        
        unsigned int GetFlags() const;
        const SRasterizerDescription& GetDescription() const;

    protected:

        CRasterizerState();
       ~CRasterizerState();
        
    protected:
        
        unsigned int m_Flags;
        SRasterizerDescription m_Description;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CRasterizerState> CRasterizerStatePtr;
} // namespace Gfx