
#pragma once

#include "base/base_defines.h"
#include "base/base_is_bool.h"

#include <string>
#include <vector>

namespace CORE
{
    // -----------------------------------------------------------------------------
    // Complex string from STD
    // -----------------------------------------------------------------------------
    template<typename T>
    struct SIsComplexString : public SIsBool<false>
    { };

    template<>
    struct SIsComplexString<std::string> : public SIsBool<true>
    { };

    // -----------------------------------------------------------------------------
    // Collections from STD
    // -----------------------------------------------------------------------------
    template<typename T>
    struct SIsCollection : public SIsBool<false>
    { };

    template<typename T>
    struct SIsCollection<std::vector<T>> : public SIsBool<true>
    { };
} // namespace CORE