
#pragma once

#include "base/base_defines.h"
#include "base/base_is_bool.h"

namespace CORE
{
    // -----------------------------------------------------------------------------
    // Integral
    // -----------------------------------------------------------------------------
    template<typename T>
    struct SIsIntegral : public SIsBool<false>
    { };

    template<>
    struct SIsIntegral<bool> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<char> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<wchar_t> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<signed char> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<unsigned char> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<signed short> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<unsigned short> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<signed int> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<unsigned int> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<signed long> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<unsigned long> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<signed long long> : public SIsBool<true>
    { };

    template<>
    struct SIsIntegral<unsigned long long> : public SIsBool<true>
    { };
} // namespace CORE