
#pragma once

#include "base/base_defines.h"
#include "base/base_is_bool.h"

// -----------------------------------------------------------------------------
// Further type traits
// -----------------------------------------------------------------------------
namespace CORE
{
    // -----------------------------------------------------------------------------
    // Floating Point
    // -----------------------------------------------------------------------------
    template<typename T>
    struct SIsFloatingPoint : public SIsBool<false>
    { };

    template<>
    struct SIsFloatingPoint<float> : public SIsBool<true>
    { };

    template<>
    struct SIsFloatingPoint<double> : public SIsBool<true>
    { };

    template<>
    struct SIsFloatingPoint<long double> : public SIsBool<true>
    { };
} // namespace CORE