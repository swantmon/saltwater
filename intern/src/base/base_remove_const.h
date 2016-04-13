
#pragma once

#include "base/base_defines.h"

namespace CORE
{
    template<typename T>
    struct SRemoveConst
    {
        typedef T X;
    };

    template<typename T>
    struct SRemoveConst<const T>
    {
        typedef T X;
    };

    template<typename T>
    struct SRemoveConst<const T&>
    {
        typedef T& X;
    };
} // namespace CORE