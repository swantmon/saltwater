
#pragma once

#include "base/base_defines.h"
#include "base/base_is_bool.h"

// -----------------------------------------------------------------------------
// Logical Arithmetic
// -----------------------------------------------------------------------------
namespace CORE
{
    // -----------------------------------------------------------------------------
    // Or
    // -----------------------------------------------------------------------------
    template<bool TCond1, bool TCond2, bool TCond3 = false, bool TCond4 = false, bool TCond5 = false, bool TCond6 = false>
    struct SOr;

    template<bool TCond1, bool TCond2, bool TCond3, bool TCond4, bool TCond5, bool TCond6>
    struct SOr : public SIsBool<true>
    { };

    template<>
    struct SOr<false, false, false, false, false> : public SIsBool<false>
    { };

    // -----------------------------------------------------------------------------
    // And
    // -----------------------------------------------------------------------------
    template<bool TCond1, bool TCond2, bool TCond3 = true, bool TCond4 = true, bool TCond5 = true, bool TCond6 = true>
    struct SAnd;

    template<bool TCond1, bool TCond2, bool TCond3, bool TCond4, bool TCond5, bool TCond6>
    struct SAnd : public SIsBool<false>
    { };

    template<>
    struct SAnd<true, true, true, true, true> : public SIsBool<true>
    { };

    // -----------------------------------------------------------------------------
    // If
    // -----------------------------------------------------------------------------
    template<bool TCondition, typename TElement1, typename TElement2>
    struct SIf
    {
        typedef TElement1 X;
    };

    template<typename TElement1, typename TElement2>
    struct SIf<false, TElement1, TElement2>
    {
        typedef TElement2 X;
    };

    // -----------------------------------------------------------------------------
    // Not
    // -----------------------------------------------------------------------------
    template<bool TCondition>
    struct SNot : public SIsBool<true>
    { };

    template<>
    struct SNot<true> : public SIsBool<false>
    { };
} // namespace CORE