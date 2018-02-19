
#pragma once

#include "base/base_defines.h"
#include "base/base_typedef.h"

#include <typeinfo>

namespace CORE
{
    class CTypeInfo
    {
    public:

        template<class T>
        static CORE::ID GetTypeID();
    };
} // namespace CORE

namespace CORE
{
    template<class T>
    CORE::ID CTypeInfo::GetTypeID()
    {
        return typeid(T).hash_code();
    }
} // namespace CORE
