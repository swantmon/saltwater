
#pragma once

#include "base/base_defines.h"

namespace CORE
{
	struct SNull
    { };

    template<typename T>
    struct SRemovePointer
    {
        typedef SNull X;
    };

    template<typename T>
    struct SRemovePointer<T*>
    {
        typedef T X;
    };

    template<typename T>
    struct SRemovePointer<T*&>
    {
        typedef T X;
    };
} // namespace CORE