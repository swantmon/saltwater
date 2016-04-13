
#pragma once

#include "base/base_defines.h"
#include "base/base_is_bool.h"

namespace CORE
{
    // -----------------------------------------------------------------------------
    // Reference
    // -----------------------------------------------------------------------------
    template<typename T>
    struct SIsReference : public SIsBool<false>
    { };

    template<typename T>
    struct SIsReference<T&> : public SIsBool<true>
    { };
} // namespace CORE