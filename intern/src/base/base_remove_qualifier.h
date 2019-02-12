
#pragma once

#include "base/base_defines.h"
#include "base/base_remove_const.h"
#include "base/base_remove_volatile.h"

namespace CORE
{
    template<typename T>
    struct SRemoveQualifier
    {
    private:
        typedef typename SRemoveConst<T>::X XNonConst;

    public:
        typedef typename SRemoveVolatile<XNonConst>::X X;
    };
} // namespace CORE