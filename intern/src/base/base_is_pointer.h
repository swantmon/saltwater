
#pragma once

#include "base/base_defines.h"
#include "base_is_bool.h"

namespace CORE
{
    // -----------------------------------------------------------------------------
    // Pointer
    // -----------------------------------------------------------------------------
    template<typename T>
    struct SIsPointer : public SIsBool<false>
    { };

    template<typename T>
    struct SIsPointer<T*> : public SIsBool<true>
    { };

    template<typename T>
    struct SIsPointer<T*&> : public SIsBool<true>
    { };
} // namespace CORE