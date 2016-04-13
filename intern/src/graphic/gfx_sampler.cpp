
#include "graphic/gfx_sampler.h"

namespace Gfx
{
    CSampler::CSampler()
        : m_Filter                  (UndefinedFilter)
        , m_ComparisonFunction      (UndefinedComparisonFunction)
        , m_Type                    (UndefinedSampler)
    {
        m_AddressMode[0] = UndefinedAddressMode;
        m_AddressMode[1] = UndefinedAddressMode;
        m_AddressMode[2] = UndefinedAddressMode;
    }

    // -----------------------------------------------------------------------------

    CSampler::~CSampler()
    {
    }

    // -----------------------------------------------------------------------------

    CSampler::EFilter CSampler::GetFilter() const
    {
        return m_Filter;
    }

    // -----------------------------------------------------------------------------

    CSampler::EAddressMode CSampler::GetAddressModeU() const
    {
        return m_AddressMode[0];
    }

    // -----------------------------------------------------------------------------

    CSampler::EAddressMode CSampler::GetAddressModeV() const
    {
        return m_AddressMode[1];
    }

    // -----------------------------------------------------------------------------

    CSampler::EAddressMode CSampler::GetAddressModeW() const
    {
        return m_AddressMode[2];
    }

    // -----------------------------------------------------------------------------

    CSampler::EComparisonFunction CSampler::GetComparisonFunction() const
    {
        return m_ComparisonFunction;
    }

    // -----------------------------------------------------------------------------

    CSampler::ESampler CSampler::GetType() const
    {
        return m_Type;
    }
} // namespace Gfx