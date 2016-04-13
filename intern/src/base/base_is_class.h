
#pragma once

#include "base/base_defines.h"
#include "base/base_is_bool.h"

namespace CORE
{
    template<typename T>
    struct SIsClass : public SIsBool<__is_class(T)>
    { };

} // namespace CORE