
#pragma once

#include "base/base_managed_pool.h"

#include "engine/graphic/gfx_sampler.h"

namespace Gfx
{
    class CSamplerSet : public Base::CManagedPoolItemBase
    {
        public:

            static const unsigned int s_MaxNumberOfSamplers = 16;

        public:

            unsigned int GetNumberOfSamplers() const;

            CSamplerPtr GetSampler(unsigned int _Index);

        protected:

            unsigned int m_NumberOfSamplers;
            CSamplerPtr  m_SamplerPtrs[s_MaxNumberOfSamplers];

        protected:

            CSamplerSet();
           ~CSamplerSet();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CSamplerSet> CSamplerSetPtr;
} // namespace Gfx
