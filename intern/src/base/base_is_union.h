
#pragma once

#include "base/base_defines.h"
#include "base/base_is_bool.h"

namespace CORE
{
    // -----------------------------------------------------------------------------
    // Union
    // -----------------------------------------------------------------------------
    template<typename T>
    struct SIsUnion : public SIsBool<__is_union(T)>
    { };
} // namespace CORE