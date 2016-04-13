
#pragma once

#include "base/base_defines.h"
#include "base/base_is_bool.h"

namespace CORE
{
    // -----------------------------------------------------------------------------
    // Array
    // -----------------------------------------------------------------------------
    template<typename T>
    struct SIsArray : public SIsBool<false>
    { };

    template<typename T, unsigned int TNumberOfElements>
    struct SIsArray<T[TNumberOfElements]> : public SIsBool<true>
    { };

    template<typename T>
    struct SIsArray<T[]> : public SIsBool<true>
    { };
} // namespace CORE