
#include "engine/engine_precompiled.h"

#include "graphic/gfx_sampler_set.h"

#include <assert.h>

namespace Gfx
{
    CSamplerSet::CSamplerSet()
        : m_NumberOfSamplers(0)
    {
    }

    // -----------------------------------------------------------------------------

    CSamplerSet::~CSamplerSet()
    {
    }

    // -----------------------------------------------------------------------------

    unsigned int CSamplerSet::GetNumberOfSamplers() const
    {
        return m_NumberOfSamplers;
    }

    // -----------------------------------------------------------------------------

    CSamplerPtr CSamplerSet::GetSampler(unsigned int _Index)
    {
        assert(_Index < s_MaxNumberOfSamplers);

        return m_SamplerPtrs[_Index];
    }
} // namespace Gfx
