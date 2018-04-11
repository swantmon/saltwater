
#pragma once

#include "base/base_managed_pool.h"

#include "engine/graphic/gfx_blend_description.h"

namespace Gfx
{
    class CBlendState : public Base::CManagedPoolItemBase
    {
    public:

        enum EState
        {
            Default,
            AlphaBlend,
            AdditionBlend,
            DestinationBlend,
            NumberOfStates,
            UndefinedState = -1,
        };

    public:

        static const unsigned int s_MaxNumberOfBlendFactors = 4;

    public:

        void SetBlendFactors(const float* _pBlendFactors);
        const float* GetBlendFactors() const;

        void SetSampleMask(unsigned int _SampleMask);
        unsigned int GetSampleMask() const;
    
        unsigned int GetFlags() const;
        
        const SBlendDescription& GetDescription() const;

    protected:

        CBlendState();
       ~CBlendState();

    protected:

        float             m_BlendFactors[s_MaxNumberOfBlendFactors];
        unsigned int      m_SampleMask;
        unsigned int      m_Flags;
        SBlendDescription m_Description;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CBlendState> CBlendStatePtr;
} // namespace Gfx