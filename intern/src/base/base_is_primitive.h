
#pragma once

#include "base/base_defines.h"
#include "base/base_is_bool.h"
#include "base/base_is_integral.h"
#include "base/base_is_floating_point.h"
#include "base/base_logical_arithmetic.h"

namespace CORE
{
    // -----------------------------------------------------------------------------
    // Bound of elements check
    // -----------------------------------------------------------------------------
    template<typename T>
    struct SIsPrimitive : public SIsBool< SOr< SIsIntegral<T>::Value, SIsFloatingPoint<T>::Value > ::Value >
    { };
} // namespace CORE