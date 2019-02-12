
#pragma once

#include "base/base_defines.h"

namespace CORE
{
    template<typename T>
    struct SRemoveVolatile
    {
        typedef T X;
    };

    template<typename T>
    struct SRemoveVolatile<volatile T>
    {
        typedef T X;
    };

    template<typename T>
    struct SRemoveVolatile<volatile T&>
    {
        typedef T& X;
    };
} // namespace CORE